// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 33:
//   StringInfo:            constructors, getters, SubstringByTextElements, static helpers
//   TaiwanCalendar:        era constant, GetEra, GetErasCount, GetYear
//   TextElementEnumerator: MoveNext/GetTextElement/Reset, ASCII and UTF-8 multi-byte
//   TextInfo:              ToLower/ToUpper/ToTitleCase, Clone, ReadOnly, operator==
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Globalization/StringInfo.hpp"
#include "System/Globalization/TaiwanCalendar.hpp"
#include "System/Globalization/TextElementEnumerator.hpp"
#include "System/Globalization/TextInfo.hpp"
#include "System/DateTime.hpp"

using System::Globalization::StringInfo;
using System::Globalization::TaiwanCalendar;
using System::Globalization::TextElementEnumerator;
using System::Globalization::TextInfo;

// ===========================================================================
// StringInfo
// ===========================================================================

TEST(StringInfoBatch33Test, DefaultCtor) {
    StringInfo si;
    EXPECT_EQ(si.getStringProperty(), "");
    EXPECT_EQ(si.getLengthInTextElementsProperty(), 0);
}

TEST(StringInfoBatch33Test, StringCtor) {
    StringInfo si("hello");
    EXPECT_EQ(si.getStringProperty(), "hello");
    EXPECT_EQ(si.getLengthInTextElementsProperty(), 5);
}

TEST(StringInfoBatch33Test, SetString) {
    StringInfo si;
    si.setStringProperty("world");
    EXPECT_EQ(si.getStringProperty(), "world");
}

TEST(StringInfoBatch33Test, SubstringByTextElements_StartOnly) {
    StringInfo si("hello");
    EXPECT_EQ(si.SubstringByTextElements(2), "llo");
}

TEST(StringInfoBatch33Test, SubstringByTextElements_StartAndLength) {
    StringInfo si("hello");
    EXPECT_EQ(si.SubstringByTextElements(1, 3), "ell");
}

