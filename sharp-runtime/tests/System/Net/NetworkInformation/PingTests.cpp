// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/NetworkInformation/Ping.hpp"

using namespace System::Net::NetworkInformation;
using System::Net::IPAddress;

TEST(PingTests, Send_Loopback_Succeeds) {
    Ping ping;
    PingReply reply = ping.Send(IPAddress::Loopback);
    EXPECT_EQ(reply.getStatusProperty(), IPStatus::Success);
    EXPECT_EQ(reply.getAddressProperty(), IPAddress::Loopback);
    EXPECT_GE(reply.getRoundtripTimeProperty(), 0);
    EXPECT_EQ(reply.getBufferProperty().size(), 32u);
}

TEST(PingTests, Send_LoopbackByString_Succeeds) {
    Ping ping;
    PingReply reply = ping.Send(std::string("127.0.0.1"));
    EXPECT_EQ(reply.getStatusProperty(), IPStatus::Success);
}

TEST(PingTests, Send_CustomBuffer_EchoedBack) {
    Ping ping;
    std::vector<SharpRuntime::bytecs> buffer{1, 2, 3, 4, 5};
    PingReply reply = ping.Send(IPAddress::Loopback, 5000, buffer);
    EXPECT_EQ(reply.getStatusProperty(), IPStatus::Success);
    EXPECT_EQ(reply.getBufferProperty(), buffer);
}

TEST(PingTests, Send_WithOptions_Succeeds) {
    Ping ping;
    PingOptions options(64, true);
    PingReply reply = ping.Send(IPAddress::Loopback, 5000, std::vector<SharpRuntime::bytecs>{9, 9}, options);
    EXPECT_EQ(reply.getStatusProperty(), IPStatus::Success);
    ASSERT_TRUE(reply.getOptionsProperty().has_value());
    EXPECT_EQ(reply.getOptionsProperty()->getTtlProperty(), 64);
}

TEST(PingTests, Send_NegativeTimeout_Throws) {
    Ping ping;
    EXPECT_THROW(ping.Send(IPAddress::Loopback, -1), System::ArgumentOutOfRangeException);
}

TEST(PingTests, Send_BufferTooLarge_Throws) {
    Ping ping;
    std::vector<SharpRuntime::bytecs> buffer(65501);
    EXPECT_THROW(ping.Send(IPAddress::Loopback, 5000, buffer), System::ArgumentException);
}

TEST(PingTests, Send_AnyAddress_Throws) {
    Ping ping;
    EXPECT_THROW(ping.Send(IPAddress::Any), System::ArgumentException);
}

TEST(PingTests, Send_EmptyHostName_Throws) {
    Ping ping;
    EXPECT_THROW(ping.Send(std::string("")), System::ArgumentException);
}

TEST(PingTests, SendPingAsync_Loopback_Succeeds) {
    Ping ping;
    auto task = ping.SendPingAsync(IPAddress::Loopback);
    PingReply reply = task.getResultProperty();
    EXPECT_EQ(reply.getStatusProperty(), IPStatus::Success);
}
