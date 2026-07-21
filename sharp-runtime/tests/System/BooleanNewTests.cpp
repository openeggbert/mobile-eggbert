// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Boolean.hpp"

using System::Boolean;

// Additional Boolean tests (basic Parse/TryParse/ToString already in PrimitiveTypeTests2.cpp)

TEST(BooleanNewTests, CompareTo_TrueTrue_Zero) {
    EXPECT_EQ(Boolean::CompareTo(true, true), 0);
}
TEST(BooleanNewTests, CompareTo_FalseFalse_Zero) {
    EXPECT_EQ(Boolean::CompareTo(false, false), 0);
}
TEST(BooleanNewTests, CompareTo_TrueFalse_Positive) {
    EXPECT_GT(Boolean::CompareTo(true, false), 0);
}
TEST(BooleanNewTests, CompareTo_FalseTrue_Negative) {
    EXPECT_LT(Boolean::CompareTo(false, true), 0);
}

TEST(BooleanNewTests, Equals_TrueTrue_True) {
    EXPECT_TRUE(Boolean::Equals(true, true));
}
TEST(BooleanNewTests, Equals_FalseFalse_True) {
    EXPECT_TRUE(Boolean::Equals(false, false));
}
TEST(BooleanNewTests, Equals_TrueFalse_False) {
    EXPECT_FALSE(Boolean::Equals(true, false));
}

TEST(BooleanNewTests, GetHashCode_True_IsOne) {
    EXPECT_EQ(Boolean::GetHashCode(true), 1);
}
TEST(BooleanNewTests, GetHashCode_False_IsZero) {
    EXPECT_EQ(Boolean::GetHashCode(false), 0);
}

TEST(BooleanNewTests, ToString_True_IsTrue) {
    EXPECT_EQ(Boolean::ToString(true), "True");
}
TEST(BooleanNewTests, ToString_False_IsFalse) {
    EXPECT_EQ(Boolean::ToString(false), "False");
}

TEST(BooleanNewTests, TrueString_IsTrue) {
    EXPECT_EQ(std::string(Boolean::TrueString), "True");
}
TEST(BooleanNewTests, FalseString_IsFalse) {
    EXPECT_EQ(std::string(Boolean::FalseString), "False");
}

// ---------------------------------------------------------------------------
// Parse / TryParse — case-insensitive and whitespace trimming (.NET parity)
// ---------------------------------------------------------------------------

TEST(BooleanNewTests, Parse_UppercaseTRUE_ReturnsTrue) {
    EXPECT_TRUE(Boolean::Parse("TRUE"));
}
TEST(BooleanNewTests, Parse_UppercaseFALSE_ReturnsFalse) {
    EXPECT_FALSE(Boolean::Parse("FALSE"));
}
TEST(BooleanNewTests, Parse_MixedCase_ReturnsTrue) {
    EXPECT_TRUE(Boolean::Parse("tRuE"));
}
TEST(BooleanNewTests, Parse_MixedCaseFalse_ReturnsFalse) {
    EXPECT_FALSE(Boolean::Parse("fAlSe"));
}
TEST(BooleanNewTests, Parse_LeadingWhitespace_ReturnsTrue) {
    EXPECT_TRUE(Boolean::Parse("  True"));
}
TEST(BooleanNewTests, Parse_TrailingWhitespace_ReturnsFalse) {
    EXPECT_FALSE(Boolean::Parse("False  "));
}
TEST(BooleanNewTests, Parse_BothWhitespace_ReturnsTrue) {
    EXPECT_TRUE(Boolean::Parse("  true  "));
}
TEST(BooleanNewTests, Parse_InvalidString_Throws) {
    EXPECT_THROW(Boolean::Parse("yes"), System::FormatException);
}

TEST(BooleanNewTests, TryParse_UppercaseTRUE_Succeeds) {
    bool r = false;
    EXPECT_TRUE(Boolean::TryParse("TRUE", r));
    EXPECT_TRUE(r);
}
TEST(BooleanNewTests, TryParse_UppercaseFALSE_Succeeds) {
    bool r = true;
    EXPECT_TRUE(Boolean::TryParse("FALSE", r));
    EXPECT_FALSE(r);
}
TEST(BooleanNewTests, TryParse_WhitespacePadded_Succeeds) {
    bool r = false;
    EXPECT_TRUE(Boolean::TryParse("\t True \n", r));
    EXPECT_TRUE(r);
}
TEST(BooleanNewTests, TryParse_InvalidString_ReturnsFalse) {
    bool r = true;
    EXPECT_FALSE(Boolean::TryParse("maybe", r));
    EXPECT_FALSE(r);
}
