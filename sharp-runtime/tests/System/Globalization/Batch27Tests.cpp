// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 27:
//   CompareInfo:           getLCIDProperty, all comparison methods
//   CompareOptions:        enum values, operator|, operator&
//   CultureInfo:           int ctor, CurrentUICulture, all properties
//   CultureNotFoundException: all ctors, getInvalidCultureIdProperty
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Globalization/CompareInfo.hpp"
#include "System/Globalization/CompareOptions.hpp"
#include "System/Globalization/CultureInfo.hpp"
#include "System/Globalization/CultureNotFoundException.hpp"
#include "System/Globalization/DateTimeFormatInfo.hpp"
#include "System/Globalization/NumberFormatInfo.hpp"
#include <string>

using System::Globalization::CompareInfo;
using System::Globalization::CompareOptions;
using System::Globalization::SortKey;
using System::Globalization::CultureInfo;
using System::Globalization::CultureNotFoundException;

// ===========================================================================
// CompareOptions
// ===========================================================================

TEST(CompareOptionsBatch27Test, Values) {
    EXPECT_EQ(static_cast<int>(CompareOptions::None),             0x00000000);
    EXPECT_EQ(static_cast<int>(CompareOptions::IgnoreCase),       0x00000001);
    EXPECT_EQ(static_cast<int>(CompareOptions::Ordinal),          0x40000000);
    EXPECT_EQ(static_cast<int>(CompareOptions::OrdinalIgnoreCase), 0x10000000);
}

TEST(CompareOptionsBatch27Test, OperatorOr) {
    auto combined = CompareOptions::IgnoreCase | CompareOptions::IgnoreNonSpace;
    EXPECT_EQ(static_cast<int>(combined), 0x03);
}

TEST(CompareOptionsBatch27Test, OperatorAnd) {
    auto opts = CompareOptions::IgnoreCase | CompareOptions::IgnoreWidth;
    auto masked = opts & CompareOptions::IgnoreCase;
    EXPECT_EQ(masked, CompareOptions::IgnoreCase);
}

// ===========================================================================
// CompareInfo
// ===========================================================================

TEST(CompareInfoBatch27Test, GetCompareInfo_ByName) {
    auto ci = CompareInfo::GetCompareInfo("fr-FR");
    EXPECT_EQ(ci.getNameProperty(), "fr-FR");
}

TEST(CompareInfoBatch27Test, GetCompareInfo_ByLCID) {
    auto ci = CompareInfo::GetCompareInfo(1033);
    EXPECT_EQ(ci.getNameProperty(), "en-US");
}

TEST(CompareInfoBatch27Test, LCIDProperty) {
    auto ci = CompareInfo::GetCompareInfo("en-US");
    EXPECT_EQ(ci.getLCIDProperty(), 0);
}

TEST(CompareInfoBatch27Test, Compare_CaseSensitive) {
    CompareInfo ci("en-US");
    EXPECT_LT(ci.Compare("abc", "abd"), 0);
    EXPECT_EQ(ci.Compare("abc", "abc"), 0);
    EXPECT_GT(ci.Compare("b", "a"), 0);
}

TEST(CompareInfoBatch27Test, Compare_IgnoreCase) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.Compare("Hello", "hello", CompareOptions::IgnoreCase), 0);
    EXPECT_NE(ci.Compare("Hello", "hello"), 0);
}

TEST(CompareInfoBatch27Test, Compare_OrdinalIgnoreCase) {
    // Regression: OrdinalIgnoreCase (0x10000000) and IgnoreCase (0x1) are distinct,
    // non-overlapping bits; a prior implementation only checked the IgnoreCase bit, so
    // passing OrdinalIgnoreCase alone silently fell through to case-sensitive comparison.
    // .NET's real invariant-mode CompareInfo treats both as case-insensitive
    // (CompareInfo.Invariant.cs: `(options & (IgnoreCase | OrdinalIgnoreCase)) != 0`).
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.Compare("Hello", "hello", CompareOptions::OrdinalIgnoreCase), 0);
}

