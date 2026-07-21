// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Half.hpp"
#include "System/Span.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include <cmath>
#include <limits>

using System::Half;

// Additional Half tests (basic FromSingle/comparison already in Task40Tests.cpp)

TEST(HalfNewTests, IsNaN_NaN_True) { EXPECT_TRUE(Half::IsNaN(Half::NaN)); }
TEST(HalfNewTests, IsNaN_Zero_False) { EXPECT_FALSE(Half::IsNaN(Half::Zero)); }
TEST(HalfNewTests, IsNaN_PositiveInfinity_False) {
    EXPECT_FALSE(Half::IsNaN(Half::PositiveInfinity));
}

TEST(HalfNewTests, IsInfinity_PositiveInfinity_True) {
    EXPECT_TRUE(Half::IsInfinity(Half::PositiveInfinity));
}
TEST(HalfNewTests, IsInfinity_NegativeInfinity_True) {
    EXPECT_TRUE(Half::IsInfinity(Half::NegativeInfinity));
}
TEST(HalfNewTests, IsInfinity_One_False) {
    EXPECT_FALSE(Half::IsInfinity(Half::FromSingle(1.0f)));
}

TEST(HalfNewTests, IsPositiveInfinity_True) {
    EXPECT_TRUE(Half::IsPositiveInfinity(Half::PositiveInfinity));
}
TEST(HalfNewTests, IsPositiveInfinity_NegInf_False) {
    EXPECT_FALSE(Half::IsPositiveInfinity(Half::NegativeInfinity));
}

TEST(HalfNewTests, IsNegativeInfinity_True) {
    EXPECT_TRUE(Half::IsNegativeInfinity(Half::NegativeInfinity));
}
TEST(HalfNewTests, IsNegativeInfinity_PosInf_False) {
    EXPECT_FALSE(Half::IsNegativeInfinity(Half::PositiveInfinity));
}

TEST(HalfNewTests, IsFinite_One_True) {
    EXPECT_TRUE(Half::IsFinite(Half::FromSingle(1.0f)));
}
TEST(HalfNewTests, IsFinite_Infinity_False) {
    EXPECT_FALSE(Half::IsFinite(Half::PositiveInfinity));
}
TEST(HalfNewTests, IsFinite_NaN_False) {
    EXPECT_FALSE(Half::IsFinite(Half::NaN));
}

TEST(HalfNewTests, IsNegative_NegInf_True) {
    EXPECT_TRUE(Half::IsNegative(Half::NegativeInfinity));
}
TEST(HalfNewTests, IsNegative_Zero_False) {
    EXPECT_FALSE(Half::IsNegative(Half::Zero));
}
TEST(HalfNewTests, IsNegative_NegativeOne_True) {
    EXPECT_TRUE(Half::IsNegative(Half::FromSingle(-1.0f)));
}

