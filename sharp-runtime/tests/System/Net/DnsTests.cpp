// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Dns.hpp"
#include "System/Net/IPHostEntry.hpp"
#include "System/Net/Sockets/SocketError.hpp"
#include "System/Net/Sockets/SocketException.hpp"

using System::Net::Dns;
using System::Net::IPAddress;
using System::Net::IPHostEntry;
using System::Net::Sockets::AddressFamily;
using System::Net::Sockets::SocketError;
using System::Net::Sockets::SocketException;

// ===========================================================================
// IPHostEntry
// ===========================================================================

TEST(IPHostEntryTests, DefaultConstructor_EmptyFields) {
    IPHostEntry entry;
    EXPECT_EQ(entry.getHostNameProperty(), "");
    EXPECT_TRUE(entry.getAliasesProperty().empty());
    EXPECT_TRUE(entry.getAddressListProperty().empty());
}

TEST(IPHostEntryTests, Setters_UpdateFields) {
    IPHostEntry entry;
    entry.setHostNameProperty("example.com");
    entry.setAliasesProperty({"www.example.com"});
    entry.setAddressListProperty({IPAddress::Loopback});
    EXPECT_EQ(entry.getHostNameProperty(), "example.com");
    EXPECT_EQ(entry.getAliasesProperty().size(), 1u);
    EXPECT_EQ(entry.getAddressListProperty().size(), 1u);
    EXPECT_EQ(entry.getAddressListProperty()[0], IPAddress::Loopback);
}

// ===========================================================================
// SocketError / SocketException
// ===========================================================================

TEST(SocketErrorTests, Success_IsZero) {
    EXPECT_EQ(static_cast<int>(SocketError::Success), 0);
}

TEST(SocketExceptionTests, Constructor_StoresErrorCode) {
    SocketException ex(SocketError::HostNotFound);
    EXPECT_EQ(ex.getSocketErrorCodeProperty(), SocketError::HostNotFound);
}

TEST(SocketExceptionTests, IntConstructor_StoresErrorCode) {
    SocketException ex(static_cast<SharpRuntime::intcs>(SocketError::TimedOut));
    EXPECT_EQ(ex.getSocketErrorCodeProperty(), SocketError::TimedOut);
}

// ===========================================================================
// Dns
// ===========================================================================

TEST(DnsTests, GetHostName_ReturnsNonEmptyString) {
    std::string name = Dns::GetHostName();
    EXPECT_FALSE(name.empty());
}

TEST(DnsTests, GetHostAddresses_LiteralIPv4_ReturnsSameAddress) {
    auto addrs = Dns::GetHostAddresses("127.0.0.1");
    ASSERT_EQ(addrs.size(), 1u);
    EXPECT_EQ(addrs[0], IPAddress::Loopback);
}

// Regression test for a wave-3 audit finding: requesting AddressFamily::InterNetworkV6 for
// any host unconditionally returned an empty result -- IPv6 resolution was effectively
// disabled entirely, silently, with no distinction between "no IPv6 address exists" and
// "IPv6 support is turned off". Asking for an IPv6-only resolution of an IPv4-only literal
// address has no valid answer, so it now surfaces as a SocketException(HostNotFound) --
// consistent with how every other unresolvable-address case in this API already behaves --
// instead of a silent empty vector indistinguishable from "checked and found nothing".
TEST(DnsTests, GetHostAddresses_RequestingIPv6Only_MismatchedIPv4Literal_Throws) {
    EXPECT_THROW(Dns::GetHostAddresses("127.0.0.1", AddressFamily::InterNetworkV6), SocketException);
}

TEST(DnsTests, GetHostAddresses_LiteralIPv6_ReturnsSameAddress) {
    auto addrs = Dns::GetHostAddresses("::1");
    ASSERT_FALSE(addrs.empty());
    for (const auto& a : addrs) {
        EXPECT_TRUE(a.getIsIPv6Property());
        EXPECT_EQ(a, IPAddress::IPv6Loopback);
    }
}

TEST(DnsTests, GetHostAddresses_RequestingIPv6_LiteralIPv6_ReturnsAddress) {
    auto addrs = Dns::GetHostAddresses("::1", AddressFamily::InterNetworkV6);
    ASSERT_FALSE(addrs.empty());
    EXPECT_EQ(addrs[0], IPAddress::IPv6Loopback);
}

// Verified against the real getaddrinfo() behavior of this build host: requesting AF_INET6
// for the numeric literal "127.0.0.1" fails with EAI_ADDRFAMILY, and requesting AF_INET for
// the numeric literal "::1" is likewise rejected -- an IPv4-only request for an IPv6 literal
// has no valid answer either.
TEST(DnsTests, GetHostAddresses_RequestingIPv4Only_MismatchedIPv6Literal_Throws) {
    EXPECT_THROW(Dns::GetHostAddresses("::1", AddressFamily::InterNetwork), SocketException);
}

TEST(DnsTests, GetHostEntry_LiteralIPv4_ReturnsEntryWithSameAddress) {
    IPHostEntry entry = Dns::GetHostEntry("127.0.0.1");
    ASSERT_FALSE(entry.getAddressListProperty().empty());
    EXPECT_EQ(entry.getAddressListProperty()[0], IPAddress::Loopback);
}

TEST(DnsTests, GetHostEntry_LiteralIPv6_ReturnsEntryWithSameAddress) {
    IPHostEntry entry = Dns::GetHostEntry("::1", AddressFamily::InterNetworkV6);
    ASSERT_FALSE(entry.getAddressListProperty().empty());
    EXPECT_EQ(entry.getAddressListProperty()[0], IPAddress::IPv6Loopback);
}

TEST(DnsTests, GetHostEntry_ByAddress_Loopback_ResolvesSomeName) {
    IPHostEntry entry = Dns::GetHostEntry(IPAddress::Loopback);
    EXPECT_FALSE(entry.getHostNameProperty().empty());
}

// Regression test: GetHostEntry(const IPAddress&) previously called address.getAddressProperty()
// unconditionally, which throws SocketException for any IPv6 address (per its documented
// contract) -- there was no code path at all for a reverse lookup of an IPv6 address.
TEST(DnsTests, GetHostEntry_ByAddress_IPv6Loopback_ResolvesSomeName) {
    IPHostEntry entry = Dns::GetHostEntry(IPAddress::IPv6Loopback);
    EXPECT_FALSE(entry.getHostNameProperty().empty());
}

TEST(DnsTests, GetHostAddresses_UnresolvableHost_Throws) {
    EXPECT_THROW(Dns::GetHostAddresses("this-host-should-not-exist.invalid"), SocketException);
}
