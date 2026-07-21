// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/IPAddress.hpp"
#include "System/Net/IPEndPoint.hpp"
#include "System/Net/HttpStatusCode.hpp"
#include "System/Net/WebUtility.hpp"
#include "System/Net/DecompressionMethods.hpp"
#include "System/Net/HttpRequestHeader.hpp"
#include "System/Net/HttpResponseHeader.hpp"
#include "System/Net/HttpVersion.hpp"
#include "System/Net/IPNetwork.hpp"
#include "System/FormatException.hpp"
#include "System/IndexOutOfRangeException.hpp"
#include "System/Net/ProtocolViolationException.hpp"
#include "System/Net/Sockets/SocketException.hpp"
#include "System/Net/WebExceptionStatus.hpp"
#include "System/Net/WebException.hpp"

using System::Net::IPAddress;
using System::Net::IPEndPoint;
using System::Net::HttpStatusCode;
using System::Net::WebUtility;
using System::Net::DecompressionMethods;
using System::Net::HttpRequestHeader;
using System::Net::HttpResponseHeader;
using System::Net::HttpRequestHeaderGetName;
using System::Net::HttpResponseHeaderGetName;
using System::Net::HttpVersion;
using System::Net::IPNetwork;
using System::Net::ProtocolViolationException;
using System::Net::WebExceptionStatus;
using System::Net::WebException;

// ===========================================================================
// IPAddress
// ===========================================================================

TEST(IPAddressTests, DefaultConstructor_AddressIsZero) {
    IPAddress ip;
    EXPECT_EQ(ip.getAddressProperty(), 0u);
}

TEST(IPAddressTests, Uint32Constructor_StoresAddress) {
    IPAddress ip(0x7F000001u);
    EXPECT_EQ(ip.getAddressProperty(), 0x7F000001u);
}

TEST(IPAddressTests, Parse_Loopback_ToString) {
    IPAddress ip = IPAddress::Parse("127.0.0.1");
    EXPECT_EQ(ip.ToString(), "127.0.0.1");
}

TEST(IPAddressTests, Parse_AllZeros) {
    IPAddress ip = IPAddress::Parse("0.0.0.0");
    EXPECT_EQ(ip.ToString(), "0.0.0.0");
}

TEST(IPAddressTests, Parse_Broadcast) {
    IPAddress ip = IPAddress::Parse("255.255.255.255");
    EXPECT_EQ(ip.ToString(), "255.255.255.255");
}

TEST(IPAddressTests, Parse_ArbitraryAddress) {
    IPAddress ip = IPAddress::Parse("192.168.1.100");
    EXPECT_EQ(ip.ToString(), "192.168.1.100");
}

TEST(IPAddressTests, Parse_InvalidAddress_Throws) {
    EXPECT_THROW(IPAddress::Parse("not.an.ip"), System::FormatException);
}

TEST(IPAddressTests, Parse_ShortForm_ExpandsIntoLastSegment) {
    // Verified against IPv4AddressHelper.Common.cs's ParseNonCanonical (the algorithm
    // IPAddress.Parse's IPv4 path actually uses): "fewer than 3 dots" is a legal "short form"
    // where the final segment absorbs the remaining bytes -- "192.168.1" means
    // 192.168.0.1 (192 << 24 | 168 << 16 | 1), not an error. The previous
    // sscanf("%u.%u.%u.%u%c", ...)-based implementation only ever accepted exactly 4 dotted
    // decimal segments, incorrectly rejecting this real .NET-valid input.
    IPAddress ip = IPAddress::Parse("192.168.1");
    EXPECT_EQ(ip.ToString(), "192.168.0.1");
}

TEST(IPAddressTests, Parse_SingleSegment_IsWholeThirtyTwoBitValue) {
    // "3232235777" == 0xC0A80101 == 192.168.1.1, with zero dots.
    IPAddress ip = IPAddress::Parse("3232235777");
    EXPECT_EQ(ip.ToString(), "192.168.1.1");
}

TEST(IPAddressTests, Parse_HexAndOctalSegments_Accepted) {
    // 0xC0 == 192, 0250 (octal) == 168.
    IPAddress ip = IPAddress::Parse("0xC0.0250.0.1");
    EXPECT_EQ(ip.ToString(), "192.168.0.1");
}

TEST(IPAddressTests, Parse_TrailingGarbage_Throws) {
    EXPECT_THROW(IPAddress::Parse("192.168.1.1extra"), System::FormatException);
}

TEST(IPAddressTests, Parse_EmptyTrailingSegment_Throws) {
    EXPECT_THROW(IPAddress::Parse("192.168.1."), System::FormatException);
}

TEST(IPAddressTests, Parse_TooManyDots_Throws) {
    EXPECT_THROW(IPAddress::Parse("1.2.3.4.5"), System::FormatException);
}

TEST(IPAddressTests, Parse_SegmentOverflowsMaxIPv4Value_Throws) {
    // A digit run long enough to overflow uint.MaxValue -- previously this invoked undefined
    // behavior via sscanf's %u conversion (C11 7.21.6.2p10) instead of cleanly failing.
    EXPECT_THROW(IPAddress::Parse("99999999999999999999"), System::FormatException);
}

