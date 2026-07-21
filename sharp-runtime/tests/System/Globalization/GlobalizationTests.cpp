// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Note: Calendar.hpp, GregorianCalendar.hpp, and ISOWeek.hpp are tested in tests/CalendarTests.cpp.
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Globalization/CultureInfo.hpp"
#include "System/Globalization/NumberFormatInfo.hpp"
#include "System/Globalization/RegionInfo.hpp"
#include "System/Globalization/StringInfo.hpp"
#include "System/Globalization/UnicodeCategory.hpp"

using System::Globalization::CultureInfo;
using System::Globalization::NumberFormatInfo;
using System::Globalization::RegionInfo;
using System::Globalization::StringInfo;
using System::Globalization::UnicodeCategory;

// ===========================================================================
// CultureInfo
// ===========================================================================

TEST(CultureInfoTests, InvariantCulture_NameIsEmpty) {
    EXPECT_EQ(CultureInfo::getInvariantCultureProperty().getNameProperty(), "");
}

// Real .NET's invariant culture has IsNeutralCulture == false (CultureData.cs:
// `invariant._bNeutral = false;`).
TEST(CultureInfoTests, InvariantCulture_IsNotNeutral) {
    EXPECT_FALSE(CultureInfo::getInvariantCultureProperty().getIsNeutralCultureProperty());
}

TEST(CultureInfoTests, InvariantCulture_IsReadOnly) {
    EXPECT_TRUE(CultureInfo::getInvariantCultureProperty().getIsReadOnlyProperty());
}

TEST(CultureInfoTests, CurrentCulture_DefaultsToInvariantByValue) {
    // CurrentCulture is independently settable (matching .NET's mutable CurrentCulture
    // property), so it is a distinct object from InvariantCulture, not the same instance —
    // but its default value matches InvariantCulture until overridden.
    const auto& current = CultureInfo::getCurrentCultureProperty();
    const auto& invariant = CultureInfo::getInvariantCultureProperty();
    EXPECT_EQ(current.getNameProperty(), invariant.getNameProperty());
    EXPECT_EQ(current.getIsNeutralCultureProperty(), invariant.getIsNeutralCultureProperty());
}

TEST(CultureInfoTests, DefaultConstructor_NameIsEmpty) {
    CultureInfo ci;
    EXPECT_EQ(ci.getNameProperty(), "");
}

TEST(CultureInfoTests, NamedConstructor_StoresName) {
    CultureInfo ci("en-US");
    EXPECT_EQ(ci.getNameProperty(), "en-US");
}

TEST(CultureInfoTests, NamedConstructor_IsNotNeutral) {
    CultureInfo ci("en-US");
    EXPECT_FALSE(ci.getIsNeutralCultureProperty());
}

TEST(CultureInfoTests, NamedConstructor_IsNotReadOnly) {
    CultureInfo ci("en-US");
    EXPECT_FALSE(ci.getIsReadOnlyProperty());
}

TEST(CultureInfoTests, InvariantCulture_ReturnsSameInstanceEveryCall) {
    const CultureInfo& a = CultureInfo::getInvariantCultureProperty();
    const CultureInfo& b = CultureInfo::getInvariantCultureProperty();
    EXPECT_EQ(&a, &b);
}

TEST(CultureInfoTests, NamedConstructor_DifferentNames) {
    CultureInfo en("en");
    CultureInfo de("de");
    EXPECT_NE(en.getNameProperty(), de.getNameProperty());
}

// ===========================================================================
// NumberFormatInfo
// ===========================================================================

TEST(NumberFormatInfoTests, InvariantInfo_IsReadOnly) {
    EXPECT_TRUE(NumberFormatInfo::getInvariantInfoProperty().getIsReadOnlyProperty());
}

TEST(NumberFormatInfoTests, InvariantInfo_DecimalSeparatorIsDot) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getNumberDecimalSeparatorProperty(), ".");
}

TEST(NumberFormatInfoTests, InvariantInfo_GroupSeparatorIsComma) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getNumberGroupSeparatorProperty(), ",");
}

TEST(NumberFormatInfoTests, InvariantInfo_CurrencyDecimalSeparatorIsDot) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getCurrencyDecimalSeparatorProperty(), ".");
}

TEST(NumberFormatInfoTests, InvariantInfo_CurrencyGroupSeparatorIsComma) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getCurrencyGroupSeparatorProperty(), ",");
}

TEST(NumberFormatInfoTests, InvariantInfo_CurrencySymbolIsCurrencySign) {
    // Regression: was previously (wrongly) "$". .NET's real invariant default is U+00A4
    // (CURRENCY SIGN), verified against NumberFormatInfo.cs's field initializer
    // (_currencySymbol = "\x00a4") rather than that file's own stale prose doc-comment
    // table, which incorrectly says "$".
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getCurrencySymbolProperty(), "¤");
}

