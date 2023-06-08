// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <map>
#include <stdexcept>
#include <vector>

namespace facebook::fboss::platform::platform_manager {

class PlatformI2cExplorer {
 public:
  // This function takes as input the list of `i2cBussesFromMainBoard` defined
  // in the platform_manager_config.thrift, and outputs a map from
  // `i2cBussesFromMainBoard` to the corresponding i2c bus names assigned on the
  // board by the kernel.
  std::map<std::string, std::string> getBusesfromBsp(
      const std::vector<std::string>& i2cBussesFromMainBoard);

  // Returns the FRU Type name based on the contents read from the EEPROM
  std::string getFruTypeName(const std::string& eepromPath);

  bool createI2cDevice(
      const std::string& deviceName,
      const std::string& busName,
      uint8_t addr);

  bool createI2cMux(
      const std::string& deviceName,
      const std::string& busName,
      uint8_t addr,
      uint8_t numChannels);

  // Returns the I2C Buses whih were created for the channels behind the I2C Mux
  // at `busName`@`addr`. They are listed in the ascending order of channels. It
  // reads the children of /sys/bus/i2c/devices/`busName`-`addr`/ to obtain
  // this. This function needs to be called after `createI2cMux()` for the mux.
  // Otherwise it throws an exception.
  std::vector<std::string> getMuxChannelI2CBuses(
      const std::string& busName,
      uint8_t addr);

  // Return sysfs path to the device at `addr` on `i2cBusName`.
  static std::string getI2cPath(const std::string& i2cBusName, uint8_t addr);
};

} // namespace facebook::fboss::platform::platform_manager