TEST(IPAddressTests, Parse_LeadingMinusSign_Throws) {
    // sscanf's %u historically accepts a leading '-' (wrapping to a huge unsigned value on
    // most libc implementations); real .NET rejects it outright since '-' isn't a valid digit.
    EXPECT_THROW(IPAddress::Parse("-1.2.3.4"), System::FormatException);
}

TEST(IPAddressTests, ByteArrayConstructor_WrongLength_ThrowsArgumentException) {
    std::vector<SharpRuntime::bytecs> bytes{1, 2, 3};
    EXPECT_THROW(IPAddress ip(bytes), System::ArgumentException);
}

TEST(IPAddressTests, Equality_SameAddress_True) {
    IPAddress a = IPAddress::Parse("10.0.0.1");
    IPAddress b = IPAddress::Parse("10.0.0.1");
    EXPECT_TRUE(a == b);
}

TEST(IPAddressTests, Equality_DifferentAddress_False) {
    IPAddress a = IPAddress::Parse("10.0.0.1");
    IPAddress b = IPAddress::Parse("10.0.0.2");
    EXPECT_FALSE(a == b);
}

TEST(IPAddressTests, Inequality_DifferentAddress_True) {
    IPAddress a = IPAddress::Parse("1.2.3.4");
    IPAddress b = IPAddress::Parse("4.3.2.1");
    EXPECT_TRUE(a != b);
}

TEST(IPAddressTests, StaticLoopback_Is127_0_0_1) {
    EXPECT_EQ(IPAddress::Loopback.ToString(), "127.0.0.1");
}

TEST(IPAddressTests, StaticAny_Is0_0_0_0) {
    EXPECT_EQ(IPAddress::Any.ToString(), "0.0.0.0");
}

TEST(IPAddressTests, StaticBroadcast_Is255_255_255_255) {
    EXPECT_EQ(IPAddress::Broadcast.ToString(), "255.255.255.255");
}

TEST(IPAddressTests, StaticNone_SameAsBroadcast) {
    EXPECT_EQ(IPAddress::None, IPAddress::Broadcast);
}

TEST(IPAddressTests, Parse_RoundTrip_PreservesAddress) {
    std::string addr = "172.16.254.1";
    EXPECT_EQ(IPAddress::Parse(addr).ToString(), addr);
}

TEST(IPAddressTests, AddressFamily_IPv4) {
    EXPECT_EQ(IPAddress::Loopback.getAddressFamilyProperty(), System::Net::Sockets::AddressFamily::InterNetwork);
}

TEST(IPAddressTests, TryParse_Invalid_ReturnsFalse) {
    IPAddress result;
    EXPECT_FALSE(IPAddress::TryParse("not an ip", result));
}

TEST(IPAddressTests, TryParse_Valid_ReturnsTrue) {
    IPAddress result;
    EXPECT_TRUE(IPAddress::TryParse("8.8.8.8", result));
    EXPECT_EQ(result.ToString(), "8.8.8.8");
}

TEST(IPAddressTests, GetAddressBytes_IPv4) {
    IPAddress ip = IPAddress::Parse("192.168.1.100");
    auto bytes = ip.GetAddressBytes();
    ASSERT_EQ(bytes.size(), 4u);
    EXPECT_EQ(bytes[0], 192);
    EXPECT_EQ(bytes[1], 168);
    EXPECT_EQ(bytes[2], 1);
    EXPECT_EQ(bytes[3], 100);
}

TEST(IPAddressTests, IsLoopback_IPv4) {
    EXPECT_TRUE(IPAddress::IsLoopback(IPAddress::Parse("127.0.0.1")));
    EXPECT_TRUE(IPAddress::IsLoopback(IPAddress::Parse("127.5.5.5")));
    EXPECT_FALSE(IPAddress::IsLoopback(IPAddress::Parse("10.0.0.1")));
}

// ===========================================================================
// IPAddress — IPv6
// ===========================================================================

TEST(IPAddressIPv6Tests, Parse_FullForm) {
    IPAddress ip = IPAddress::Parse("2001:0db8:0000:0000:0000:ff00:0042:8329");
    EXPECT_TRUE(ip.getIsIPv6Property());
    EXPECT_EQ(ip.getAddressFamilyProperty(), System::Net::Sockets::AddressFamily::InterNetworkV6);
}

TEST(IPAddressIPv6Tests, Parse_CompressedForm_RoundTrips) {
    IPAddress ip = IPAddress::Parse("2001:db8::ff00:42:8329");
    EXPECT_EQ(ip.ToString(), "2001:db8::ff00:42:8329");
}

TEST(IPAddressIPv6Tests, Parse_Loopback) {
    IPAddress ip = IPAddress::Parse("::1");
    EXPECT_EQ(ip, IPAddress::IPv6Loopback);
    EXPECT_EQ(ip.ToString(), "::1");
}

TEST(IPAddressIPv6Tests, Parse_Any) {
    IPAddress ip = IPAddress::Parse("::");
    EXPECT_EQ(ip, IPAddress::IPv6Any);
    EXPECT_EQ(ip.ToString(), "::");
}

TEST(IPAddressIPv6Tests, Parse_TrailingDoubleColon) {
    IPAddress ip = IPAddress::Parse("2001:db8::");
    auto bytes = ip.GetAddressBytes();
    ASSERT_EQ(bytes.size(), 16u);
    EXPECT_EQ(bytes[0], 0x20);
    EXPECT_EQ(bytes[1], 0x01);
}

