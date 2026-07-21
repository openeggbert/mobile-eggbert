// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/EndPoint.hpp"
#include "System/Net/DnsEndPoint.hpp"
#include "System/Net/SocketAddress.hpp"
#include "System/Net/Sockets/AddressFamily.hpp"
#include "System/ArgumentException.hpp"
#include "System/IndexOutOfRangeException.hpp"

using System::Net::EndPoint;
using System::Net::DnsEndPoint;
using System::Net::SocketAddress;
using System::Net::Sockets::AddressFamily;

// ===========================================================================
// AddressFamily / SocketAddress
// ===========================================================================

TEST(SocketAddressTests, Constructor_SetsFamilyAndSize) {
    SocketAddress addr(AddressFamily::InterNetwork);
    EXPECT_EQ(addr.getFamilyProperty(), AddressFamily::InterNetwork);
    EXPECT_EQ(addr.getSizeProperty(), SocketAddress::IPv4AddressSize);
}

TEST(SocketAddressTests, GetMaximumAddressSize_KnownFamilies) {
    EXPECT_EQ(SocketAddress::GetMaximumAddressSize(AddressFamily::InterNetwork), SocketAddress::IPv4AddressSize);
    EXPECT_EQ(SocketAddress::GetMaximumAddressSize(AddressFamily::InterNetworkV6), SocketAddress::IPv6AddressSize);
    EXPECT_EQ(SocketAddress::GetMaximumAddressSize(AddressFamily::Unix), SocketAddress::UdsAddressSize);
    EXPECT_EQ(SocketAddress::GetMaximumAddressSize(AddressFamily::Unspecified), SocketAddress::MaxAddressSize);
}

TEST(SocketAddressTests, Indexer_ReadWrite) {
    SocketAddress addr(AddressFamily::InterNetwork);
    addr[2] = 127;
    addr[3] = 0;
    EXPECT_EQ(addr[2], 127);
    EXPECT_EQ(addr[3], 0);
}

TEST(SocketAddressTests, Indexer_OutOfRange_Throws) {
    SocketAddress addr(AddressFamily::InterNetwork);
    EXPECT_THROW((void)addr[1000], System::IndexOutOfRangeException);
}

TEST(SocketAddressTests, Equals_SameContents_True) {
    SocketAddress a(AddressFamily::InterNetwork);
    SocketAddress b(AddressFamily::InterNetwork);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(SocketAddressTests, Equals_DifferentContents_False) {
    SocketAddress a(AddressFamily::InterNetwork);
    SocketAddress b(AddressFamily::InterNetwork);
    b[2] = 99;
    EXPECT_FALSE(a.Equals(b));
}

TEST(SocketAddressTests, SetSize_OutOfRange_Throws) {
    SocketAddress addr(AddressFamily::InterNetwork);
    EXPECT_THROW(addr.setSizeProperty(-1), std::exception);
    EXPECT_THROW(addr.setSizeProperty(10000), std::exception);
}

TEST(SocketAddressTests, ToString_ContainsFamilyAndSize) {
    SocketAddress addr(AddressFamily::InterNetwork);
    std::string s = addr.ToString();
    EXPECT_NE(s.find("InterNetwork"), std::string::npos);
    EXPECT_NE(s.find("16"), std::string::npos);
}

// ===========================================================================
// EndPoint (base class)
// ===========================================================================

TEST(EndPointTests, BaseClass_AddressFamily_ThrowsNotImplemented) {
    EndPoint ep;
    EXPECT_THROW((void)ep.getAddressFamilyProperty(), System::NotImplementedException);
}

TEST(EndPointTests, BaseClass_Serialize_ThrowsNotImplemented) {
    EndPoint ep;
    EXPECT_THROW((void)ep.Serialize(), System::NotImplementedException);
}

// ===========================================================================
// DnsEndPoint
// ===========================================================================

TEST(DnsEndPointTests, TwoArgConstructor_DefaultsToUnspecified) {
    DnsEndPoint ep("example.com", 80);
    EXPECT_EQ(ep.getHostProperty(), "example.com");
    EXPECT_EQ(ep.getPortProperty(), 80);
    EXPECT_EQ(ep.getAddressFamilyProperty(), AddressFamily::Unspecified);
}

TEST(DnsEndPointTests, ThreeArgConstructor_SetsFamily) {
    DnsEndPoint ep("example.com", 443, AddressFamily::InterNetwork);
    EXPECT_EQ(ep.getAddressFamilyProperty(), AddressFamily::InterNetwork);
}

TEST(DnsEndPointTests, Constructor_EmptyHost_Throws) {
    EXPECT_THROW(DnsEndPoint("", 80), System::ArgumentException);
}

TEST(DnsEndPointTests, Constructor_PortOutOfRange_Throws) {
    EXPECT_THROW(DnsEndPoint("example.com", -1), std::exception);
    EXPECT_THROW(DnsEndPoint("example.com", 70000), std::exception);
}

TEST(DnsEndPointTests, Constructor_UnsupportedFamily_Throws) {
    EXPECT_THROW(DnsEndPoint("example.com", 80, AddressFamily::Unix), System::ArgumentException);
}

TEST(DnsEndPointTests, ToString_Format) {
    DnsEndPoint ep("example.com", 80, AddressFamily::InterNetwork);
    EXPECT_EQ(ep.ToString(), "InterNetwork/example.com:80");
}

TEST(DnsEndPointTests, Equals_CaseInsensitiveHost) {
    DnsEndPoint a("Example.com", 80, AddressFamily::InterNetwork);
    DnsEndPoint b("example.COM", 80, AddressFamily::InterNetwork);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(DnsEndPointTests, Equals_DifferentPort_False) {
    DnsEndPoint a("example.com", 80);
    DnsEndPoint b("example.com", 81);
    EXPECT_FALSE(a.Equals(b));
}

TEST(DnsEndPointTests, IsA_EndPoint) {
    DnsEndPoint ep("example.com", 80);
    EndPoint* base = &ep;
    EXPECT_EQ(base->getAddressFamilyProperty(), AddressFamily::Unspecified);
}
