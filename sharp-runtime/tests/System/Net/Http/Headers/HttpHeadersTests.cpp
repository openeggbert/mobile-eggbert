// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/HttpHeaders.hpp"
#include "System/Net/Http/Headers/HttpHeadersNonValidated.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"

using System::Net::Http::Headers::HttpHeaders;
using System::Net::Http::Headers::HttpHeadersNonValidated;

namespace {
    // HttpHeaders has a protected constructor (matching .NET's abstract class) — a minimal
    // concrete subclass is needed to instantiate it for testing.
    class TestHeaders : public HttpHeaders {
    public:
        TestHeaders() = default;
    };
}

TEST(HttpHeadersTests, Add_ThenGetValues) {
    TestHeaders h;
    h.Add("X-Custom", "value1");
    auto values = h.GetValues("X-Custom");
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "value1");
}

TEST(HttpHeadersTests, Add_MultipleValues) {
    TestHeaders h;
    h.Add("Accept", "text/html");
    h.Add("Accept", "application/json");
    auto values = h.GetValues("Accept");
    ASSERT_EQ(values.size(), 2u);
}

TEST(HttpHeadersTests, Add_ListOverload) {
    TestHeaders h;
    h.Add("Accept", std::vector<std::string>{"text/html", "application/json"});
    EXPECT_EQ(h.GetValues("Accept").size(), 2u);
}

TEST(HttpHeadersTests, Contains_CaseInsensitive) {
    TestHeaders h;
    h.Add("Content-Type", "text/plain");
    EXPECT_TRUE(h.Contains("content-type"));
}

TEST(HttpHeadersTests, GetValues_Missing_Throws) {
    TestHeaders h;
    EXPECT_THROW(h.GetValues("X-Missing"), System::InvalidOperationException);
}

TEST(HttpHeadersTests, TryGetValues_Missing_ReturnsFalse) {
    TestHeaders h;
    std::vector<std::string> values;
    EXPECT_FALSE(h.TryGetValues("X-Missing", values));
}

TEST(HttpHeadersTests, Remove_DeletesHeader) {
    TestHeaders h;
    h.Add("X-Custom", "value1");
    EXPECT_TRUE(h.Remove("X-Custom"));
    EXPECT_FALSE(h.Contains("X-Custom"));
}

TEST(HttpHeadersTests, Remove_Missing_ReturnsFalse) {
    TestHeaders h;
    EXPECT_FALSE(h.Remove("X-Missing"));
}

TEST(HttpHeadersTests, Clear_RemovesAll) {
    TestHeaders h;
    h.Add("X-Custom", "value1");
    h.Clear();
    EXPECT_EQ(h.getCountProperty(), 0);
}

TEST(HttpHeadersTests, Add_InvalidNameChars_Throws) {
    TestHeaders h;
    EXPECT_THROW(h.Add("Bad Name", "value"), System::FormatException);
}

TEST(HttpHeadersTests, Add_EmptyName_Throws) {
    TestHeaders h;
    EXPECT_THROW(h.Add("", "value"), System::ArgumentException);
}

TEST(HttpHeadersTests, Add_InvalidValueCrlf_Throws) {
    TestHeaders h;
    EXPECT_THROW(h.Add("X-Custom", "bad\r\nvalue"), System::FormatException);
}

// Real .NET's System.Net.Http.Headers.HttpHeaders rejects ANY embedded CR/LF/NUL, unlike the
// separate, older WebHeaderCollection which tolerates a "\r\n " obs-fold sequence for backward
// compatibility -- an obs-fold-shaped value like "bar\r\n evil: value" must still be rejected
// here (accepting it would let ToString() serialize an injected header line verbatim).
TEST(HttpHeadersTests, Add_ObsFoldShapedValue_Throws) {
    TestHeaders h;
    EXPECT_THROW(h.Add("X-Custom", "bar\r\n evil: value"), System::FormatException);
}

TEST(HttpHeadersTests, Add_EmbeddedNul_Throws) {
    TestHeaders h;
    std::string value = "bad";
    value += '\0';
    value += "value";
    EXPECT_THROW(h.Add("X-Custom", value), System::FormatException);
}

TEST(HttpHeadersTests, TryAddWithoutValidation_SkipsValidation) {
    TestHeaders h;
    EXPECT_TRUE(h.TryAddWithoutValidation("X-Custom", "value1"));
    EXPECT_EQ(h.GetValues("X-Custom")[0], "value1");
}

TEST(HttpHeadersTests, ToString_Format) {
    TestHeaders h;
    h.Add("Host", "example.com");
    EXPECT_EQ(h.ToString(), "Host: example.com\r\n\r\n");
}

TEST(HttpHeadersTests, Count_ReflectsDistinctNames) {
    TestHeaders h;
    h.Add("Accept", "a");
    h.Add("Accept", "b");
    h.Add("Host", "example.com");
    EXPECT_EQ(h.getCountProperty(), 2);
}

// ---------------------------------------------------------------------------
// HttpHeadersNonValidated
// ---------------------------------------------------------------------------

TEST(HttpHeadersNonValidatedTests, DelegatesToUnderlyingHeaders) {
    TestHeaders h;
    h.Add("X-Custom", "value1");
    HttpHeadersNonValidated view(h);
    EXPECT_TRUE(view.Contains("X-Custom"));
    std::vector<std::string> values;
    EXPECT_TRUE(view.TryGetValues("X-Custom", values));
    EXPECT_EQ(values[0], "value1");
    EXPECT_EQ(view.getCountProperty(), 1);
}