TEST(IPAddressIPv6Tests, Parse_WithScopeId) {
    IPAddress ip = IPAddress::Parse("fe80::1%3");
    EXPECT_EQ(ip.getScopeIdProperty(), 3);
}

// Regression test for a wave-3 audit finding: the scope-ID numeric parse used std::stoul,
// which throws std::out_of_range (an unrelated std:: exception type, not a System::
// exception) for a many-all-digit scope string exceeding unsigned long's range. That
// exception propagated straight out of TryParse -- which must never throw -- instead of the
// parse simply failing, matching IPAddressParser.cs's use of uint.TryParse (non-throwing).
TEST(IPAddressIPv6Tests, TryParse_ScopeIdOverflow_ReturnsFalseWithoutThrowing) {
    IPAddress ip;
    bool ok = false;
    EXPECT_NO_THROW(ok = IPAddress::TryParse("fe80::1%999999999999999999999999999999", ip));
    EXPECT_FALSE(ok);
}

TEST(IPAddressIPv6Tests, Parse_ScopeIdOverflow_ThrowsFormatException) {
    EXPECT_THROW(IPAddress::Parse("fe80::1%999999999999999999999999999999"), System::FormatException);
}

TEST(IPAddressIPv6Tests, Parse_ScopeIdJustOverUint32Max_ReturnsFalseNotSilentlyTruncated) {
    // 4294967296 == UINT32_MAX + 1. The old std::stoul-then-narrow path would have wrapped
    // this to scope 0 instead of failing the parse.
    IPAddress ip;
    EXPECT_FALSE(IPAddress::TryParse("fe80::1%4294967296", ip));
}

TEST(IPAddressIPv6Tests, Parse_EmbeddedIPv4) {
    IPAddress ip = IPAddress::Parse("::ffff:192.168.1.1");
    EXPECT_TRUE(ip.getIsIPv4MappedToIPv6Property());
    EXPECT_EQ(ip.ToString(), "::ffff:192.168.1.1");
}

TEST(IPAddressIPv6Tests, Parse_Invalid_Throws) {
    EXPECT_THROW(IPAddress::Parse("garbage:::too:many:colons"), System::FormatException);
}

TEST(IPAddressIPv6Tests, GetAddressProperty_Throws) {
    IPAddress ip = IPAddress::Parse("::1");
    EXPECT_THROW((void)ip.getAddressProperty(), System::Net::Sockets::SocketException);
}

TEST(IPAddressIPv6Tests, GetScopeIdProperty_OnIPv4_Throws) {
    EXPECT_THROW((void)IPAddress::Loopback.getScopeIdProperty(), System::Net::Sockets::SocketException);
}

TEST(IPAddressIPv6Tests, SetScopeIdProperty_OnIPv4_Throws) {
    IPAddress ip = IPAddress::Loopback;
    EXPECT_THROW(ip.setScopeIdProperty(1), System::Net::Sockets::SocketException);
}

TEST(IPAddressIPv6Tests, MapToIPv6_ThenMapToIPv4_RoundTrips) {
    IPAddress v4 = IPAddress::Parse("192.168.1.1");
    IPAddress v6 = v4.MapToIPv6();
    EXPECT_TRUE(v6.getIsIPv4MappedToIPv6Property());
    IPAddress back = v6.MapToIPv4();
    EXPECT_EQ(back, v4);
}

TEST(IPAddressIPv6Tests, IsLoopback_IPv6) {
    EXPECT_TRUE(IPAddress::IsLoopback(IPAddress::IPv6Loopback));
    EXPECT_FALSE(IPAddress::IsLoopback(IPAddress::Parse("2001:db8::1")));
}

// Regression test for a wave-3 audit finding: IsLoopback only checked equality against ::1,
// silently returning false for the IPv4-mapped loopback representation. Verified against
// IPAddress.cs's IsLoopback, which explicitly also checks
// s_loopbackMappedToIPv6 (::ffff:127.0.0.1).
TEST(IPAddressIPv6Tests, IsLoopback_IPv4MappedLoopback_ReturnsTrue) {
    EXPECT_TRUE(IPAddress::IsLoopback(IPAddress::Parse("::ffff:127.0.0.1")));
}

TEST(IPAddressIPv6Tests, IsIPv6LinkLocal) {
    EXPECT_TRUE(IPAddress::Parse("fe80::1").getIsIPv6LinkLocalProperty());
    EXPECT_FALSE(IPAddress::Parse("2001:db8::1").getIsIPv6LinkLocalProperty());
}

TEST(IPAddressIPv6Tests, Equality) {
    EXPECT_EQ(IPAddress::Parse("2001:db8::1"), IPAddress::Parse("2001:0db8:0000:0000:0000:0000:0000:0001"));
}

