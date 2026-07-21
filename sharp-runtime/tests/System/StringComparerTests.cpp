// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <memory>
#include "System/StringComparer.hpp"
#include "System/ArgumentException.hpp"

using System::StringComparer;
using System::OrdinalComparer;
using System::CultureAwareComparer;
using System::StringComparison;

// ---------------------------------------------------------------------------
// OrdinalComparer — type identity
// ---------------------------------------------------------------------------

TEST(OrdinalComparerTests, Ordinal_ReturnsOrdinalComparer) {
    auto cmp = StringComparer::Ordinal();
    EXPECT_NE(std::dynamic_pointer_cast<OrdinalComparer>(cmp), nullptr);
}

TEST(OrdinalComparerTests, OrdinalIgnoreCase_ReturnsOrdinalComparer) {
    auto cmp = StringComparer::OrdinalIgnoreCase();
    EXPECT_NE(std::dynamic_pointer_cast<OrdinalComparer>(cmp), nullptr);
}

// ---------------------------------------------------------------------------
// OrdinalComparer — getIgnoreCaseProperty
// ---------------------------------------------------------------------------

TEST(OrdinalComparerTests, CaseSensitive_IgnoreCase_False) {
    OrdinalComparer cmp(false);
    EXPECT_FALSE(cmp.getIgnoreCaseProperty());
}

TEST(OrdinalComparerTests, CaseInsensitive_IgnoreCase_True) {
    OrdinalComparer cmp(true);
    EXPECT_TRUE(cmp.getIgnoreCaseProperty());
}

// ---------------------------------------------------------------------------
// OrdinalComparer — Compare
// ---------------------------------------------------------------------------

TEST(OrdinalComparerTests, Compare_Less) {
    OrdinalComparer cmp(false);
    EXPECT_LT(cmp.Compare("abc", "abd"), 0);
}

TEST(OrdinalComparerTests, Compare_Equal) {
    OrdinalComparer cmp(false);
    EXPECT_EQ(cmp.Compare("abc", "abc"), 0);
}

TEST(OrdinalComparerTests, Compare_Greater) {
    OrdinalComparer cmp(false);
    EXPECT_GT(cmp.Compare("abd", "abc"), 0);
}

TEST(OrdinalComparerTests, Compare_CaseSensitive_DifferentCase) {
    OrdinalComparer cmp(false);
    EXPECT_NE(cmp.Compare("ABC", "abc"), 0);
}

TEST(OrdinalComparerTests, Compare_IgnoreCase_Equal) {
    OrdinalComparer cmp(true);
    EXPECT_EQ(cmp.Compare("ABC", "abc"), 0);
}

TEST(OrdinalComparerTests, Compare_IgnoreCase_Less) {
    OrdinalComparer cmp(true);
    EXPECT_LT(cmp.Compare("aaa", "bbb"), 0);
}

// ---------------------------------------------------------------------------
// OrdinalComparer — Equals
// ---------------------------------------------------------------------------

TEST(OrdinalComparerTests, Equals_SameString_True) {
    OrdinalComparer cmp(false);
    EXPECT_TRUE(cmp.Equals("hello", "hello"));
}

TEST(OrdinalComparerTests, Equals_DifferentCase_False) {
    OrdinalComparer cmp(false);
    EXPECT_FALSE(cmp.Equals("Hello", "hello"));
}

TEST(OrdinalComparerTests, Equals_IgnoreCase_DifferentCase_True) {
    OrdinalComparer cmp(true);
    EXPECT_TRUE(cmp.Equals("HELLO", "hello"));
}

TEST(OrdinalComparerTests, Equals_IgnoreCase_DifferentString_False) {
    OrdinalComparer cmp(true);
    EXPECT_FALSE(cmp.Equals("hello", "world"));
}

// ---------------------------------------------------------------------------
// OrdinalComparer — GetHashCode
// ---------------------------------------------------------------------------

TEST(OrdinalComparerTests, GetHashCode_SameString_SameHash) {
    OrdinalComparer cmp(false);
    EXPECT_EQ(cmp.GetHashCode("test"), cmp.GetHashCode("test"));
}

