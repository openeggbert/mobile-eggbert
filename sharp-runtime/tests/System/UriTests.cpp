// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <memory>
#include "System/Uri.hpp"
#include "System/UriFormatException.hpp"

using System::Uri;
using System::UriKind;

// ---------------------------------------------------------------------------
// Basic absolute HTTP URI
// ---------------------------------------------------------------------------

TEST(UriTests, Http_IsAbsolute) {
    Uri u("http://example.com/path");
    EXPECT_TRUE(u.getIsAbsoluteUriProperty());
}

TEST(UriTests, Http_Scheme) {
    Uri u("http://example.com/path");
    EXPECT_EQ(u.getSchemeProperty(), "http");
}

TEST(UriTests, Http_Host) {
    Uri u("http://example.com/path");
    EXPECT_EQ(u.getHostProperty(), "example.com");
}

TEST(UriTests, Http_DefaultPort_Is80) {
    Uri u("http://example.com/path");
    EXPECT_EQ(u.getPortProperty(), 80);
}

TEST(UriTests, Http_AbsolutePath) {
    Uri u("http://example.com/path/to/resource");
    EXPECT_EQ(u.getAbsolutePathProperty(), "/path/to/resource");
}

TEST(UriTests, Http_AbsoluteUri_EqualsInput) {
    std::string raw = "http://example.com/path";
    Uri u(raw);
    EXPECT_EQ(u.getAbsoluteUriProperty(), raw);
}

TEST(UriTests, Http_ToString_EqualsAbsoluteUri) {
    Uri u("http://example.com/");
    EXPECT_EQ(u.ToString(), u.getAbsoluteUriProperty());
}

// ---------------------------------------------------------------------------
// HTTPS
// ---------------------------------------------------------------------------

TEST(UriTests, Https_Scheme) {
    Uri u("https://secure.example.com/login");
    EXPECT_EQ(u.getSchemeProperty(), "https");
}

TEST(UriTests, Https_DefaultPort_Is443) {
    Uri u("https://secure.example.com/");
    EXPECT_EQ(u.getPortProperty(), 443);
}

// ---------------------------------------------------------------------------
// Explicit port
// ---------------------------------------------------------------------------

TEST(UriTests, ExplicitPort_Parsed) {
    Uri u("http://example.com:8080/api");
    EXPECT_EQ(u.getPortProperty(), 8080);
}

TEST(UriTests, ExplicitPort_Host_ExcludesPort) {
    Uri u("http://example.com:8080/api");
    EXPECT_EQ(u.getHostProperty(), "example.com");
}

// ---------------------------------------------------------------------------
// Query and fragment
// ---------------------------------------------------------------------------

TEST(UriTests, Query_IncludesQuestionMark) {
    Uri u("http://example.com/search?q=hello&lang=en");
    EXPECT_EQ(u.getQueryProperty(), "?q=hello&lang=en");
}

TEST(UriTests, Query_EmptyWhenAbsent) {
    Uri u("http://example.com/page");
    EXPECT_TRUE(u.getQueryProperty().empty());
}

TEST(UriTests, Fragment_IncludesHash) {
    Uri u("http://example.com/page#section1");
    EXPECT_EQ(u.getFragmentProperty(), "#section1");
}

TEST(UriTests, Fragment_EmptyWhenAbsent) {
    Uri u("http://example.com/page");
    EXPECT_TRUE(u.getFragmentProperty().empty());
}

TEST(UriTests, PathAndQuery_CombinesPathAndQuery) {
    Uri u("http://example.com/search?q=test");
    EXPECT_EQ(u.getPathAndQueryProperty(), "/search?q=test");
}

TEST(UriTests, PathAndQuery_NoQueryIsJustPath) {
    Uri u("http://example.com/about");
    EXPECT_EQ(u.getPathAndQueryProperty(), "/about");
}

// ---------------------------------------------------------------------------
// User info
// ---------------------------------------------------------------------------