// Regression test for a wave-3 audit finding: GetHashCode() only combined numbers_[0..3] (the
// first 64 bits), so two IPv6 addresses sharing a /64 prefix but differing only in their host
// suffix -- the common case for SLAAC/EUI-64-derived addresses -- always hashed identically.
// Verified against IPAddress.cs's GetHashCode, which combines all 128 bits (via four uint32
// values spanning all 8 numbers_ groups) plus the scope ID.
TEST(IPAddressIPv6Tests, GetHashCode_DiffersWhenOnlyLower64BitsDiffer) {
    IPAddress a = IPAddress::Parse("2001:db8::1");
    IPAddress b = IPAddress::Parse("2001:db8::2");
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

TEST(IPAddressIPv6Tests, GetHashCode_EqualAddresses_SameHash) {
    IPAddress a = IPAddress::Parse("2001:db8::1");
    IPAddress b = IPAddress::Parse("2001:0db8:0000:0000:0000:0000:0000:0001");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// IPEndPoint
// ===========================================================================

TEST(IPEndPointTests, DefaultConstructor_ZeroAddressZeroPort) {
    IPEndPoint ep;
    EXPECT_EQ(ep.getAddressProperty().getAddressProperty(), 0u);
    EXPECT_EQ(ep.getPortProperty(), 0);
}

TEST(IPEndPointTests, Constructor_IPAddressAndPort) {
    IPEndPoint ep(IPAddress::Loopback, 8080);
    EXPECT_EQ(ep.getAddressProperty(), IPAddress::Loopback);
    EXPECT_EQ(ep.getPortProperty(), 8080);
}

TEST(IPEndPointTests, Constructor_Uint32AndPort) {
    IPEndPoint ep(0x7F000001u, 443);
    EXPECT_EQ(ep.getAddressProperty().getAddressProperty(), 0x7F000001u);
    EXPECT_EQ(ep.getPortProperty(), 443);
}

TEST(IPEndPointTests, Constructor_LongcsAndPort_InRange) {
    IPEndPoint ep(static_cast<SharpRuntime::longcs>(0x7F000001), 443);
    EXPECT_EQ(ep.getAddressProperty().getAddressProperty(), 0x7F000001u);
}

// Regression test for a wave-3 audit finding: the longcs-taking constructor did
// static_cast<uint32_t>(address) with no range check, silently truncating any value outside
// uint32_t range instead of throwing. Verified against IPAddress.cs's IPAddress(long)
// constructor (which IPEndPoint(long, int) delegates to in real .NET), which throws
// ArgumentOutOfRangeException via ThrowIfGreaterThan((ulong)newAddress, 0xFFFFFFFF).
TEST(IPEndPointTests, Constructor_LongcsAndPort_TooLarge_Throws) {
    EXPECT_THROW(IPEndPoint(static_cast<SharpRuntime::longcs>(0x100000000LL), 443),
                 System::ArgumentOutOfRangeException);
}

TEST(IPEndPointTests, Constructor_LongcsAndPort_Negative_Throws) {
    EXPECT_THROW(IPEndPoint(static_cast<SharpRuntime::longcs>(-1), 443),
                 System::ArgumentOutOfRangeException);
}

TEST(IPEndPointTests, ToString_LoopbackPort80) {
    IPEndPoint ep(IPAddress::Loopback, 80);
    EXPECT_EQ(ep.ToString(), "127.0.0.1:80");
}

TEST(IPEndPointTests, ToString_AnyPort0) {
    IPEndPoint ep(IPAddress::Any, 0);
    EXPECT_EQ(ep.ToString(), "0.0.0.0:0");
}

TEST(IPEndPointTests, SetAddress_UpdatesAddress) {
    IPEndPoint ep;
    ep.setAddressProperty(IPAddress::Loopback);
    EXPECT_EQ(ep.getAddressProperty(), IPAddress::Loopback);
}

TEST(IPEndPointTests, SetPort_UpdatesPort) {
    IPEndPoint ep;
    ep.setPortProperty(9090);
    EXPECT_EQ(ep.getPortProperty(), 9090);
}

TEST(IPEndPointTests, Equality_SameAddressAndPort_True) {
    IPEndPoint a(IPAddress::Loopback, 80);
    IPEndPoint b(IPAddress::Loopback, 80);
    EXPECT_TRUE(a == b);
}

TEST(IPEndPointTests, Equality_DifferentPort_False) {
    IPEndPoint a(IPAddress::Loopback, 80);
    IPEndPoint b(IPAddress::Loopback, 443);
    EXPECT_FALSE(a == b);
}

TEST(IPEndPointTests, Inequality_DifferentAddress_True) {
    IPEndPoint a(IPAddress::Any, 80);
    IPEndPoint b(IPAddress::Loopback, 80);
    EXPECT_TRUE(a != b);
}

TEST(IPEndPointTests, MinPort_IsZero) {
    EXPECT_EQ(static_cast<int>(IPEndPoint::MinPort), 0);
}

TEST(IPEndPointTests, MaxPort_Is65535) {
    EXPECT_EQ(static_cast<int>(IPEndPoint::MaxPort), 65535);
}

TEST(IPEndPointTests, Constructor_PortOutOfRange_Throws) {
    EXPECT_THROW(IPEndPoint(IPAddress::Loopback, -1), std::exception);
    EXPECT_THROW(IPEndPoint(IPAddress::Loopback, 70000), std::exception);
}

TEST(IPEndPointTests, SetPort_OutOfRange_Throws) {
    IPEndPoint ep;
    EXPECT_THROW(ep.setPortProperty(-1), std::exception);
    EXPECT_THROW(ep.setPortProperty(70000), std::exception);
}

TEST(IPEndPointTests, IsA_EndPoint) {
    IPEndPoint ep(IPAddress::Loopback, 80);
    System::Net::EndPoint* base = &ep;
    EXPECT_EQ(base->getAddressFamilyProperty(), System::Net::Sockets::AddressFamily::InterNetwork);
}

TEST(IPEndPointTests, ToString_IPv6_HasBrackets) {
    IPEndPoint ep(IPAddress::IPv6Loopback, 8080);
    EXPECT_EQ(ep.ToString(), "[::1]:8080");
}

TEST(IPEndPointTests, GetHashCode_SameEndpoint_Equal) {
    IPEndPoint a(IPAddress::Loopback, 80);
    IPEndPoint b(IPAddress::Loopback, 80);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(IPEndPointTests, Serialize_ThenCreate_RoundTrips_IPv4) {
    IPEndPoint ep(IPAddress::Parse("192.168.1.42"), 12345);
    auto addr = ep.Serialize();
    auto recreated = ep.Create(addr);
    auto* asIPEndPoint = dynamic_cast<IPEndPoint*>(recreated.get());
    ASSERT_NE(asIPEndPoint, nullptr);
    EXPECT_EQ(asIPEndPoint->getAddressProperty(), ep.getAddressProperty());
    EXPECT_EQ(asIPEndPoint->getPortProperty(), ep.getPortProperty());
}

TEST(IPEndPointTests, Serialize_ThenCreate_RoundTrips_IPv6) {
    IPEndPoint ep(IPAddress::Parse("2001:db8::1"), 443);
    auto addr = ep.Serialize();
    auto recreated = ep.Create(addr);
    auto* asIPEndPoint = dynamic_cast<IPEndPoint*>(recreated.get());
    ASSERT_NE(asIPEndPoint, nullptr);
    EXPECT_EQ(asIPEndPoint->getAddressProperty(), ep.getAddressProperty());
    EXPECT_EQ(asIPEndPoint->getPortProperty(), ep.getPortProperty());
}

TEST(IPEndPointTests, Parse_IPv4WithPort) {
    IPEndPoint ep = IPEndPoint::Parse("10.0.0.5:9000");
    EXPECT_EQ(ep.getAddressProperty(), IPAddress::Parse("10.0.0.5"));
    EXPECT_EQ(ep.getPortProperty(), 9000);
}

TEST(IPEndPointTests, Parse_IPv6WithPort) {
    IPEndPoint ep = IPEndPoint::Parse("[2001:db8::1]:443");
    EXPECT_EQ(ep.getAddressProperty(), IPAddress::Parse("2001:db8::1"));
    EXPECT_EQ(ep.getPortProperty(), 443);
}

TEST(IPEndPointTests, Parse_AddressOnly_PortDefaultsToZero) {
    IPEndPoint ep = IPEndPoint::Parse("10.0.0.5");
    EXPECT_EQ(ep.getPortProperty(), 0);
}

TEST(IPEndPointTests, TryParse_Invalid_ReturnsFalse) {
    IPEndPoint result;
    EXPECT_FALSE(IPEndPoint::TryParse("not an endpoint", result));
}

TEST(IPEndPointTests, Parse_Invalid_Throws) {
    EXPECT_THROW(IPEndPoint::Parse("garbage"), System::FormatException);
}

// ===========================================================================
// HttpStatusCode
// ===========================================================================

TEST(HttpStatusCodeTests, OK_Is200) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::OK), 200);
}

