// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/Text/Encodings/Web/HtmlEncoder.hpp"
#include "System/Text/Encodings/Web/JavaScriptEncoder.hpp"
#include "System/Text/Encodings/Web/UrlEncoder.hpp"

using System::Text::Encodings::Web::HtmlEncoder;
using System::Text::Encodings::Web::JavaScriptEncoder;
using System::Text::Encodings::Web::UrlEncoder;

// ---------------------------------------------------------------------------
// HtmlEncoder
// Reference: /rv/tmp/runtime/src/libraries/System.Text.Encodings.Web/tests/HtmlEncoderTests.cs
// ---------------------------------------------------------------------------

TEST(EncodingWebTests, HtmlEncodeEmptyString) {
    EXPECT_EQ(HtmlEncoder::Encode(""), "");
}

TEST(EncodingWebTests, HtmlEncodePlainTextUnchanged) {
    EXPECT_EQ(HtmlEncoder::Encode("Hello, world!"), "Hello, world!");
    EXPECT_EQ(HtmlEncoder::Encode("abc"), "abc");
}

// Official .NET inline-data test vectors
TEST(EncodingWebTests, HtmlEncodeAmpersand) {
    EXPECT_EQ(HtmlEncoder::Encode("&"), "&amp;");
}

TEST(EncodingWebTests, HtmlEncodeLessThan) {
    EXPECT_EQ(HtmlEncoder::Encode("<"), "&lt;");
}

TEST(EncodingWebTests, HtmlEncodeGreaterThan) {
    EXPECT_EQ(HtmlEncoder::Encode(">"), "&gt;");
}

TEST(EncodingWebTests, HtmlEncodeDoubleQuote) {
    EXPECT_EQ(HtmlEncoder::Encode("\""), "&quot;");
}

TEST(EncodingWebTests, HtmlEncodeSingleQuote) {
    EXPECT_EQ(HtmlEncoder::Encode("'"), "&#x27;");
}

// Official .NET composite assertions
TEST(EncodingWebTests, HtmlEncodeAtBeginning) {
    EXPECT_EQ(HtmlEncoder::Encode("&Hello, there!"), "&amp;Hello, there!");
}

TEST(EncodingWebTests, HtmlEncodeAtEnd) {
    EXPECT_EQ(HtmlEncoder::Encode("Hello, there!&"), "Hello, there!&amp;");
}

TEST(EncodingWebTests, HtmlEncodeInMiddle) {
    EXPECT_EQ(HtmlEncoder::Encode("Hello, &there!"), "Hello, &amp;there!");
}

TEST(EncodingWebTests, HtmlEncodeInterspersed) {
    EXPECT_EQ(HtmlEncoder::Encode("Hello, <there>!"), "Hello, &lt;there&gt;!");
}

TEST(EncodingWebTests, HtmlEncodeAllSpecialChars) {
    EXPECT_EQ(HtmlEncoder::Encode("&<>\"'"), "&amp;&lt;&gt;&quot;&#x27;");
}

TEST(EncodingWebTests, HtmlEncodeDefaultSameAsStatic) {
    const std::string input = "<b>Hello & 'world'</b>";
    EXPECT_EQ(HtmlEncoder::Default().Encode(input, 0, static_cast<int>(input.size())),
              HtmlEncoder::Encode(input));
}

// ---------------------------------------------------------------------------
// UrlEncoder
// Reference: /rv/tmp/runtime/src/libraries/System.Text.Encodings.Web/tests/UrlEncoderTests.cs
// ---------------------------------------------------------------------------

TEST(EncodingWebTests, UrlEncodeEmptyString) {
    EXPECT_EQ(UrlEncoder::Encode(""), "");
}

TEST(EncodingWebTests, UrlEncodeUnreservedCharsUnchanged) {
    // RFC 3986 unreserved: A-Z a-z 0-9 - _ . ~
    EXPECT_EQ(UrlEncoder::Encode("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~"),
              "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~");
}

// Official .NET assertions
TEST(EncodingWebTests, UrlEncodeAmpersandAtBeginning) {
    EXPECT_EQ(UrlEncoder::Encode("&Hello,there!"), "%26Hello%2Cthere%21");
}

