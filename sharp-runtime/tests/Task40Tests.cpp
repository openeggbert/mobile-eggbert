// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Task 40: Span, Half, Int128, UInt128, DateTimeOffset, TimeOnly, DBNull,
// FormattableString, OperatingSystem, BFloat16, DivisionRounding, StringComparer,
// Progress, UnicodeRange/Ranges, CancellationTokenRegistration,
// KeyNotFoundException, ReferenceEqualityComparer, ReadonlyProperty.
#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include <vector>

#include "System/Span.hpp"
#include "System/Half.hpp"
#include "System/Int128.hpp"
#include "System/UInt128.hpp"
#include "System/DateTimeOffset.hpp"
#include "System/TimeOnly.hpp"
#include "System/DBNull.hpp"
#include "System/IConvertible.hpp"
#include "System/InvalidCastException.hpp"
#include "System/TypeCode.hpp"
#include "System/FormattableString.hpp"
#include "System/Runtime/CompilerServices/FormattableStringFactory.hpp"
#include "System/OperatingSystem.hpp"
#include "System/Numerics/BFloat16.hpp"
#include "System/Numerics/DivisionRounding.hpp"
#include "System/StringComparer.hpp"
#include "System/Progress.hpp"
#include "System/Text/Unicode/UnicodeRange.hpp"
#include "System/Text/Unicode/UnicodeRanges.hpp"
#include "System/Threading/CancellationTokenRegistration.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/Collections/Generic/ReferenceEqualityComparer.hpp"
#include "SharpRuntime/Experimental/ReadonlyProperty.hpp"

// ===========================================================================
// Span<T>
// ===========================================================================

using System::Span;
using System::ReadOnlySpan;

TEST(SpanTests, DefaultCtor_IsEmpty) {
    Span<int> s;
    EXPECT_EQ(s.getLengthProperty(), 0);
    EXPECT_TRUE(s.getIsEmptyProperty());
}

TEST(SpanTests, PtrLen_Ctor_LengthCorrect) {
    int arr[] = {1, 2, 3};
    Span<int> s(arr, 3);
    EXPECT_EQ(s.getLengthProperty(), 3);
    EXPECT_FALSE(s.getIsEmptyProperty());
}

TEST(SpanTests, IndexOperator_ReadsAndWrites) {
    int arr[] = {10, 20, 30};
    Span<int> s(arr, 3);
    EXPECT_EQ(s[0], 10);
    s[1] = 99;
    EXPECT_EQ(arr[1], 99);
}

TEST(SpanTests, IndexOperator_OutOfRange_Throws) {
    int arr[] = {1};
    Span<int> s(arr, 1);
    EXPECT_THROW(s[1], System::IndexOutOfRangeException);
}

TEST(SpanTests, Slice_SubRange) {
    int arr[] = {1, 2, 3, 4, 5};
    Span<int> s(arr, 5);
    auto sl = s.Slice(1, 3);
    EXPECT_EQ(sl.getLengthProperty(), 3);
    EXPECT_EQ(sl[0], 2);
    EXPECT_EQ(sl[2], 4);
}

// Slice(start, length)'s bounds check used to compute `start + length` directly in intcs
// (int32) arithmetic, which itself signed-overflows for large start/length -- confirmed real
// UB via a standalone UBSan repro -- and worse, the wrapped (very negative) sum then compared
// as <= the span's actual length, silently BYPASSING the check entirely instead of throwing.
// Real .NET's Span<T>.Slice(int,int) explicitly guards against exactly this by casting to
// unsigned and using subtraction instead of addition.
TEST(SpanTests, Slice_StartPlusLengthOverflow_ThrowsInsteadOfBypassingCheck) {
    int arr[] = {1, 2, 3, 4, 5};
    Span<int> s(arr, 5);
    EXPECT_THROW(s.Slice(2147483647, 10), System::ArgumentOutOfRangeException);
}

TEST(SpanTests, Slice_NegativeLength_Throws) {
    int arr[] = {1, 2, 3};
    Span<int> s(arr, 3);
    EXPECT_THROW(s.Slice(0, -1), System::ArgumentOutOfRangeException);
}

TEST(SpanTests, FromVector_Ctor) {
    std::vector<int> v = {5, 6, 7};
    Span<int> s(v);
    EXPECT_EQ(s.getLengthProperty(), 3);
    EXPECT_EQ(s[2], 7);
}

TEST(SpanTests, RangeFor_IteratesAll) {
    int arr[] = {1, 2, 3};
    Span<int> s(arr, 3);
    int sum = 0;
    for (int x : s) sum += x;
    EXPECT_EQ(sum, 6);
}

TEST(SpanTests, Empty_IsEmpty) {
    auto s = Span<int>::Empty();
    EXPECT_TRUE(s.getIsEmptyProperty());
    EXPECT_EQ(s.getLengthProperty(), 0);
}

TEST(SpanTests, Slice_OneArg_ToEnd) {
    int arr[] = {1, 2, 3, 4};
    Span<int> s(arr, 4);
    auto sl = s.Slice(2);
    EXPECT_EQ(sl.getLengthProperty(), 2);
    EXPECT_EQ(sl[0], 3);
    EXPECT_EQ(sl[1], 4);
}

TEST(SpanTests, CopyTo_Succeeds) {
    int src[] = {10, 20, 30};
    int dst[] = {0, 0, 0};
    Span<int> s(src, 3);
    Span<int> d(dst, 3);
    s.CopyTo(d);
    EXPECT_EQ(dst[0], 10);
    EXPECT_EQ(dst[2], 30);
}

TEST(SpanTests, TryCopyTo_SucceedsWhenFits) {
    int src[] = {1, 2};
    int dst[] = {0, 0, 0};
    Span<int> s(src, 2);
    Span<int> d(dst, 3);
    EXPECT_TRUE(s.TryCopyTo(d));
    EXPECT_EQ(dst[0], 1);
}

TEST(SpanTests, TryCopyTo_FailsWhenTooShort) {
    int src[] = {1, 2, 3};
    int dst[] = {0, 0};
    Span<int> s(src, 3);
    Span<int> d(dst, 2);
    EXPECT_FALSE(s.TryCopyTo(d));
}

TEST(SpanTests, ToArray_ReturnsCopy) {
    int arr[] = {7, 8, 9};
    Span<int> s(arr, 3);
    auto v = s.ToArray();
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[1], 8);
}

TEST(SpanTests, Fill_SetsAllElements) {
    int arr[] = {0, 0, 0, 0};
    Span<int> s(arr, 4);
    s.Fill(42);
    for (int x : arr) EXPECT_EQ(x, 42);
}

TEST(SpanTests, Clear_ZeroesAllElements) {
    int arr[] = {1, 2, 3};
    Span<int> s(arr, 3);
    s.Clear();
    for (int x : arr) EXPECT_EQ(x, 0);
}

TEST(SpanTests, ToString_CharSpan) {
    char buf[] = {'h', 'i'};
    Span<char> s(buf, 2);
    EXPECT_EQ(s.ToString(), "hi");
}