TEST(HttpStatusCodeTests, Created_Is201) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::Created), 201);
}

TEST(HttpStatusCodeTests, NoContent_Is204) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::NoContent), 204);
}

TEST(HttpStatusCodeTests, MovedPermanently_Is301) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::MovedPermanently), 301);
}

TEST(HttpStatusCodeTests, NotModified_Is304) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::NotModified), 304);
}

TEST(HttpStatusCodeTests, Unused_Is306) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::Unused), 306);
}

TEST(HttpStatusCodeTests, BadRequest_Is400) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::BadRequest), 400);
}

TEST(HttpStatusCodeTests, Unauthorized_Is401) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::Unauthorized), 401);
}

TEST(HttpStatusCodeTests, Forbidden_Is403) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::Forbidden), 403);
}

TEST(HttpStatusCodeTests, NotFound_Is404) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::NotFound), 404);
}

TEST(HttpStatusCodeTests, InternalServerError_Is500) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::InternalServerError), 500);
}

TEST(HttpStatusCodeTests, NotImplemented_Is501) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::NotImplemented), 501);
}

TEST(HttpStatusCodeTests, ServiceUnavailable_Is503) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::ServiceUnavailable), 503);
}

TEST(HttpStatusCodeTests, Continue_Is100) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::Continue), 100);
}

TEST(HttpStatusCodeTests, TooManyRequests_Is429) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::TooManyRequests), 429);
}

TEST(HttpStatusCodeTests, Ambiguous_SameAsMultipleChoices) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::Ambiguous),
              static_cast<int>(HttpStatusCode::MultipleChoices));
}

TEST(HttpStatusCodeTests, Moved_SameAsMovedPermanently) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::Moved),
              static_cast<int>(HttpStatusCode::MovedPermanently));
}