TEST(NumberFormatInfoTests, InvariantInfo_NegativeSignIsMinus) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getNegativeSignProperty(), "-");
}

TEST(NumberFormatInfoTests, InvariantInfo_PositiveSignIsPlus) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getPositiveSignProperty(), "+");
}

TEST(NumberFormatInfoTests, InvariantInfo_PercentSymbol) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getPercentSymbolProperty(), "%");
}

TEST(NumberFormatInfoTests, InvariantInfo_NaNSymbol) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getNaNSymbolProperty(), "NaN");
}

TEST(NumberFormatInfoTests, InvariantInfo_PositiveInfinitySymbol) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getPositiveInfinitySymbolProperty(), "Infinity");
}

TEST(NumberFormatInfoTests, InvariantInfo_NegativeInfinitySymbol) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getNegativeInfinitySymbolProperty(), "-Infinity");
}

TEST(NumberFormatInfoTests, InvariantInfo_DecimalDigitsIsTwo) {
    EXPECT_EQ(NumberFormatInfo::getInvariantInfoProperty().getNumberDecimalDigitsProperty(), 2);
}

TEST(NumberFormatInfoTests, CurrentInfo_SameInstanceAsInvariant) {
    EXPECT_EQ(&NumberFormatInfo::getCurrentInfoProperty(), &NumberFormatInfo::getInvariantInfoProperty());
}

TEST(NumberFormatInfoTests, InvariantInfo_ReturnsSameInstanceEveryCall) {
    const NumberFormatInfo& a = NumberFormatInfo::getInvariantInfoProperty();
    const NumberFormatInfo& b = NumberFormatInfo::getInvariantInfoProperty();
    EXPECT_EQ(&a, &b);
}

TEST(NumberFormatInfoTests, DefaultInstance_NotReadOnly) {
    NumberFormatInfo nfi;
    EXPECT_FALSE(nfi.getIsReadOnlyProperty());
}

TEST(NumberFormatInfoTests, MutableInstance_FieldsModifiable) {
    NumberFormatInfo nfi;
    nfi.setNumberDecimalSeparatorProperty(",");
    nfi.setNumberGroupSeparatorProperty(".");
    EXPECT_EQ(nfi.getNumberDecimalSeparatorProperty(), ",");
    EXPECT_EQ(nfi.getNumberGroupSeparatorProperty(), ".");
}

TEST(NumberFormatInfoTests, MutableInstance_CurrencySymbolModifiable) {
    NumberFormatInfo nfi;
    nfi.setCurrencySymbolProperty("€");
    EXPECT_EQ(nfi.getCurrencySymbolProperty(), "€");
}

TEST(NumberFormatInfoTests, SetOnReadOnlyInstance_Throws) {
    NumberFormatInfo ro = NumberFormatInfo::ReadOnly(NumberFormatInfo());
    EXPECT_THROW(ro.setCurrencySymbolProperty("€"), System::InvalidOperationException);
}

// ===========================================================================
// RegionInfo
// ===========================================================================

TEST(RegionInfoTests, Constructor_StoresName) {
    RegionInfo r("US");
    EXPECT_EQ(r.getNameProperty(), "US");
}

TEST(RegionInfoTests, TwoLetterISORegionName_TwoChars) {
    RegionInfo r("US");
    EXPECT_EQ(r.getTwoLetterISORegionNameProperty().size(), 2u);
    EXPECT_EQ(r.getTwoLetterISORegionNameProperty(), "US");
}

TEST(RegionInfoTests, ThreeLetterISORegionName_ThreeChars) {
    RegionInfo r("USA");
    EXPECT_EQ(r.getThreeLetterISORegionNameProperty().size(), 3u);
    EXPECT_EQ(r.getThreeLetterISORegionNameProperty(), "USA");
}

TEST(RegionInfoTests, CurrencySymbol_Default) {
    RegionInfo r("US");
    EXPECT_EQ(r.getCurrencySymbolProperty(), "$");
}

TEST(RegionInfoTests, ISOCurrencySymbol_Default) {
    RegionInfo r("US");
    EXPECT_EQ(r.getISOCurrencySymbolProperty(), "USD");
}

// The US uses the customary (non-metric) measurement system; real .NET's
// RegionInfo("US").IsMetric is false (RegionInfo.cs).
TEST(RegionInfoTests, IsMetric_US_IsFalse) {
    RegionInfo r("US");
    EXPECT_FALSE(r.getIsMetricProperty());
}

