// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for: System enums, Nullable, Lazy, HashCode, ArraySegment,
//            Index, Range, ValueTuple, DateOnly, WeakReference.
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "System/StringComparison.hpp"
#include "System/StringSplitOptions.hpp"
#include "System/MidpointRounding.hpp"
#include "System/DayOfWeek.hpp"
#include "System/DateTimeKind.hpp"
#include "System/TypeCode.hpp"
#include "System/PlatformID.hpp"
#include "System/EnvironmentVariableTarget.hpp"
#include "System/Nullable.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Lazy.hpp"
#include "System/HashCode.hpp"
#include "System/ArraySegment.hpp"
#include "System/Index.hpp"
#include "System/Range.hpp"
#include "System/ValueTuple.hpp"
#include "System/DateOnly.hpp"
#include "System/WeakReference.hpp"
#include "System/IFormattable.hpp"
#include "System/ISpanFormattable.hpp"

using System::StringComparison;
using System::StringSplitOptions;
using System::MidpointRounding;
using System::DayOfWeek;
using System::DateTimeKind;
using System::TypeCode;
using System::PlatformID;
using System::EnvironmentVariableTarget;
using System::Nullable;
using System::Lazy;
using System::HashCode;
using System::ArraySegment;
using System::Index;
using System::Range;
using System::ValueTuple1;
using System::ValueTuple2;
using System::ValueTuple3;
using System::ValueTuple4;
using System::ValueTuple;
using System::ValueTuple5;
using System::ValueTuple6;
using System::ValueTuple7;
using System::ValueTuple8;
using System::MakeValueTuple;
using System::DateOnly;
using System::WeakReference;
using System::WeakReferenceT;

// ===========================================================================
// StringComparison
// ===========================================================================

TEST(StringComparisonTests, Ordinal_IsFour) {
    EXPECT_EQ(static_cast<int>(StringComparison::Ordinal), 4);
}
TEST(StringComparisonTests, OrdinalIgnoreCase_IsFive) {
    EXPECT_EQ(static_cast<int>(StringComparison::OrdinalIgnoreCase), 5);
}
TEST(StringComparisonTests, CurrentCulture_IsZero) {
    EXPECT_EQ(static_cast<int>(StringComparison::CurrentCulture), 0);
}

// ===========================================================================
// StringSplitOptions
// ===========================================================================

TEST(StringSplitOptionsTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(StringSplitOptions::None), 0);
}
TEST(StringSplitOptionsTests, RemoveEmptyEntries_IsOne) {
    EXPECT_EQ(static_cast<int>(StringSplitOptions::RemoveEmptyEntries), 1);
}
TEST(StringSplitOptionsTests, TrimEntries_IsTwo) {
    EXPECT_EQ(static_cast<int>(StringSplitOptions::TrimEntries), 2);
}
TEST(StringSplitOptionsTests, OrOperator) {
    auto combined = StringSplitOptions::RemoveEmptyEntries | StringSplitOptions::TrimEntries;
    EXPECT_EQ(static_cast<int>(combined), 3);
}

// ===========================================================================
// MidpointRounding
// ===========================================================================

TEST(MidpointRoundingTests, ToEven_IsZero)              { EXPECT_EQ(static_cast<int>(MidpointRounding::ToEven), 0); }
TEST(MidpointRoundingTests, AwayFromZero_IsOne)          { EXPECT_EQ(static_cast<int>(MidpointRounding::AwayFromZero), 1); }
TEST(MidpointRoundingTests, ToPositiveInfinity_IsFour)   { EXPECT_EQ(static_cast<int>(MidpointRounding::ToPositiveInfinity), 4); }

// ===========================================================================
// DayOfWeek
// ===========================================================================

TEST(DayOfWeekTests, Sunday_IsZero)      { EXPECT_EQ(static_cast<int>(DayOfWeek::Sunday),    0); }
TEST(DayOfWeekTests, Monday_IsOne)       { EXPECT_EQ(static_cast<int>(DayOfWeek::Monday),    1); }
TEST(DayOfWeekTests, Tuesday_IsTwo)      { EXPECT_EQ(static_cast<int>(DayOfWeek::Tuesday),   2); }
TEST(DayOfWeekTests, Wednesday_IsThree)  { EXPECT_EQ(static_cast<int>(DayOfWeek::Wednesday), 3); }
TEST(DayOfWeekTests, Thursday_IsFour)    { EXPECT_EQ(static_cast<int>(DayOfWeek::Thursday),  4); }
TEST(DayOfWeekTests, Friday_IsFive)      { EXPECT_EQ(static_cast<int>(DayOfWeek::Friday),    5); }
TEST(DayOfWeekTests, Saturday_IsSix)     { EXPECT_EQ(static_cast<int>(DayOfWeek::Saturday),  6); }
TEST(DayOfWeekTests, CastRoundTrip)      { EXPECT_EQ(static_cast<DayOfWeek>(3), DayOfWeek::Wednesday); }
TEST(DayOfWeekTests, Distinct_Sun_Sat)   { EXPECT_NE(DayOfWeek::Sunday, DayOfWeek::Saturday); }

// ===========================================================================
// DateTimeKind
// ===========================================================================

TEST(DateTimeKindTests, Unspecified_IsZero) { EXPECT_EQ(static_cast<int>(DateTimeKind::Unspecified), 0); }
TEST(DateTimeKindTests, Utc_IsOne)          { EXPECT_EQ(static_cast<int>(DateTimeKind::Utc), 1); }
TEST(DateTimeKindTests, Local_IsTwo)        { EXPECT_EQ(static_cast<int>(DateTimeKind::Local), 2); }

// ===========================================================================
// TypeCode
// ===========================================================================