TEST(UriTests, UserInfo_Parsed) {
    Uri u("ftp://user:pass@ftp.example.com/file.txt");
    EXPECT_EQ(u.getUserInfoProperty(), "user:pass");
}

TEST(UriTests, UserInfo_EmptyWhenAbsent) {
    Uri u("http://example.com/");
    EXPECT_TRUE(u.getUserInfoProperty().empty());
}

// ---------------------------------------------------------------------------
// Authority
// ---------------------------------------------------------------------------

TEST(UriTests, Authority_DefaultPort_ExcludesPort) {
    Uri u("http://example.com/");
    EXPECT_EQ(u.getAuthorityProperty(), "example.com");
}

TEST(UriTests, Authority_NonDefaultPort_IncludesPort) {
    Uri u("http://example.com:9000/");
    EXPECT_EQ(u.getAuthorityProperty(), "example.com:9000");
}

// ---------------------------------------------------------------------------
// Loopback detection
// ---------------------------------------------------------------------------

TEST(UriTests, Loopback_Localhost_IsLoopback) {
    Uri u("http://localhost/");
    EXPECT_TRUE(u.getIsLoopbackProperty());
}

TEST(UriTests, Loopback_127_IsLoopback) {
    Uri u("http://127.0.0.1/");
    EXPECT_TRUE(u.getIsLoopbackProperty());
}

TEST(UriTests, Loopback_ExternalHost_IsNotLoopback) {
    Uri u("http://example.com/");
    EXPECT_FALSE(u.getIsLoopbackProperty());
}

// Regression for ticket 340: the previous `host_ == "::1"` comparison could never match, since
// a parsed IPv6 literal's host_ retains its surrounding brackets ("[::1]", matching .NET's own
// bracketed Host property for IPv6). Also verifies "localhost" is matched case-insensitively,
// per Uri.cs's DomainNameHelper (StringComparison.OrdinalIgnoreCase).
TEST(UriTests, Loopback_IPv6_IsLoopback) {
    Uri u("http://[::1]:8080/");
    EXPECT_TRUE(u.getIsLoopbackProperty());
}

TEST(UriTests, Loopback_LocalhostUppercase_IsLoopback) {
    Uri u("http://LOCALHOST/");
    EXPECT_TRUE(u.getIsLoopbackProperty());
}

// ---------------------------------------------------------------------------
// Relative URI
// ---------------------------------------------------------------------------

TEST(UriTests, Relative_IsNotAbsolute) {
    Uri u("/relative/path");
    EXPECT_FALSE(u.getIsAbsoluteUriProperty());
}

TEST(UriTests, Relative_UriKind_Accepted) {
    EXPECT_NO_THROW(Uri("/path/to/resource", UriKind::Relative));
}

TEST(UriTests, Absolute_UriKind_RejectsRelative) {
    EXPECT_THROW(Uri("/path/only", UriKind::Absolute), System::UriFormatException);
}

// ---------------------------------------------------------------------------
// TryCreate
// ---------------------------------------------------------------------------

TEST(UriTests, TryCreate_ValidAbsolute_ReturnsTrue) {
    std::shared_ptr<Uri> result;
    bool ok = Uri::TryCreate("http://example.com/", UriKind::Absolute, result);
    EXPECT_TRUE(ok);
    EXPECT_NE(result, nullptr);
}

TEST(UriTests, TryCreate_RelativeAsAbsolute_ReturnsFalse) {
    std::shared_ptr<Uri> result;
    bool ok = Uri::TryCreate("/relative", UriKind::Absolute, result);
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, nullptr);
}

