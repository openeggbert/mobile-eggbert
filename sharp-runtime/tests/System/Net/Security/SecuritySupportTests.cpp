// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Net/Security/AuthenticationLevel.hpp"
#include "System/Net/Security/EncryptionPolicy.hpp"
#include "System/Net/Security/SslApplicationProtocol.hpp"
#include "System/Net/Security/SslPolicyErrors.hpp"
#include "System/Net/Security/TlsCipherSuite.hpp"

using namespace System::Net::Security;

TEST(AuthenticationLevelTests, Values) {
    EXPECT_EQ(static_cast<int>(AuthenticationLevel::None), 0);
    EXPECT_EQ(static_cast<int>(AuthenticationLevel::MutualAuthRequested), 1);
    EXPECT_EQ(static_cast<int>(AuthenticationLevel::MutualAuthRequired), 2);
}

TEST(EncryptionPolicyTests, Values) {
    EXPECT_EQ(static_cast<int>(EncryptionPolicy::RequireEncryption), 0);
    EXPECT_EQ(static_cast<int>(EncryptionPolicy::AllowNoEncryption), 1);
    EXPECT_EQ(static_cast<int>(EncryptionPolicy::NoEncryption), 2);
}

TEST(SslPolicyErrorsTests, Values) {
    EXPECT_EQ(static_cast<int>(SslPolicyErrors::None), 0x0);
    EXPECT_EQ(static_cast<int>(SslPolicyErrors::RemoteCertificateNotAvailable), 0x1);
    EXPECT_EQ(static_cast<int>(SslPolicyErrors::RemoteCertificateNameMismatch), 0x2);
    EXPECT_EQ(static_cast<int>(SslPolicyErrors::RemoteCertificateChainErrors), 0x4);
}

TEST(SslPolicyErrorsTests, BitwiseOperators) {
    auto combined = SslPolicyErrors::RemoteCertificateNotAvailable | SslPolicyErrors::RemoteCertificateNameMismatch;
    EXPECT_EQ(static_cast<int>(combined), 0x3);
    EXPECT_EQ(combined & SslPolicyErrors::RemoteCertificateNameMismatch, SslPolicyErrors::RemoteCertificateNameMismatch);
    EXPECT_EQ(combined & SslPolicyErrors::RemoteCertificateChainErrors, SslPolicyErrors::None);
}

TEST(TlsCipherSuiteTests, Values) {
    EXPECT_EQ(static_cast<uint16_t>(TlsCipherSuite::TLS_NULL_WITH_NULL_NULL), 0x0000);
    EXPECT_EQ(static_cast<uint16_t>(TlsCipherSuite::TLS_RSA_WITH_NULL_MD5), 0x0001);
    EXPECT_EQ(static_cast<uint16_t>(TlsCipherSuite::TLS_AES_128_GCM_SHA256), 0x1301);
}

TEST(SslApplicationProtocolTests, WellKnownValues) {
    EXPECT_EQ(SslApplicationProtocol::Http3.ToString(), "h3");
    EXPECT_EQ(SslApplicationProtocol::Http2.ToString(), "h2");
    EXPECT_EQ(SslApplicationProtocol::Http11.ToString(), "http/1.1");
    EXPECT_NE(SslApplicationProtocol::Http2, SslApplicationProtocol::Http3);
}

TEST(SslApplicationProtocolTests, FromString) {
    SslApplicationProtocol proto("h2");
    EXPECT_EQ(proto, SslApplicationProtocol::Http2);
    EXPECT_EQ(proto.ToString(), "h2");
}

TEST(SslApplicationProtocolTests, FromBytes) {
    std::vector<SharpRuntime::bytecs> bytes{'h', '2'};
    SslApplicationProtocol proto(bytes);
    EXPECT_EQ(proto, SslApplicationProtocol::Http2);
}

TEST(SslApplicationProtocolTests, EmptyProtocol_Throws) {
    EXPECT_THROW(SslApplicationProtocol(std::vector<SharpRuntime::bytecs>{}), System::ArgumentException);
}

TEST(SslApplicationProtocolTests, TooLongProtocol_Throws) {
    std::vector<SharpRuntime::bytecs> bytes(256, 'a');
    EXPECT_THROW(SslApplicationProtocol{bytes}, System::ArgumentException);
}

TEST(SslApplicationProtocolTests, GetHashCode_MatchesForEqualProtocols) {
    SslApplicationProtocol a("h2");
    SslApplicationProtocol b("h2");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(SslApplicationProtocolTests, ToString_InvalidUtf8_FallsBackToHexDump) {
    std::vector<SharpRuntime::bytecs> bytes{
        static_cast<SharpRuntime::bytecs>(0xFF), static_cast<SharpRuntime::bytecs>(0x00),
        static_cast<SharpRuntime::bytecs>(0xAB)};
    SslApplicationProtocol proto(bytes);
    EXPECT_EQ(proto.ToString(), "0xff 0x00 0xab");
}

TEST(SslApplicationProtocolTests, ToString_ValidUtf8_DecodesAsText) {
    std::vector<SharpRuntime::bytecs> bytes{
        static_cast<SharpRuntime::bytecs>('a'), static_cast<SharpRuntime::bytecs>('b'),
        static_cast<SharpRuntime::bytecs>('c')};
    SslApplicationProtocol proto(bytes);
    EXPECT_EQ(proto.ToString(), "abc");
}
