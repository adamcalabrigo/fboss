/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include "fboss/agent/platforms/common/PlatformMapping.h"

namespace facebook::fboss {

class MultiPimPlatformMapping : public PlatformMapping {
 public:
  explicit MultiPimPlatformMapping(const std::string& jsonPlatformMappingStr);

  PlatformMapping* getPimPlatformMapping(uint8_t pimID);

  std::unique_ptr<PlatformMapping> getPimPlatformMappingUniquePtr(
      uint8_t pimID);

 protected:
  std::map<uint8_t, std::unique_ptr<PlatformMapping>> pims_;

 private:
  // Forbidden copy constructor and assignment operator
  MultiPimPlatformMapping(MultiPimPlatformMapping const&) = delete;
  MultiPimPlatformMapping& operator=(MultiPimPlatformMapping const&) = delete;
};
} // namespace facebook::fboss
