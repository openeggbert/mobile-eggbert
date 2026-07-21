// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 28:
//   CultureTypes:        enum values, operator|, operator&
//   DateTimeFormatInfo:  properties, day/month name methods, era stubs, genitive names
//   DateTimeStyles:      enum values, operator|, operator&
//   DaylightTime:        constructor, getStartProperty/getEndProperty/getDeltaProperty
//   DigitShapes:         enum values
#include <algorithm>
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Globalization/CultureTypes.hpp"
#include "System/Globalization/DateTimeFormatInfo.hpp"
#include "System/Globalization/DateTimeStyles.hpp"
#include "System/Globalization/DaylightTime.hpp"
#include "System/Globalization/DigitShapes.hpp"
#include "System/DateTime.hpp"
#include "System/TimeSpan.hpp"

using System::Globalization::CultureTypes;
using System::Globalization::DateTimeFormatInfo;
using System::Globalization::DateTimeStyles;
using System::Globalization::DaylightTime;
using System::Globalization::DigitShapes;

// ===========================================================================
// CultureTypes
// ===========================================================================

TEST(CultureTypesBatch28Test, Values) {
    EXPECT_EQ(static_cast<int>(CultureTypes::NeutralCultures),        0x0001);
    EXPECT_EQ(static_cast<int>(CultureTypes::SpecificCultures),       0x0002);
    EXPECT_EQ(static_cast<int>(CultureTypes::InstalledWin32Cultures),  0x0004);
    EXPECT_EQ(static_cast<int>(CultureTypes::AllCultures),             0x0007);
    EXPECT_EQ(static_cast<int>(CultureTypes::UserCustomCulture),       0x0008);
    EXPECT_EQ(static_cast<int>(CultureTypes::ReplacementCultures),     0x0010);
    EXPECT_EQ(static_cast<int>(CultureTypes::WindowsOnlyCultures),     0x0020);
    EXPECT_EQ(static_cast<int>(CultureTypes::FrameworkCultures),       0x0040);
}

TEST(CultureTypesBatch28Test, OperatorOr) {
    auto combined = CultureTypes::NeutralCultures | CultureTypes::SpecificCultures;
    EXPECT_EQ(static_cast<int>(combined), 0x0003);
}

TEST(CultureTypesBatch28Test, OperatorAnd) {
    auto combined = CultureTypes::AllCultures;
    auto masked = combined & CultureTypes::NeutralCultures;
    EXPECT_EQ(masked, CultureTypes::NeutralCultures);
}

// ===========================================================================
// DateTimeStyles
// ===========================================================================

TEST(DateTimeStylesBatch28Test, Values) {
    EXPECT_EQ(static_cast<int>(DateTimeStyles::None),                 0x00000000);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::AllowLeadingWhite),    0x00000001);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::AllowTrailingWhite),   0x00000002);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::AllowInnerWhite),      0x00000004);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::AllowWhiteSpaces),     0x00000007);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::NoCurrentDateDefault), 0x00000008);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::AdjustToUniversal),    0x00000010);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::AssumeLocal),          0x00000020);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::AssumeUniversal),      0x00000040);
    EXPECT_EQ(static_cast<int>(DateTimeStyles::RoundtripKind),        0x00000080);
}

TEST(DateTimeStylesBatch28Test, OperatorOr) {
    auto combined = DateTimeStyles::AllowLeadingWhite | DateTimeStyles::AllowTrailingWhite;
    EXPECT_EQ(static_cast<int>(combined), 0x0003);
}

TEST(DateTimeStylesBatch28Test, OperatorAnd) {
    auto combined = DateTimeStyles::AllowWhiteSpaces;
    auto masked = combined & DateTimeStyles::AllowInnerWhite;
    EXPECT_EQ(masked, DateTimeStyles::AllowInnerWhite);
}

TEST(DateTimeStylesBatch28Test, AllowWhiteSpacesIsUnion) {
    auto manual = DateTimeStyles::AllowLeadingWhite
                | DateTimeStyles::AllowTrailingWhite
                | DateTimeStyles::AllowInnerWhite;
    EXPECT_EQ(manual, DateTimeStyles::AllowWhiteSpaces);
}

// ===========================================================================
// DigitShapes
// ===========================================================================

TEST(DigitShapesBatch28Test, Values) {
    EXPECT_EQ(static_cast<int>(DigitShapes::Context),        0);
    EXPECT_EQ(static_cast<int>(DigitShapes::None),           1);
    EXPECT_EQ(static_cast<int>(DigitShapes::NativeNational), 2);
}

// ===========================================================================
// DaylightTime
// ===========================================================================

TEST(DaylightTimeBatch28Test, Constructor) {
    System::DateTime start(2024, 3, 31);
    System::DateTime end(2024, 10, 27);
    System::TimeSpan delta = System::TimeSpan::FromHours(1);
    DaylightTime dt(start, end, delta);
    EXPECT_EQ(dt.getStartProperty(), start);
    EXPECT_EQ(dt.getEndProperty(),   end);
    EXPECT_EQ(dt.getDeltaProperty(), delta);
}