TEST(SpanTests, EqualityOperator_SameSpan) {
    int arr[] = {1, 2, 3};
    Span<int> a(arr, 3);
    Span<int> b(arr, 3);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(SpanTests, EqualityOperator_DifferentPointer) {
    int a1[] = {1, 2, 3};
    int a2[] = {1, 2, 3};
    Span<int> a(a1, 3);
    Span<int> b(a2, 3);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(SpanTests, ImplicitConversion_ToReadOnlySpan) {
    int arr[] = {4, 5, 6};
    Span<int> s(arr, 3);
    ReadOnlySpan<int> ros = s;
    EXPECT_EQ(ros.getLengthProperty(), 3);
    EXPECT_EQ(ros[0], 4);
}

TEST(ReadOnlySpanTests, Ctor_LengthAndAccess) {
    const int arr[] = {7, 8, 9};
    ReadOnlySpan<int> rs(arr, 3);
    EXPECT_EQ(rs.getLengthProperty(), 3);
    EXPECT_EQ(rs[0], 7);
}

TEST(ReadOnlySpanTests, Slice_Works) {
    const int arr[] = {10, 20, 30, 40};
    ReadOnlySpan<int> rs(arr, 4);
    auto sl = rs.Slice(2, 2);
    EXPECT_EQ(sl[0], 30);
    EXPECT_EQ(sl[1], 40);
}

// ===========================================================================
// Half
// ===========================================================================

TEST(HalfTests, FromSingle_Zero_IsZero) {
    System::Half h = System::Half::FromSingle(0.0f);
    EXPECT_NEAR(h.ToSingle(), 0.0f, 1e-6f);
}

TEST(HalfTests, FromSingle_One_RoundTrip) {
    System::Half h = System::Half::FromSingle(1.0f);
    EXPECT_NEAR(h.ToSingle(), 1.0f, 1e-3f);
}

TEST(HalfTests, StaticZero_IsZero) {
    EXPECT_NEAR(System::Half::Zero.ToSingle(), 0.0f, 1e-6f);
}

TEST(HalfTests, StaticPositiveInfinity_IsInf) {
    EXPECT_TRUE(std::isinf(System::Half::PositiveInfinity.ToSingle()));
    EXPECT_GT(System::Half::PositiveInfinity.ToSingle(), 0.0f);
}

TEST(HalfTests, StaticNegativeInfinity_IsNegInf) {
    EXPECT_TRUE(std::isinf(System::Half::NegativeInfinity.ToSingle()));
    EXPECT_LT(System::Half::NegativeInfinity.ToSingle(), 0.0f);
}

TEST(HalfTests, StaticNaN_IsNaN) {
    EXPECT_TRUE(std::isnan(System::Half::NaN.ToSingle()));
}

TEST(HalfTests, Comparison_LessThan) {
    auto h1 = System::Half::FromSingle(1.0f);
    auto h2 = System::Half::FromSingle(2.0f);
    EXPECT_TRUE(h1 < h2);
    EXPECT_FALSE(h2 < h1);
}

TEST(HalfTests, Equality) {
    auto h1 = System::Half::FromSingle(3.0f);
    auto h2 = System::Half::FromSingle(3.0f);
    EXPECT_EQ(h1, h2);
}

TEST(HalfTests, ExplicitConversion_ToFloat) {
    auto h = System::Half::FromSingle(5.0f);
    float f = static_cast<float>(h);
    EXPECT_NEAR(f, 5.0f, 1e-2f);
}

// ===========================================================================
// Int128
// ===========================================================================

TEST(Int128Tests, DefaultCtor_IsZero) {
    System::Int128 v;
    EXPECT_EQ(v.getLowerProperty(), uint64_t(0));
    EXPECT_EQ(v.getUpperProperty(), uint64_t(0));
}

TEST(Int128Tests, Addition) {
    System::Int128 a(1), b(2);
    System::Int128 c = a + b;
    EXPECT_EQ(static_cast<long long>(c), 3LL);
}

TEST(Int128Tests, Subtraction) {
    System::Int128 a(10), b(3);
    EXPECT_EQ(static_cast<long long>(a - b), 7LL);
}

TEST(Int128Tests, Multiplication) {
    System::Int128 a(6), b(7);
    EXPECT_EQ(static_cast<long long>(a * b), 42LL);
}

TEST(Int128Tests, Negation) {
    System::Int128 a(5);
    EXPECT_EQ(static_cast<long long>(-a), -5LL);
}

TEST(Int128Tests, ToString_Zero) {
    EXPECT_EQ(System::Int128().ToString(), "0");
}

TEST(Int128Tests, ToString_Positive) {
    System::Int128 v(123);
    EXPECT_EQ(v.ToString(), "123");
}

TEST(Int128Tests, Comparison) {
    System::Int128 a(1), b(2);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
    EXPECT_TRUE(a != b);
}

TEST(Int128Tests, StaticZero_One) {
    EXPECT_EQ(static_cast<long long>(System::Int128::Zero()), 0LL);
    EXPECT_EQ(static_cast<long long>(System::Int128::One()), 1LL);
}

TEST(Int128Tests, Equals_SameValue) {
    System::Int128 a(42), b(42);
    EXPECT_TRUE(a.Equals(b));
}

TEST(Int128Tests, Equals_DifferentValues) {
    System::Int128 a(1), b(2);
    EXPECT_FALSE(a.Equals(b));
}

TEST(Int128Tests, GetHashCode_SameValueSameHash) {
    System::Int128 a(99), b(99);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(Int128Tests, CompareTo_Less) {
    System::Int128 a(1), b(2);
    EXPECT_EQ(a.CompareTo(b), -1);
}

TEST(Int128Tests, CompareTo_Equal) {
    System::Int128 a(5), b(5);
    EXPECT_EQ(a.CompareTo(b), 0);
}

TEST(Int128Tests, CompareTo_Greater) {
    System::Int128 a(10), b(3);
    EXPECT_EQ(a.CompareTo(b), 1);
}

TEST(Int128Tests, ToString_Negative) {
    System::Int128 v(-42);
    EXPECT_EQ(v.ToString(), "-42");
}

TEST(Int128Tests, Parse_Positive) {
    System::Int128 v = System::Int128::Parse("170141183460469231731687303715884105727");
    EXPECT_EQ(v, System::Int128::MaxValue());
}

TEST(Int128Tests, Parse_Negative) {
    System::Int128 v = System::Int128::Parse("-42");
    EXPECT_EQ(static_cast<long long>(v), -42LL);
}

TEST(Int128Tests, Parse_Invalid_Throws) {
    EXPECT_THROW(System::Int128::Parse("abc"), System::FormatException);
}

TEST(Int128Tests, TryParse_Success) {
    System::Int128 result;
    EXPECT_TRUE(System::Int128::TryParse("12345", result));
    EXPECT_EQ(static_cast<long long>(result), 12345LL);
}

TEST(Int128Tests, TryParse_Failure) {
    System::Int128 result;
    EXPECT_FALSE(System::Int128::TryParse("not_a_number", result));
}

TEST(Int128Tests, Abs_Positive) {
    System::Int128 v(7);
    EXPECT_EQ(System::Int128::Abs(v), System::Int128(7));
}

TEST(Int128Tests, Abs_Negative) {
    System::Int128 v(-7);
    EXPECT_EQ(System::Int128::Abs(v), System::Int128(7));
}

TEST(Int128Tests, DivRem) {
    System::Int128 dividend(17), divisor(5), remainder;
    System::Int128 quotient = System::Int128::DivRem(dividend, divisor, remainder);
    EXPECT_EQ(static_cast<long long>(quotient),  3LL);
    EXPECT_EQ(static_cast<long long>(remainder), 2LL);
}

TEST(Int128Tests, DivisionByZero_Throws) {
    System::Int128 a(42), zero(0);
    EXPECT_THROW(a / zero, System::DivideByZeroException);
}

TEST(Int128Tests, ModuloByZero_Throws) {
    System::Int128 a(42), zero(0);
    EXPECT_THROW(a % zero, System::DivideByZeroException);
}

TEST(Int128Tests, LeadingZeroCount_One) {
    System::Int128 v(1);
    EXPECT_EQ(System::Int128::LeadingZeroCount(v), 127);
}

TEST(Int128Tests, LeadingZeroCount_Zero) {
    System::Int128 v(0);
    EXPECT_EQ(System::Int128::LeadingZeroCount(v), 128);
}

TEST(Int128Tests, TrailingZeroCount_One) {
    System::Int128 v(1);
    EXPECT_EQ(System::Int128::TrailingZeroCount(v), 0);
}

TEST(Int128Tests, TrailingZeroCount_Four) {
    System::Int128 v(4);
    EXPECT_EQ(System::Int128::TrailingZeroCount(v), 2);
}

TEST(Int128Tests, PopCount) {
    System::Int128 v(7); // binary 111
    EXPECT_EQ(System::Int128::PopCount(v), 3);
}

TEST(Int128Tests, RotateLeft_ByZero) {
    System::Int128 v(1);
    EXPECT_EQ(System::Int128::RotateLeft(v, 0), System::Int128(1));
}

TEST(Int128Tests, RotateLeft_By1) {
    System::Int128 v(1);
    EXPECT_EQ(System::Int128::RotateLeft(v, 1), System::Int128(2));
}

TEST(Int128Tests, RotateRight_By1) {
    System::Int128 v(2);
    EXPECT_EQ(System::Int128::RotateRight(v, 1), System::Int128(1));
}

TEST(Int128Tests, UpperLower_TwoArgCtor) {
    System::Int128 v(uint64_t(0), uint64_t(0xDEADBEEFu));
    EXPECT_EQ(v.getLowerProperty(), uint64_t(0xDEADBEEFu));
    EXPECT_EQ(v.getUpperProperty(), uint64_t(0));
}

TEST(Int128Tests, MinValue_IsNegative) {
    EXPECT_TRUE(System::Int128::MinValue() < System::Int128::Zero());
}

TEST(Int128Tests, MaxValue_IsPositive) {
    EXPECT_TRUE(System::Int128::MaxValue() > System::Int128::Zero());
}

TEST(Int128Tests, NegativeOne_IsMinusOne) {
    EXPECT_EQ(static_cast<long long>(System::Int128::NegativeOne()), -1LL);
}

// Regression test for a wave-3 audit finding: this threw std::overflow_error (an unrelated
// std:: exception type invisible to code catching System::Exception&) instead of
// System::OverflowException, despite Int128.hpp's own doc-comment already saying "matching
// .NET's OverflowException".
TEST(Int128Tests, Abs_MinValue_Throws) {
    // .NET's Abs(Int128.MinValue) throws OverflowException, since -MinValue does
    // not fit in a signed Int128 (matches Int64.Abs(Int64.MinValue) in this codebase).
    EXPECT_THROW(System::Int128::Abs(System::Int128::MinValue()), System::OverflowException);
}

TEST(Int128Tests, ShiftLeft_MasksShiftAmountLike_DotNet) {
    // .NET masks the shift amount to its low 7 bits (mod 128) instead of it being
    // undefined behavior out of range, unlike raw __int128's native << operator.
    System::Int128 v(1);
    EXPECT_EQ(v << 128, v << 0);
    EXPECT_EQ(v << 129, v << 1);
}

TEST(Int128Tests, ShiftRight_MasksShiftAmountLike_DotNet) {
    System::Int128 v(4);
    EXPECT_EQ(v >> 128, v >> 0);
    EXPECT_EQ(v >> 129, v >> 1);
}

TEST(Int128Tests, ToString_Format_Hex) {
    System::Int128 v(255);
    EXPECT_EQ(v.ToString("X"), "FF");
    EXPECT_EQ(v.ToString("x"), "ff");
    EXPECT_EQ(v.ToString("X4"), "00FF");
}

TEST(Int128Tests, ToString_Format_Decimal) {
    System::Int128 v(42);
    EXPECT_EQ(v.ToString("D"), "42");
    EXPECT_EQ(v.ToString("D5"), "00042");
    System::Int128 neg(-42);
    EXPECT_EQ(neg.ToString("D5"), "-00042");
}

TEST(Int128Tests, ToString_Format_General) {
    EXPECT_EQ(System::Int128(7).ToString("G"), "7");
}

TEST(Int128Tests, Clamp_WithinRange_ReturnsValue) {
    System::Int128 v(5);
    EXPECT_EQ(System::Int128::Clamp(v, System::Int128(0), System::Int128(10)), v);
}

TEST(Int128Tests, Clamp_BelowMin_ReturnsMin) {
    EXPECT_EQ(System::Int128::Clamp(System::Int128(-5), System::Int128(0), System::Int128(10)), System::Int128(0));
}

TEST(Int128Tests, Clamp_AboveMax_ReturnsMax) {
    EXPECT_EQ(System::Int128::Clamp(System::Int128(50), System::Int128(0), System::Int128(10)), System::Int128(10));
}

TEST(Int128Tests, Clamp_MinGreaterThanMax_Throws) {
    EXPECT_THROW(System::Int128::Clamp(System::Int128(5), System::Int128(10), System::Int128(0)), System::ArgumentException);
}

TEST(Int128Tests, Max_ReturnsLarger) {
    EXPECT_EQ(System::Int128::Max(System::Int128(3), System::Int128(7)), System::Int128(7));
}

TEST(Int128Tests, Min_ReturnsSmaller) {
    EXPECT_EQ(System::Int128::Min(System::Int128(3), System::Int128(7)), System::Int128(3));
}

TEST(Int128Tests, Sign_Negative) {
    EXPECT_EQ(System::Int128::Sign(System::Int128(-9)), -1);
}

TEST(Int128Tests, Sign_Zero) {
    EXPECT_EQ(System::Int128::Sign(System::Int128::Zero()), 0);
}

TEST(Int128Tests, Sign_Positive) {
    EXPECT_EQ(System::Int128::Sign(System::Int128(9)), 1);
}

TEST(Int128Tests, IsEvenInteger_True) {
    EXPECT_TRUE(System::Int128::IsEvenInteger(System::Int128(4)));
    EXPECT_FALSE(System::Int128::IsEvenInteger(System::Int128(5)));
}

TEST(Int128Tests, IsOddInteger_True) {
    EXPECT_TRUE(System::Int128::IsOddInteger(System::Int128(5)));
    EXPECT_FALSE(System::Int128::IsOddInteger(System::Int128(4)));
}

TEST(Int128Tests, IsPow2_PowersOfTwo) {
    EXPECT_TRUE(System::Int128::IsPow2(System::Int128(1)));
    EXPECT_TRUE(System::Int128::IsPow2(System::Int128(64)));
    EXPECT_FALSE(System::Int128::IsPow2(System::Int128(63)));
}

TEST(Int128Tests, IsPow2_NegativeOrZero_False) {
    // Matches .NET: negative values and zero are never powers of two.
    EXPECT_FALSE(System::Int128::IsPow2(System::Int128::Zero()));
    EXPECT_FALSE(System::Int128::IsPow2(System::Int128(-4)));
}

TEST(Int128Tests, Log2_Zero_IsZero) {
    // Matches .NET: Log2(0) is 0, not an error.
    EXPECT_EQ(System::Int128::Log2(System::Int128::Zero()), 0);
}

TEST(Int128Tests, Log2_PowerOfTwo) {
    EXPECT_EQ(System::Int128::Log2(System::Int128(64)), 6);
}

TEST(Int128Tests, Log2_CrossesUpperHalf) {
    // 2^64 requires the upper 64-bit word, exercising the hi != 0 branch.
    System::Int128 v(uint64_t(1), uint64_t(0));
    EXPECT_EQ(System::Int128::Log2(v), 64);
}

TEST(Int128Tests, Log2_Negative_Throws) {
    EXPECT_THROW(System::Int128::Log2(System::Int128(-1)), System::ArgumentOutOfRangeException);
}

// operator+/-/* previously used raw signed __int128 arithmetic, which is genuine C++ UB on
// overflow (confirmed via a standalone UBSan repro) even though real .NET's own unchecked
// operators are defined to wrap silently -- these lock in the wrapping behavior.
TEST(Int128Tests, OperatorPlus_MaxValuePlusOne_WrapsToMinValue) {
    EXPECT_EQ(System::Int128::MaxValue() + System::Int128::One(), System::Int128::MinValue());
}

TEST(Int128Tests, OperatorMinus_MinValueMinusOne_WrapsToMaxValue) {
    EXPECT_EQ(System::Int128::MinValue() - System::Int128::One(), System::Int128::MaxValue());
}

TEST(Int128Tests, OperatorMultiply_Overflow_WrapsWithoutUB) {
    // MaxValue * 2 overflows; must not crash/UB, just wrap per real .NET's unchecked operator.
    System::Int128 result = System::Int128::MaxValue() * System::Int128(2);
    EXPECT_EQ(result, System::Int128(-2));
}

// Matches real .NET's unchecked unary operator- => Zero - value: negating MinValue wraps back
// to MinValue itself, a two's-complement quirk (Int128::Abs() is the method that throws for
// this case, not the raw operator). Confirmed real UB in plain __int128 negation via UBSan.
TEST(Int128Tests, UnaryNegate_MinValue_WrapsToItself) {
    EXPECT_EQ(-System::Int128::MinValue(), System::Int128::MinValue());
}

// Unlike +/-/*, real .NET's Int128 operator/ (and operator%, which is defined in terms of it)
// explicitly throws OverflowException for MinValue/-1, rather than wrapping -- confirmed real
// UB in plain __int128 division/modulo for this exact case via a standalone UBSan repro.
TEST(Int128Tests, OperatorDivide_MinValueByNegativeOne_ThrowsOverflow) {
    EXPECT_THROW(System::Int128::MinValue() / System::Int128(-1), System::OverflowException);
}

TEST(Int128Tests, OperatorModulo_MinValueByNegativeOne_ThrowsOverflow) {
    EXPECT_THROW(System::Int128::MinValue() % System::Int128(-1), System::OverflowException);
}

TEST(Int128Tests, OperatorDivide_ByZero_ThrowsDivideByZero) {
    EXPECT_THROW(System::Int128::One() / System::Int128::Zero(), System::DivideByZeroException);
}

// ===========================================================================
// UInt128
// ===========================================================================

TEST(UInt128Tests, DefaultCtor_IsZero) {
    System::UInt128 v;
    EXPECT_EQ(v.getLowerProperty(), uint64_t(0));
    EXPECT_EQ(v.getUpperProperty(), uint64_t(0));
}

TEST(UInt128Tests, Addition) {
    System::UInt128 a(1), b(2);
    EXPECT_EQ(static_cast<unsigned long long>(a + b), 3ULL);
}

TEST(UInt128Tests, Multiplication) {
    System::UInt128 a(9), b(9);
    EXPECT_EQ(static_cast<unsigned long long>(a * b), 81ULL);
}

TEST(UInt128Tests, ToString_Zero) {
    EXPECT_EQ(System::UInt128().ToString(), "0");
}

TEST(UInt128Tests, ToString_Value) {
    System::UInt128 v(999);
    EXPECT_EQ(v.ToString(), "999");
}

TEST(UInt128Tests, Comparison) {
    System::UInt128 a(10), b(20);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
    EXPECT_EQ(a, a);
}

TEST(UInt128Tests, StaticZero_One) {
    EXPECT_EQ(static_cast<unsigned long long>(System::UInt128::Zero()), 0ULL);
    EXPECT_EQ(static_cast<unsigned long long>(System::UInt128::One()), 1ULL);
}

// ===========================================================================
// DateTimeOffset
// ===========================================================================

using System::DateTimeOffset;
using System::DateTime;
using System::TimeSpan;

TEST(DateTimeOffsetTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(DateTimeOffset dto);
}

TEST(DateTimeOffsetTests, CtorWithDateTime_StoresDateTime) {
    DateTime dt;
    DateTimeOffset dto(dt, TimeSpan());
    EXPECT_EQ(dto.getDateTimeProperty(), dt);
}

TEST(DateTimeOffsetTests, CtorWithOffset_StoresOffset) {
    DateTime dt(2020, 1, 1, 12, 0, 0);
    TimeSpan offset = TimeSpan::FromHours(2);
    DateTimeOffset dto(dt, offset);
    EXPECT_NEAR(dto.getOffsetProperty().getTotalHoursProperty(), 2.0, 1e-9);
}

TEST(DateTimeOffsetTests, Equality_SameValues) {
    DateTime dt;
    DateTimeOffset a(dt, TimeSpan()), b(dt, TimeSpan());
    EXPECT_EQ(a, b);
}

TEST(DateTimeOffsetTests, Inequality_DifferentOffset) {
    DateTime dt(2020, 1, 1, 12, 0, 0);
    DateTimeOffset a(dt, TimeSpan()), b(dt, TimeSpan::FromHours(1));
    EXPECT_NE(a, b);
}

TEST(DateTimeOffsetTests, ToString_NonEmpty) {
    DateTimeOffset dto;
    EXPECT_FALSE(dto.ToString().empty());
}
TEST(DateTimeOffsetTests, UtcNow_UtcTicks_Positive) {
    auto dto = DateTimeOffset::getUtcNowProperty();
    EXPECT_GT(dto.getUtcTicksProperty(), 0LL);
    EXPECT_EQ(dto.getOffsetProperty(), TimeSpan::Zero);
}
TEST(DateTimeOffsetTests, Now_HasOffset) {
    auto dto = DateTimeOffset::getNowProperty();
    EXPECT_GT(dto.getUtcTicksProperty(), 0LL);
}
TEST(DateTimeOffsetTests, ComponentAccessors) {
    DateTime dt(2024, 6, 15, 10, 30, 45, 0);
    TimeSpan off = TimeSpan::FromHours(2);
    DateTimeOffset dto(dt, off);
    EXPECT_EQ(dto.getYearProperty(),  2024);
    EXPECT_EQ(dto.getMonthProperty(), 6);
    EXPECT_EQ(dto.getDayProperty(),   15);
    EXPECT_EQ(dto.getHourProperty(),  10);
    EXPECT_EQ(dto.getMinuteProperty(),30);
    EXPECT_EQ(dto.getSecondProperty(),45);
}
TEST(DateTimeOffsetTests, AddHours_ShiftsTime) {
    DateTime dt(2024, 1, 1, 0, 0, 0, 0);
    DateTimeOffset dto(dt, TimeSpan::Zero);
    auto dto2 = dto.AddHours(3);
    EXPECT_EQ(dto2.getHourProperty(), 3);
}
TEST(DateTimeOffsetTests, AddDays_ShiftsDate) {
    DateTime dt(2024, 1, 10, 0, 0, 0, 0);
    DateTimeOffset dto(dt, TimeSpan::Zero);
    auto dto2 = dto.AddDays(5);
    EXPECT_EQ(dto2.getDayProperty(), 15);
}
TEST(DateTimeOffsetTests, Subtract_TwoDates_GivesTimeSpan) {
    DateTime dt1(2024, 1, 1, 0, 0, 0, 0);
    DateTime dt2(2024, 1, 2, 0, 0, 0, 0);
    DateTimeOffset a(dt1, TimeSpan::Zero), b(dt2, TimeSpan::Zero);
    TimeSpan diff = b - a;
    EXPECT_NEAR(diff.getTotalHoursProperty(), 24.0, 0.001);
}
TEST(DateTimeOffsetTests, Operators_Plus_Minus) {
    DateTime dt(2024, 6, 1, 12, 0, 0, 0);
    DateTimeOffset dto(dt, TimeSpan::Zero);
    auto dto2 = dto + TimeSpan::FromHours(1);
    EXPECT_EQ(dto2.getHourProperty(), 13);
    auto dto3 = dto2 - TimeSpan::FromHours(1);
    EXPECT_EQ(dto3.getHourProperty(), 12);
}
TEST(DateTimeOffsetTests, ComparisonOperators) {
    DateTime dt1(2024, 1, 1, 0, 0, 0, 0), dt2(2024, 1, 2, 0, 0, 0, 0);
    DateTimeOffset a(dt1, TimeSpan::Zero), b(dt2, TimeSpan::Zero);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(b >= b);
}
TEST(DateTimeOffsetTests, CompareTo) {
    DateTime dt1(2024, 1, 1, 0, 0, 0, 0), dt2(2024, 1, 2, 0, 0, 0, 0);
    DateTimeOffset a(dt1, TimeSpan::Zero), b(dt2, TimeSpan::Zero);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_GT(b.CompareTo(a), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}
TEST(DateTimeOffsetTests, Parse_ISO8601_WithOffset) {
    DateTimeOffset dto = DateTimeOffset::Parse("2024-06-15T10:30:00+02:00");
    EXPECT_EQ(dto.getYearProperty(),  2024);
    EXPECT_EQ(dto.getHourProperty(),  10);
    EXPECT_NEAR(dto.getOffsetProperty().getTotalHoursProperty(), 2.0, 0.001);
}
TEST(DateTimeOffsetTests, Parse_ISO8601_Z) {
    DateTimeOffset dto = DateTimeOffset::Parse("2024-06-15T10:30:00Z");
    EXPECT_EQ(dto.getOffsetProperty(), TimeSpan::Zero);
}
TEST(DateTimeOffsetTests, TryParse_Invalid_ReturnsFalse) {
    DateTimeOffset dto;
    EXPECT_FALSE(DateTimeOffset::TryParse("not-a-date", dto));
}
TEST(DateTimeOffsetTests, AddMonths) {
    DateTime dt(2024, 1, 31, 0, 0, 0, 0);
    DateTimeOffset dto(dt, TimeSpan::Zero);
    auto dto2 = dto.AddMonths(1);
    EXPECT_EQ(dto2.getMonthProperty(), 2);
}
TEST(DateTimeOffsetTests, AddYears) {
    DateTime dt(2024, 6, 1, 0, 0, 0, 0);
    DateTimeOffset dto(dt, TimeSpan::Zero);
    auto dto2 = dto.AddYears(2);
    EXPECT_EQ(dto2.getYearProperty(), 2026);
}
TEST(DateTimeOffsetTests, ToUniversalTime_ZeroOffset) {
    DateTime dt(2024, 6, 1, 12, 0, 0, 0);
    TimeSpan off = TimeSpan::FromHours(2);
    DateTimeOffset dto(dt, off);
    auto utc = dto.ToUniversalTime();
    EXPECT_EQ(utc.getOffsetProperty(), TimeSpan::Zero);
    EXPECT_EQ(utc.getHourProperty(), 10); // 12 - 2 = 10
}
TEST(DateTimeOffsetTests, ToString_WithFormat_O) {
    DateTime dt(2024, 6, 15, 10, 30, 0, 0);
    DateTimeOffset dto(dt, TimeSpan::FromHours(2));
    std::string s = dto.ToString(std::string("O"));
    EXPECT_NE(s.find("2024"), std::string::npos);
    EXPECT_NE(s.find("+02:00"), std::string::npos);
}

TEST(DateTimeOffsetTests, ToString_WithFormat_r_ProducesRfc1123) {
    DateTimeOffset dto(2015, 10, 21, 7, 28, 0, TimeSpan::Zero);
    EXPECT_EQ(dto.ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

// Regression tests for ticket 354: real .NET's TryFormatR/TryFormatu (DateTimeFormat.cs) convert
// to UTC (subtracting the offset) BEFORE formatting, and "R"/"r" always emits the literal "GMT"
// (RFC 1123 has no offset notation). The previous code used the ORIGINAL offset-relative clock
// time directly for BOTH "R" and "u", producing the wrong time and (for "R") an invalid trailing
// token (the raw offset instead of "GMT") for any non-zero offset.
TEST(DateTimeOffsetTests, ToString_WithFormat_r_NonZeroOffset_ConvertsToUtcAndAlwaysUsesGmt) {
    // 07:28 +02:00 == 05:28 UTC.
    DateTimeOffset dto(2015, 10, 21, 7, 28, 0, TimeSpan::FromHours(2));
    EXPECT_EQ(dto.ToString("r"), "Wed, 21 Oct 2015 05:28:00 GMT");
}

TEST(DateTimeOffsetTests, ToString_WithFormat_u_NonZeroOffset_ConvertsToUtc) {
    DateTimeOffset dto(2015, 10, 21, 7, 28, 0, TimeSpan::FromHours(2));
    EXPECT_EQ(dto.ToString("u"), "2015-10-21 05:28:00Z");
}

// Regression for ticket 354: the "O" round-trip format previously omitted the fractional-second
// component entirely, losing millisecond precision on round-trip.
TEST(DateTimeOffsetTests, ToString_WithFormat_O_IncludesMilliseconds) {
    DateTime dt(2024, 6, 15, 10, 30, 0, 123);
    DateTimeOffset dto(dt, TimeSpan::FromHours(2));
    EXPECT_EQ(dto.ToString("O"), "2024-06-15T10:30:00.123+02:00");
}

// Regression for ticket 354: TryParse's contract is to never throw, only report failure via its
// bool return -- but the DateTimeOffset(DateTime, TimeSpan) constructor it calls internally
// validates the offset (must be within +-14 hours) and throws ArgumentOutOfRangeException on
// violation. A syntactically well-formed offset that exceeds 14 hours previously let that
// exception escape TryParse uncaught.
TEST(DateTimeOffsetTests, TryParse_SyntacticallyValidButOutOfRangeOffset_ReturnsFalse) {
    DateTimeOffset dto;
    EXPECT_FALSE(DateTimeOffset::TryParse("2024-01-15T10:30:00+15:00", dto));
}

// ===========================================================================
// TimeOnly
// ===========================================================================

using System::TimeOnly;

TEST(TimeOnlyTests, DefaultCtor_AllZero) {
    TimeOnly t;
    EXPECT_EQ(t.getHourProperty(), 0);
    EXPECT_EQ(t.getMinuteProperty(), 0);
    EXPECT_EQ(t.getSecondProperty(), 0);
    EXPECT_EQ(t.getMillisecondProperty(), 0);
}

TEST(TimeOnlyTests, HourMinute_Ctor) {
    TimeOnly t(13, 45);
    EXPECT_EQ(t.getHourProperty(), 13);
    EXPECT_EQ(t.getMinuteProperty(), 45);
}

TEST(TimeOnlyTests, HourMinuteSecond_Ctor) {
    TimeOnly t(9, 30, 15);
    EXPECT_EQ(t.getSecondProperty(), 15);
}

TEST(TimeOnlyTests, ToString_Format) {
    TimeOnly t(8, 5, 3);
    EXPECT_EQ(t.ToString(), "08:05:03");
}

TEST(TimeOnlyTests, Comparison_Earlier_Less) {
    TimeOnly t1(9, 0), t2(10, 0);
    EXPECT_TRUE(t1 < t2);
    EXPECT_FALSE(t2 < t1);
}

TEST(TimeOnlyTests, Equality) {
    TimeOnly a(12, 30, 0), b(12, 30, 0);
    EXPECT_EQ(a, b);
    EXPECT_FALSE(a != b);
}

// ===========================================================================
// DBNull
// ===========================================================================

TEST(DBNullTests, Value_ReturnsSingleton) {
    auto& a = System::DBNull::Value();
    auto& b = System::DBNull::Value();
    EXPECT_EQ(&a, &b);
}

TEST(DBNullTests, ToString_ReturnsEmptyString) {
    EXPECT_EQ(System::DBNull::Value().ToString(), "");
}

TEST(DBNullTests, ToString_WithProvider_ReturnsEmptyString) {
    EXPECT_EQ(System::DBNull::Value().ToString(nullptr), "");
}

TEST(DBNullTests, GetTypeCode_IsDBNull) {
    EXPECT_EQ(System::DBNull::Value().GetTypeCode(), System::TypeCode::DBNull);
}

TEST(DBNullTests, IsA_IConvertible) {
    System::IConvertible* p = &System::DBNull::Value();
    EXPECT_NE(p, nullptr);
}

TEST(DBNullTests, ToBoolean_Throws) {
    EXPECT_THROW(System::DBNull::Value().ToBoolean(), System::InvalidCastException);
}

TEST(DBNullTests, ToInt32_Throws) {
    EXPECT_THROW(System::DBNull::Value().ToInt32(), System::InvalidCastException);
}

TEST(DBNullTests, ToDouble_Throws) {
    EXPECT_THROW(System::DBNull::Value().ToDouble(), System::InvalidCastException);
}

TEST(DBNullTests, ToChar_Throws) {
    EXPECT_THROW(System::DBNull::Value().ToChar(), System::InvalidCastException);
}

TEST(DBNullTests, ToInt64_Throws) {
    EXPECT_THROW(System::DBNull::Value().ToInt64(), System::InvalidCastException);
}

TEST(DBNullTests, ToSingle_Throws) {
    EXPECT_THROW(System::DBNull::Value().ToSingle(), System::InvalidCastException);
}

// ===========================================================================
// FormattableString
// ===========================================================================

TEST(FormattableStringTests, Format_StoredCorrectly) {
    System::FormattableString fs("{0} and {1}", {"hello", "world"});
    EXPECT_EQ(fs.getFormatProperty(), "{0} and {1}");
}

TEST(FormattableStringTests, ArgumentCount_Correct) {
    System::FormattableString fs("{0}", {"a"});
    EXPECT_EQ(fs.getArgumentCountProperty(), 1);
}

TEST(FormattableStringTests, GetArgument_ByIndex) {
    System::FormattableString fs("{0} {1}", {"foo", "bar"});
    EXPECT_EQ(fs.GetArgument(0), "foo");
    EXPECT_EQ(fs.GetArgument(1), "bar");
}

TEST(FormattableStringTests, ToString_SubstitutesPlaceholders) {
    System::FormattableString fs("Hello, {0}!", {"World"});
    EXPECT_EQ(fs.ToString(), "Hello, World!");
}

TEST(FormattableStringTests, Invariant_ReturnsToString) {
    System::FormattableString fs("{0}+{1}={2}", {"1", "2", "3"});
    EXPECT_EQ(System::FormattableString::Invariant(fs), "1+2=3");
}

TEST(FormattableStringTests, GetArguments_ReturnsAllArgs) {
    System::FormattableString fs("{0} {1}", {"alpha", "beta"});
    auto args = fs.GetArguments();
    ASSERT_EQ(static_cast<int>(args.size()), 2);
    EXPECT_EQ(args[0], "alpha");
    EXPECT_EQ(args[1], "beta");
}

TEST(FormattableStringTests, GetArguments_EmptyWhenNoArgs) {
    System::FormattableString fs("no placeholders");
    EXPECT_TRUE(fs.GetArguments().empty());
}

TEST(FormattableStringTests, ToString_WithNullProvider_SameAsToString) {
    System::FormattableString fs("value={0}", {"42"});
    EXPECT_EQ(fs.ToString(nullptr), "value=42");
}

TEST(FormattableStringTests, CurrentCulture_ReturnsToString) {
    System::FormattableString fs("{0} world", {"hello"});
    EXPECT_EQ(System::FormattableString::CurrentCulture(fs), "hello world");
}

TEST(FormattableStringTests, GetArgument_OutOfRange_Throws) {
    System::FormattableString fs("{0}", {"x"});
    EXPECT_THROW(fs.GetArgument(5), System::IndexOutOfRangeException);
}

// ===========================================================================
// FormattableStringFactory
// ===========================================================================

TEST(FormattableStringFactoryTests, Create_NoArgs) {
    auto fs = System::Runtime::CompilerServices::FormattableStringFactory::Create("hello");
    EXPECT_EQ(fs.getFormatProperty(), "hello");
    EXPECT_EQ(fs.getArgumentCountProperty(), 0);
}

TEST(FormattableStringFactoryTests, Create_WithArgs_ToString) {
    auto fs = System::Runtime::CompilerServices::FormattableStringFactory::Create(
        "{0} + {1} = {2}", {"1", "2", "3"});
    EXPECT_EQ(fs.ToString(), "1 + 2 = 3");
}

TEST(FormattableStringFactoryTests, Create_PreservesArguments) {
    auto fs = System::Runtime::CompilerServices::FormattableStringFactory::Create(
        "{0}", {"hello"});
    EXPECT_EQ(fs.GetArgument(0), "hello");
}

// ===========================================================================
// OperatingSystem
// ===========================================================================

using System::OperatingSystem;
using System::PlatformID;
using System::Version;

TEST(OperatingSystemTests, Platform_StoredCorrectly) {
    OperatingSystem os(PlatformID::Unix, Version(5, 0));
    EXPECT_EQ(os.getPlatformProperty(), PlatformID::Unix);
}

TEST(OperatingSystemTests, Version_StoredCorrectly) {
    OperatingSystem os(PlatformID::Unix, Version(5, 4));
    EXPECT_EQ(os.getVersionProperty().Major, 5);
    EXPECT_EQ(os.getVersionProperty().Minor, 4);
}

TEST(OperatingSystemTests, IsLinux_TrueOnLinux) {
#ifdef __linux__
    EXPECT_TRUE(OperatingSystem::IsLinux());
#else
    EXPECT_FALSE(OperatingSystem::IsLinux());
#endif
}

TEST(OperatingSystemTests, IsWindows_FalseOnLinux) {
#ifdef __linux__
    EXPECT_FALSE(OperatingSystem::IsWindows());
#endif
}

TEST(OperatingSystemTests, IsAndroid_FalseOnNonAndroidBuild) {
    EXPECT_FALSE(OperatingSystem::IsAndroid());
}

TEST(OperatingSystemTests, VersionString_ContainsPlatformName) {
    OperatingSystem os(PlatformID::Unix, Version(1, 0));
    EXPECT_NE(os.getVersionStringProperty().find("Unix"), std::string::npos);
}
TEST(OperatingSystemTests, Clone_ReturnsCopy) {
    OperatingSystem os(PlatformID::Unix, Version(3, 1));
    auto copy = os.Clone();
    ASSERT_NE(copy, nullptr);
    EXPECT_EQ(copy->getPlatformProperty(), PlatformID::Unix);
    EXPECT_EQ(copy->getVersionProperty().Major, 3);
}
TEST(OperatingSystemTests, ToString_ContainsPlatformName) {
    OperatingSystem os(PlatformID::Win32NT, Version(10, 0));
    EXPECT_NE(os.ToString().find("Windows"), std::string::npos);
}
TEST(OperatingSystemTests, IsWasi_False) {
    EXPECT_FALSE(OperatingSystem::IsWasi());
}
TEST(OperatingSystemTests, IsTvOS_False) {
    EXPECT_FALSE(OperatingSystem::IsTvOS());
}
TEST(OperatingSystemTests, IsWatchOS_False) {
    EXPECT_FALSE(OperatingSystem::IsWatchOS());
}
TEST(OperatingSystemTests, IsMacCatalyst_False) {
    EXPECT_FALSE(OperatingSystem::IsMacCatalyst());
}
TEST(OperatingSystemTests, IsOSPlatform_Linux_CaseInsensitive) {
#ifdef __linux__
    EXPECT_TRUE(OperatingSystem::IsOSPlatform("linux"));
    EXPECT_TRUE(OperatingSystem::IsOSPlatform("Linux"));
    EXPECT_TRUE(OperatingSystem::IsOSPlatform("LINUX"));
#else
    EXPECT_FALSE(OperatingSystem::IsOSPlatform("linux"));
#endif
}
TEST(OperatingSystemTests, IsOSPlatform_UnknownPlatform_False) {
    EXPECT_FALSE(OperatingSystem::IsOSPlatform("Haiku"));
    EXPECT_FALSE(OperatingSystem::IsOSPlatform(""));
}
TEST(OperatingSystemTests, IsOSPlatform_Android_False) {
    EXPECT_FALSE(OperatingSystem::IsOSPlatform("android"));
}
TEST(OperatingSystemTests, IsWindowsVersionAtLeast_MatchesIsWindows) {
    EXPECT_EQ(OperatingSystem::IsWindowsVersionAtLeast(10), OperatingSystem::IsWindows());
}
TEST(OperatingSystemTests, IsAndroidVersionAtLeast_AlwaysFalse) {
    EXPECT_FALSE(OperatingSystem::IsAndroidVersionAtLeast(9));
}
TEST(OperatingSystemTests, IsOSPlatformVersionAtLeast_Linux) {
#ifdef __linux__
    EXPECT_TRUE(OperatingSystem::IsOSPlatformVersionAtLeast("linux", 4));
#else
    EXPECT_FALSE(OperatingSystem::IsOSPlatformVersionAtLeast("linux", 4));
#endif
}
TEST(OperatingSystemTests, IsOSPlatformVersionAtLeast_Android_False) {
    EXPECT_FALSE(OperatingSystem::IsOSPlatformVersionAtLeast("android", 12));
}

// ===========================================================================
// BFloat16
// ===========================================================================

using System::Numerics::BFloat16;

TEST(BFloat16Tests, Zero_IsZeroFloat) {
    EXPECT_NEAR(static_cast<float>(BFloat16::Zero()), 0.0f, 1e-6f);
}

TEST(BFloat16Tests, One_IsOneFloat) {
    EXPECT_NEAR(static_cast<float>(BFloat16::One()), 1.0f, 1e-2f);
}

TEST(BFloat16Tests, NegativeOne_IsNegOne) {
    EXPECT_NEAR(static_cast<float>(BFloat16::NegativeOne()), -1.0f, 1e-2f);
}

TEST(BFloat16Tests, FromFloat_RoundTrip) {
    BFloat16 v(2.0f);
    EXPECT_NEAR(static_cast<float>(v), 2.0f, 0.1f);
}

TEST(BFloat16Tests, Addition) {
    BFloat16 a(1.0f), b(2.0f);
    float result = static_cast<float>(a + b);
    EXPECT_NEAR(result, 3.0f, 0.1f);
}

TEST(BFloat16Tests, IsNaN_NaN) {
    EXPECT_TRUE(BFloat16::IsNaN(BFloat16::NaN()));
}

TEST(BFloat16Tests, IsNaN_NonNaN) {
    EXPECT_FALSE(BFloat16::IsNaN(BFloat16::One()));
}

TEST(BFloat16Tests, NaN_UsesNegativeSignBit) {
    EXPECT_EQ(BFloat16::NaN().getBitsProperty(), uint16_t(0xFFC0u));
}

TEST(BFloat16Tests, Equality_NaNIsNeverEqualToItself) {
    EXPECT_FALSE(BFloat16::NaN() == BFloat16::NaN());
    EXPECT_TRUE(BFloat16::NaN() != BFloat16::NaN());
}

TEST(BFloat16Tests, Equality_PositiveAndNegativeZeroAreEqual) {
    BFloat16 posZero(uint16_t(0x0000u));
    BFloat16 negZero(uint16_t(0x8000u));
    EXPECT_TRUE(posZero == negZero);
    EXPECT_FALSE(posZero != negZero);
}

TEST(BFloat16Tests, IsInfinity_PosInf) {
    EXPECT_TRUE(BFloat16::IsPositiveInfinity(BFloat16::PositiveInfinity()));
}

TEST(BFloat16Tests, IsNegativeInfinity) {
    EXPECT_TRUE(BFloat16::IsNegativeInfinity(BFloat16::NegativeInfinity()));
}

TEST(BFloat16Tests, Comparison_LessThan) {
    EXPECT_TRUE(BFloat16(1.0f) < BFloat16(2.0f));
    EXPECT_FALSE(BFloat16(2.0f) < BFloat16(1.0f));
}

TEST(BFloat16Tests, Negate) {
    BFloat16 v(3.0f);
    EXPECT_NEAR(static_cast<float>(-v), -3.0f, 0.2f);
}

// ===========================================================================
// DivisionRounding
// ===========================================================================

using System::Numerics::DivisionRounding;

TEST(DivisionRoundingTests, Values) {
    EXPECT_EQ(static_cast<int>(DivisionRounding::Truncate), 0);
    EXPECT_EQ(static_cast<int>(DivisionRounding::Floor),    1);
    EXPECT_EQ(static_cast<int>(DivisionRounding::Ceiling),  2);
}

// ===========================================================================
// StringComparer
// ===========================================================================

TEST(StringComparerTests, Ordinal_Compare_Less) {
    auto cmp = System::StringComparer::Ordinal();
    EXPECT_LT(cmp->Compare("abc", "abd"), 0);
}

TEST(StringComparerTests, Ordinal_Compare_Equal) {
    auto cmp = System::StringComparer::Ordinal();
    EXPECT_EQ(cmp->Compare("abc", "abc"), 0);
}

TEST(StringComparerTests, Ordinal_Equals_True) {
    auto cmp = System::StringComparer::Ordinal();
    EXPECT_TRUE(cmp->Equals("hello", "hello"));
}

TEST(StringComparerTests, Ordinal_Equals_False) {
    auto cmp = System::StringComparer::Ordinal();
    EXPECT_FALSE(cmp->Equals("Hello", "hello"));
}

TEST(StringComparerTests, OrdinalIgnoreCase_Equals_CaseInsensitive) {
    auto cmp = System::StringComparer::OrdinalIgnoreCase();
    EXPECT_TRUE(cmp->Equals("ABC", "abc"));
}

TEST(StringComparerTests, OrdinalIgnoreCase_Compare_Equal) {
    auto cmp = System::StringComparer::OrdinalIgnoreCase();
    EXPECT_EQ(cmp->Compare("XYZ", "xyz"), 0);
}

TEST(StringComparerTests, GetHashCode_SameInput_SameHash) {
    auto cmp = System::StringComparer::Ordinal();
    EXPECT_EQ(cmp->GetHashCode("test"), cmp->GetHashCode("test"));
}

TEST(StringComparerTests, InvariantCulture_IsSameAsOrdinal) {
    auto a = System::StringComparer::InvariantCulture();
    EXPECT_TRUE(a->Equals("foo", "foo"));
}

// ===========================================================================
// Progress<T>
// ===========================================================================

TEST(ProgressTests, Report_InvokesHandler) {
    int received = -1;
    System::Progress<int> p([&received](int v) { received = v; });
    p.Report(42);
    EXPECT_EQ(received, 42);
}

TEST(ProgressTests, DefaultCtor_Report_NoThrow) {
    System::Progress<int> p;
    EXPECT_NO_THROW(p.Report(1));
}

TEST(ProgressTests, AddHandler_BothCalled) {
    int count = 0;
    System::Progress<int> p([&count](int) { ++count; });
    p.addProgressChangedHandler([&count](int) { ++count; });
    p.Report(0);
    EXPECT_EQ(count, 2);
}
TEST(ProgressTests, MultipleEventHandlers_AllCalled) {
    int sum = 0;
    System::Progress<int> p;
    p.addProgressChangedHandler([&sum](int v) { sum += v; });
    p.addProgressChangedHandler([&sum](int v) { sum += v * 2; });
    p.Report(3);
    EXPECT_EQ(sum, 9); // 3 + 6
}
TEST(ProgressTests, IProgress_Polymorphic) {
    int received = -1;
    System::Progress<int> p([&received](int v) { received = v; });
    System::IProgress<int>* ip = &p;
    ip->Report(77);
    EXPECT_EQ(received, 77);
}
TEST(ProgressTests, OnReport_Override_Called) {
    struct TrackingProgress : System::Progress<int> {
        int onReportCalls = 0;
    protected:
        void OnReport(const int& value) override {
            ++onReportCalls;
            System::Progress<int>::OnReport(value);
        }
    };
    int received = -1;
    TrackingProgress p;
    p.addProgressChangedHandler([&received](int v) { received = v; });
    p.Report(5);
    EXPECT_EQ(p.onReportCalls, 1);
    EXPECT_EQ(received, 5);
}
TEST(ProgressTests, Report_StringType) {
    std::string last;
    System::Progress<std::string> p([&last](std::string s) { last = std::move(s); });
    p.Report("hello");
    EXPECT_EQ(last, "hello");
}

TEST(ProgressTests, Report_MultipleValues_AllReceived) {
    std::vector<int> got;
    System::Progress<int> p([&got](int v) { got.push_back(v); });
    p.Report(1);
    p.Report(2);
    p.Report(3);
    ASSERT_EQ(got.size(), 3u);
    EXPECT_EQ(got[0], 1);
    EXPECT_EQ(got[1], 2);
    EXPECT_EQ(got[2], 3);
}

TEST(ProgressTests, Report_DoubleType_Works) {
    double received = 0.0;
    System::Progress<double> p([&received](double v) { received = v; });
    p.Report(3.14);
    EXPECT_DOUBLE_EQ(received, 3.14);
}

TEST(IProgressTests, IsAbstract) {
    static_assert(std::is_abstract_v<System::IProgress<int>>,
                  "IProgress<T> must be abstract");
}

TEST(IProgressTests, Progress_IsA_IProgress) {
    static_assert(std::is_base_of_v<System::IProgress<int>, System::Progress<int>>,
                  "Progress<T> must inherit IProgress<T>");
}

// ===========================================================================
// UnicodeRange / UnicodeRanges
// ===========================================================================

using System::Text::Unicode::UnicodeRange;
using System::Text::Unicode::UnicodeRanges;

TEST(UnicodeRangeTests, Ctor_StoresValues) {
    UnicodeRange r(0x0041, 26);
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0041);
    EXPECT_EQ(r.getLengthProperty(), 26);
}

TEST(UnicodeRangeTests, Create_AtoZ) {
    auto r = UnicodeRange::Create(u'A', u'Z');
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0041);
    EXPECT_EQ(r.getLengthProperty(), 26);
}

TEST(UnicodeRangeTests, Create_InvalidOrder_Throws) {
    EXPECT_THROW(UnicodeRange::Create(u'Z', u'A'), System::ArgumentOutOfRangeException);
}

TEST(UnicodeRangesTests, All_Covers_BMP) {
    auto r = UnicodeRanges::getAllProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0000);
    EXPECT_EQ(r.getLengthProperty(), 0x10000);
}

TEST(UnicodeRangesTests, BasicLatin_Starts0) {
    auto r = UnicodeRanges::getBasicLatinProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0000);
    EXPECT_EQ(r.getLengthProperty(), 128);
}

TEST(UnicodeRangesTests, Cyrillic_StartsAt0x0400) {
    auto r = UnicodeRanges::getCyrillicProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0400);
}

TEST(UnicodeRangesTests, None_LengthZero) {
    auto r = UnicodeRanges::getNoneProperty();
    EXPECT_EQ(r.getLengthProperty(), 0);
}

// ===========================================================================
// CancellationTokenRegistration
// ===========================================================================

using System::Threading::CancellationTokenRegistration;

TEST(CancellationTokenRegistrationTests, DefaultCtor_IsNotActive) {
    // A default-constructed registration has no associated callback to unregister.
    CancellationTokenRegistration reg;
    EXPECT_FALSE(reg.getIsActiveProperty());
}

TEST(CancellationTokenRegistrationTests, Dispose_BecomesInactive) {
    CancellationTokenRegistration reg;
    reg.Dispose();
    EXPECT_FALSE(reg.getIsActiveProperty());
}

TEST(CancellationTokenRegistrationTests, Unregister_BecomesInactive) {
    CancellationTokenRegistration reg;
    reg.Unregister();
    EXPECT_FALSE(reg.getIsActiveProperty());
}

// ===========================================================================
// KeyNotFoundException
// ===========================================================================

using System::Collections::Generic::KeyNotFoundException;

TEST(KeyNotFoundExceptionTests, DefaultCtor_WhatNonEmpty) {
    KeyNotFoundException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(KeyNotFoundExceptionTests, MessageCtor_WhatContainsMessage) {
    KeyNotFoundException ex("key 'foo' not found");
    EXPECT_NE(std::string(ex.what()).find("foo"), std::string::npos);
}

TEST(KeyNotFoundExceptionTests, IsA_SystemException) {
    EXPECT_THROW(throw KeyNotFoundException(), System::SystemException);
}

// ===========================================================================
// ReferenceEqualityComparer<T>
// ===========================================================================

using System::Collections::Generic::ReferenceEqualityComparer;

TEST(ReferenceEqualityComparerTests, SamePointer_Equals_True) {
    int x = 5;
    ReferenceEqualityComparer<int> cmp;
    EXPECT_TRUE(cmp.Equals(&x, &x));
}

TEST(ReferenceEqualityComparerTests, DifferentPointer_Equals_False) {
    int a = 1, b = 1;
    ReferenceEqualityComparer<int> cmp;
    EXPECT_FALSE(cmp.Equals(&a, &b));
}

TEST(ReferenceEqualityComparerTests, GetHashCode_SamePtr_SameHash) {
    int x = 7;
    ReferenceEqualityComparer<int> cmp;
    EXPECT_EQ(cmp.GetHashCode(&x), cmp.GetHashCode(&x));
}

TEST(ReferenceEqualityComparerTests, Instance_ReturnsSingleton) {
    auto& a = ReferenceEqualityComparer<int>::Instance();
    auto& b = ReferenceEqualityComparer<int>::Instance();
    EXPECT_EQ(&a, &b);
}

// ===========================================================================
// ReadonlyProperty (SharpRuntime::Experimental)
// ===========================================================================

using SharpRuntime::Experimental::ReadOnlyProperty;

TEST(ReadOnlyPropertyTests, Getter_ReturnsValue) {
    ReadOnlyProperty<int> p([](){ return 42; });
    EXPECT_EQ(p.get(), 42);
}

TEST(ReadOnlyPropertyTests, ImplicitConversion_Works) {
    ReadOnlyProperty<std::string> p([](){ return std::string("hi"); });
    std::string v = p;
    EXPECT_EQ(v, "hi");
}