TEST(CompareInfoBatch27Test, IndexOf_OrdinalIgnoreCase) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.IndexOf("Hello World", "WORLD", CompareOptions::OrdinalIgnoreCase), 6);
}

TEST(CompareInfoBatch27Test, GetSortKey_OrdinalIgnoreCase_ProducesEqualKeys) {
    CompareInfo ci("en-US");
    auto k1 = ci.GetSortKey("HELLO", CompareOptions::OrdinalIgnoreCase);
    auto k2 = ci.GetSortKey("hello", CompareOptions::OrdinalIgnoreCase);
    EXPECT_TRUE(k1 == k2);
}

TEST(CompareInfoBatch27Test, GetHashCode_OrdinalIgnoreCase_MatchesAcrossCase) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.GetHashCode("HELLO", CompareOptions::OrdinalIgnoreCase),
              ci.GetHashCode("hello", CompareOptions::OrdinalIgnoreCase));
}

TEST(CompareInfoBatch27Test, Compare_SubstringOverload) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.Compare("hello world", 6, 5, "world", 0, 5), 0);
}

TEST(CompareInfoBatch27Test, Compare_SubstringOverload_OutOfRange_Throws) {
    CompareInfo ci("en-US");
    EXPECT_THROW(ci.Compare("hello", 3, 10, "world", 0, 5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ci.Compare("hello", -1, 2, "world", 0, 5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ci.Compare("hello", 0, 2, "world", 3, 10), System::ArgumentOutOfRangeException);
}

TEST(CompareInfoBatch27Test, IsPrefix_IsSuffix) {
    CompareInfo ci("en-US");
    EXPECT_TRUE(ci.IsPrefix("hello", "hel"));
    EXPECT_FALSE(ci.IsPrefix("hello", "world"));
    EXPECT_TRUE(ci.IsSuffix("hello", "llo"));
    EXPECT_FALSE(ci.IsSuffix("hello", "hel"));
}

TEST(CompareInfoBatch27Test, IsPrefix_IgnoreCase) {
    CompareInfo ci("en-US");
    EXPECT_TRUE(ci.IsPrefix("Hello", "HELL", CompareOptions::IgnoreCase));
}

TEST(CompareInfoBatch27Test, IndexOf_LastIndexOf) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.IndexOf("abcabc", "bc"), 1);
    EXPECT_EQ(ci.LastIndexOf("abcabc", "bc"), 4);
    EXPECT_EQ(ci.IndexOf("abcabc", "xyz"), -1);
}

TEST(CompareInfoBatch27Test, IndexOf_IgnoreCase) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.IndexOf("Hello World", "world", CompareOptions::IgnoreCase), 6);
}

TEST(CompareInfoBatch27Test, IsSortable) {
    EXPECT_TRUE(CompareInfo::IsSortable(u'A'));
    EXPECT_TRUE(CompareInfo::IsSortable("anything"));
}

TEST(CompareInfoBatch27Test, GetSortKey) {
    CompareInfo ci("en-US");
    auto sk = ci.GetSortKey("test");
    EXPECT_EQ(sk.getOriginalStringProperty(), "test");
}

TEST(CompareInfoBatch27Test, GetSortKey_IgnoreCase_ProducesEqualKeys) {
    // Sort keys must be consistent with Compare: strings equal under IgnoreCase
    // should produce equal sort keys, matching the .NET contract.
    CompareInfo ci("en-US");
    auto skLower = ci.GetSortKey("hello", CompareOptions::IgnoreCase);
    auto skUpper = ci.GetSortKey("HELLO", CompareOptions::IgnoreCase);
    EXPECT_EQ(SortKey::Compare(skLower, skUpper), 0);
}