TEST(DaylightTimeBatch28Test, DeltaZero) {
    System::DateTime start(2024, 1, 1);
    System::DateTime end(2024, 1, 1);
    System::TimeSpan delta;
    DaylightTime dt(start, end, delta);
    EXPECT_EQ(dt.getDeltaProperty().getTotalHoursProperty(), 0.0);
}

// ===========================================================================
// DateTimeFormatInfo
// ===========================================================================

TEST(DateTimeFormatInfoBatch28Test, DefaultCtor) {
    DateTimeFormatInfo dtfi;
    EXPECT_FALSE(dtfi.getIsReadOnlyProperty());
}

TEST(DateTimeFormatInfoBatch28Test, InvariantInfo_IsReadOnly) {
    const auto& inv = DateTimeFormatInfo::getInvariantInfoProperty();
    EXPECT_TRUE(inv.getIsReadOnlyProperty());
}

TEST(DateTimeFormatInfoBatch28Test, CurrentInfo_IsReadOnly) {
    const auto& cur = DateTimeFormatInfo::getCurrentInfoProperty();
    EXPECT_TRUE(cur.getIsReadOnlyProperty());
}

TEST(DateTimeFormatInfoBatch28Test, ReadOnly_MakesReadOnly) {
    DateTimeFormatInfo dtfi;
    auto ro = DateTimeFormatInfo::ReadOnly(dtfi);
    EXPECT_TRUE(ro.getIsReadOnlyProperty());
}

TEST(DateTimeFormatInfoBatch28Test, Clone_IsMutable) {
    DateTimeFormatInfo dtfi;
    auto clone = dtfi.Clone();
    EXPECT_FALSE(clone.getIsReadOnlyProperty());
}

// Clone() previously copied isReadOnly_ verbatim, so cloning a read-only instance (e.g.
// InvariantInfo) produced another read-only clone instead of a mutable one -- real .NET
// always resets _isReadOnly = false on the clone (DateTimeFormatInfo.cs), regardless of the
// source's read-only status.
TEST(DateTimeFormatInfoBatch28Test, Clone_OfReadOnlyInstance_IsStillMutable) {
    const auto& readOnly = DateTimeFormatInfo::getInvariantInfoProperty();
    ASSERT_TRUE(readOnly.getIsReadOnlyProperty());
    auto clone = readOnly.Clone();
    EXPECT_FALSE(clone.getIsReadOnlyProperty());
}

TEST(DateTimeFormatInfoBatch28Test, FormatPatterns) {
    DateTimeFormatInfo dtfi;
    EXPECT_FALSE(dtfi.getFullDateTimePatternProperty().empty());
    EXPECT_FALSE(dtfi.getShortDatePatternProperty().empty());
    EXPECT_FALSE(dtfi.getLongTimePatternProperty().empty());
    EXPECT_FALSE(dtfi.getAMDesignatorProperty().empty());
    EXPECT_FALSE(dtfi.getPMDesignatorProperty().empty());
}

TEST(DateTimeFormatInfoBatch28Test, SetFormatPattern_ReadOnly_Throws) {
    auto ro = DateTimeFormatInfo::ReadOnly(DateTimeFormatInfo());
    EXPECT_THROW(ro.setFullDateTimePatternProperty("x"), System::InvalidOperationException);
}

TEST(DateTimeFormatInfoBatch28Test, SetFirstDayOfWeek_OutOfRange_Throws) {
    DateTimeFormatInfo dtfi;
    EXPECT_THROW(dtfi.setFirstDayOfWeekProperty(static_cast<System::DayOfWeek>(7)),
                 System::ArgumentOutOfRangeException);
}

TEST(DateTimeFormatInfoBatch28Test, SetFullDateTimePattern_MutatesValue) {
    DateTimeFormatInfo dtfi;
    dtfi.setFullDateTimePatternProperty("custom");
    EXPECT_EQ(dtfi.getFullDateTimePatternProperty(), "custom");
}

TEST(DateTimeFormatInfoBatch28Test, ReadOnlyPatterns) {
    DateTimeFormatInfo dtfi;
    EXPECT_FALSE(dtfi.getRFC1123PatternProperty().empty());
    EXPECT_FALSE(dtfi.getSortableDateTimePatternProperty().empty());
    EXPECT_FALSE(dtfi.getUniversalSortableDateTimePatternProperty().empty());
}

TEST(DateTimeFormatInfoBatch28Test, GetDayName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetDayName(System::DayOfWeek::Sunday),    "Sunday");
    EXPECT_EQ(dtfi.GetDayName(System::DayOfWeek::Saturday),  "Saturday");
}

TEST(DateTimeFormatInfoBatch28Test, GetAbbreviatedDayName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetAbbreviatedDayName(System::DayOfWeek::Monday), "Mon");
}

TEST(DateTimeFormatInfoBatch28Test, GetShortestDayName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetShortestDayName(System::DayOfWeek::Wednesday), "We");
}

TEST(DateTimeFormatInfoBatch28Test, GetMonthName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetMonthName(1),  "January");
    EXPECT_EQ(dtfi.GetMonthName(12), "December");
}

