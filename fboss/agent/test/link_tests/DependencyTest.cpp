// Copyright 2004-present Facebook. All Rights Reserved.

#include "fboss/agent/hw/test/HwTestEcmpUtils.h"
#include "fboss/agent/test/link_tests/LinkTest.h"

using namespace facebook::fboss;

TEST_F(LinkTest, ecmpShrink) {
  auto setup = [this]() {
    const auto cabledPorts = getVlanOwningCabledPorts();
    programDefaultRoute(cabledPorts, sw()->getLocalMac(scope(cabledPorts)));
  };
  auto verify = [this]() {
    auto ecmpPorts = getVlanOwningCabledPorts();
    EXPECT_NO_THROW(waitForAllCabledPorts(true));
    EXPECT_EQ(
        utility::getEcmpSizeInHw(
            sw()->getHw_DEPRECATED(),
            {folly::IPAddress("::"), 0},
            RouterID(0),
            ecmpPorts.size()),
        ecmpPorts.size());

    std::vector<PortID> ports;
    for (const auto& port : ecmpPorts) {
      setPortStatus(port.phyPortID(), false);
      ports.push_back(port.phyPortID());
    }
    EXPECT_NO_THROW(waitForLinkStatus(ports, false));
    EXPECT_EQ(
        utility::getEcmpSizeInHw(
            sw()->getHw_DEPRECATED(),
            {folly::IPAddress("::"), 0},
            RouterID(0),
            ecmpPorts.size()),
        0);

    for (const auto& port : ecmpPorts) {
      setPortStatus(port.phyPortID(), true);
    }
    EXPECT_NO_THROW(waitForLinkStatus(ports, true));
  };

  verifyAcrossWarmBoots(setup, verify);
}