TEST(HttpStatusCodeTests, UnprocessableEntity_SameAsUnprocessableContent) {
    EXPECT_EQ(static_cast<int>(HttpStatusCode::UnprocessableEntity),
              static_cast<int>(HttpStatusCode::UnprocessableContent));
}

// ===========================================================================
// WebUtility
// ===========================================================================

TEST(WebUtilityTests, HtmlEncode_Ampersand) {
    EXPECT_EQ(WebUtility::HtmlEncode("a&b"), "a&amp;b");
}

TEST(WebUtilityTests, HtmlEncode_LessThan) {
    EXPECT_EQ(WebUtility::HtmlEncode("<p>"), "&lt;p&gt;");
}

TEST(WebUtilityTests, HtmlEncode_GreaterThan) {
    EXPECT_EQ(WebUtility::HtmlEncode(">"), "&gt;");
}

TEST(WebUtilityTests, HtmlEncode_DoubleQuote) {
    EXPECT_EQ(WebUtility::HtmlEncode("\"hello\""), "&quot;hello&quot;");
}

TEST(WebUtilityTests, HtmlEncode_SingleQuote) {
    EXPECT_EQ(WebUtility::HtmlEncode("it's"), "it&#39;s");
}

TEST(WebUtilityTests, HtmlEncode_NoSpecialChars_Unchanged) {
    EXPECT_EQ(WebUtility::HtmlEncode("hello world"), "hello world");
}

TEST(WebUtilityTests, HtmlEncode_Empty_ReturnsEmpty) {
    EXPECT_EQ(WebUtility::HtmlEncode(""), "");
}

TEST(WebUtilityTests, HtmlDecode_Ampersand) {
    EXPECT_EQ(WebUtility::HtmlDecode("a&amp;b"), "a&b");
}

TEST(WebUtilityTests, HtmlDecode_LtGt) {
    EXPECT_EQ(WebUtility::HtmlDecode("&lt;p&gt;"), "<p>");
}

TEST(WebUtilityTests, HtmlDecode_Quot) {
    EXPECT_EQ(WebUtility::HtmlDecode("&quot;hi&quot;"), "\"hi\"");
}

TEST(WebUtilityTests, HtmlDecode_Hash39) {
    EXPECT_EQ(WebUtility::HtmlDecode("it&#39;s"), "it's");
}

TEST(WebUtilityTests, HtmlDecode_NoEntities_Unchanged) {
    EXPECT_EQ(WebUtility::HtmlDecode("plain text"), "plain text");
}

TEST(WebUtilityTests, HtmlDecode_DecimalNumericEntity) {
    EXPECT_EQ(WebUtility::HtmlDecode("&#65;&#66;&#67;"), "ABC");
}

TEST(WebUtilityTests, HtmlDecode_HexNumericEntity) {
    EXPECT_EQ(WebUtility::HtmlDecode("&#x41;&#x42;"), "AB");
}

TEST(WebUtilityTests, HtmlDecode_NamedEntity_Nbsp) {
    EXPECT_EQ(WebUtility::HtmlDecode("a&nbsp;b"), "a\xC2\xA0" "b");
}

TEST(WebUtilityTests, HtmlDecode_NamedEntity_Apos) {
    EXPECT_EQ(WebUtility::HtmlDecode("it&apos;s"), "it's");
}

TEST(WebUtilityTests, HtmlDecode_UnknownEntity_LeftUnchanged) {
    EXPECT_EQ(WebUtility::HtmlDecode("&unknownentity;"), "&unknownentity;");
}

TEST(WebUtilityTests, HtmlDecode_InvalidNumericEntity_LeftUnchanged) {
    EXPECT_EQ(WebUtility::HtmlDecode("&#notanumber;"), "&#notanumber;");
}

TEST(WebUtilityTests, HtmlEncode_HtmlDecode_RoundTrip) {
    std::string original = "<a href=\"url\">link & more</a>";
    EXPECT_EQ(WebUtility::HtmlDecode(WebUtility::HtmlEncode(original)), original);
}

TEST(WebUtilityTests, UrlEncode_SpaceBecomesPlus) {
    EXPECT_EQ(WebUtility::UrlEncode("hello world"), "hello+world");
}

TEST(WebUtilityTests, UrlEncode_AlphanumericUnchanged) {
    EXPECT_EQ(WebUtility::UrlEncode("abc123"), "abc123");
}

// Regression test for a wave-3 audit finding: the safe-character set didn't match real
// .NET's s_safeUrlChars ("Url safe chars as defined by RFC 1738.4, minus '+'" --
// letters/digits/-_.!*(), notably including !*() and excluding ~). This test previously
// asserted "~" stays literal, encoding the old, wrong safe-char-set.
TEST(WebUtilityTests, UrlEncode_SafeCharsUnchanged) {
    EXPECT_EQ(WebUtility::UrlEncode("-_.!*()"), "-_.!*()");
}

TEST(WebUtilityTests, UrlEncode_Tilde_IsPercentEncoded) {
    EXPECT_EQ(WebUtility::UrlEncode("~"), "%7E");
}

TEST(WebUtilityTests, UrlEncode_SpecialCharsPercent) {
    std::string encoded = WebUtility::UrlEncode("a=b");
    EXPECT_EQ(encoded, "a%3Db");
}