TEST(OrdinalComparerTests, GetHashCode_CaseSensitive_DifferentCaseDifferentHash) {
    OrdinalComparer cmp(false);
    EXPECT_NE(cmp.GetHashCode("ABC"), cmp.GetHashCode("abc"));
}

TEST(OrdinalComparerTests, GetHashCode_IgnoreCase_SameHashForDifferentCase) {
    OrdinalComparer cmp(true);
    EXPECT_EQ(cmp.GetHashCode("ABC"), cmp.GetHashCode("abc"));
}

// ---------------------------------------------------------------------------
// CultureAwareComparer — type identity
// ---------------------------------------------------------------------------

TEST(CultureAwareComparerTests, InvariantCulture_ReturnsCultureAwareComparer) {
    auto cmp = StringComparer::InvariantCulture();
    EXPECT_NE(std::dynamic_pointer_cast<CultureAwareComparer>(cmp), nullptr);
}

TEST(CultureAwareComparerTests, InvariantCultureIgnoreCase_ReturnsCultureAwareComparer) {
    auto cmp = StringComparer::InvariantCultureIgnoreCase();
    EXPECT_NE(std::dynamic_pointer_cast<CultureAwareComparer>(cmp), nullptr);
}

TEST(CultureAwareComparerTests, CurrentCulture_ReturnsCultureAwareComparer) {
    auto cmp = StringComparer::CurrentCulture();
    EXPECT_NE(std::dynamic_pointer_cast<CultureAwareComparer>(cmp), nullptr);
}

TEST(CultureAwareComparerTests, CurrentCultureIgnoreCase_ReturnsCultureAwareComparer) {
    auto cmp = StringComparer::CurrentCultureIgnoreCase();
    EXPECT_NE(std::dynamic_pointer_cast<CultureAwareComparer>(cmp), nullptr);
}

// ---------------------------------------------------------------------------
// CultureAwareComparer — getIgnoreCaseProperty
// ---------------------------------------------------------------------------

TEST(CultureAwareComparerTests, CaseSensitive_IgnoreCase_False) {
    CultureAwareComparer cmp(false);
    EXPECT_FALSE(cmp.getIgnoreCaseProperty());
}

TEST(CultureAwareComparerTests, CaseInsensitive_IgnoreCase_True) {
    CultureAwareComparer cmp(true);
    EXPECT_TRUE(cmp.getIgnoreCaseProperty());
}

// ---------------------------------------------------------------------------
// CultureAwareComparer — Compare / Equals / GetHashCode
// ---------------------------------------------------------------------------

TEST(CultureAwareComparerTests, Compare_Equal) {
    CultureAwareComparer cmp(false);
    EXPECT_EQ(cmp.Compare("foo", "foo"), 0);
}

TEST(CultureAwareComparerTests, Compare_Less) {
    CultureAwareComparer cmp(false);
    EXPECT_LT(cmp.Compare("bar", "foo"), 0);
}

TEST(CultureAwareComparerTests, Equals_CaseSensitive_True) {
    CultureAwareComparer cmp(false);
    EXPECT_TRUE(cmp.Equals("hello", "hello"));
}

TEST(CultureAwareComparerTests, Equals_CaseSensitive_False) {
    CultureAwareComparer cmp(false);
    EXPECT_FALSE(cmp.Equals("Hello", "hello"));
}

TEST(CultureAwareComparerTests, Equals_IgnoreCase_True) {
    CultureAwareComparer cmp(true);
    EXPECT_TRUE(cmp.Equals("WORLD", "world"));
}

TEST(CultureAwareComparerTests, GetHashCode_Consistent) {
    CultureAwareComparer cmp(false);
    EXPECT_EQ(cmp.GetHashCode("key"), cmp.GetHashCode("key"));
}

TEST(CultureAwareComparerTests, GetHashCode_IgnoreCase_SameHash) {
    CultureAwareComparer cmp(true);
    EXPECT_EQ(cmp.GetHashCode("KEY"), cmp.GetHashCode("key"));
}