TEST(UriTests, TryCreate_ValidRelative_ReturnsTrue) {
    std::shared_ptr<Uri> result;
    bool ok = Uri::TryCreate("/path", UriKind::Relative, result);
    EXPECT_TRUE(ok);
    EXPECT_NE(result, nullptr);
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------

TEST(UriTests, Equality_SameString_Equal) {
    Uri a("http://example.com/path");
    Uri b("http://example.com/path");
    EXPECT_TRUE(a == b);
}

TEST(UriTests, Equality_DifferentString_NotEqual) {
    Uri a("http://example.com/foo");
    Uri b("http://example.com/bar");
    EXPECT_TRUE(a != b);
}

// ---------------------------------------------------------------------------
// FTP scheme
// ---------------------------------------------------------------------------

TEST(UriTests, Ftp_DefaultPort_Is21) {
    Uri u("ftp://ftp.example.com/pub/file.txt");
    EXPECT_EQ(u.getPortProperty(), 21);
}

TEST(UriTests, Ftp_Scheme) {
    Uri u("ftp://ftp.example.com/pub/file.txt");
    EXPECT_EQ(u.getSchemeProperty(), "ftp");
}

// ---------------------------------------------------------------------------
// Opaque (non-hierarchical) absolute URIs — no "//" authority
// ---------------------------------------------------------------------------

TEST(UriTests, Mailto_IsAbsolute) {
    Uri u("mailto:user@example.com");
    EXPECT_TRUE(u.getIsAbsoluteUriProperty());
}

TEST(UriTests, Mailto_Scheme) {
    Uri u("mailto:user@example.com");
    EXPECT_EQ(u.getSchemeProperty(), "mailto");
}

TEST(UriTests, Mailto_OpaquePartInPath) {
    Uri u("mailto:user@example.com");
    EXPECT_EQ(u.getAbsolutePathProperty(), "user@example.com");
}

TEST(UriTests, Mailto_HostIsEmpty) {
    Uri u("mailto:user@example.com");
    EXPECT_TRUE(u.getHostProperty().empty());
}

TEST(UriTests, Urn_IsAbsolute) {
    Uri u("urn:isbn:0-395-36341-1");
    EXPECT_TRUE(u.getIsAbsoluteUriProperty());
    EXPECT_EQ(u.getSchemeProperty(), "urn");
    EXPECT_EQ(u.getAbsolutePathProperty(), "isbn:0-395-36341-1");
}

TEST(UriTests, Opaque_WithQueryAndFragment) {
    Uri u("tel:+1-555-0100?ext=42#note");
    EXPECT_EQ(u.getSchemeProperty(), "tel");
    EXPECT_EQ(u.getAbsolutePathProperty(), "+1-555-0100");
    EXPECT_EQ(u.getQueryProperty(), "?ext=42");
    EXPECT_EQ(u.getFragmentProperty(), "#note");
}

// ---------------------------------------------------------------------------
// Combine (base, relative) — RFC 3986 §5.3 merge + dot-segment removal
// ---------------------------------------------------------------------------

TEST(UriTests, Combine_TrailingSlashBase_AppendsRelative) {
    Uri base("http://example.com/a/b/");
    Uri combined(base, "c");
    EXPECT_EQ(combined.getAbsoluteUriProperty(), "http://example.com/a/b/c");
}

TEST(UriTests, Combine_NoTrailingSlashBase_ReplacesLastSegment) {
    Uri base("http://example.com/a/b");
    Uri combined(base, "c");
    EXPECT_EQ(combined.getAbsoluteUriProperty(), "http://example.com/a/c");
}

TEST(UriTests, Combine_DotDotSegment_ResolvesUpOneLevel) {
    Uri base("http://example.com/a/b/");
    Uri combined(base, "../c");
    EXPECT_EQ(combined.getAbsoluteUriProperty(), "http://example.com/a/c");
}

TEST(UriTests, Combine_AbsolutePathRelative_ReplacesWholePath) {
    Uri base("http://example.com/a/b/");
    Uri combined(base, "/absolute-path");
    EXPECT_EQ(combined.getAbsoluteUriProperty(), "http://example.com/absolute-path");
}

TEST(UriTests, Combine_DotSegment_IsRemoved) {
    Uri base("http://example.com/a/b/");
    Uri combined(base, "./c");
    EXPECT_EQ(combined.getAbsoluteUriProperty(), "http://example.com/a/b/c");
}

// ---------------------------------------------------------------------------
// Default ports — must match .NET's built-in scheme table exactly
// ---------------------------------------------------------------------------

TEST(UriTests, DefaultPort_UnregisteredScheme_IsMinusOne) {
    // "ssh" is not a .NET built-in scheme and has no default port.
    Uri u("ssh://host/");
    EXPECT_EQ(u.getPortProperty(), -1);
}

// ---------------------------------------------------------------------------
// Malformed port — verified against Uri.cs's port-parsing loop (ParsingError.BadPort for a
// non-digit character in the port position, or a value > 0xFFFF; both surface as
// UriFormatException). The previous code let std::stoi's exception fall through to
// `host_ = authority`, silently mangling host_ into the whole "host:badport" text instead of
// rejecting the URI, and didn't range-check a successfully-parsed but oversized port at all.
// ---------------------------------------------------------------------------

TEST(UriTests, Port_NonNumeric_Throws) {
    EXPECT_THROW(Uri("http://example.com:abc/path"), System::UriFormatException);
}

TEST(UriTests, Port_TooLarge_Throws) {
    EXPECT_THROW(Uri("http://example.com:99999/path"), System::UriFormatException);
}

TEST(UriTests, Port_IntOverflow_Throws) {
    EXPECT_THROW(Uri("http://example.com:99999999999999999999/path"), System::UriFormatException);
}

TEST(UriTests, Port_MaxValid_Parsed) {
    Uri u("http://example.com:65535/path");
    EXPECT_EQ(u.getPortProperty(), 65535);
}

// ---------------------------------------------------------------------------
// Combine with an opaque (non-"//"-form) absolute relativeUri — verified against Uri.cs's
// CreateUri/ResolveHelper: relativeUri is parsed standalone first, and if it is itself absolute
// (hierarchical OR opaque), the base is discarded entirely. The previous check only recognized
// the "://" hierarchical form via a raw substring search.
// ---------------------------------------------------------------------------

TEST(UriTests, Combine_OpaqueAbsoluteRelative_DiscardsBase) {
    Uri base("http://example.com/a/b/");
    Uri combined(base, "mailto:user@example.com");
    EXPECT_EQ(combined.getSchemeProperty(), "mailto");
    EXPECT_EQ(combined.getAbsolutePathProperty(), "user@example.com");
}

TEST(UriTests, Combine_UrnAbsoluteRelative_DiscardsBase) {
    Uri base("http://example.com/a/b/");
    Uri combined(base, "urn:isbn:0-395-36341-1");
    EXPECT_EQ(combined.getSchemeProperty(), "urn");
}

// ---------------------------------------------------------------------------
// Combine preserves the base's userInfo — RFC 3986 §5.3: "if defined, userinfo, host, port
// [are] copied from base" into the merged authority. The previous code omitted
// baseUri.userInfo_ entirely when reconstructing the merged authority string.
// ---------------------------------------------------------------------------

TEST(UriTests, Combine_PreservesBaseUserInfo) {
    Uri base("http://user:pass@example.com/a/b/");
    Uri combined(base, "c");
    EXPECT_EQ(combined.getUserInfoProperty(), "user:pass");
    EXPECT_EQ(combined.getAbsoluteUriProperty(), "http://user:pass@example.com/a/b/c");
}

// ---------------------------------------------------------------------------
// OriginalString -- was entirely missing from this port (C++ counterpart of
// .NET Uri.OriginalString), unlike getAbsoluteUriProperty() which existed but
// is documented (per real .NET semantics) to sometimes differ.
// ---------------------------------------------------------------------------

TEST(UriTests, OriginalString_MatchesConstructorInput) {
    Uri u("http://example.com/path?q=1#frag");
    EXPECT_EQ(u.getOriginalStringProperty(), "http://example.com/path?q=1#frag");
}

TEST(UriTests, OriginalString_WorksForRelativeUri) {
    // Real .NET's AbsoluteUri throws for a relative Uri; OriginalString never does.
    Uri u("relative/path", UriKind::Relative);
    EXPECT_EQ(u.getOriginalStringProperty(), "relative/path");
}