TEST(RegionInfoTests, CurrentRegion_NameIsUS) {
    EXPECT_EQ(RegionInfo::getCurrentRegionProperty().getNameProperty(), "US");
}

TEST(RegionInfoTests, EnglishName_MatchesConstructorArg) {
    RegionInfo r("Germany");
    EXPECT_EQ(r.getEnglishNameProperty(), "Germany");
}

TEST(RegionInfoTests, NativeName_MatchesConstructorArg) {
    RegionInfo r("France");
    EXPECT_EQ(r.getNativeNameProperty(), "France");
}

TEST(RegionInfoTests, CurrentRegion_ReturnsSameInstanceEveryCall) {
    const RegionInfo& a = RegionInfo::getCurrentRegionProperty();
    const RegionInfo& b = RegionInfo::getCurrentRegionProperty();
    EXPECT_EQ(&a, &b);
}

// ===========================================================================
// StringInfo
// ===========================================================================

TEST(StringInfoTests, DefaultConstructor_EmptyString) {
    StringInfo si;
    EXPECT_EQ(si.getStringProperty(), "");
}

TEST(StringInfoTests, Constructor_StoresString) {
    StringInfo si("hello");
    EXPECT_EQ(si.getStringProperty(), "hello");
}

TEST(StringInfoTests, SetString_UpdatesValue) {
    StringInfo si("abc");
    si.setStringProperty("xyz");
    EXPECT_EQ(si.getStringProperty(), "xyz");
}

TEST(StringInfoTests, LengthInTextElements_AsciiString) {
    StringInfo si("hello");
    EXPECT_EQ(si.getLengthInTextElementsProperty(), 5);
}

TEST(StringInfoTests, LengthInTextElements_EmptyString) {
    StringInfo si("");
    EXPECT_EQ(si.getLengthInTextElementsProperty(), 0);
}

TEST(StringInfoTests, LengthInTextElements_SingleChar) {
    StringInfo si("X");
    EXPECT_EQ(si.getLengthInTextElementsProperty(), 1);
}

TEST(StringInfoTests, SubstringByTextElements_StartAndLength) {
    StringInfo si("abcdef");
    EXPECT_EQ(si.SubstringByTextElements(0, 3), "abc");
}

TEST(StringInfoTests, SubstringByTextElements_FromMiddle) {
    StringInfo si("abcdef");
    EXPECT_EQ(si.SubstringByTextElements(2, 2), "cd");
}

TEST(StringInfoTests, SubstringByTextElements_ToEnd) {
    StringInfo si("abcdef");
    EXPECT_EQ(si.SubstringByTextElements(3), "def");
}

TEST(StringInfoTests, SubstringByTextElements_FromStart_ToEnd) {
    StringInfo si("hello");
    EXPECT_EQ(si.SubstringByTextElements(0), "hello");
}

TEST(StringInfoTests, GetNextTextElement_DefaultIndex_FirstChar) {
    EXPECT_EQ(StringInfo::GetNextTextElement("hello"), "h");
}

TEST(StringInfoTests, GetNextTextElement_AtOffset) {
    EXPECT_EQ(StringInfo::GetNextTextElement("hello", 2), "l");
}

TEST(StringInfoTests, GetNextTextElement_LastChar) {
    EXPECT_EQ(StringInfo::GetNextTextElement("hello", 4), "o");
}

TEST(StringInfoTests, GetNextTextElement_AtEnd_ReturnsEmpty) {
    EXPECT_EQ(StringInfo::GetNextTextElement("hi", 2), ""); // exactly at end: valid, empty
}

// .NET's real bound is `(uint)index > (uint)str.Length` (StringInfo.cs): an index strictly
// past the end throws ArgumentOutOfRangeException rather than silently returning ""/0.
TEST(StringInfoTests, GetNextTextElement_PastEnd_Throws) {
    EXPECT_THROW(StringInfo::GetNextTextElement("hi", 5), System::ArgumentOutOfRangeException);
}

TEST(StringInfoTests, GetNextTextElementLength_AsciiIsOne) {
    EXPECT_EQ(StringInfo::GetNextTextElementLength("hello", 0), 1);
}

TEST(StringInfoTests, GetNextTextElementLength_AtEnd_IsZero) {
    EXPECT_EQ(StringInfo::GetNextTextElementLength("hi", 2), 0); // exactly at end: valid, zero
}

TEST(StringInfoTests, GetNextTextElementLength_PastEnd_Throws) {
    EXPECT_THROW(StringInfo::GetNextTextElementLength("hi", 10), System::ArgumentOutOfRangeException);
}