TEST(TypeCodeTests, Empty_IsZero)    { EXPECT_EQ(static_cast<int>(TypeCode::Empty), 0); }
TEST(TypeCodeTests, Boolean_IsThree) { EXPECT_EQ(static_cast<int>(TypeCode::Boolean), 3); }
TEST(TypeCodeTests, Int32_IsNine)    { EXPECT_EQ(static_cast<int>(TypeCode::Int32), 9); }
TEST(TypeCodeTests, String_Is18)     { EXPECT_EQ(static_cast<int>(TypeCode::String), 18); }
TEST(TypeCodeTests, DateTime_Is16)   { EXPECT_EQ(static_cast<int>(TypeCode::DateTime), 16); }

// ===========================================================================
// PlatformID
// ===========================================================================

TEST(PlatformIDTests, Win32S_IsZero)        { EXPECT_EQ(static_cast<int>(PlatformID::Win32S),       0); }
TEST(PlatformIDTests, Win32Windows_IsOne)   { EXPECT_EQ(static_cast<int>(PlatformID::Win32Windows), 1); }
TEST(PlatformIDTests, Win32NT_IsTwo)        { EXPECT_EQ(static_cast<int>(PlatformID::Win32NT),      2); }
TEST(PlatformIDTests, WinCE_IsThree)        { EXPECT_EQ(static_cast<int>(PlatformID::WinCE),        3); }
TEST(PlatformIDTests, Unix_IsFour)          { EXPECT_EQ(static_cast<int>(PlatformID::Unix),         4); }
TEST(PlatformIDTests, Xbox_IsFive)          { EXPECT_EQ(static_cast<int>(PlatformID::Xbox),         5); }
TEST(PlatformIDTests, MacOSX_IsSix)         { EXPECT_EQ(static_cast<int>(PlatformID::MacOSX),       6); }
TEST(PlatformIDTests, Other_IsSeven)        { EXPECT_EQ(static_cast<int>(PlatformID::Other),        7); }
TEST(PlatformIDTests, Distinct_Win32NT_Unix) {
    EXPECT_NE(static_cast<int>(PlatformID::Win32NT), static_cast<int>(PlatformID::Unix));
}
TEST(PlatformIDTests, CastRoundTrip) {
    PlatformID pid = static_cast<PlatformID>(4);
    EXPECT_EQ(pid, PlatformID::Unix);
}

// ===========================================================================
// EnvironmentVariableTarget
// ===========================================================================

TEST(EnvironmentVariableTargetTests, Process_IsZero)  { EXPECT_EQ(static_cast<int>(EnvironmentVariableTarget::Process), 0); }
TEST(EnvironmentVariableTargetTests, User_IsOne)      { EXPECT_EQ(static_cast<int>(EnvironmentVariableTarget::User),    1); }
TEST(EnvironmentVariableTargetTests, Machine_IsTwo)   { EXPECT_EQ(static_cast<int>(EnvironmentVariableTarget::Machine), 2); }

// ===========================================================================
// Nullable<T>
// ===========================================================================

TEST(NullableTests, Default_HasValueFalse) {
    Nullable<int> n;
    EXPECT_FALSE(n.getHasValueProperty());
}
TEST(NullableTests, ValueCtor_HasValueTrue) {
    Nullable<int> n(42);
    EXPECT_TRUE(n.getHasValueProperty());
    EXPECT_EQ(n.getValueProperty(), 42);
}
TEST(NullableTests, GetValue_NoValue_Throws) {
    Nullable<int> n;
    EXPECT_THROW(n.getValueProperty(), System::InvalidOperationException);
}
TEST(NullableTests, GetValueOrDefault_NoValue_ReturnsDefault) {
    Nullable<int> n;
    EXPECT_EQ(n.GetValueOrDefault(), 0);
}
TEST(NullableTests, GetValueOrDefault_WithFallback) {
    Nullable<int> n;
    EXPECT_EQ(n.GetValueOrDefault(99), 99);
}
TEST(NullableTests, GetValueOrDefault_HasValue_ReturnsValue) {
    Nullable<int> n(7);
    EXPECT_EQ(n.GetValueOrDefault(99), 7);
}
TEST(NullableTests, OperatorBool_True_False) {
    Nullable<int> yes(1);
    Nullable<int> no;
    EXPECT_TRUE(static_cast<bool>(yes));
    EXPECT_FALSE(static_cast<bool>(no));
}
TEST(NullableTests, Equality_BothHaveValue) {
    EXPECT_EQ(Nullable<int>(5), Nullable<int>(5));
    EXPECT_NE(Nullable<int>(5), Nullable<int>(6));
}
TEST(NullableTests, Equality_NulloptComparison) {
    Nullable<int> n;
    EXPECT_TRUE(n == std::nullopt);
    Nullable<int> m(1);
    EXPECT_FALSE(m == std::nullopt);
}

TEST(NullableTests, Equals_SameValue_True) {
    Nullable<int> a(10), b(10);
    EXPECT_TRUE(a.Equals(b));
}

TEST(NullableTests, Equals_DifferentValues_False) {
    Nullable<int> a(10), b(20);
    EXPECT_FALSE(a.Equals(b));
}

TEST(NullableTests, Equals_BothNull_True) {
    Nullable<int> a, b;
    EXPECT_TRUE(a.Equals(b));
}

TEST(NullableTests, GetHashCode_HasValue_NonZero) {
    Nullable<int> n(42);
    EXPECT_NE(n.GetHashCode(), static_cast<std::size_t>(0));
}

TEST(NullableTests, GetHashCode_NoValue_IsZero) {
    Nullable<int> n;
    EXPECT_EQ(n.GetHashCode(), static_cast<std::size_t>(0));
}

TEST(NullableTests, ToString_HasValue_ReturnsString) {
    Nullable<int> n(7);
    EXPECT_EQ(n.ToString(), "7");
}

TEST(NullableTests, ToString_NoValue_ReturnsEmpty) {
    Nullable<int> n;
    EXPECT_EQ(n.ToString(), "");
}

// ---------------------------------------------------------------------------
// NullableHelper (static class counterpart)
// ---------------------------------------------------------------------------

TEST(NullableHelperTests, Compare_BothNull_Zero) {
    Nullable<int> a, b;
    EXPECT_EQ(System::NullableHelper::Compare(a, b), 0);
}

