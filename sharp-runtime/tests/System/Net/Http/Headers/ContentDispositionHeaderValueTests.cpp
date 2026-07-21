// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/ContentDispositionHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/TimeSpan.hpp"

using System::Net::Http::Headers::ContentDispositionHeaderValue;

TEST(ContentDispositionHeaderValueTests, Constructor_DispositionTypeOnly) {
    ContentDispositionHeaderValue v("attachment");
    EXPECT_EQ(v.getDispositionTypeProperty(), "attachment");
    EXPECT_TRUE(v.getParametersProperty().empty());
    EXPECT_EQ(v.ToString(), "attachment");
}

TEST(ContentDispositionHeaderValueTests, Constructor_InvalidType_Throws) {
    EXPECT_THROW(ContentDispositionHeaderValue("bad type"), System::FormatException);
    EXPECT_THROW(ContentDispositionHeaderValue(""), System::ArgumentException);
}

TEST(ContentDispositionHeaderValueTests, Name_GetSet) {
    ContentDispositionHeaderValue v("form-data");
    v.setNameProperty("field1");
    EXPECT_EQ(v.getNameProperty(), "field1");
    EXPECT_EQ(v.ToString(), "form-data; name=\"field1\"");
}

TEST(ContentDispositionHeaderValueTests, Name_SetEmpty_RemovesParameter) {
    ContentDispositionHeaderValue v("form-data");
    v.setNameProperty("field1");
    v.setNameProperty("");
    EXPECT_TRUE(v.getNameProperty().empty());
    EXPECT_TRUE(v.getParametersProperty().empty());
}

TEST(ContentDispositionHeaderValueTests, FileName_GetSet) {
    ContentDispositionHeaderValue v("attachment");
    v.setFileNameProperty("report.pdf");
    EXPECT_EQ(v.getFileNameProperty(), "report.pdf");
    EXPECT_EQ(v.ToString(), "attachment; filename=\"report.pdf\"");
}

TEST(ContentDispositionHeaderValueTests, FileNameStar_RoundTrips) {
    ContentDispositionHeaderValue v("attachment");
    v.setFileNameStarProperty("Naïve.txt");
    EXPECT_EQ(v.getFileNameStarProperty(), "Naïve.txt");
}

TEST(ContentDispositionHeaderValueTests, FileNameStar_ExtraQuote_TreatedAsMalformed) {
    // Real .NET's TryDecode5987 requires *exactly* two single quotes in the whole string (the
    // charset'language'value delimiters) and rejects a third. A well-formed encoder must
    // percent-encode any literal apostrophe in the value, so a raw third quote indicates
    // malformed input -- previously silently accepted, decoding everything after the *second*
    // quote (including the extra quote character) as if it were the value.
    ContentDispositionHeaderValue v("attachment");
    v.getParametersProperty().push_back(
        System::Net::Http::Headers::NameValueHeaderValue("filename*", "UTF-8'en'a'b"));
    EXPECT_EQ(v.getFileNameStarProperty(), "");
}

TEST(ContentDispositionHeaderValueTests, FileNameStar_AsciiOnly_NotPercentEncoded) {
    ContentDispositionHeaderValue v("attachment");
    v.setFileNameStarProperty("simple.txt");
    // ASCII letters/digits/dot are RFC 5987 attr-chars, so no percent-encoding needed.
    bool foundParam = false;
    for (const auto& p : v.getParametersProperty()) {
        if (p.getNameProperty() == "filename*") {
            EXPECT_EQ(p.getValueProperty(), "UTF-8''simple.txt");
            foundParam = true;
        }
    }
    EXPECT_TRUE(foundParam);
}

TEST(ContentDispositionHeaderValueTests, Size_GetSet) {
    ContentDispositionHeaderValue v("attachment");
    v.setSizeProperty(12345);
    ASSERT_TRUE(v.getSizeProperty().has_value());
    EXPECT_EQ(*v.getSizeProperty(), 12345);
}

// Regression test for a wave-3 audit finding: threw std::out_of_range (an unrelated std::
// exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's ContentDispositionHeaderValue
// .Size setter throws (via ArgumentOutOfRangeException.ThrowIfNegative) for a negative size.
TEST(ContentDispositionHeaderValueTests, Size_Negative_Throws) {
    ContentDispositionHeaderValue v("attachment");
    EXPECT_THROW(v.setSizeProperty(-1), System::ArgumentOutOfRangeException);
}

TEST(ContentDispositionHeaderValueTests, CreationDate_RoundTrips) {
    ContentDispositionHeaderValue v("attachment");
    System::DateTimeOffset dt(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    v.setCreationDateProperty(dt);
    ASSERT_TRUE(v.getCreationDateProperty().has_value());
    EXPECT_EQ(v.getCreationDateProperty()->ToString("r"), dt.ToString("r"));
}

TEST(ContentDispositionHeaderValueTests, Parse_SimpleType) {
    auto v = ContentDispositionHeaderValue::Parse("attachment");
    EXPECT_EQ(v.getDispositionTypeProperty(), "attachment");
}

TEST(ContentDispositionHeaderValueTests, Parse_WithParameters) {
    auto v = ContentDispositionHeaderValue::Parse("form-data; name=\"field1\"; filename=\"file.txt\"");
    EXPECT_EQ(v.getDispositionTypeProperty(), "form-data");
    EXPECT_EQ(v.getNameProperty(), "field1");
    EXPECT_EQ(v.getFileNameProperty(), "file.txt");
}

TEST(ContentDispositionHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(ContentDispositionHeaderValue::Parse("bad type"), System::FormatException);
}

TEST(ContentDispositionHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    ContentDispositionHeaderValue result("x");
    EXPECT_FALSE(ContentDispositionHeaderValue::TryParse("bad type", result));
}

TEST(ContentDispositionHeaderValueTests, Equals_SameTypeAndParameters) {
    auto a = ContentDispositionHeaderValue::Parse("form-data; name=\"field1\"");
    auto b = ContentDispositionHeaderValue::Parse("form-data; name=\"field1\"");
    EXPECT_TRUE(a == b);
}

TEST(ContentDispositionHeaderValueTests, Equals_DispositionTypeCaseInsensitive) {
    auto a = ContentDispositionHeaderValue::Parse("Form-Data; name=\"field1\"");
    auto b = ContentDispositionHeaderValue::Parse("form-data; name=\"field1\"");
    EXPECT_TRUE(a == b);
}

TEST(ContentDispositionHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    auto a = ContentDispositionHeaderValue::Parse("form-data; name=\"field1\"");
    auto b = ContentDispositionHeaderValue::Parse("form-data; name=\"field1\"");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