TEST(WebUtilityTests, UrlEncode_Empty_ReturnsEmpty) {
    EXPECT_EQ(WebUtility::UrlEncode(""), "");
}

TEST(WebUtilityTests, UrlDecode_PlusBecomesSpace) {
    EXPECT_EQ(WebUtility::UrlDecode("hello+world"), "hello world");
}

TEST(WebUtilityTests, UrlDecode_PercentEncoded) {
    EXPECT_EQ(WebUtility::UrlDecode("a%3Db"), "a=b");
}

TEST(WebUtilityTests, UrlDecode_NoEncoding_Unchanged) {
    EXPECT_EQ(WebUtility::UrlDecode("plain"), "plain");
}

TEST(WebUtilityTests, UrlEncode_UrlDecode_RoundTrip) {
    std::string original = "key=hello world&other=1+2";
    EXPECT_EQ(WebUtility::UrlDecode(WebUtility::UrlEncode(original)), original);
}

TEST(WebUtilityTests, UrlDecode_MalformedPercentSequence_LeftUnchanged_DoesNotThrow) {
    // Regression: previously std::stoi(..., 16) threw an uncaught std::invalid_argument when
    // neither character after '%' is a valid hex digit. Verified against WebUtility.cs's
    // UrlDecodeInternal: real .NET never throws here -- a malformed sequence just leaves '%'
    // as a literal character and continues decoding the rest of the string normally.
    EXPECT_EQ(WebUtility::UrlDecode("100%complete"), "100%complete");
}

TEST(WebUtilityTests, UrlDecode_PartiallyValidHexDigit_LeftUnchanged) {
    // Only one of the two characters after '%' is a valid hex digit -- still malformed.
    EXPECT_EQ(WebUtility::UrlDecode("100%cZ"), "100%cZ");
}

TEST(WebUtilityTests, UrlDecode_TrailingIncompletePercentSequence_LeftUnchanged) {
    EXPECT_EQ(WebUtility::UrlDecode("abc%4"), "abc%4");
    EXPECT_EQ(WebUtility::UrlDecode("abc%"), "abc%");
}

// ===========================================================================
// DecompressionMethods
// ===========================================================================

TEST(DecompressionMethodsTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(DecompressionMethods::None), 0);
}

TEST(DecompressionMethodsTests, FlagsCombine) {
    auto v = DecompressionMethods::GZip | DecompressionMethods::Deflate;
    EXPECT_EQ(static_cast<int>(v & DecompressionMethods::GZip), static_cast<int>(DecompressionMethods::GZip));
    EXPECT_EQ(static_cast<int>(v & DecompressionMethods::Deflate), static_cast<int>(DecompressionMethods::Deflate));
    EXPECT_EQ(static_cast<int>(v & DecompressionMethods::Brotli), 0);
}

TEST(DecompressionMethodsTests, All_ContainsEveryKnownMethod) {
    auto all = DecompressionMethods::All;
    EXPECT_NE(static_cast<int>(all & DecompressionMethods::GZip), 0);
    EXPECT_NE(static_cast<int>(all & DecompressionMethods::Deflate), 0);
    EXPECT_NE(static_cast<int>(all & DecompressionMethods::Brotli), 0);
    EXPECT_NE(static_cast<int>(all & DecompressionMethods::Zstandard), 0);
}

// ===========================================================================
// HttpRequestHeader / HttpResponseHeader
// ===========================================================================

TEST(HttpRequestHeaderTests, GetName_KnownValues) {
    EXPECT_EQ(HttpRequestHeaderGetName(HttpRequestHeader::CacheControl), "Cache-Control");
    EXPECT_EQ(HttpRequestHeaderGetName(HttpRequestHeader::ContentMd5), "Content-MD5");
    EXPECT_EQ(HttpRequestHeaderGetName(HttpRequestHeader::UserAgent), "User-Agent");
}

TEST(HttpResponseHeaderTests, GetName_KnownValues) {
    EXPECT_EQ(HttpResponseHeaderGetName(HttpResponseHeader::SetCookie), "Set-Cookie");
    EXPECT_EQ(HttpResponseHeaderGetName(HttpResponseHeader::WwwAuthenticate), "WWW-Authenticate");
    EXPECT_EQ(HttpResponseHeaderGetName(HttpResponseHeader::ETag), "ETag");
}

TEST(HttpRequestHeaderTests, GetName_OutOfRange_ThrowsIndexOutOfRangeException) {
    EXPECT_THROW(HttpRequestHeaderGetName(static_cast<HttpRequestHeader>(999)), System::IndexOutOfRangeException);
}

TEST(HttpResponseHeaderTests, GetName_OutOfRange_ThrowsIndexOutOfRangeException) {
    EXPECT_THROW(HttpResponseHeaderGetName(static_cast<HttpResponseHeader>(999)), System::IndexOutOfRangeException);
}

// ===========================================================================
// HttpVersion
// ===========================================================================

TEST(HttpVersionTests, Version11_Is1Dot1) {
    EXPECT_EQ(HttpVersion::Version11.Major, 1);
    EXPECT_EQ(HttpVersion::Version11.Minor, 1);
}