TEST(NullableHelperTests, Compare_FirstNull_Negative) {
    Nullable<int> a;
    Nullable<int> b(1);
    EXPECT_LT(System::NullableHelper::Compare(a, b), 0);
}

TEST(NullableHelperTests, Compare_SecondNull_Positive) {
    Nullable<int> a(1);
    Nullable<int> b;
    EXPECT_GT(System::NullableHelper::Compare(a, b), 0);
}

TEST(NullableHelperTests, Compare_EqualValues_Zero) {
    Nullable<int> a(5), b(5);
    EXPECT_EQ(System::NullableHelper::Compare(a, b), 0);
}

TEST(NullableHelperTests, Equals_BothNull_True) {
    Nullable<int> a, b;
    EXPECT_TRUE(System::NullableHelper::Equals(a, b));
}

TEST(NullableHelperTests, Equals_SameValue_True) {
    Nullable<int> a(3), b(3);
    EXPECT_TRUE(System::NullableHelper::Equals(a, b));
}

TEST(NullableHelperTests, Equals_DifferentValues_False) {
    Nullable<int> a(3), b(4);
    EXPECT_FALSE(System::NullableHelper::Equals(a, b));
}

// ===========================================================================
// Lazy<T>
// ===========================================================================

TEST(LazyTests, DefaultCtor_NotCreatedYet) {
    Lazy<int> lz;
    EXPECT_FALSE(lz.getIsValueCreatedProperty());
}
TEST(LazyTests, GetValue_CreatesOnFirstAccess) {
    Lazy<int> lz([]() { return 42; });
    EXPECT_FALSE(lz.getIsValueCreatedProperty());
    EXPECT_EQ(lz.getValueProperty(), 42);
    EXPECT_TRUE(lz.getIsValueCreatedProperty());
}
TEST(LazyTests, GetValue_SameValueOnRepeatedAccess) {
    int callCount = 0;
    Lazy<int> lz([&]() { ++callCount; return 7; });
    EXPECT_EQ(lz.getValueProperty(), 7);
    EXPECT_EQ(lz.getValueProperty(), 7);
    EXPECT_EQ(callCount, 1);
}
TEST(LazyTests, Value_AliasForGetValue) {
    Lazy<std::string> lz([]() { return std::string("hello"); });
    EXPECT_EQ(lz.Value(), "hello");
}
TEST(LazyTests, PrecomputedValue_IsCreatedImmediately) {
    Lazy<int> lz(42);
    EXPECT_TRUE(lz.getIsValueCreatedProperty());
    EXPECT_EQ(lz.getValueProperty(), 42);
}
TEST(LazyTests, PrecomputedValue_NoFactoryInvoked) {
    Lazy<std::string> lz(std::string("pre"));
    EXPECT_EQ(lz.Value(), "pre");
    EXPECT_TRUE(lz.getIsValueCreatedProperty());
}
TEST(LazyTests, BoolCtor_True_ThreadSafe_DefaultValue) {
    Lazy<int> lz(true);
    EXPECT_FALSE(lz.getIsValueCreatedProperty());
    EXPECT_EQ(lz.getValueProperty(), 0);
    EXPECT_EQ(lz.getModeProperty(), System::LazyThreadSafetyMode::ExecutionAndPublication);
}
TEST(LazyTests, BoolCtor_False_NonThreadSafe_DefaultValue) {
    Lazy<int> lz(false);
    EXPECT_EQ(lz.getModeProperty(), System::LazyThreadSafetyMode::None);
    EXPECT_EQ(lz.getValueProperty(), 0);
}
TEST(LazyTests, ModeCtor_PublicationOnly) {
    Lazy<int> lz(System::LazyThreadSafetyMode::PublicationOnly);
    EXPECT_EQ(lz.getModeProperty(), System::LazyThreadSafetyMode::PublicationOnly);
    EXPECT_EQ(lz.getValueProperty(), 0);
}
TEST(LazyTests, FactoryBool_True_ExecutionAndPublication) {
    Lazy<int> lz([]() { return 99; }, true);
    EXPECT_EQ(lz.getModeProperty(), System::LazyThreadSafetyMode::ExecutionAndPublication);
    EXPECT_EQ(lz.getValueProperty(), 99);
}
TEST(LazyTests, FactoryBool_False_None) {
    Lazy<int> lz([]() { return 55; }, false);
    EXPECT_EQ(lz.getModeProperty(), System::LazyThreadSafetyMode::None);
    EXPECT_EQ(lz.getValueProperty(), 55);
}
TEST(LazyTests, FactoryMode_None) {
    Lazy<int> lz([]() { return 7; }, System::LazyThreadSafetyMode::None);
    EXPECT_EQ(lz.getModeProperty(), System::LazyThreadSafetyMode::None);
    EXPECT_EQ(lz.getValueProperty(), 7);
}
TEST(LazyTests, DefaultCtor_ModeIsExecutionAndPublication) {
    Lazy<int> lz;
    EXPECT_EQ(lz.getModeProperty(), System::LazyThreadSafetyMode::ExecutionAndPublication);
}
TEST(LazyTests, ToString_NotCreated) {
    Lazy<int> lz;
    EXPECT_EQ(lz.ToString(), "Value is not created.");
}
TEST(LazyTests, ToString_Created) {
    // .NET's Lazy<T>.ToString() returns Value.ToString() once created, not a fixed
    // placeholder - verified against Lazy.cs.
    Lazy<int> lz([]() { return 1; });
    lz.getValueProperty();
    EXPECT_EQ(lz.ToString(), "1");
}
TEST(LazyTests, ToString_PrecomputedIsCreated) {
    Lazy<int> lz(5);
    EXPECT_EQ(lz.ToString(), "5");
}

// ===========================================================================
// HashCode
// ===========================================================================

