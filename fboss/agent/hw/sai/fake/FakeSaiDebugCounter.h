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

#include "fboss/agent/hw/sai/fake/FakeManager.h"

#include <vector>

extern "C" {
#include <sai.h>
}
namespace facebook::fboss {

class FakeDebugCounter {
 public:
  FakeDebugCounter();
  using DropReasons = std::vector<int32_t>;

  void setDropReasons(const DropReasons& inDrop) {
    dropReasons_ = inDrop;
  }
  const DropReasons& getDropReasons() const {
    return dropReasons_;
  }
  void setType(sai_debug_counter_type_t type) {
    counterType_ = type;
  }

  sai_debug_counter_type_t getType() const {
    return counterType_;
  }

  void setBindMethod(sai_debug_counter_bind_method_t bindMethod) {
    bindMethod_ = bindMethod;
  }

  sai_debug_counter_bind_method_t getBindMethod() const {
    return bindMethod_;
  }
  sai_uint32_t getIndex() const {
    return index_;
  }

  sai_object_id_t id;

 private:
  sai_uint32_t index_{0};
  sai_debug_counter_type_t counterType_;
  sai_debug_counter_bind_method_t bindMethod_;
  DropReasons dropReasons_;
};

using FakeDebugCounterManager = FakeManager<sai_object_id_t, FakeDebugCounter>;

void populate_debug_counter_api(sai_debug_counter_api_t** debug_counter_api);

} // namespace facebook::fboss