TEST(HttpVersionTests, Unknown_IsZeroZero) {
    EXPECT_EQ(HttpVersion::Unknown.Major, 0);
    EXPECT_EQ(HttpVersion::Unknown.Minor, 0);
}

TEST(HttpVersionTests, Version20_Is2Dot0) {
    EXPECT_EQ(HttpVersion::Version20.Major, 2);
    EXPECT_EQ(HttpVersion::Version20.Minor, 0);
}

// ===========================================================================
// IPNetwork
// ===========================================================================

TEST(IPNetworkTests, Constructor_NormalizesBaseAddress) {
    IPNetwork net(IPAddress::Parse("192.168.1.123"), 24);
    EXPECT_EQ(net.getBaseAddressProperty(), IPAddress::Parse("192.168.1.0"));
    EXPECT_EQ(net.getPrefixLengthProperty(), 24);
}

TEST(IPNetworkTests, Constructor_PrefixOutOfRange_Throws) {
    EXPECT_THROW(IPNetwork(IPAddress::Loopback, 33), System::ArgumentOutOfRangeException);
    EXPECT_THROW(IPNetwork(IPAddress::Loopback, -1), System::ArgumentOutOfRangeException);
}

TEST(IPNetworkTests, Contains_AddressInSubnet_True) {
    IPNetwork net(IPAddress::Parse("192.168.1.0"), 24);
    EXPECT_TRUE(net.Contains(IPAddress::Parse("192.168.1.200")));
    EXPECT_FALSE(net.Contains(IPAddress::Parse("192.168.2.1")));
}

TEST(IPNetworkTests, Contains_PrefixZero_ContainsEverything) {
    IPNetwork net(IPAddress::Any, 0);
    EXPECT_TRUE(net.Contains(IPAddress::Parse("8.8.8.8")));
    EXPECT_TRUE(net.Contains(IPAddress::Loopback));
}

TEST(IPNetworkTests, Contains_PrefixFull_OnlyExactAddress) {
    IPNetwork net(IPAddress::Parse("10.0.0.1"), 32);
    EXPECT_TRUE(net.Contains(IPAddress::Parse("10.0.0.1")));
    EXPECT_FALSE(net.Contains(IPAddress::Parse("10.0.0.2")));
}

TEST(IPNetworkTests, Contains_DifferentFamily_False) {
    IPNetwork net(IPAddress::Parse("192.168.1.0"), 24);
    EXPECT_FALSE(net.Contains(IPAddress::Parse("::1")));
}

TEST(IPNetworkTests, ToString_CidrNotation) {
    IPNetwork net(IPAddress::Parse("192.168.1.0"), 24);
    EXPECT_EQ(net.ToString(), "192.168.1.0/24");
}

TEST(IPNetworkTests, Parse_ValidCidr) {
    IPNetwork net = IPNetwork::Parse("10.0.0.0/8");
    EXPECT_EQ(net.getBaseAddressProperty(), IPAddress::Parse("10.0.0.0"));
    EXPECT_EQ(net.getPrefixLengthProperty(), 8);
}

TEST(IPNetworkTests, Parse_IPv6Cidr) {
    IPNetwork net = IPNetwork::Parse("2001:db8::/32");
    EXPECT_EQ(net.getPrefixLengthProperty(), 32);
    EXPECT_TRUE(net.Contains(IPAddress::Parse("2001:db8::1")));
    EXPECT_FALSE(net.Contains(IPAddress::Parse("2001:db9::1")));
}

TEST(IPNetworkTests, Parse_Invalid_Throws) {
    EXPECT_THROW(IPNetwork::Parse("not-a-network"), System::FormatException);
}

TEST(IPNetworkTests, TryParse_Invalid_ReturnsFalse) {
    IPNetwork result;
    EXPECT_FALSE(IPNetwork::TryParse("garbage/abc", result));
}

TEST(IPNetworkTests, Equality) {
    IPNetwork a(IPAddress::Parse("192.168.1.0"), 24);
    IPNetwork b(IPAddress::Parse("192.168.1.123"), 24);
    EXPECT_EQ(a, b);
}

// ===========================================================================
// ProtocolViolationException / WebException / WebExceptionStatus
// ===========================================================================

TEST(ProtocolViolationExceptionTests, Message_IsPreserved) {
    ProtocolViolationException ex("bad protocol");
    EXPECT_STREQ(ex.what(), "bad protocol");
}

TEST(ProtocolViolationExceptionTests, IsA_InvalidOperationException) {
    ProtocolViolationException ex("x");
    System::InvalidOperationException* base = &ex;
    EXPECT_STREQ(base->what(), "x");
}

TEST(WebExceptionStatusTests, Success_IsZero) {
    EXPECT_EQ(static_cast<int>(WebExceptionStatus::Success), 0);
}

TEST(WebExceptionTests, DefaultConstructor_StatusIsUnknownError) {
    WebException ex;
    EXPECT_EQ(ex.getStatusProperty(), WebExceptionStatus::UnknownError);
}

TEST(WebExceptionTests, MessageAndStatus_ArePreserved) {
    WebException ex("timed out", WebExceptionStatus::Timeout);
    EXPECT_STREQ(ex.what(), "timed out");
    EXPECT_EQ(ex.getStatusProperty(), WebExceptionStatus::Timeout);
}