TEST(StringInfoBatch33Test, SubstringByTextElements_StartOutOfRange_Throws) {
    StringInfo si("hello");
    EXPECT_THROW(si.SubstringByTextElements(5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(si.SubstringByTextElements(-1), System::ArgumentOutOfRangeException);
}

TEST(StringInfoBatch33Test, SubstringByTextElements_LengthOutOfRange_Throws) {
    StringInfo si("hello");
    EXPECT_THROW(si.SubstringByTextElements(0, 6), System::ArgumentOutOfRangeException);
    EXPECT_THROW(si.SubstringByTextElements(0, -1), System::ArgumentOutOfRangeException);
}

TEST(StringInfoBatch33Test, GetTextElementEnumerator_InvalidIndex_ThrowsArgumentOutOfRange) {
    EXPECT_THROW(StringInfo::GetTextElementEnumerator("hello", -1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(StringInfo::GetTextElementEnumerator("hello", 6), System::ArgumentOutOfRangeException);
}

TEST(StringInfoBatch33Test, GetNextTextElement) {
    EXPECT_EQ(StringInfo::GetNextTextElement("abc", 0), "a");
    EXPECT_EQ(StringInfo::GetNextTextElement("abc", 2), "c");
    EXPECT_EQ(StringInfo::GetNextTextElement("abc", 3), ""); // exactly at end: valid, empty
}

// .NET's real bound is `(uint)index > (uint)str.Length` (StringInfo.cs) -- index at or
// negative from the end is one thing (valid, or negative-throws), but an index strictly
// past the end throws ArgumentOutOfRangeException; it does not silently return "".
TEST(StringInfoBatch33Test, GetNextTextElement_PastEnd_Throws) {
    EXPECT_THROW(StringInfo::GetNextTextElement("abc", 5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(StringInfo::GetNextTextElement("abc", -1), System::ArgumentOutOfRangeException);
}

TEST(StringInfoBatch33Test, GetNextTextElementLength) {
    EXPECT_EQ(StringInfo::GetNextTextElementLength("abc", 0), 1);
    EXPECT_EQ(StringInfo::GetNextTextElementLength("abc", 3), 0); // exactly at end: valid, zero
}

TEST(StringInfoBatch33Test, GetNextTextElementLength_PastEnd_Throws) {
    EXPECT_THROW(StringInfo::GetNextTextElementLength("abc", 9), System::ArgumentOutOfRangeException);
    EXPECT_THROW(StringInfo::GetNextTextElementLength("abc", -1), System::ArgumentOutOfRangeException);
}

TEST(StringInfoBatch33Test, ParseCombiningCharacters) {
    auto v = StringInfo::ParseCombiningCharacters("hi");
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 0); // starting byte index of 'h'
    EXPECT_EQ(v[1], 1); // starting byte index of 'i'
}

// ===========================================================================
// TaiwanCalendar
// ===========================================================================

TEST(TaiwanCalendarBatch33Test, EraConstant) {
    EXPECT_EQ(TaiwanCalendar::TaiwanEra,      1);
    EXPECT_EQ(TaiwanCalendar::GregorianOffset, 1911);
}

TEST(TaiwanCalendarBatch33Test, GetEra) {
    TaiwanCalendar tc;
    EXPECT_EQ(tc.GetEra(System::DateTime(2024, 1, 1)), TaiwanCalendar::TaiwanEra);
}

TEST(TaiwanCalendarBatch33Test, GetErasCount) {
    TaiwanCalendar tc;
    EXPECT_EQ(tc.GetErasCount(), 1);
}

TEST(TaiwanCalendarBatch33Test, Eras_ContainsTaiwanEra) {
    TaiwanCalendar tc;
    auto eras = tc.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], TaiwanCalendar::TaiwanEra);
}

TEST(TaiwanCalendarBatch33Test, GetYear_2024_Is113) {
    TaiwanCalendar tc;
    EXPECT_EQ(tc.GetYear(System::DateTime(2024, 6, 1)), 113);
}

TEST(TaiwanCalendarBatch33Test, GetYear_1912_Is1) {
    TaiwanCalendar tc;
    EXPECT_EQ(tc.GetYear(System::DateTime(1912, 1, 1)), 1);
}

// ===========================================================================
// TextElementEnumerator
// ===========================================================================

TEST(TextElementEnumeratorBatch33Test, ASCIIString) {
    TextElementEnumerator e("hi");
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.GetTextElement(), "h");
    EXPECT_EQ(e.getElementIndexProperty(), 0);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.GetTextElement(), "i");
    EXPECT_FALSE(e.MoveNext());
}

TEST(TextElementEnumeratorBatch33Test, EmptyString) {
    TextElementEnumerator e("");
    EXPECT_FALSE(e.MoveNext());
}

TEST(TextElementEnumeratorBatch33Test, BeforeFirstMoveNext_Throws) {
    TextElementEnumerator e("x");
    EXPECT_THROW(e.GetTextElement(), System::InvalidOperationException);
}

TEST(TextElementEnumeratorBatch33Test, AfterExhausted_Throws) {
    TextElementEnumerator e("x");
    EXPECT_TRUE(e.MoveNext());
    EXPECT_FALSE(e.MoveNext());
    EXPECT_THROW(e.GetTextElement(), System::InvalidOperationException);
    EXPECT_THROW(e.getElementIndexProperty(), System::InvalidOperationException);
}

TEST(TextElementEnumeratorBatch33Test, Reset) {
    TextElementEnumerator e("ab");
    e.MoveNext();
    e.Reset();
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.GetTextElement(), "a");
}

TEST(TextElementEnumeratorBatch33Test, UTF8TwoByteSequence) {
    // "ü" in UTF-8 is 2 bytes: 0xC3 0xBC
    std::string s = "\xC3\xBC";
    TextElementEnumerator e(s);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.GetTextElement(), s);
    EXPECT_FALSE(e.MoveNext());
}

TEST(TextElementEnumeratorBatch33Test, GetCurrent_SameAsGetTextElement) {
    TextElementEnumerator e("z");
    e.MoveNext();
    EXPECT_EQ(e.getCurrentProperty(), e.GetTextElement());
}

// ===========================================================================
// TextInfo
// ===========================================================================

TEST(TextInfoBatch33Test, DefaultCultureName) {
    TextInfo ti;
    EXPECT_EQ(ti.getCultureNameProperty(), "en-US");
}

TEST(TextInfoBatch33Test, CustomCultureName) {
    TextInfo ti("fr-FR");
    EXPECT_EQ(ti.getCultureNameProperty(), "fr-FR");
}

TEST(TextInfoBatch33Test, DefaultNotReadOnly) {
    TextInfo ti;
    EXPECT_FALSE(ti.getIsReadOnlyProperty());
}