TEST(DateTimeFormatInfoBatch28Test, GetAbbreviatedMonthName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetAbbreviatedMonthName(6), "Jun");
}

TEST(DateTimeFormatInfoBatch28Test, GetMonthName_OutOfRange) {
    DateTimeFormatInfo dtfi;
    EXPECT_THROW(dtfi.GetMonthName(0),  System::ArgumentOutOfRangeException);
    EXPECT_THROW(dtfi.GetMonthName(14), System::ArgumentOutOfRangeException);
}

TEST(DateTimeFormatInfoBatch28Test, GenitiveNames) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.getAbbreviatedMonthGenitiveNamesProperty()[0], "Jan");
    EXPECT_EQ(dtfi.getMonthGenitiveNamesProperty()[0],            "January");
}

// GetEraName's real invariant-culture value is "A.D." (the full era name), not "AD" (which is
// the *abbreviated* name returned by GetAbbreviatedEraName) -- verified against CalendarData.cs:
// `invariant.saEraNames = ["A.D."]` vs `invariant.saAbbrevEraNames = ["AD"]`. An invalid era
// now throws ArgumentOutOfRangeException instead of silently returning "".
TEST(DateTimeFormatInfoBatch28Test, GetEraName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetEraName(1), "A.D.");
    EXPECT_THROW(dtfi.GetEraName(2), System::ArgumentOutOfRangeException);
}

TEST(DateTimeFormatInfoBatch28Test, GetAbbreviatedEraName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetAbbreviatedEraName(1), "AD");
    EXPECT_THROW(dtfi.GetAbbreviatedEraName(2), System::ArgumentOutOfRangeException);
}

// .NET's GetEra(string) compares case-insensitively (DateTimeFormatInfo.cs).
TEST(DateTimeFormatInfoBatch28Test, GetEra) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetEra("AD"),    1);
    EXPECT_EQ(dtfi.GetEra("A.D."), 1);
    EXPECT_EQ(dtfi.GetEra("ad"),   1);
    EXPECT_EQ(dtfi.GetEra("a.d."), 1);
    EXPECT_EQ(dtfi.GetEra("BC"),   -1);
}

TEST(DateTimeFormatInfoBatch28Test, GetAllDateTimePatterns) {
    DateTimeFormatInfo dtfi;
    auto patterns = dtfi.GetAllDateTimePatterns();
    EXPECT_FALSE(patterns.empty());
}

TEST(DateTimeFormatInfoBatch28Test, GetAllDateTimePatterns_ByFormat) {
    DateTimeFormatInfo dtfi;
    EXPECT_FALSE(dtfi.GetAllDateTimePatterns('d').empty());
    EXPECT_FALSE(dtfi.GetAllDateTimePatterns('D').empty());
    EXPECT_FALSE(dtfi.GetAllDateTimePatterns('T').empty());
}

TEST(DateTimeFormatInfoBatch28Test, GetAllDateTimePatterns_UnrecognizedFormat_Throws) {
    // Real .NET throws ArgumentException(nameof(format)) for an unrecognized standard
    // format character rather than returning an empty collection.
    DateTimeFormatInfo dtfi;
    EXPECT_THROW(dtfi.GetAllDateTimePatterns('?'), System::ArgumentException);
}

TEST(DateTimeFormatInfoBatch28Test, GetAllDateTimePatterns_RoundtripAndUniversal_DoNotThrow) {
    // Regression: 'o'/'O'/'U' previously fell through to the default ArgumentException case,
    // but real .NET's format switch handles all of "dDfFgGmMoOrRstTuUyY".
    DateTimeFormatInfo dtfi;
    EXPECT_FALSE(dtfi.GetAllDateTimePatterns('o').empty());
    EXPECT_FALSE(dtfi.GetAllDateTimePatterns('O').empty());
    EXPECT_FALSE(dtfi.GetAllDateTimePatterns('U').empty());
    EXPECT_EQ(dtfi.GetAllDateTimePatterns('F'), dtfi.GetAllDateTimePatterns('U'));
}

TEST(DateTimeFormatInfoBatch28Test, GetAllDateTimePatterns_NoArg_CoversAllStandardFormats) {
    // Regression: the no-arg overload previously returned only 7 hardcoded raw pattern
    // fields, silently omitting RFC1123/sortable/universal-sortable/round-trip patterns.
    // Real .NET's version loops over every standard format character and concatenates.
    DateTimeFormatInfo dtfi;
    auto all = dtfi.GetAllDateTimePatterns();
    auto contains = [&](const std::string& s) {
        return std::find(all.begin(), all.end(), s) != all.end();
    };
    EXPECT_TRUE(contains(dtfi.getRFC1123PatternProperty()));
    EXPECT_TRUE(contains(dtfi.getSortableDateTimePatternProperty()));
    EXPECT_TRUE(contains(dtfi.getUniversalSortableDateTimePatternProperty()));
    EXPECT_TRUE(contains("yyyy'-'MM'-'dd'T'HH':'mm':'ss.fffffffK"));
}

TEST(DateTimeFormatInfoBatch28Test, NativeCalendarName) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.getNativeCalendarNameProperty(), "");
}