TEST(CompareInfoBatch27Test, GetHashCode) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.GetHashCode("same", CompareOptions::None),
              ci.GetHashCode("same", CompareOptions::None));
}

TEST(CompareInfoBatch27Test, GetHashCode_IgnoreCase_MatchesAcrossCase) {
    CompareInfo ci("en-US");
    EXPECT_EQ(ci.GetHashCode("Hello", CompareOptions::IgnoreCase),
              ci.GetHashCode("hello", CompareOptions::IgnoreCase));
}

TEST(CompareInfoBatch27Test, EqualityAndToString) {
    CompareInfo a("en-US"), b("en-US"), c("fr-FR");
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_EQ(a.ToString(), "CompareInfo - en-US");
}

// ===========================================================================
// CultureInfo
// ===========================================================================

TEST(CultureInfoBatch27Test, DefaultCtor) {
    CultureInfo ci;
    EXPECT_EQ(ci.getNameProperty(), "");
    EXPECT_FALSE(ci.getIsNeutralCultureProperty());
    EXPECT_FALSE(ci.getIsReadOnlyProperty());
}

TEST(CultureInfoBatch27Test, StringCtor) {
    CultureInfo ci("de-DE");
    EXPECT_EQ(ci.getNameProperty(), "de-DE");
}

TEST(CultureInfoBatch27Test, IntCtor) {
    CultureInfo ci(1033);
    EXPECT_EQ(ci.getNameProperty(), "en-US");
}

// Real .NET's invariant culture has IsNeutralCulture == false (CultureData.cs:
// `invariant._bNeutral = false;`) -- it is neither a neutral culture (a language without a
// region, e.g. "en") nor a specific one; it's read-only, but not neutral.
TEST(CultureInfoBatch27Test, InvariantCulture) {
    const auto& inv = CultureInfo::getInvariantCultureProperty();
    EXPECT_EQ(inv.getNameProperty(), "");
    EXPECT_FALSE(inv.getIsNeutralCultureProperty());
    EXPECT_TRUE(inv.getIsReadOnlyProperty());
}

TEST(CultureInfoBatch27Test, CurrentCulture_ReturnsInvariant) {
    const auto& cur = CultureInfo::getCurrentCultureProperty();
    EXPECT_FALSE(cur.getIsNeutralCultureProperty());
}

TEST(CultureInfoBatch27Test, CurrentUICulture_ReturnsInvariant) {
    const auto& ui = CultureInfo::getCurrentUICultureProperty();
    EXPECT_FALSE(ui.getIsNeutralCultureProperty());
}

TEST(CultureInfoBatch27Test, SetCurrentCulture_ChangesGetter) {
    CultureInfo previous = CultureInfo::getCurrentCultureProperty();
    CultureInfo::setCurrentCultureProperty(CultureInfo("de-DE"));
    EXPECT_EQ(CultureInfo::getCurrentCultureProperty().getNameProperty(), "de-DE");
    CultureInfo::setCurrentCultureProperty(previous); // restore for other tests
}

TEST(CultureInfoBatch27Test, SetCurrentUICulture_ChangesGetter) {
    CultureInfo previous = CultureInfo::getCurrentUICultureProperty();
    CultureInfo::setCurrentUICultureProperty(CultureInfo("ja-JP"));
    EXPECT_EQ(CultureInfo::getCurrentUICultureProperty().getNameProperty(), "ja-JP");
    CultureInfo::setCurrentUICultureProperty(previous); // restore for other tests
}

// ---------------------------------------------------------------------------
// CultureInfo(int) validation (verified against CultureInfo.cs)
// ---------------------------------------------------------------------------

TEST(CultureInfoBatch27Test, IntCtor_Negative_Throws) {
    EXPECT_THROW(CultureInfo ci(-1), System::ArgumentOutOfRangeException);
}

TEST(CultureInfoBatch27Test, IntCtor_LocaleNeutral_Throws) {
    EXPECT_THROW(CultureInfo ci(0x0000), System::Globalization::CultureNotFoundException);
}

