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

namespace facebook::fboss {

enum class FlexPortMode {
  FOURX10G,
  FOURX25G,
  ONEX40G,
  TWOX50G,
  ONEX100G,
  ONEX400G,
};
} // namespace facebook::fboss