TEST(HalfNewTests, GetHashCode_SameValue_SameHash) {
    Half a = Half::FromSingle(3.14f);
    Half b = Half::FromSingle(3.14f);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(HalfNewTests, ToString_NonEmpty) {
    Half h = Half::FromSingle(1.5f);
    EXPECT_FALSE(h.ToString().empty());
}

TEST(HalfNewTests, MaxValue_RoundTrip_GreaterThanZero) {
    EXPECT_GT(Half::MaxValue.ToSingle(), 0.0f);
}
TEST(HalfNewTests, MinValue_RoundTrip_LessThanZero) {
    EXPECT_LT(Half::MinValue.ToSingle(), 0.0f);
}
TEST(HalfNewTests, Epsilon_RoundTrip_GreaterThanZero) {
    EXPECT_GT(Half::Epsilon.ToSingle(), 0.0f);
}

// ---------------------------------------------------------------------------
// Known .NET bit-pattern constants (from Half.cs's private *Bits fields and
// HalfTests.cs's BitConverter.HalfToUInt16Bits assertions).
// ---------------------------------------------------------------------------

TEST(HalfNewTests, KnownBits_Epsilon) { EXPECT_EQ(Half::Epsilon.bits, 0x0001u); }
TEST(HalfNewTests, KnownBits_PositiveInfinity) { EXPECT_EQ(Half::PositiveInfinity.bits, 0x7C00u); }
TEST(HalfNewTests, KnownBits_NegativeInfinity) { EXPECT_EQ(Half::NegativeInfinity.bits, 0xFC00u); }
TEST(HalfNewTests, KnownBits_NaN) { EXPECT_EQ(Half::NaN.bits, 0xFE00u); }
TEST(HalfNewTests, KnownBits_MinValue) { EXPECT_EQ(Half::MinValue.bits, 0xFBFFu); }
TEST(HalfNewTests, KnownBits_MaxValue) { EXPECT_EQ(Half::MaxValue.bits, 0x7BFFu); }
TEST(HalfNewTests, KnownBits_One) { EXPECT_EQ(Half::One.bits, 0x3C00u); }
TEST(HalfNewTests, KnownBits_NegativeOne) { EXPECT_EQ(Half::NegativeOne.bits, 0xBC00u); }
TEST(HalfNewTests, KnownBits_NegativeZero) { EXPECT_EQ(Half::NegativeZero.bits, 0x8000u); }
TEST(HalfNewTests, KnownBits_E) { EXPECT_EQ(Half::E.bits, 0x4170u); }
TEST(HalfNewTests, KnownBits_Pi) { EXPECT_EQ(Half::Pi.bits, 0x4248u); }
TEST(HalfNewTests, KnownBits_Tau) { EXPECT_EQ(Half::Tau.bits, 0x4648u); }

TEST(HalfNewTests, FromSingle_OnePointZero_MatchesKnownBits) {
    EXPECT_EQ(Half::FromSingle(1.0f).bits, 0x3C00u);
}
TEST(HalfNewTests, FromSingle_TwoPointZero_MatchesKnownBits) {
    EXPECT_EQ(Half::FromSingle(2.0f).bits, 0x4000u);
}
TEST(HalfNewTests, FromSingle_NegativeOne_MatchesKnownBits) {
    EXPECT_EQ(Half::FromSingle(-1.0f).bits, 0xBC00u);
}
TEST(HalfNewTests, FromSingle_Pi_MatchesKnownBits) {
    // 0x4248 = 3.140625, the correctly-rounded Half nearest to float pi (3.1415927f).
    EXPECT_EQ(Half::FromSingle(3.14159265358979323846f).bits, 0x4248u);
}
TEST(HalfNewTests, FromSingle_E_MatchesKnownBits) {
    EXPECT_EQ(Half::FromSingle(2.71828182845904523536f).bits, 0x4170u);
}
TEST(HalfNewTests, FromSingle_HugeValue_IsInfinity) {
    EXPECT_TRUE(Half::IsPositiveInfinity(Half::FromSingle(1e30f)));
    EXPECT_TRUE(Half::IsNegativeInfinity(Half::FromSingle(-1e30f)));
}
TEST(HalfNewTests, FromSingle_TinyValue_IsZero) {
    EXPECT_EQ(Half::FromSingle(1e-30f).bits, 0x0000u);
    EXPECT_EQ(Half::FromSingle(-1e-30f).bits, 0x8000u);
}
TEST(HalfNewTests, FromSingle_NaN_PreservesSign) {
    EXPECT_EQ(Half::FromSingle(-std::numeric_limits<float>::quiet_NaN()).bits, 0xFE00u);
}

// Subnormal half handling: the old implementation mis-scaled subnormals by treating
// the raw mantissa bits as a *float* subnormal (wrong by a factor of ~2^85). Verify the
// magnitude is now actually correct, not just "greater than zero".
TEST(HalfNewTests, Epsilon_ToSingle_HasCorrectMagnitude) {
    // Half.Epsilon = 2^-24 ~= 5.9604645e-8.
    EXPECT_NEAR(Half::Epsilon.ToSingle(), 5.9604645e-8f, 1e-14f);
}
TEST(HalfNewTests, MaxSubnormal_ToSingle_HasCorrectMagnitude) {
    // bits 0x03FF: largest subnormal, value = 1023 * 2^-24.
    Half h(0x03FF);
    EXPECT_NEAR(h.ToSingle(), 1023.0f * std::pow(2.0f, -24.0f), 1e-12f);
}
TEST(HalfNewTests, SubnormalRoundTrip_FromSingle) {
    // A value squarely in Half's subnormal range should round-trip through FromSingle/ToSingle.
    float tiny = 20.0f * 5.9604645e-8f; // 20 * Epsilon
    Half h = Half::FromSingle(tiny);
    EXPECT_TRUE(Half::IsSubnormal(h));
    EXPECT_NEAR(h.ToSingle(), tiny, 1e-12f);
}

TEST(HalfNewTests, IsNormal_One_True) { EXPECT_TRUE(Half::IsNormal(Half::One)); }
TEST(HalfNewTests, IsNormal_Subnormal_False) { EXPECT_FALSE(Half::IsNormal(Half::Epsilon)); }
TEST(HalfNewTests, IsNormal_Zero_False) { EXPECT_FALSE(Half::IsNormal(Half::Zero)); }
TEST(HalfNewTests, IsSubnormal_Epsilon_True) { EXPECT_TRUE(Half::IsSubnormal(Half::Epsilon)); }
TEST(HalfNewTests, IsSubnormal_One_False) { EXPECT_FALSE(Half::IsSubnormal(Half::One)); }
TEST(HalfNewTests, IsSubnormal_Zero_False) { EXPECT_FALSE(Half::IsSubnormal(Half::Zero)); }

// ---------------------------------------------------------------------------
// operator== IEEE 754 semantics (the old bit-exact operator== got both of these wrong).
// ---------------------------------------------------------------------------

TEST(HalfNewTests, OperatorEquals_PositiveAndNegativeZero_AreEqual) {
    EXPECT_TRUE(Half::Zero == Half::NegativeZero);
    EXPECT_FALSE(Half::Zero != Half::NegativeZero);
}
TEST(HalfNewTests, OperatorEquals_NaN_NotEqualToItself) {
    EXPECT_FALSE(Half::NaN == Half::NaN);
    EXPECT_TRUE(Half::NaN != Half::NaN);
}
TEST(HalfNewTests, Equals_NaN_EqualToItself) {
    // Half.Equals (unlike operator==) treats NaN as equal to NaN, matching .NET.
    EXPECT_TRUE(Half::NaN.Equals(Half::NaN));
}
TEST(HalfNewTests, Equals_PositiveAndNegativeZero_AreEqual) {
    EXPECT_TRUE(Half::Zero.Equals(Half::NegativeZero));
}
TEST(HalfNewTests, Equals_DifferentValues_False) {
    EXPECT_FALSE(Half::One.Equals(Half::FromSingle(2.0f)));
}

// ---------------------------------------------------------------------------
// CompareTo
// ---------------------------------------------------------------------------

TEST(HalfNewTests, CompareTo_Less) { EXPECT_LT(Half::One.CompareTo(Half::FromSingle(2.0f)), 0); }
TEST(HalfNewTests, CompareTo_Greater) { EXPECT_GT(Half::FromSingle(2.0f).CompareTo(Half::One), 0); }
TEST(HalfNewTests, CompareTo_Equal) { EXPECT_EQ(Half::One.CompareTo(Half::One), 0); }
TEST(HalfNewTests, CompareTo_NaN_SortsBelowEverythingElse) {
    // Matches .NET: when `this` is NaN and `other` isn't, CompareTo returns -1 (NaN sorts
    // first in ascending order), the same convention as Double/Single.CompareTo.
    EXPECT_LT(Half::NaN.CompareTo(Half::MaxValue), 0);
    EXPECT_GT(Half::MaxValue.CompareTo(Half::NaN), 0);
}
TEST(HalfNewTests, CompareTo_NaN_EqualsNaN) {
    EXPECT_EQ(Half::NaN.CompareTo(Half::NaN), 0);
}

// ---------------------------------------------------------------------------
// GetHashCode: all NaNs and both zeros normalize to the same hash.
// ---------------------------------------------------------------------------

TEST(HalfNewTests, GetHashCode_ZerosMatch) {
    EXPECT_EQ(Half::Zero.GetHashCode(), Half::NegativeZero.GetHashCode());
}
// Regression test (ticket 268): real Half.GetHashCode() does `bits &= PositiveInfinityBits`
// (an AND-mask), not an assignment to PositiveInfinityBits (0x7C00) -- verified against
// Half.cs directly. For NaN the exponent field is already all-ones, so the mask is a no-op in
// effect (0x7C00 survives). But for zero (+0 or -0, exponent field 0), the mask clears
// everything to 0 -- NOT 0x7C00, which is what this test previously (incorrectly) asserted.
// Both +0 and -0 still hash equally to each other either way (satisfying the hash contract,
// since Equals() treats them as equal), so this was a real divergence from .NET's actual hash
// *value*, not a broken hash table.
TEST(HalfNewTests, GetHashCode_NaNMatchesPositiveInfinityPattern) {
    EXPECT_EQ(Half::NaN.GetHashCode(), static_cast<SharpRuntime::intcs>(0x7C00));
    EXPECT_EQ(Half::Zero.GetHashCode(), static_cast<SharpRuntime::intcs>(0));
}

TEST(HalfNewTests, GetHashCode_PositiveAndNegativeZero_Match) {
    EXPECT_EQ(Half::Zero.GetHashCode(), Half::NegativeZero.GetHashCode());
}

// ---------------------------------------------------------------------------
// Arithmetic operators
// ---------------------------------------------------------------------------

TEST(HalfNewTests, OperatorAdd) {
    EXPECT_FLOAT_EQ((Half::FromSingle(1.5f) + Half::FromSingle(2.5f)).ToSingle(), 4.0f);
}
TEST(HalfNewTests, OperatorSubtract) {
    EXPECT_FLOAT_EQ((Half::FromSingle(5.0f) - Half::FromSingle(2.0f)).ToSingle(), 3.0f);
}
TEST(HalfNewTests, OperatorMultiply) {
    EXPECT_FLOAT_EQ((Half::FromSingle(3.0f) * Half::FromSingle(4.0f)).ToSingle(), 12.0f);
}
TEST(HalfNewTests, OperatorDivide) {
    EXPECT_FLOAT_EQ((Half::FromSingle(10.0f) / Half::FromSingle(4.0f)).ToSingle(), 2.5f);
}
TEST(HalfNewTests, OperatorModulo) {
    EXPECT_FLOAT_EQ((Half::FromSingle(5.0f) % Half::FromSingle(3.0f)).ToSingle(), 2.0f);
}
TEST(HalfNewTests, OperatorUnaryMinus) {
    EXPECT_EQ((-Half::One).bits, Half::NegativeOne.bits);
}
TEST(HalfNewTests, OperatorUnaryPlus) {
    EXPECT_EQ((+Half::One).bits, Half::One.bits);
}
TEST(HalfNewTests, OperatorPreIncrement) {
    Half h = Half::One;
    Half& ref = ++h;
    EXPECT_FLOAT_EQ(h.ToSingle(), 2.0f);
    EXPECT_EQ(&ref, &h);
}
TEST(HalfNewTests, OperatorPostIncrement) {
    Half h = Half::One;
    Half old = h++;
    EXPECT_FLOAT_EQ(old.ToSingle(), 1.0f);
    EXPECT_FLOAT_EQ(h.ToSingle(), 2.0f);
}
TEST(HalfNewTests, OperatorPreDecrement) {
    Half h = Half::FromSingle(2.0f);
    --h;
    EXPECT_FLOAT_EQ(h.ToSingle(), 1.0f);
}
TEST(HalfNewTests, OperatorPostDecrement) {
    Half h = Half::FromSingle(2.0f);
    Half old = h--;
    EXPECT_FLOAT_EQ(old.ToSingle(), 2.0f);
    EXPECT_FLOAT_EQ(h.ToSingle(), 1.0f);
}

// ---------------------------------------------------------------------------
// Double conversions
// ---------------------------------------------------------------------------

TEST(HalfNewTests, ToDouble_MatchesToSingle) {
    Half h = Half::FromSingle(3.5f);
    EXPECT_DOUBLE_EQ(h.ToDouble(), 3.5);
}
TEST(HalfNewTests, FromDouble_RoundTrips) {
    Half h = Half::FromDouble(3.5);
    EXPECT_FLOAT_EQ(h.ToSingle(), 3.5f);
}
TEST(HalfNewTests, ExplicitOperatorDouble) {
    Half h = Half::FromSingle(2.25f);
    EXPECT_DOUBLE_EQ(static_cast<double>(h), 2.25);
}

// ---------------------------------------------------------------------------
// Parse / TryParse
// ---------------------------------------------------------------------------

TEST(HalfNewTests, Parse_ValidString) {
    EXPECT_FLOAT_EQ(Half::Parse("3.5").ToSingle(), 3.5f);
}
TEST(HalfNewTests, Parse_InvalidString_Throws) {
    EXPECT_THROW(Half::Parse("not-a-number"), std::exception);
}
TEST(HalfNewTests, TryParse_Valid) {
    Half h;
    EXPECT_TRUE(Half::TryParse("2.5", h));
    EXPECT_FLOAT_EQ(h.ToSingle(), 2.5f);
}
TEST(HalfNewTests, TryParse_Invalid_ReturnsFalse) {
    Half h;
    EXPECT_FALSE(Half::TryParse("garbage", h));
}

// ---------------------------------------------------------------------------
// ToString(format) / TryFormat
// ---------------------------------------------------------------------------

TEST(HalfNewTests, ToString_NaN) { EXPECT_EQ(Half::NaN.ToString(), "NaN"); }
TEST(HalfNewTests, ToString_PositiveInfinity) { EXPECT_EQ(Half::PositiveInfinity.ToString(), "Infinity"); }
TEST(HalfNewTests, ToString_NegativeInfinity) { EXPECT_EQ(Half::NegativeInfinity.ToString(), "-Infinity"); }
TEST(HalfNewTests, ToString_FormatF2) {
    EXPECT_EQ(Half::FromSingle(3.5f).ToString("F2"), "3.50");
}

TEST(HalfNewTests, TryFormat_FitsBuffer) {
    Half h = Half::FromSingle(1.5f);
    char buf[16];
    SharpRuntime::intcs written = 0;
    System::Span<char> dest(buf, 16);
    EXPECT_TRUE(h.TryFormat(dest, written));
    EXPECT_EQ(std::string(buf, static_cast<size_t>(written)), h.ToString());
}
TEST(HalfNewTests, TryFormat_TooSmallReturnsFalse) {
    Half h = Half::FromSingle(123.456f);
    char buf[4];
    SharpRuntime::intcs written = -1;
    System::Span<char> dest(buf, 4);
    EXPECT_FALSE(h.TryFormat(dest, written));
    EXPECT_EQ(written, 0);
}
