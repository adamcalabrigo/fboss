/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "fboss/lib/HwWriteBehavior.h"

#include <atomic>

namespace facebook::fboss {
namespace {
std::atomic<HwWriteBehavior> curBehavior{HwWriteBehavior::WRITE};

void setHwWriteBehavior(HwWriteBehavior behavior) {
  curBehavior = behavior;
}
} // namespace
HwWriteBehavior getHwWriteBehavior() {
  return curBehavior;
}

HwWriteBehaviorRAII::HwWriteBehaviorRAII(HwWriteBehavior behavior) {
  prevBehavior_ = getHwWriteBehavior();
  setHwWriteBehavior(behavior);
}

HwWriteBehaviorRAII::~HwWriteBehaviorRAII() {
  setHwWriteBehavior(prevBehavior_);
}
} // namespace facebook::fboss