TEST(CultureInfoBatch27Test, IntCtor_LocaleUserDefault_Throws) {
    EXPECT_THROW(CultureInfo ci(0x0400), System::Globalization::CultureNotFoundException);
}

TEST(CultureInfoBatch27Test, IntCtor_LocaleSystemDefault_Throws) {
    EXPECT_THROW(CultureInfo ci(0x0800), System::Globalization::CultureNotFoundException);
}

TEST(CultureInfoBatch27Test, IntCtor_LocaleCustomDefault_Throws) {
    EXPECT_THROW(CultureInfo ci(0x0C00), System::Globalization::CultureNotFoundException);
}

TEST(CultureInfoBatch27Test, IntCtor_LocaleCustomUnspecified_Throws) {
    EXPECT_THROW(CultureInfo ci(0x1000), System::Globalization::CultureNotFoundException);
}

// ---------------------------------------------------------------------------
// EnglishName / NativeName / DisplayName / ISO names
// ---------------------------------------------------------------------------

TEST(CultureInfoBatch27Test, EnglishName_Invariant) {
    EXPECT_EQ(CultureInfo::getInvariantCultureProperty().getEnglishNameProperty(),
              "Invariant Language (Invariant Country)");
}

TEST(CultureInfoBatch27Test, EnglishName_EnUS) {
    CultureInfo ci(1033);
    EXPECT_EQ(ci.getEnglishNameProperty(), "English (United States)");
}

TEST(CultureInfoBatch27Test, EnglishName_UnknownCulture_FallsBackToName) {
    CultureInfo ci("de-DE");
    EXPECT_EQ(ci.getEnglishNameProperty(), "de-DE");
}

TEST(CultureInfoBatch27Test, NativeName_MatchesEnglishName) {
    CultureInfo ci(1033);
    EXPECT_EQ(ci.getNativeNameProperty(), ci.getEnglishNameProperty());
}

TEST(CultureInfoBatch27Test, DisplayName_MatchesEnglishName) {
    CultureInfo ci(1033);
    EXPECT_EQ(ci.getDisplayNameProperty(), ci.getEnglishNameProperty());
}

TEST(CultureInfoBatch27Test, TwoLetterISOLanguageName_Invariant) {
    EXPECT_EQ(CultureInfo::getInvariantCultureProperty().getTwoLetterISOLanguageNameProperty(), "iv");
}

TEST(CultureInfoBatch27Test, TwoLetterISOLanguageName_EnUS) {
    CultureInfo ci(1033);
    EXPECT_EQ(ci.getTwoLetterISOLanguageNameProperty(), "en");
}

TEST(CultureInfoBatch27Test, TwoLetterISOLanguageName_DerivedFromSubtag) {
    CultureInfo ci("de-DE");
    EXPECT_EQ(ci.getTwoLetterISOLanguageNameProperty(), "de");
}

TEST(CultureInfoBatch27Test, ThreeLetterISOLanguageName_Invariant) {
    EXPECT_EQ(CultureInfo::getInvariantCultureProperty().getThreeLetterISOLanguageNameProperty(), "ivl");
}

TEST(CultureInfoBatch27Test, ThreeLetterISOLanguageName_EnUS) {
    CultureInfo ci(1033);
    EXPECT_EQ(ci.getThreeLetterISOLanguageNameProperty(), "eng");
}

TEST(CultureInfoBatch27Test, ThreeLetterISOLanguageName_UnknownCulture_IsEmpty) {
    CultureInfo ci("de-DE");
    EXPECT_EQ(ci.getThreeLetterISOLanguageNameProperty(), "");
}

// ---------------------------------------------------------------------------
// NumberFormat / DateTimeFormat wiring
// ---------------------------------------------------------------------------