// ---------------------------------------------------------------------------
// StringComparer::FromComparison
// ---------------------------------------------------------------------------

TEST(StringComparerTests, FromComparison_Ordinal_ReturnsOrdinalComparer) {
    auto cmp = StringComparer::FromComparison(StringComparison::Ordinal);
    EXPECT_NE(std::dynamic_pointer_cast<OrdinalComparer>(cmp), nullptr);
    EXPECT_FALSE(std::dynamic_pointer_cast<OrdinalComparer>(cmp)->getIgnoreCaseProperty());
}

TEST(StringComparerTests, FromComparison_OrdinalIgnoreCase_ReturnsOrdinalIgnoreCase) {
    auto cmp = StringComparer::FromComparison(StringComparison::OrdinalIgnoreCase);
    auto oc = std::dynamic_pointer_cast<OrdinalComparer>(cmp);
    EXPECT_NE(oc, nullptr);
    EXPECT_TRUE(oc->getIgnoreCaseProperty());
}

TEST(StringComparerTests, FromComparison_InvariantCulture_ReturnsCultureAware) {
    auto cmp = StringComparer::FromComparison(StringComparison::InvariantCulture);
    EXPECT_NE(std::dynamic_pointer_cast<CultureAwareComparer>(cmp), nullptr);
}

TEST(StringComparerTests, FromComparison_InvariantCultureIgnoreCase_ReturnsCultureAwareIgnoreCase) {
    auto cmp = StringComparer::FromComparison(StringComparison::InvariantCultureIgnoreCase);
    auto cc = std::dynamic_pointer_cast<CultureAwareComparer>(cmp);
    EXPECT_NE(cc, nullptr);
    EXPECT_TRUE(cc->getIgnoreCaseProperty());
}

TEST(StringComparerTests, FromComparison_CurrentCulture_ReturnsCultureAware) {
    auto cmp = StringComparer::FromComparison(StringComparison::CurrentCulture);
    EXPECT_NE(std::dynamic_pointer_cast<CultureAwareComparer>(cmp), nullptr);
}

TEST(StringComparerTests, FromComparison_CurrentCultureIgnoreCase_ReturnsCultureAwareIgnoreCase) {
    auto cmp = StringComparer::FromComparison(StringComparison::CurrentCultureIgnoreCase);
    auto cc = std::dynamic_pointer_cast<CultureAwareComparer>(cmp);
    EXPECT_NE(cc, nullptr);
    EXPECT_TRUE(cc->getIgnoreCaseProperty());
}

// ---------------------------------------------------------------------------
// StringComparer — static factory consistency
// ---------------------------------------------------------------------------

TEST(StringComparerTests, Ordinal_NotNull) {
    EXPECT_NE(StringComparer::Ordinal(), nullptr);
}

TEST(StringComparerTests, OrdinalIgnoreCase_NotNull) {
    EXPECT_NE(StringComparer::OrdinalIgnoreCase(), nullptr);
}

TEST(StringComparerTests, InvariantCulture_Equals_SameString) {
    auto cmp = StringComparer::InvariantCulture();
    EXPECT_TRUE(cmp->Equals("hello", "hello"));
    EXPECT_FALSE(cmp->Equals("Hello", "hello"));
}

TEST(StringComparerTests, InvariantCultureIgnoreCase_Equals_CaseInsensitive) {
    auto cmp = StringComparer::InvariantCultureIgnoreCase();
    EXPECT_TRUE(cmp->Equals("Hello", "hello"));
}

TEST(StringComparerTests, CurrentCulture_Equals_SameString) {
    auto cmp = StringComparer::CurrentCulture();
    EXPECT_TRUE(cmp->Equals("world", "world"));
}

TEST(StringComparerTests, CurrentCultureIgnoreCase_Equals_CaseInsensitive) {
    auto cmp = StringComparer::CurrentCultureIgnoreCase();
    EXPECT_TRUE(cmp->Equals("ABC", "abc"));
}
