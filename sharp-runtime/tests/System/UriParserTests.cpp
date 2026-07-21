// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriParser.hpp"
#include "System/Uri.hpp"
#include "System/NotImplementedException.hpp"

using System::UriParser;
using System::NotImplementedException;

TEST(UriParserTest, IsKnownSchemeHttp) {
    EXPECT_TRUE(UriParser::IsKnownScheme("http"));
}

TEST(UriParserTest, IsKnownSchemeHttps) {
    EXPECT_TRUE(UriParser::IsKnownScheme("https"));
}

TEST(UriParserTest, IsKnownSchemeFtp) {
    EXPECT_TRUE(UriParser::IsKnownScheme("ftp"));
}

TEST(UriParserTest, IsKnownSchemeFile) {
    EXPECT_TRUE(UriParser::IsKnownScheme("file"));
}

TEST(UriParserTest, IsKnownSchemeUnknown) {
    EXPECT_FALSE(UriParser::IsKnownScheme("custom-scheme"));
}

// ---------------------------------------------------------------------------
// Regression: the known-scheme table previously listed "wais" (not one of
// .NET's actual registered UriParser schemes) and was missing "ws"/"wss"
// (WebSocket), "uuid", and "vsmacros" -- fixed to match UriSyntax.cs exactly.
// ---------------------------------------------------------------------------

TEST(UriParserTest, IsKnownSchemeWs) {
    EXPECT_TRUE(UriParser::IsKnownScheme("ws"));
}

TEST(UriParserTest, IsKnownSchemeWss) {
    EXPECT_TRUE(UriParser::IsKnownScheme("wss"));
}

TEST(UriParserTest, IsKnownSchemeUuid) {
    EXPECT_TRUE(UriParser::IsKnownScheme("uuid"));
}

TEST(UriParserTest, IsKnownSchemeVsmacros) {
    EXPECT_TRUE(UriParser::IsKnownScheme("vsmacros"));
}

TEST(UriParserTest, IsKnownSchemeWaisIsNotActuallyRegistered) {
    // "wais" is a historical RFC 1738 scheme, NOT one of .NET's registered UriParser schemes.
    EXPECT_FALSE(UriParser::IsKnownScheme("wais"));
}

TEST(UriParserTest, IsKnownSchemeMailtoGopherNewsNntpTelnetLdapNetTcpNetPipe) {
    EXPECT_TRUE(UriParser::IsKnownScheme("mailto"));
    EXPECT_TRUE(UriParser::IsKnownScheme("gopher"));
    EXPECT_TRUE(UriParser::IsKnownScheme("news"));
    EXPECT_TRUE(UriParser::IsKnownScheme("nntp"));
    EXPECT_TRUE(UriParser::IsKnownScheme("telnet"));
    EXPECT_TRUE(UriParser::IsKnownScheme("ldap"));
    EXPECT_TRUE(UriParser::IsKnownScheme("net.tcp"));
    EXPECT_TRUE(UriParser::IsKnownScheme("net.pipe"));
}

TEST(UriParserTest, IsKnownSchemeIsCaseInsensitive) {
    // URI schemes are case-insensitive per RFC 3986; matches this class's own doc comment.
    EXPECT_TRUE(UriParser::IsKnownScheme("HTTP"));
    EXPECT_TRUE(UriParser::IsKnownScheme("Https"));
}

namespace {
class TestParser final : public UriParser {
public:
    bool IsBaseOf(const System::Uri& /*b*/, const System::Uri& /*r*/) override { return true; }
};
}

TEST(UriParserTest, SubclassIsBaseOf) {
    TestParser p;
    System::Uri base("http://example.com");
    System::Uri rel("http://example.com/path");
    EXPECT_TRUE(p.IsBaseOf(base, rel));
}

TEST(UriParserTest, GetComponents_DefaultThrowsNotImplementedException) {
    TestParser p;
    System::Uri u("http://example.com");
    EXPECT_THROW(p.GetComponents(u, System::UriComponents::Scheme, System::UriFormat::Unescaped),
                 NotImplementedException);
}