TEST(CultureInfoBatch27Test, NumberFormat_DefaultsToInvariantContent) {
    CultureInfo ci("de-DE");
    EXPECT_EQ(ci.getNumberFormatProperty().getNumberDecimalSeparatorProperty(), ".");
}

TEST(CultureInfoBatch27Test, NumberFormat_Settable) {
    CultureInfo ci("de-DE");
    System::Globalization::NumberFormatInfo nfi;
    nfi.setNumberDecimalSeparatorProperty(",");
    ci.setNumberFormatProperty(nfi);
    EXPECT_EQ(ci.getNumberFormatProperty().getNumberDecimalSeparatorProperty(), ",");
}

TEST(CultureInfoBatch27Test, NumberFormat_SetOnReadOnly_Throws) {
    CultureInfo ci = CultureInfo::getInvariantCultureProperty();
    System::Globalization::NumberFormatInfo nfi;
    EXPECT_THROW(ci.setNumberFormatProperty(nfi), System::InvalidOperationException);
}

TEST(CultureInfoBatch27Test, DateTimeFormat_Settable) {
    CultureInfo ci("de-DE");
    System::Globalization::DateTimeFormatInfo dtfi;
    EXPECT_NO_THROW(ci.setDateTimeFormatProperty(dtfi));
}

TEST(CultureInfoBatch27Test, DateTimeFormat_SetOnReadOnly_Throws) {
    CultureInfo ci = CultureInfo::getInvariantCultureProperty();
    System::Globalization::DateTimeFormatInfo dtfi;
    EXPECT_THROW(ci.setDateTimeFormatProperty(dtfi), System::InvalidOperationException);
}

// ---------------------------------------------------------------------------
// Equals / GetHashCode / ToString
// ---------------------------------------------------------------------------

