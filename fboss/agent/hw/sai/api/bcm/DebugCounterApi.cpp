// Copyright 2004-present Facebook. All Rights Reserved.

#include "fboss/agent/hw/sai/api/DebugCounterApi.h"
#include "fboss/agent/hw/sai/api/SaiVersion.h"

extern "C" {
#include <experimental/saidebugcounterextensions.h>
#include <sai.h>
}

namespace facebook::fboss {

std::optional<sai_int32_t> SaiDebugCounterTraits::trapDrops() {
#if defined BRCM_SAI_SDK_GTE_11_0
  return SAI_IN_DROP_REASON_ALL_TRAP_DROPS;
#endif
  return std::nullopt;
}
} // namespace facebook::fboss