TEST(EncodingWebTests, UrlEncodeAmpersandAtEnd) {
    EXPECT_EQ(UrlEncoder::Encode("Hello,there!&"), "Hello%2Cthere%21%26");
}

TEST(EncodingWebTests, UrlEncodeSpace) {
    EXPECT_EQ(UrlEncoder::Encode(" "), "%20");
}

TEST(EncodingWebTests, UrlEncodeEquals) {
    EXPECT_EQ(UrlEncoder::Encode("="), "%3D");
}

TEST(EncodingWebTests, UrlEncodeSlash) {
    EXPECT_EQ(UrlEncoder::Encode("/"), "%2F");
}

TEST(EncodingWebTests, UrlEncodeTypicalQueryString) {
    EXPECT_EQ(UrlEncoder::Encode("key=value&other=123"),
              "key%3Dvalue%26other%3D123");
}

TEST(EncodingWebTests, UrlEncodeRoundtrip) {
    const std::string original = "Hello, world! & more=stuff";
    EXPECT_EQ(UrlEncoder::Decode(UrlEncoder::Encode(original)), original);
}

TEST(EncodingWebTests, UrlDecodePercent20IsSpace) {
    EXPECT_EQ(UrlEncoder::Decode("%20"), " ");
}

TEST(EncodingWebTests, UrlDecodePercent26IsAmpersand) {
    EXPECT_EQ(UrlEncoder::Decode("%26"), "&");
}

TEST(EncodingWebTests, UrlDecodePlusIsSpace) {
    EXPECT_EQ(UrlEncoder::Decode("hello+world"), "hello world");
}

TEST(EncodingWebTests, UrlDecodePassthroughUnencoded) {
    EXPECT_EQ(UrlEncoder::Decode("abc-_.~"), "abc-_.~");
}

// ---------------------------------------------------------------------------
// JavaScriptEncoder
// Reference: /rv/tmp/runtime/src/libraries/System.Text.Encodings.Web/tests/JavaScriptEncoderTests.cs
// ---------------------------------------------------------------------------

TEST(EncodingWebTests, JavaScriptEncodeEmptyString) {
    EXPECT_EQ(JavaScriptEncoder::Encode(""), "");
}

TEST(EncodingWebTests, JavaScriptEncodePlainTextUnchanged) {
    EXPECT_EQ(JavaScriptEncoder::Encode("Hello, world!"), "Hello, world!");
}

TEST(EncodingWebTests, JavaScriptEncodeBackslash) {
    EXPECT_EQ(JavaScriptEncoder::Encode("\\"), "\\\\");
}

TEST(EncodingWebTests, JavaScriptEncodeDoubleQuote) {
    EXPECT_EQ(JavaScriptEncoder::Encode("\""), "\\\"");
}

TEST(EncodingWebTests, JavaScriptEncodeNewline) {
    EXPECT_EQ(JavaScriptEncoder::Encode("\n"), "\\n");
}

TEST(EncodingWebTests, JavaScriptEncodeCarriageReturn) {
    EXPECT_EQ(JavaScriptEncoder::Encode("\r"), "\\r");
}

TEST(EncodingWebTests, JavaScriptEncodeTab) {
    EXPECT_EQ(JavaScriptEncoder::Encode("\t"), "\\t");
}

TEST(EncodingWebTests, JavaScriptEncodeControlChar) {
    // \x01 is a control char < 0x20, encoded as 
    EXPECT_EQ(JavaScriptEncoder::Encode("\x01"), "\\u0001");
}

TEST(EncodingWebTests, JavaScriptEncodeJsonStringValue) {
    EXPECT_EQ(JavaScriptEncoder::Encode("say \"hello\"\nworld\\done"),
              "say \\\"hello\\\"\\nworld\\\\done");
}

TEST(EncodingWebTests, JavaScriptEncodeDefaultSameAsStatic) {
    const std::string input = "foo\nbar\"baz";
    EXPECT_EQ(JavaScriptEncoder::Default().Encode(input),
              JavaScriptEncoder::Encode(input));
}