TEST(CultureInfoBatch27Test, Equals_SameName_True) {
    CultureInfo a("en-US");
    CultureInfo b("en-US");
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(CultureInfoBatch27Test, Equals_DifferentName_False) {
    CultureInfo a("en-US");
    CultureInfo b("de-DE");
    EXPECT_FALSE(a.Equals(b));
    EXPECT_FALSE(a == b);
}

TEST(CultureInfoBatch27Test, GetHashCode_MatchesForEqualNames) {
    CultureInfo a("en-US");
    CultureInfo b("en-US");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(CultureInfoBatch27Test, ToString_ReturnsName) {
    CultureInfo ci("fr-FR");
    EXPECT_EQ(ci.ToString(), "fr-FR");
}

// ---------------------------------------------------------------------------
// GetCultureInfo overloads
// ---------------------------------------------------------------------------

TEST(CultureInfoBatch27Test, GetCultureInfo_ByName_IsReadOnly) {
    CultureInfo ci = CultureInfo::GetCultureInfo("de-DE");
    EXPECT_EQ(ci.getNameProperty(), "de-DE");
    EXPECT_TRUE(ci.getIsReadOnlyProperty());
}

TEST(CultureInfoBatch27Test, GetCultureInfo_ByLcid_IsReadOnly) {
    CultureInfo ci = CultureInfo::GetCultureInfo(1033);
    EXPECT_EQ(ci.getNameProperty(), "en-US");
    EXPECT_TRUE(ci.getIsReadOnlyProperty());
}

TEST(CultureInfoBatch27Test, GetCultureInfo_ByLcid_InvalidLcid_Throws) {
    EXPECT_THROW(CultureInfo::GetCultureInfo(0x0000), System::Globalization::CultureNotFoundException);
}

TEST(CultureInfoBatch27Test, GetCultureInfo_ByNamePredefinedOnly_IsReadOnly) {
    CultureInfo ci = CultureInfo::GetCultureInfo("ja-JP", true);
    EXPECT_EQ(ci.getNameProperty(), "ja-JP");
    EXPECT_TRUE(ci.getIsReadOnlyProperty());
}

// ===========================================================================
// CultureNotFoundException
// ===========================================================================

TEST(CultureNotFoundExceptionBatch27Test, DefaultCtor) {
    CultureNotFoundException ex;
    EXPECT_NE(std::string(ex.what()).find("not supported"), std::string::npos);
    EXPECT_EQ(ex.getInvalidCultureNameProperty(), "");
    EXPECT_EQ(ex.getInvalidCultureIdProperty(), -1);
}

TEST(CultureNotFoundExceptionBatch27Test, MessageCtor) {
    CultureNotFoundException ex("bad culture");
    EXPECT_NE(std::string(ex.what()).find("bad culture"), std::string::npos);
}

TEST(CultureNotFoundExceptionBatch27Test, ParamNameAndMessageCtor) {
    CultureNotFoundException ex("cultureName", "not found");
    EXPECT_EQ(ex.getInvalidCultureNameProperty(), "");
}

TEST(CultureNotFoundExceptionBatch27Test, ParamNameAndInvalidCultureNameAndMessageCtor) {
    CultureNotFoundException ex("cultureName", "xx-XX", "not found");
    EXPECT_EQ(ex.getInvalidCultureNameProperty(), "xx-XX");
}

TEST(CultureNotFoundExceptionBatch27Test, MessageAndInvalidCultureNameAndInnerCtor) {
    CultureNotFoundException ex("not found", "xx-XX", std::exception_ptr{});
    EXPECT_EQ(ex.getInvalidCultureNameProperty(), "xx-XX");
}

TEST(CultureNotFoundExceptionBatch27Test, MessageAndIdAndInnerCtor) {
    CultureNotFoundException ex("bad id", 9999, std::exception_ptr{});
    EXPECT_EQ(ex.getInvalidCultureIdProperty(), 9999);
}

TEST(CultureNotFoundExceptionBatch27Test, ParamNameAndIdAndMessageCtor) {
    CultureNotFoundException ex("param", 1234, "culture error");
    EXPECT_EQ(ex.getInvalidCultureIdProperty(), 1234);
}

// Real .NET's CultureNotFoundException.Message override always appends
// "{value} is an invalid culture identifier." on a new line whenever an invalid culture
// name/ID was supplied -- verify the C++ port's message composition matches exactly.

TEST(CultureNotFoundExceptionBatch27Test, MessageIncludesInvalidCultureNameSuffix_WithParamName) {
    CultureNotFoundException ex("cultureName", "xx-XX", "not found");
    EXPECT_EQ(ex.getMessageProperty(),
              "not found (Parameter 'cultureName')\nxx-XX is an invalid culture identifier.");
}

TEST(CultureNotFoundExceptionBatch27Test, MessageIncludesInvalidCultureNameSuffix_NoParamName) {
    CultureNotFoundException ex("not found", "xx-XX", std::exception_ptr{});
    EXPECT_EQ(ex.getMessageProperty(), "not found\nxx-XX is an invalid culture identifier.");
}

TEST(CultureNotFoundExceptionBatch27Test, MessageIncludesFormattedInvalidCultureId_NoParamName) {
    CultureNotFoundException ex("bad id", 9999, std::exception_ptr{});
    EXPECT_EQ(ex.getMessageProperty(), "bad id\n9999 (0x270f) is an invalid culture identifier.");
}

TEST(CultureNotFoundExceptionBatch27Test, MessageIncludesFormattedInvalidCultureId_WithParamName) {
    CultureNotFoundException ex("param", 1234, "culture error");
    EXPECT_EQ(ex.getMessageProperty(),
              "culture error (Parameter 'param')\n1234 (0x04d2) is an invalid culture identifier.");
}

TEST(CultureNotFoundExceptionBatch27Test, MessageUnaffectedWhenNoInvalidCultureInfo) {
    CultureNotFoundException ex("cultureName", "not found");
    EXPECT_EQ(ex.getMessageProperty(), "not found (Parameter 'cultureName')");
}