TEST(HashCodeTests, Combine1_SameValueSameHash) {
    EXPECT_EQ(HashCode::Combine(42), HashCode::Combine(42));
}
TEST(HashCodeTests, Combine1_DifferentValueDifferentHash) {
    EXPECT_NE(HashCode::Combine(1), HashCode::Combine(2));
}
TEST(HashCodeTests, Combine2_Works) {
    int h1 = HashCode::Combine(1, 2);
    int h2 = HashCode::Combine(1, 2);
    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, HashCode::Combine(2, 1));
}
TEST(HashCodeTests, Combine3_Works) {
    int h = HashCode::Combine(1, 2, 3);
    EXPECT_EQ(h, HashCode::Combine(1, 2, 3));
}
TEST(HashCodeTests, Combine4_Works) {
    int h = HashCode::Combine(1, 2, 3, 4);
    EXPECT_EQ(h, HashCode::Combine(1, 2, 3, 4));
    EXPECT_NE(h, HashCode::Combine(1, 2, 3, 5));
}
TEST(HashCodeTests, AddAndToHashCode) {
    HashCode hc;
    hc.Add(10);
    hc.Add(20);
    HashCode hc2;
    hc2.Add(10);
    hc2.Add(20);
    EXPECT_EQ(hc.ToHashCode(), hc2.ToHashCode());
}
TEST(HashCodeTests, Combine5_Works) {
    int h = HashCode::Combine(1, 2, 3, 4, 5);
    EXPECT_EQ(h, HashCode::Combine(1, 2, 3, 4, 5));
    EXPECT_NE(h, HashCode::Combine(1, 2, 3, 4, 6));
}
TEST(HashCodeTests, Combine6_Works) {
    int h = HashCode::Combine(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(h, HashCode::Combine(1, 2, 3, 4, 5, 6));
    EXPECT_NE(h, HashCode::Combine(1, 2, 3, 4, 5, 7));
}
TEST(HashCodeTests, Combine7_Works) {
    int h = HashCode::Combine(1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(h, HashCode::Combine(1, 2, 3, 4, 5, 6, 7));
    EXPECT_NE(h, HashCode::Combine(1, 2, 3, 4, 5, 6, 8));
}
TEST(HashCodeTests, Combine8_Works) {
    int h = HashCode::Combine(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(h, HashCode::Combine(1, 2, 3, 4, 5, 6, 7, 8));
    EXPECT_NE(h, HashCode::Combine(1, 2, 3, 4, 5, 6, 7, 9));
}
TEST(HashCodeTests, AddWithComparer_UsesComparerHash) {
    struct ConstComparer : System::Collections::Generic::IEqualityComparer<int> {
        bool Equals(const int&, const int&) const override { return true; }
        int GetHashCode(const int&) const override { return 99; }
    };
    ConstComparer cmp;
    HashCode hc1, hc2;
    hc1.Add(42, cmp);
    hc2.Add(99, cmp); // different value, same comparer hash → same result
    EXPECT_EQ(hc1.ToHashCode(), hc2.ToHashCode());
}
TEST(HashCodeTests, AddBytes_SameInput_SameHash) {
    std::vector<uint8_t> bytes = {1, 2, 3, 4};
    HashCode hc1, hc2;
    hc1.AddBytes(bytes);
    hc2.AddBytes(bytes);
    EXPECT_EQ(hc1.ToHashCode(), hc2.ToHashCode());
}
TEST(HashCodeTests, AddBytes_DifferentInput_DifferentHash) {
    HashCode hc1, hc2;
    hc1.AddBytes({1, 2, 3});
    hc2.AddBytes({4, 5, 6});
    EXPECT_NE(hc1.ToHashCode(), hc2.ToHashCode());
}
TEST(HashCodeTests, AddBytes_RawPointer_MatchesVector) {
    uint8_t buf[] = {10, 20, 30};
    std::vector<uint8_t> vec(buf, buf + 3);
    HashCode hc1, hc2;
    hc1.AddBytes(buf, 3);
    hc2.AddBytes(vec);
    EXPECT_EQ(hc1.ToHashCode(), hc2.ToHashCode());
}

// ===========================================================================
// ArraySegment<T>
// ===========================================================================

TEST(ArraySegmentTests, FullArray_OffsetZeroCountFull) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    ArraySegment<int> seg(v);
    EXPECT_EQ(seg.getOffsetProperty(), 0);
    EXPECT_EQ(seg.getCountProperty(), 5);
}
TEST(ArraySegmentTests, PartialSegment_CorrectOffsetCount) {
    std::vector<int> v = {10, 20, 30, 40};
    ArraySegment<int> seg(v, 1, 2);
    EXPECT_EQ(seg.getOffsetProperty(), 1);
    EXPECT_EQ(seg.getCountProperty(), 2);
}
TEST(ArraySegmentTests, OperatorBracket_ReadsCorrectElement) {
    std::vector<int> v = {10, 20, 30, 40};
    ArraySegment<int> seg(v, 1, 3);
    EXPECT_EQ(seg[0], 20);
    EXPECT_EQ(seg[2], 40);
}
TEST(ArraySegmentTests, OperatorBracket_OutOfRange_Throws) {
    std::vector<int> v = {1, 2, 3};
    ArraySegment<int> seg(v, 0, 2);
    EXPECT_THROW(seg[5], System::ArgumentOutOfRangeException);
}
TEST(ArraySegmentTests, Slice_SubSegment) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    ArraySegment<int> seg(v, 1, 4);
    ArraySegment<int> sliced = seg.Slice(1, 2);
    EXPECT_EQ(sliced.getOffsetProperty(), 2);
    EXPECT_EQ(sliced.getCountProperty(), 2);
    EXPECT_EQ(sliced[0], 3);
}
TEST(ArraySegmentTests, OutOfRangeCtor_Throws) {
    std::vector<int> v = {1, 2};
    EXPECT_THROW((ArraySegment<int>(v, 0, 10)), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// Index
// ===========================================================================

TEST(IndexTests, FromStart_ValueAndNotFromEnd) {
    Index idx = Index::FromStart(3);
    EXPECT_EQ(idx.getValueProperty(), 3);
    EXPECT_FALSE(idx.getIsFromEndProperty());
}
TEST(IndexTests, FromEnd_IsFromEnd) {
    Index idx = Index::FromEnd(2);
    EXPECT_EQ(idx.getValueProperty(), 2);
    EXPECT_TRUE(idx.getIsFromEndProperty());
}
TEST(IndexTests, GetOffset_FromStart) {
    Index idx = Index::FromStart(2);
    EXPECT_EQ(idx.GetOffset(10), 2);
}
TEST(IndexTests, GetOffset_FromEnd) {
    Index idx = Index::FromEnd(2);
    EXPECT_EQ(idx.GetOffset(10), 8);
}
TEST(IndexTests, Start_OffsetZero) {
    EXPECT_EQ(Index::Start().GetOffset(5), 0);
}
TEST(IndexTests, End_OffsetEqualsLength) {
    EXPECT_EQ(Index::End().GetOffset(5), 5);
}
TEST(IndexTests, NegativeValue_Throws) {
    EXPECT_THROW(Index(-1), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// Range
// ===========================================================================

TEST(RangeTests, All_StartsAtZeroEndsAtLength) {
    Range r = Range::getAllProperty();
    auto [offset, length] = r.GetOffsetAndLength(10);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(length, 10);
}
TEST(RangeTests, StartAt_CorrectOffsetAndLength) {
    Range r = Range::StartAt(Index::FromStart(3));
    auto [offset, length] = r.GetOffsetAndLength(10);
    EXPECT_EQ(offset, 3);
    EXPECT_EQ(length, 7);
}
TEST(RangeTests, EndAt_CorrectOffsetAndLength) {
    Range r = Range::EndAt(Index::FromStart(5));
    auto [offset, length] = r.GetOffsetAndLength(10);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(length, 5);
}
TEST(RangeTests, ExplicitRange_CorrectValues) {
    Range r(Index::FromStart(2), Index::FromStart(7));
    auto [offset, length] = r.GetOffsetAndLength(10);
    EXPECT_EQ(offset, 2);
    EXPECT_EQ(length, 5);
}

// ===========================================================================
// ValueTuple
// ===========================================================================

TEST(ValueTupleTests, Tuple1_StoresAndCompares) {
    auto t1 = MakeValueTuple(42);
    auto t2 = MakeValueTuple(42);
    EXPECT_EQ(t1.Item1, 42);
    EXPECT_TRUE(t1 == t2);
}
TEST(ValueTupleTests, Tuple2_StoresAndCompares) {
    auto t = MakeValueTuple(1, std::string("hi"));
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item2, "hi");
    EXPECT_NE(t, MakeValueTuple(1, std::string("bye")));
}
TEST(ValueTupleTests, Tuple3_StoresAndCompares) {
    auto t = MakeValueTuple(1, 2, 3);
    EXPECT_EQ(t.Item3, 3);
    EXPECT_TRUE(t == MakeValueTuple(1, 2, 3));
}
TEST(ValueTupleTests, Tuple4_StoresAndCompares) {
    auto t = MakeValueTuple(1, 2, 3, 4);
    EXPECT_EQ(t.Item4, 4);
    EXPECT_FALSE(t == MakeValueTuple(1, 2, 3, 0));
}

TEST(ValueTupleTests, Tuple1_Equals) {
    auto a = MakeValueTuple(10);
    auto b = MakeValueTuple(10);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(MakeValueTuple(99)));
}

TEST(ValueTupleTests, Tuple2_Equals) {
    auto a = MakeValueTuple(1, 2);
    EXPECT_TRUE(a.Equals(MakeValueTuple(1, 2)));
    EXPECT_FALSE(a.Equals(MakeValueTuple(1, 3)));
}

TEST(ValueTupleTests, Tuple1_GetHashCode_SameForEqualValues) {
    auto a = MakeValueTuple(5);
    auto b = MakeValueTuple(5);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTupleTests, Tuple2_GetHashCode_SameForEqualValues) {
    auto a = MakeValueTuple(1, 2);
    auto b = MakeValueTuple(1, 2);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTupleTests, Tuple1_CompareTo_Less) {
    auto a = MakeValueTuple(1);
    auto b = MakeValueTuple(2);
    EXPECT_EQ(a.CompareTo(b), -1);
    EXPECT_EQ(b.CompareTo(a),  1);
    EXPECT_EQ(a.CompareTo(a),  0);
}

TEST(ValueTupleTests, Tuple2_CompareTo_Lexicographic) {
    auto a = MakeValueTuple(1, 1);
    auto b = MakeValueTuple(1, 2);
    EXPECT_EQ(a.CompareTo(b), -1);
}

TEST(ValueTupleTests, Tuple1_OrderingOperators) {
    auto a = MakeValueTuple(1);
    auto b = MakeValueTuple(2);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(a >= a);
}

TEST(ValueTupleTests, Tuple1_ToString) {
    auto t = MakeValueTuple(42);
    EXPECT_EQ(t.ToString(), "(42)");
}

TEST(ValueTupleTests, Tuple2_ToString) {
    auto t = MakeValueTuple(1, 2);
    EXPECT_EQ(t.ToString(), "(1, 2)");
}

TEST(ValueTupleTests, Tuple3_ToString) {
    auto t = MakeValueTuple(1, 2, 3);
    EXPECT_EQ(t.ToString(), "(1, 2, 3)");
}

TEST(ValueTupleTests, Tuple5_StoresAndCompares) {
    auto t = MakeValueTuple(1, 2, 3, 4, 5);
    EXPECT_EQ(t.Item5, 5);
    EXPECT_TRUE(t == MakeValueTuple(1, 2, 3, 4, 5));
    EXPECT_FALSE(t == MakeValueTuple(1, 2, 3, 4, 0));
}

TEST(ValueTupleTests, Tuple6_StoresAndCompares) {
    auto t = MakeValueTuple(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(t.Item6, 6);
    EXPECT_TRUE(t == MakeValueTuple(1, 2, 3, 4, 5, 6));
}

TEST(ValueTupleTests, Tuple7_StoresAndCompares) {
    auto t = MakeValueTuple(1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(t.Item7, 7);
    EXPECT_TRUE(t == MakeValueTuple(1, 2, 3, 4, 5, 6, 7));
}

TEST(ValueTupleTests, Tuple7_ToString) {
    auto t = MakeValueTuple(1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5, 6, 7)");
}

TEST(ValueTupleTests, Tuple8_StoresAndCompares) {
    auto t = MakeValueTuple(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(t.Item7, 7);
    EXPECT_EQ(t.Rest.Item1, 8);
    EXPECT_TRUE(t == MakeValueTuple(1, 2, 3, 4, 5, 6, 7, 8));
    EXPECT_FALSE(t == MakeValueTuple(1, 2, 3, 4, 5, 6, 7, 0));
}

TEST(ValueTupleTests, Tuple8_ToString) {
    auto t = MakeValueTuple(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5, 6, 7, 8)");
}

TEST(ValueTupleTests, Tuple8_CompareTo) {
    auto a = MakeValueTuple(1, 2, 3, 4, 5, 6, 7, 1);
    auto b = MakeValueTuple(1, 2, 3, 4, 5, 6, 7, 2);
    EXPECT_EQ(a.CompareTo(b), -1);
    EXPECT_EQ(b.CompareTo(a),  1);
    EXPECT_EQ(a.CompareTo(a),  0);
}

TEST(ValueTupleTests, ZeroElement_DefaultCtor) {
    ValueTuple t;
    EXPECT_EQ(t.ToString(), "()");
    EXPECT_EQ(t.GetHashCode(), 0);
    EXPECT_EQ(t.CompareTo(ValueTuple{}), 0);
    EXPECT_TRUE(t.Equals(ValueTuple{}));
    EXPECT_TRUE(t == ValueTuple{});
    EXPECT_FALSE(t != ValueTuple{});
}

TEST(ValueTupleTests, Create_ZeroElement) {
    auto t = ValueTuple::Create();
    EXPECT_EQ(t.ToString(), "()");
}

TEST(ValueTupleTests, Create_1) {
    auto t = ValueTuple::Create(42);
    EXPECT_EQ(t.Item1, 42);
}

TEST(ValueTupleTests, Create_2) {
    auto t = ValueTuple::Create(1, 2);
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item2, 2);
}

TEST(ValueTupleTests, Create_7) {
    auto t = ValueTuple::Create(1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(t.Item7, 7);
}

TEST(ValueTupleTests, Create_8) {
    auto t = ValueTuple::Create(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(t.Item7, 7);
    EXPECT_EQ(t.Rest.Item1, 8);
}

// ===========================================================================
// DateOnly
// ===========================================================================

TEST(DateOnlyTests, DefaultCtor_YearMonthDayOne) {
    DateOnly d;
    EXPECT_EQ(d.getYearProperty(), 1);
    EXPECT_EQ(d.getMonthProperty(), 1);
    EXPECT_EQ(d.getDayProperty(), 1);
}
TEST(DateOnlyTests, Constructor_StoresYearMonthDay) {
    DateOnly d(2026, 6, 8);
    EXPECT_EQ(d.getYearProperty(), 2026);
    EXPECT_EQ(d.getMonthProperty(), 6);
    EXPECT_EQ(d.getDayProperty(), 8);
}
TEST(DateOnlyTests, ToString_Format) {
    DateOnly d(2026, 1, 5);
    EXPECT_EQ(d.ToString(), "2026-01-05");
}
TEST(DateOnlyTests, Equality) {
    EXPECT_EQ(DateOnly(2000, 1, 1), DateOnly(2000, 1, 1));
    EXPECT_NE(DateOnly(2000, 1, 1), DateOnly(2000, 1, 2));
}
TEST(DateOnlyTests, Ordering) {
    EXPECT_LT(DateOnly(2000, 1, 1), DateOnly(2000, 1, 2));
    EXPECT_GT(DateOnly(2001, 1, 1), DateOnly(2000, 12, 31));
}

// ===========================================================================
// WeakReference / WeakReferenceT<T>
// ===========================================================================

TEST(WeakReferenceTests, IsAlive_True_WhenTargetAlive) {
    auto sp = std::make_shared<int>(42);
    WeakReference wr(sp);
    EXPECT_TRUE(wr.getIsAliveProperty());
}
TEST(WeakReferenceTests, IsAlive_False_AfterTargetExpired) {
    WeakReference wr;
    {
        auto sp = std::make_shared<int>(1);
        wr.setTargetProperty(sp);
    }
    EXPECT_FALSE(wr.getIsAliveProperty());
}
TEST(WeakReferenceTests, TryGetTarget_Success) {
    auto sp = std::make_shared<int>(7);
    WeakReference wr(sp);
    std::shared_ptr<void> out;
    EXPECT_TRUE(wr.TryGetTarget(out));
    EXPECT_NE(out, nullptr);
}
TEST(WeakReferenceTTests, TryGetTarget_Generic) {
    auto sp = std::make_shared<std::string>("hello");
    WeakReferenceT<std::string> wr(sp);
    std::shared_ptr<std::string> out;
    EXPECT_TRUE(wr.TryGetTarget(out));
    EXPECT_EQ(*out, "hello");
}
TEST(WeakReferenceTTests, IsAlive_False_AfterExpiry) {
    WeakReferenceT<int> wr;
    {
        auto sp = std::make_shared<int>(99);
        wr.SetTarget(sp);
    }
    EXPECT_FALSE(wr.getIsAliveProperty());
}

TEST(WeakReferenceTests, TrackResurrection_DefaultFalse) {
    auto sp = std::make_shared<int>(1);
    WeakReference wr(sp);
    EXPECT_FALSE(wr.getTrackResurrectionProperty());
}

TEST(WeakReferenceTests, TrackResurrection_TrueWhenSet) {
    auto sp = std::make_shared<int>(1);
    WeakReference wr(sp, true);
    EXPECT_TRUE(wr.getTrackResurrectionProperty());
}

TEST(WeakReferenceTests, GetTarget_ReturnsSharedPtr) {
    auto sp = std::make_shared<int>(42);
    WeakReference wr(sp);
    EXPECT_NE(wr.getTargetProperty(), nullptr);
}

TEST(WeakReferenceTests, GetTarget_NullAfterExpiry) {
    WeakReference wr;
    {
        auto sp = std::make_shared<int>(5);
        wr.setTargetProperty(sp);
    }
    EXPECT_EQ(wr.getTargetProperty(), nullptr);
}

TEST(WeakReferenceTests, TryGetTarget_False_WhenExpired) {
    WeakReference wr;
    std::shared_ptr<void> out;
    EXPECT_FALSE(wr.TryGetTarget(out));
    EXPECT_EQ(out, nullptr);
}

TEST(WeakReferenceTTests, TrackResurrection_Stored) {
    auto sp = std::make_shared<int>(1);
    WeakReferenceT<int> wr(sp, true);
    EXPECT_TRUE(wr.getTrackResurrectionProperty());
}

TEST(WeakReferenceTTests, TryGetTarget_False_WhenExpired) {
    WeakReferenceT<int> wr;
    std::shared_ptr<int> out;
    EXPECT_FALSE(wr.TryGetTarget(out));
    EXPECT_EQ(out, nullptr);
}

TEST(WeakReferenceTTests, SetTarget_UpdatesReference) {
    WeakReferenceT<std::string> wr;
    auto sp = std::make_shared<std::string>("world");
    wr.SetTarget(sp);
    EXPECT_TRUE(wr.getIsAliveProperty());
    std::shared_ptr<std::string> out;
    wr.TryGetTarget(out);
    EXPECT_EQ(*out, "world");
}

// ===========================================================================
// IFormattable / ISpanFormattable
// ===========================================================================

namespace {
    struct FormattableInt : System::ISpanFormattable {
        int value;
        explicit FormattableInt(int v) : value(v) {}

        [[nodiscard]] std::string ToString(const std::string& /*format*/) const override {
            return std::to_string(value);
        }

        bool TryFormat(char* dest, std::size_t destLen, std::size_t& charsWritten,
                       const std::string& /*format*/) const override {
            std::string s = std::to_string(value);
            if (s.size() > destLen) { charsWritten = 0; return false; }
            std::copy(s.begin(), s.end(), dest);
            charsWritten = s.size();
            return true;
        }
    };
}

TEST(IFormattableTests, ToString_ReturnsString) {
    FormattableInt f(42);
    EXPECT_EQ(f.ToString(""), "42");
}

TEST(ISpanFormattableTests, TryFormat_Succeeds) {
    FormattableInt f(123);
    char buf[16];
    std::size_t written = 0;
    EXPECT_TRUE(f.TryFormat(buf, sizeof(buf), written, ""));
    EXPECT_EQ(written, 3u);
    EXPECT_EQ(std::string(buf, written), "123");
}

TEST(ISpanFormattableTests, TryFormat_BufferTooSmall_ReturnsFalse) {
    FormattableInt f(99999);
    char buf[3];
    std::size_t written = 0;
    EXPECT_FALSE(f.TryFormat(buf, sizeof(buf), written, ""));
    EXPECT_EQ(written, 0u);
}

TEST(ISpanFormattableTests, IsA_IFormattable) {
    FormattableInt f(1);
    EXPECT_NE(dynamic_cast<System::IFormattable*>(&f), nullptr);
}

// ===========================================================================
// IObservable / IObserver
// ===========================================================================
#include "System/IObservable.hpp"
#include "System/IObserver.hpp"

namespace {
    struct NullDisposable : System::IDisposable {
        void Dispose() override {}
    };

    struct RecordingObserver : System::IObserver<int> {
        int lastValue = -1;
        bool completed = false;
        std::string errorMsg;

        void OnNext(const int& value) override { lastValue = value; }
        void OnError(const std::exception& e) override { errorMsg = e.what(); }
        void OnCompleted() override { completed = true; }
    };

    struct SimpleObservable : System::IObservable<int> {
        std::shared_ptr<System::IObserver<int>> subscriber;

        std::shared_ptr<System::IDisposable> Subscribe(
                std::shared_ptr<System::IObserver<int>> observer) override {
            subscriber = observer;
            return std::make_shared<NullDisposable>();
        }

        void Push(int v) { if (subscriber) subscriber->OnNext(v); }
        void Complete()  { if (subscriber) subscriber->OnCompleted(); }
    };
}

TEST(IObservableTests, Subscribe_ReturnsDisposable) {
    SimpleObservable src;
    auto obs = std::make_shared<RecordingObserver>();
    auto sub = src.Subscribe(obs);
    EXPECT_NE(sub, nullptr);
}

TEST(IObservableTests, OnNext_DeliveredToObserver) {
    SimpleObservable src;
    auto obs = std::make_shared<RecordingObserver>();
    src.Subscribe(obs);
    src.Push(42);
    EXPECT_EQ(obs->lastValue, 42);
}

TEST(IObservableTests, OnCompleted_DeliveredToObserver) {
    SimpleObservable src;
    auto obs = std::make_shared<RecordingObserver>();
    src.Subscribe(obs);
    src.Complete();
    EXPECT_TRUE(obs->completed);
}

TEST(IObserverTests, OnError_DeliveredToObserver) {
    auto obs = std::make_shared<RecordingObserver>();
    std::runtime_error err("boom");
    obs->OnError(err);
    EXPECT_EQ(obs->errorMsg, "boom");
}

TEST(IObserverTests, MultipleOnNext_LastValueKept) {
    auto obs = std::make_shared<RecordingObserver>();
    obs->OnNext(1);
    obs->OnNext(2);
    obs->OnNext(3);
    EXPECT_EQ(obs->lastValue, 3);
}

TEST(IObserverTests, IsAbstract_CannotInstantiateDirect) {
    static_assert(std::is_abstract_v<System::IObserver<int>>,
                  "IObserver<T> must be abstract");
    SUCCEED();
}

TEST(IObserverTests, StringObserver_OnNext_Delivered) {
    struct StringObs : System::IObserver<std::string> {
        std::string last;
        bool done = false;
        void OnNext(const std::string& v) override { last = v; }
        void OnError(const std::exception&) override {}
        void OnCompleted() override { done = true; }
    };
    StringObs obs;
    obs.OnNext("hello");
    EXPECT_EQ(obs.last, "hello");
    obs.OnCompleted();
    EXPECT_TRUE(obs.done);
}

// ===========================================================================
// TypedReference
// ===========================================================================

#include "System/TypedReference.hpp"

TEST(TypedReferenceTests, GetHashCode_ReturnsZero) {
    System::TypedReference tr{};
    EXPECT_EQ(tr.GetHashCode(), 0);
}
TEST(TypedReferenceTests, Equals_Throws) {
    System::TypedReference a{}, b{};
    EXPECT_THROW(a.Equals(b), System::NotSupportedException);
}
TEST(TypedReferenceTests, MakeTypedReference_Throws) {
    EXPECT_THROW(System::TypedReference::MakeTypedReference(),
                 System::NotSupportedException);
}
TEST(TypedReferenceTests, SetTypedReference_Throws) {
    System::TypedReference tr{};
    EXPECT_THROW(System::TypedReference::SetTypedReference(tr, nullptr),
                 System::NotSupportedException);
}
TEST(TypedReferenceTests, GetTargetType_Throws) {
    System::TypedReference tr{};
    EXPECT_THROW(System::TypedReference::GetTargetType(tr),
                 System::NotSupportedException);
}

// ===========================================================================
// Void
// ===========================================================================

#include "System/Void.hpp"

TEST(VoidTests, DefaultConstructible) {
    System::Void v;
    (void)v;
    SUCCEED();
}

TEST(VoidTests, Equality_TwoInstances_AreEqual) {
    System::Void a, b;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(VoidTests, ToString_ReturnsEmpty) {
    System::Void v;
    EXPECT_EQ(v.ToString(), "");
}

TEST(VoidTests, UsableAsTemplateArgument) {
    System::Nullable<System::Void> n;
    EXPECT_FALSE(n.getHasValueProperty());
    n = System::Void{};
    EXPECT_TRUE(n.getHasValueProperty());
}

TEST(VoidTests, IsTrivial) {
    // System.Void is a struct with no fields — verify it's empty in C++ too
    EXPECT_EQ(sizeof(System::Void), sizeof(unsigned char)); // empty struct has size >= 1
}

// ===========================================================================
// UnitySerializationHolder
// ===========================================================================
#include "System/UnitySerializationHolder.hpp"

TEST(UnitySerializationHolderTests, NullUnity_ConstantValue) {
    EXPECT_EQ(System::UnitySerializationHolder::NullUnity, 0x0002);
}

TEST(UnitySerializationHolderTests, GetUnityTypeProperty_Stored) {
    System::UnitySerializationHolder h(System::UnitySerializationHolder::NullUnity);
    EXPECT_EQ(h.getUnityTypeProperty(), System::UnitySerializationHolder::NullUnity);
}

TEST(UnitySerializationHolderTests, GetDataProperty_DefaultEmpty) {
    System::UnitySerializationHolder h(System::UnitySerializationHolder::NullUnity);
    EXPECT_TRUE(h.getDataProperty().empty());
}

TEST(UnitySerializationHolderTests, GetDataProperty_Stored) {
    System::UnitySerializationHolder h(System::UnitySerializationHolder::NullUnity, "DBNull");
    EXPECT_EQ(h.getDataProperty(), "DBNull");
}

TEST(UnitySerializationHolderTests, GetRealObject_NullUnity_ReturnsDBNull) {
    System::UnitySerializationHolder h(System::UnitySerializationHolder::NullUnity);
    System::DBNull& result = h.GetRealObject();
    EXPECT_EQ(&result, &System::DBNull::Value());
}

TEST(UnitySerializationHolderTests, GetRealObject_InvalidType_Throws) {
    System::UnitySerializationHolder h(999, "UnknownType");
    EXPECT_THROW(h.GetRealObject(), System::ArgumentException);
}

TEST(UnitySerializationHolderTests, GetObjectData_Throws) {
    System::UnitySerializationHolder h(System::UnitySerializationHolder::NullUnity);
    EXPECT_THROW(h.GetObjectData(), System::NotSupportedException);
}

TEST(UnitySerializationHolderTests, GetRealObject_SameInstanceEachCall) {
    System::UnitySerializationHolder h(System::UnitySerializationHolder::NullUnity);
    EXPECT_EQ(&h.GetRealObject(), &h.GetRealObject());
}

// ===========================================================================
// SerializationInfo / StreamingContext
//
// Coverage gap found while auditing serialization stubs (ticket 73): both types had zero
// test coverage anywhere despite being public API surface. Kept intentionally minimal --
// they are permanent no-op stubs (see their own doc-comments) with no behavior beyond
// "constructible, destructible", so that is all there is to verify.
// ===========================================================================
#include "System/Runtime/Serialization/SerializationInfo.hpp"
#include "System/Runtime/Serialization/StreamingContext.hpp"

TEST(SerializationInfoTests, DefaultConstructible) {
    EXPECT_NO_THROW(System::Runtime::Serialization::SerializationInfo{});
}

TEST(SerializationInfoTests, TwoInstancesAreIndependent) {
    System::Runtime::Serialization::SerializationInfo a;
    System::Runtime::Serialization::SerializationInfo b;
    EXPECT_NE(&a, &b);
}

TEST(StreamingContextTests, DefaultConstructible) {
    EXPECT_NO_THROW(System::Runtime::Serialization::StreamingContext{});
}