// Non-ASCII UTF-8 consistency: "e with acute accent" is the 2-byte UTF-8 sequence 0xC3 0xA9.
// GetNextTextElement/GetNextTextElementLength/ParseCombiningCharacters/LengthInTextElements must
// all keep the 2-byte sequence together (matching GetTextElementEnumerator, which already did),
// not split it into two 1-byte fragments.
TEST(StringInfoTests, GetNextTextElement_MultiByteUtf8_KeepsFullCharacter) {
    std::string eAcute = "\xC3\xA9"; // U+00E9
    EXPECT_EQ(StringInfo::GetNextTextElement(eAcute, 0), eAcute);
}

TEST(StringInfoTests, GetNextTextElement_MultiByteUtf8_ConsistentWithEnumerator) {
    std::string s = "a\xC3\xA9z"; // 'a', e-acute (2 bytes), 'z'
    auto direct = StringInfo::GetNextTextElement(s, 1);
    auto en = StringInfo::GetTextElementEnumerator(s);
    ASSERT_TRUE(en.MoveNext()); // 'a'
    ASSERT_TRUE(en.MoveNext()); // e-acute
    EXPECT_EQ(direct, en.GetTextElement());
    EXPECT_EQ(direct, "\xC3\xA9");
}

TEST(StringInfoTests, GetNextTextElementLength_MultiByteUtf8_IsTwo) {
    std::string eAcute = "\xC3\xA9";
    EXPECT_EQ(StringInfo::GetNextTextElementLength(eAcute, 0), 2);
}

TEST(StringInfoTests, ParseCombiningCharacters_MultiByteUtf8_OneEntryPerCharacter) {
    std::string s = "a\xC3\xA9z"; // 'a'(1 byte) + e-acute(2 bytes) + 'z'(1 byte) = 3 elements
    auto v = StringInfo::ParseCombiningCharacters(s);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 0); // 'a' starts at byte 0
    EXPECT_EQ(v[1], 1); // e-acute starts at byte 1 (not split into two entries)
    EXPECT_EQ(v[2], 3); // 'z' starts at byte 3
}

TEST(StringInfoTests, LengthInTextElements_MultiByteUtf8_CountsCharactersNotBytes) {
    StringInfo si("a\xC3\xA9z"); // 4 bytes, 3 text elements
    EXPECT_EQ(si.getLengthInTextElementsProperty(), 3);
}

TEST(StringInfoTests, ParseCombiningCharacters_SplitsEachByte) {
    auto v = StringInfo::ParseCombiningCharacters("abc");
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 0); // starting index of 'a'
    EXPECT_EQ(v[1], 1); // starting index of 'b'
    EXPECT_EQ(v[2], 2); // starting index of 'c'
}

TEST(StringInfoTests, ParseCombiningCharacters_EmptyString_EmptyResult) {
    auto v = StringInfo::ParseCombiningCharacters("");
    EXPECT_TRUE(v.empty());
}

TEST(StringInfoTests, ParseCombiningCharacters_SingleChar) {
    auto v = StringInfo::ParseCombiningCharacters("Z");
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 0); // starting index of 'Z'
}

// ===========================================================================
// UnicodeCategory
// ===========================================================================

TEST(UnicodeCategoryTests, UppercaseLetter_ValueIsZero) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::UppercaseLetter), 0);
}

TEST(UnicodeCategoryTests, LowercaseLetter_ValueIsOne) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::LowercaseLetter), 1);
}

TEST(UnicodeCategoryTests, TitlecaseLetter_ValueIsTwo) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::TitlecaseLetter), 2);
}

TEST(UnicodeCategoryTests, DecimalDigitNumber_ValueIsEight) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::DecimalDigitNumber), 8);
}

TEST(UnicodeCategoryTests, SpaceSeparator_ValueIs11) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::SpaceSeparator), 11);
}

TEST(UnicodeCategoryTests, Control_ValueIs14) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::Control), 14);
}

TEST(UnicodeCategoryTests, MathSymbol_ValueIs25) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::MathSymbol), 25);
}

TEST(UnicodeCategoryTests, CurrencySymbol_ValueIs26) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::CurrencySymbol), 26);
}

TEST(UnicodeCategoryTests, OtherNotAssigned_ValueIs29) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::OtherNotAssigned), 29);
}

TEST(UnicodeCategoryTests, ConnectorPunctuation_ValueIs18) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::ConnectorPunctuation), 18);
}

TEST(UnicodeCategoryTests, DashPunctuation_ValueIs19) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::DashPunctuation), 19);
}

TEST(UnicodeCategoryTests, OtherPunctuation_ValueIs24) {
    EXPECT_EQ(static_cast<int>(UnicodeCategory::OtherPunctuation), 24);
}
