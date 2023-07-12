// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#include <chrono>
#include <exception>
#include <stdexcept>

#include <folly/FileUtil.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/protocol/Serializer.h>

#include "fboss/platform/platform_manager/PlatformExplorer.h"
#include "fboss/platform/platform_manager/PlatformValidator.h"

namespace facebook::fboss::platform::platform_manager {

PlatformExplorer::PlatformExplorer(
    std::chrono::seconds exploreInterval,
    const std::string& configFile,
    const ConfigLib& configLib) {
  std::string pmConfigJson;
  if (configFile.empty()) {
    XLOG(INFO) << "No config file was provided. Inferring from config_lib";
    pmConfigJson = configLib.getPlatformManagerConfig();
  } else {
    XLOG(INFO) << "Using config file: " << configFile;
    if (!folly::readFile(configFile.c_str(), pmConfigJson)) {
      XLOG(ERR) << "Can not find sensor config file: " + configFile;
      throw std::runtime_error(
          "Can not find sensor config file: " + configFile);
    }
  }

  try {
    apache::thrift::SimpleJSONSerializer::deserialize<PlatformConfig>(
        pmConfigJson, platformConfig_);
  } catch (const std::exception& e) {
    XLOG(ERR) << "Failed to deserialize platform config: " << e.what();
    throw;
  }
  XLOG(DBG2) << apache::thrift::SimpleJSONSerializer::serialize<std::string>(
      platformConfig_);

  if (!PlatformValidator().isValid(platformConfig_)) {
    XLOG(ERR) << "Invalid platform config";
    throw std::runtime_error("Invalid platform config");
  }

  scheduler_.addFunction([this]() { explore(); }, exploreInterval);
  scheduler_.start();
}

void PlatformExplorer::explore() {
  XLOG(INFO) << "Exploring the device";

  for (const auto& [busName, kernelBusName] : i2cExplorer_.getBusesfromBsp(
           *platformConfig_.i2cBussesFromMainBoard())) {
    updateKernelI2cBusNames("", busName, kernelBusName);
  }

  bool isChassisPresent = presenceDetector_.isPresent(
      *platformConfig_.chassisSlotConfig()->presenceDetection());

  if (!isChassisPresent) {
    XLOG(ERR) << "No chassis present";
    throw std::runtime_error("No chassis present");
  }

  exploreFRU(
      "",
      "Chassis_Slot@0",
      *platformConfig_.chassisSlotConfig(),
      "CHASSIS",
      *platformConfig_.chassisFruTypeConfig());
}

void PlatformExplorer::exploreFRU(
    const std::string& parentFruName,
    const std::string& parentSlotName,
    const SlotConfig& parentSlot,
    const std::string& fruTypeName,
    const FruTypeConfig& fruTypeConfig) {
  auto fruName =
      fmt::format("{}::{}/{}", parentFruName, parentSlotName, fruTypeName);
  int i = 0;
  for (const auto& busName : *parentSlot.outgoingI2cBusNames()) {
    auto kernelBusName = getKernelI2cBusName(parentFruName, busName);
    updateKernelI2cBusNames(
        fruName, fmt::format("INCOMING@{}", i++), kernelBusName);
  }
  exploreI2cDevices(fruName, *fruTypeConfig.i2cDeviceConfigs());
  for (const auto& [slotName, slotConfig] :
       *fruTypeConfig.outgoingSlotConfigs()) {
    bool isChildFruPlugged =
        presenceDetector_.isPresent(*slotConfig.presenceDetection());
    if (!isChildFruPlugged) {
      continue;
    }
    auto fruEepromConfig =
        *platformConfig_.slotTypeConfigs()[*slotConfig.slotType()].fruEeprom();
    auto eepromI2cBusName = getKernelI2cBusName(
        fruName,
        slotConfig.outgoingI2cBusNames()[*fruEepromConfig.incomingBusIndex()]);
    i2cExplorer_.createI2cDevice(
        *fruEepromConfig.kernelDeviceName(),
        eepromI2cBusName,
        *fruEepromConfig.address());
    auto eepromPath =
        i2cExplorer_.getI2cPath(eepromI2cBusName, *fruEepromConfig.address());
    auto pluggedInFruTypeName = i2cExplorer_.getFruTypeName(eepromPath);
    exploreFRU(
        fruName,
        slotName,
        slotConfig,
        pluggedInFruTypeName,
        platformConfig_.fruTypeConfigs()[pluggedInFruTypeName]);
  }
}

void PlatformExplorer::exploreI2cDevices(
    const std::string& fruName,
    const std::vector<I2cDeviceConfig>& i2cDeviceConfigs) {
  for (const auto& i2cDeviceConfig : i2cDeviceConfigs) {
    if (i2cDeviceConfig.numOutgoingChannels()) {
      i2cExplorer_.createI2cMux(
          *i2cDeviceConfig.kernelDeviceName(),
          getKernelI2cBusName(fruName, *i2cDeviceConfig.busName()),
          *i2cDeviceConfig.addr(),
          *i2cDeviceConfig.numOutgoingChannels());
      auto channelBusNames = i2cExplorer_.getMuxChannelI2CBuses(
          getKernelI2cBusName(fruName, *i2cDeviceConfig.busName()),
          *i2cDeviceConfig.addr());
      assert(channelBusNames.size() == i2cDeviceConfig.numOutgoingChannels());
      for (int i = 0; i < i2cDeviceConfig.numOutgoingChannels(); ++i) {
        updateKernelI2cBusNames(
            fruName,
            fmt::format("{}@{}", *i2cDeviceConfig.fruScopedName(), i),
            channelBusNames[i]);
      }
    } else {
      i2cExplorer_.createI2cDevice(
          *i2cDeviceConfig.kernelDeviceName(),
          getKernelI2cBusName(fruName, *i2cDeviceConfig.busName()),
          *i2cDeviceConfig.addr());
    }
  }
}

std::string PlatformExplorer::getKernelI2cBusName(
    const std::string& fruName,
    const std::string& fruScopeBusName) {
  return kernelI2cBusNames_[std::make_pair(fruName, fruScopeBusName)];
}

void PlatformExplorer::updateKernelI2cBusNames(
    const std::string& fruName,
    const std::string& fruScopeBusName,
    const std::string& kernelI2cBusName) {
  kernelI2cBusNames_[std::make_pair(fruName, fruScopeBusName)] =
      kernelI2cBusName;
}

} // namespace facebook::fboss::platform::platform_manager