TEST(TextInfoBatch33Test, IsRightToLeft_AlwaysFalse) {
    TextInfo ti;
    EXPECT_FALSE(ti.getIsRightToLeftProperty());
}

TEST(TextInfoBatch33Test, ListSeparator) {
    TextInfo ti;
    EXPECT_EQ(ti.getListSeparatorProperty(), ",");
    ti.setListSeparatorProperty(";");
    EXPECT_EQ(ti.getListSeparatorProperty(), ";");
}

TEST(TextInfoBatch33Test, ToLower_String) {
    TextInfo ti;
    EXPECT_EQ(ti.ToLower(std::string("HELLO")), "hello");
}

TEST(TextInfoBatch33Test, ToUpper_String) {
    TextInfo ti;
    EXPECT_EQ(ti.ToUpper(std::string("hello")), "HELLO");
}

TEST(TextInfoBatch33Test, ToTitleCase) {
    TextInfo ti;
    EXPECT_EQ(ti.ToTitleCase("hello world"), "Hello World");
}

// .NET preserves all-uppercase words (acronyms) unchanged instead of lowercasing them --
// TextInfo.cs's own comment: "Use hasLowerCase flag to prevent from lowercasing acronyms
// (like URT, USA, etc). This is in line with Word 2000 behavior of titlecasing."
TEST(TextInfoBatch33Test, ToTitleCase_PreservesAllUppercaseAcronym) {
    TextInfo ti;
    EXPECT_EQ(ti.ToTitleCase("USA"), "USA");
    EXPECT_EQ(ti.ToTitleCase("visit the USA today"), "Visit The USA Today");
}

// A word whose first letter is lowercase is still normally title-cased even if the rest
// happens to be uppercase, since .NET's hasLowerCase check covers the first letter too.
TEST(TextInfoBatch33Test, ToTitleCase_LowercaseFirstLetter_NotTreatedAsAcronym) {
    TextInfo ti;
    EXPECT_EQ(ti.ToTitleCase("uSA"), "Usa");
}

// Regression tests for ticket 319: ToTitleCase previously split words only on whitespace, but
// real .NET's word-boundary detection (TextInfo.cs's c_wordSeparatorMask) treats most
// punctuation categories (dashes, parens, quotes, other punctuation/symbols) as word
// separators too -- so a hyphenated word like "mary-jane" was incorrectly treated as one word
// ("Mary-jane") instead of two ("Mary-Jane"). The apostrophe is a documented exception (real
// .NET gives it bespoke handling this port doesn't replicate, but excluding it from the
// separator set reproduces the same observable result for the common case).
TEST(TextInfoBatch33Test, ToTitleCase_HyphenatedWord_CapitalizesBothParts) {
    TextInfo ti;
    EXPECT_EQ(ti.ToTitleCase("mary-jane watson"), "Mary-Jane Watson");
}
TEST(TextInfoBatch33Test, ToTitleCase_Parentheses_CapitalizeInsideToo) {
    TextInfo ti;
    EXPECT_EQ(ti.ToTitleCase("test(word)"), "Test(Word)");
}
TEST(TextInfoBatch33Test, ToTitleCase_Apostrophe_DoesNotCapitalizeAfter) {
    TextInfo ti;
    EXPECT_EQ(ti.ToTitleCase("o'brien"), "O'brien");
}

TEST(TextInfoBatch33Test, Clone_IsMutable) {
    auto ro = TextInfo::ReadOnly(TextInfo());
    auto clone = ro.Clone();
    EXPECT_FALSE(clone.getIsReadOnlyProperty());
}

TEST(TextInfoBatch33Test, ReadOnly_MakesReadOnly) {
    TextInfo ti;
    auto ro = TextInfo::ReadOnly(ti);
    EXPECT_TRUE(ro.getIsReadOnlyProperty());
}

TEST(TextInfoBatch33Test, SetListSeparator_OnReadOnly_Throws) {
    auto ro = TextInfo::ReadOnly(TextInfo());
    EXPECT_THROW(ro.setListSeparatorProperty(";"), System::InvalidOperationException);
}

TEST(TextInfoBatch33Test, EqualityOperator) {
    TextInfo a("en-US"), b("en-US"), c("fr-FR");
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(TextInfoBatch33Test, ToString) {
    TextInfo ti("de-DE");
    EXPECT_EQ(ti.ToString(), "TextInfo - de-DE");
}
