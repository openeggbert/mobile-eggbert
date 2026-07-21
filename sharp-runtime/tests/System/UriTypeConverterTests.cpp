// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriFormatException.hpp"
#include "System/UriTypeConverter.hpp"

using System::UriTypeConverter;
using System::Uri;

TEST(UriTypeConverterTest, CanConvertFrom) {
    UriTypeConverter c;
    EXPECT_TRUE(c.CanConvertFrom());
}

TEST(UriTypeConverterTest, CanConvertTo) {
    UriTypeConverter c;
    EXPECT_TRUE(c.CanConvertTo());
}

TEST(UriTypeConverterTest, ConvertFromString) {
    UriTypeConverter c;
    Uri uri = c.ConvertFrom("http://example.com");
    EXPECT_EQ(uri.getHostProperty(), "example.com");
}

TEST(UriTypeConverterTest, ConvertFromEmptyThrows) {
    UriTypeConverter c;
    EXPECT_THROW(c.ConvertFrom(""), System::UriFormatException);
}

TEST(UriTypeConverterTest, ConvertToString) {
    UriTypeConverter c;
    Uri uri("http://example.com/path");
    std::string s = c.ConvertTo(uri);
    EXPECT_NE(s.find("example.com"), std::string::npos);
}

TEST(UriTypeConverterTest, RoundTrip) {
    UriTypeConverter c;
    Uri original("https://test.org/api");
    std::string s = c.ConvertTo(original);
    Uri restored = c.ConvertFrom(s);
    EXPECT_EQ(restored.getHostProperty(), "test.org");
}

// ---------------------------------------------------------------------------
// Regression: real .NET's UriTypeConverter.ConvertTo returns uri.OriginalString,
// not uri.AbsoluteUri (verified against UriTypeConverter.cs) -- this matters
// because AbsoluteUri throws for a relative Uri while OriginalString never does.
// ---------------------------------------------------------------------------

TEST(UriTypeConverterTest, ConvertTo_UsesOriginalString) {
    UriTypeConverter c;
    Uri uri("http://example.com/path");
    EXPECT_EQ(c.ConvertTo(uri), uri.getOriginalStringProperty());
}
