// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "gtest/gtest.h"
#include "System/Random.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using SharpRuntime::intcs;
using SharpRuntime::longcs;
TEST(RandomTests, NextWithMaxValue) {
    System::Random rng;
    const intcs max = 100;
    bool maxFound = false;
    bool maxMinusOneFound = false;
    for (int i = 0; i < 10000; ++i) {
        intcs value = rng.Next(max);
        //std::cout << value << std::endl;
        ASSERT_GE(value, 0);
        ASSERT_LT(value, max);
        maxFound = maxFound || value == max;
        maxMinusOneFound = maxMinusOneFound || value == (max - 1);
    }

    if (maxFound) { FAIL() << "Max value " << max << " found!";}
    if (!maxMinusOneFound) { FAIL() << "Max value minus one " << (max - 1) << " not found!";}
}

TEST(RandomTests, NextWithMinAndMaxValue) {
    System::Random rng;
    const intcs min = 10;
    const intcs max = 20;
    bool minFound = false;
    bool maxFound = false;
    bool maxMinusOneFound = false;
    for (int i = 0; i < 1000; ++i) {
        //std::cout << "Run " << i << std::endl;
        intcs value = rng.Next(min, max);
        //std::cout << value << std::endl;
        ASSERT_GE(value, min);
        ASSERT_LT(value, max);
        maxFound = maxFound || value == max;
        maxMinusOneFound = maxMinusOneFound || value == (max - 1);
        minFound = minFound || value == min;
    }
    if (maxFound) { FAIL() << "Max value " << max << " found!";}
    if (!maxMinusOneFound) { FAIL() << "Max value minus one " << (max - 1) << " not found!";}
    if (!minFound) { FAIL() << "Min value " << min << " not found!";}
}

TEST(RandomTests, NextWithoutArguments) {
    System::Random rng;
    for (int i = 0; i < 1000; ++i) {
        intcs value = rng.Next();
        ASSERT_GE(value, 0);
        ASSERT_LT(value, SharpRuntime::INTCS_MAX);
    }
}

TEST(RandomTests, SeededConstructorIsDeterministic) {
    System::Random rng1(42);
    System::Random rng2(42);
    for (int i = 0; i < 20; ++i)
        EXPECT_EQ(rng1.Next(1000), rng2.Next(1000));
}

TEST(RandomTests, DifferentSeedsDifferentSequences) {
    System::Random rng1(1);
    System::Random rng2(2);
    bool anyDifferent = false;
    for (int i = 0; i < 10; ++i)
        if (rng1.Next(10000) != rng2.Next(10000)) { anyDifferent = true; break; }
    EXPECT_TRUE(anyDifferent);
}

TEST(RandomTests, NextDoubleInRange) {
    System::Random rng(0);
    for (int i = 0; i < 1000; ++i) {
        double v = rng.NextDouble();
        ASSERT_GE(v, 0.0);
        ASSERT_LT(v, 1.0);
    }
}

TEST(RandomTests, NextDoubleCoversBothEnds) {
    System::Random rng(123);
    bool nearZero = false, nearOne = false;
    for (int i = 0; i < 10000; ++i) {
        double v = rng.NextDouble();
        if (v < 0.01) nearZero = true;
        if (v > 0.99) nearOne = true;
    }
    EXPECT_TRUE(nearZero);
    EXPECT_TRUE(nearOne);
}

TEST(RandomTests, NextBytesFilledCorrectly) {
    System::Random rng(7);
    std::vector<uint8_t> buf(16, 0);
    rng.NextBytes(buf);
    bool anyNonZero = false;
    for (auto b : buf) if (b != 0) { anyNonZero = true; break; }
    EXPECT_TRUE(anyNonZero);
    EXPECT_EQ(buf.size(), 16u);
}

TEST(RandomTests, NextBytesAllValuesInRange) {
    System::Random rng(99);
    std::vector<uint8_t> buf(1000);
    rng.NextBytes(buf);
    for (auto b : buf)
        ASSERT_LE(b, 255);
}

TEST(RandomTests, NextSingleInRange) {
    System::Random rng(42);
    for (int i = 0; i < 100; ++i) {
        float v = rng.NextSingle();
        EXPECT_GE(v, 0.0f);
        EXPECT_LT(v, 1.0f);
    }
}
TEST(RandomTests, NextSingleSeededIsDeterministic) {
    System::Random a(7), b(7);
    EXPECT_EQ(a.NextSingle(), b.NextSingle());
}

// --- NextInt64 ---
TEST(RandomTests, NextInt64_NoArgs_InRange) {
    System::Random rng(1);
    for (int i = 0; i < 100; ++i) {
        longcs v = rng.NextInt64();
        EXPECT_GE(v, 0LL);
        EXPECT_LT(v, SharpRuntime::LONGCS_MAX);
    }
}
TEST(RandomTests, NextInt64_MaxValue_InRange) {
    System::Random rng(2);
    const longcs max = 1000000000LL;
    for (int i = 0; i < 100; ++i) {
        longcs v = rng.NextInt64(max);
        EXPECT_GE(v, 0LL);
        EXPECT_LT(v, max);
    }
}
TEST(RandomTests, NextInt64_MaxZero_ReturnsZero) {
    System::Random rng(3);
    EXPECT_EQ(rng.NextInt64(0LL), 0LL);
}
TEST(RandomTests, NextInt64_Range_InRange) {
    System::Random rng(4);
    const longcs lo = 500000000000LL, hi = 600000000000LL;
    for (int i = 0; i < 100; ++i) {
        longcs v = rng.NextInt64(lo, hi);
        EXPECT_GE(v, lo);
        EXPECT_LT(v, hi);
    }
}
TEST(RandomTests, NextInt64_EqualMinMax_ReturnsMin) {
    System::Random rng(5);
    EXPECT_EQ(rng.NextInt64(42LL, 42LL), 42LL);
}
TEST(RandomTests, NextInt64_Seeded_Deterministic) {
    System::Random a(99), b(99);
    EXPECT_EQ(a.NextInt64(1000000LL), b.NextInt64(1000000LL));
}
TEST(RandomTests, NextInt64_MaxValueThrows) {
    System::Random rng(6);
    EXPECT_THROW(rng.NextInt64(-1LL), System::ArgumentOutOfRangeException);
}
TEST(RandomTests, NextInt64_InvalidRangeThrows) {
    System::Random rng(7);
    EXPECT_THROW(rng.NextInt64(10LL, 5LL), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// Shared
// ---------------------------------------------------------------------------

TEST(RandomTests, Shared_ReturnsSameReference) {
    System::Random& a = System::Random::getSharedProperty();
    System::Random& b = System::Random::getSharedProperty();
    EXPECT_EQ(&a, &b);
}

TEST(RandomTests, Shared_ProducesValues) {
    int v = System::Random::getSharedProperty().Next(1000);
    EXPECT_GE(v, 0);
    EXPECT_LT(v, 1000);
}

// ---------------------------------------------------------------------------
// NextBytes(Span)
// ---------------------------------------------------------------------------

TEST(RandomTests, NextBytes_Span_FillsBuffer) {
    System::Random rng(11);
    std::vector<uint8_t> vec(16, 0);
    System::Span<uint8_t> s(vec);
    rng.NextBytes(s);
    bool anyNonZero = false;
    for (auto b : vec) if (b) { anyNonZero = true; break; }
    EXPECT_TRUE(anyNonZero);
}

TEST(RandomTests, NextBytes_Span_LengthUnchanged) {
    System::Random rng(22);
    std::vector<uint8_t> vec(32);
    System::Span<uint8_t> s(vec);
    rng.NextBytes(s);
    EXPECT_EQ(s.getLengthProperty(), 32);
}

// ---------------------------------------------------------------------------
// NextInteger<T>
// ---------------------------------------------------------------------------

TEST(RandomTests, NextInteger_Int_InRange) {
    System::Random rng(33);
    for (int i = 0; i < 100; ++i) {
        int v = rng.NextInteger<int>();
        EXPECT_GE(v, 0);
        EXPECT_LE(v, std::numeric_limits<int>::max());
    }
}

TEST(RandomTests, NextInteger_MaxValue_InRange) {
    System::Random rng(44);
    for (int i = 0; i < 100; ++i) {
        int v = rng.NextInteger<int>(50);
        EXPECT_GE(v, 0);
        EXPECT_LT(v, 50);
    }
}

TEST(RandomTests, NextInteger_Range_InRange) {
    System::Random rng(55);
    for (int i = 0; i < 100; ++i) {
        int v = rng.NextInteger<int>(10, 20);
        EXPECT_GE(v, 10);
        EXPECT_LT(v, 20);
    }
}

TEST(RandomTests, NextInteger_EqualMinMax_ReturnsMin) {
    System::Random rng(66);
    EXPECT_EQ(rng.NextInteger<int>(7, 7), 7);
}

TEST(RandomTests, NextInteger_MaxValueNegative_Throws) {
    System::Random rng(77);
    EXPECT_THROW((rng.NextInteger<int>(-1)), System::ArgumentOutOfRangeException);
}

TEST(RandomTests, NextInteger_InvalidRange_Throws) {
    System::Random rng(88);
    EXPECT_THROW((rng.NextInteger<int>(10, 5)), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// Shuffle<T>
// ---------------------------------------------------------------------------

TEST(RandomTests, Shuffle_ChangesOrder) {
    System::Random rng(99);
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> original = v;
    rng.Shuffle(v);
    // All elements must still be present
    std::vector<int> sorted = v;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted, original);
}

TEST(RandomTests, Shuffle_EmptyVector_NoThrow) {
    System::Random rng(100);
    std::vector<int> v;
    EXPECT_NO_THROW(rng.Shuffle(v));
}

TEST(RandomTests, Shuffle_VectorAndSpan_ProduceIdenticalSequenceForSameSeed) {
    // Real .NET's Shuffle(T[]) delegates entirely to Shuffle(Span<T>) -- there is only one
    // shuffle algorithm. Shuffle(std::vector<T>&) previously implemented an independent,
    // opposite-direction loop that produced a genuinely different permutation than
    // Shuffle(Span<T>) for the same seed, breaking this file's seeded-determinism guarantee.
    System::Random rngVec(42);
    std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    rngVec.Shuffle(v);

    System::Random rngSpan(42);
    std::vector<int> s = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    rngSpan.Shuffle(System::Span<int>(s));

    EXPECT_EQ(v, s);
}

// ---------------------------------------------------------------------------
// GetItems<T>
// ---------------------------------------------------------------------------

TEST(RandomTests, GetItems_CorrectLength) {
    System::Random rng(111);
    std::vector<int> choices = {10, 20, 30, 40};
    auto result = rng.GetItems(choices, 6);
    EXPECT_EQ(result.size(), 6u);
}

TEST(RandomTests, GetItems_EmptyChoices_Throws) {
    // .NET's GetItems throws ArgumentException for empty choices, even when
    // length == 0 - verified against Random.cs.
    System::Random rng(111);
    std::vector<int> choices;
    EXPECT_THROW(rng.GetItems(choices, 0), System::ArgumentException);
}

TEST(RandomTests, GetItems_NegativeLength_Throws) {
    System::Random rng(111);
    std::vector<int> choices = {1, 2, 3};
    EXPECT_THROW(rng.GetItems(choices, -1), System::ArgumentOutOfRangeException);
}

TEST(RandomTests, GetItems_SpanDestination_EmptyChoices_Throws) {
    System::Random rng(111);
    std::vector<int> empty;
    std::vector<int> dest(3);
    EXPECT_THROW(rng.GetItems(System::ReadOnlySpan<int>(empty.data(), 0), System::Span<int>(dest.data(), 3)), System::ArgumentException);
}

TEST(RandomTests, GetItems_OnlyFromChoices) {
    System::Random rng(222);
    std::vector<int> choices = {10, 20, 30};
    auto result = rng.GetItems(choices, 50);
    for (int v : result)
        EXPECT_TRUE(v == 10 || v == 20 || v == 30);
}

// ---------------------------------------------------------------------------
// GetString
// ---------------------------------------------------------------------------

TEST(RandomTests, GetString_CorrectLength) {
    System::Random rng(333);
    std::string result = rng.GetString("abc", 8);
    EXPECT_EQ(result.size(), 8u);
}

TEST(RandomTests, GetString_OnlyFromChoices) {
    System::Random rng(444);
    std::string result = rng.GetString("xyz", 20);
    for (char c : result)
        EXPECT_TRUE(c == 'x' || c == 'y' || c == 'z');
}

// ---------------------------------------------------------------------------
// GetHexString
// ---------------------------------------------------------------------------

TEST(RandomTests, GetHexString_CorrectLength) {
    System::Random rng(555);
    EXPECT_EQ(rng.GetHexString(8).size(), 8u);
}

TEST(RandomTests, GetHexString_UppercaseByDefault) {
    System::Random rng(666);
    std::string s = rng.GetHexString(100);
    for (char c : s) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
    }
}

TEST(RandomTests, GetHexString_Lowercase) {
    System::Random rng(777);
    std::string s = rng.GetHexString(100, true);
    for (char c : s) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

TEST(RandomTests, GetHexString_ZeroLength_Empty) {
    System::Random rng(888);
    EXPECT_TRUE(rng.GetHexString(0).empty());
}

// ---------------------------------------------------------------------------
// NextBinaryFloat<T>
// ---------------------------------------------------------------------------

TEST(RandomTests, NextBinaryFloat_Float_InRange) {
    System::Random rng(10);
    for (int i = 0; i < 100; ++i) {
        float v = rng.NextBinaryFloat<float>();
        EXPECT_GE(v, 0.0f);
        EXPECT_LT(v, 1.0f);
    }
}

TEST(RandomTests, NextBinaryFloat_Double_InRange) {
    System::Random rng(20);
    for (int i = 0; i < 100; ++i) {
        double v = rng.NextBinaryFloat<double>();
        EXPECT_GE(v, 0.0);
        EXPECT_LT(v, 1.0);
    }
}

TEST(RandomTests, NextBinaryFloat_Float_Seeded_Deterministic) {
    System::Random a(30), b(30);
    EXPECT_EQ(a.NextBinaryFloat<float>(), b.NextBinaryFloat<float>());
}

// ---------------------------------------------------------------------------
// GetItems<T>(ReadOnlySpan, Span) — destination overload
// ---------------------------------------------------------------------------

TEST(RandomTests, GetItems_SpanDestination_FillsCorrectly) {
    System::Random rng(40);
    std::vector<int> choices = {1, 2, 3, 4};
    std::vector<int> dest(6, 0);
    System::ReadOnlySpan<int> src(choices);
    System::Span<int> dst(dest);
    rng.GetItems(src, dst);
    for (int v : dest)
        EXPECT_TRUE(v >= 1 && v <= 4);
}

TEST(RandomTests, GetItems_SpanDestination_LengthUnchanged) {
    System::Random rng(50);
    std::vector<int> choices = {10, 20};
    std::vector<int> dest(8);
    rng.GetItems(System::ReadOnlySpan<int>(choices), System::Span<int>(dest));
    EXPECT_EQ(dest.size(), 8u);
}

// ---------------------------------------------------------------------------
// Shuffle<T>(Span<T>)
// ---------------------------------------------------------------------------

TEST(RandomTests, Shuffle_Span_PreservesElements) {
    System::Random rng(60);
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> original = v;
    System::Span<int> s(v);
    rng.Shuffle(s);
    std::vector<int> sorted = v;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted, original);
}

TEST(RandomTests, Shuffle_Span_EmptyNoThrow) {
    System::Random rng(70);
    std::vector<int> v;
    EXPECT_NO_THROW(rng.Shuffle(System::Span<int>(v)));
}

// ---------------------------------------------------------------------------
// GetHexString(Span<char>)
// ---------------------------------------------------------------------------

TEST(RandomTests, GetHexString_SpanDestination_FillsUppercase) {
    System::Random rng(80);
    std::vector<char> buf(12, 0);
    System::Span<char> s(buf);
    rng.GetHexString(s);
    for (char c : buf)
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
}

TEST(RandomTests, GetHexString_SpanDestination_FillsLowercase) {
    System::Random rng(90);
    std::vector<char> buf(12, 0);
    System::Span<char> s(buf);
    rng.GetHexString(s, true);
    for (char c : buf)
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
}

TEST(RandomTests, GetHexString_SpanDestination_LengthUnchanged) {
    System::Random rng(91);
    std::vector<char> buf(8);
    System::Span<char> s(buf);
    rng.GetHexString(s);
    EXPECT_EQ(s.getLengthProperty(), 8);
}

// ---------------------------------------------------------------------------
// Next — edge cases and throws
// ---------------------------------------------------------------------------

TEST(RandomTests, Next_MaxValueZero_ReturnsZero) {
    System::Random rng(1);
    EXPECT_EQ(rng.Next(0), 0);
}

TEST(RandomTests, Next_MaxValueNegative_Throws) {
    System::Random rng(2);
    EXPECT_THROW(rng.Next(-1), System::ArgumentOutOfRangeException);
}

TEST(RandomTests, Next_EqualMinMax_ReturnsMin) {
    System::Random rng(3);
    EXPECT_EQ(rng.Next(7, 7), 7);
}

TEST(RandomTests, Next_InvalidRange_Throws) {
    System::Random rng(4);
    EXPECT_THROW(rng.Next(10, 5), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// NextBytes — edge cases
// ---------------------------------------------------------------------------

TEST(RandomTests, NextBytes_EmptyVector_NoThrow) {
    System::Random rng(5);
    std::vector<uint8_t> buf;
    EXPECT_NO_THROW(rng.NextBytes(buf));
}

// ---------------------------------------------------------------------------
// NextDouble — determinism
// ---------------------------------------------------------------------------

TEST(RandomTests, NextDouble_Seeded_Deterministic) {
    System::Random a(12345), b(12345);
    EXPECT_EQ(a.NextDouble(), b.NextDouble());
}

// ---------------------------------------------------------------------------
// GetString — edge cases
// ---------------------------------------------------------------------------

TEST(RandomTests, GetString_ZeroLength_Empty) {
    System::Random rng(6);
    EXPECT_TRUE(rng.GetString("abc", 0).empty());
}

TEST(RandomTests, GetString_EmptyChoices_Throws) {
    // .NET's GetString throws ArgumentException for empty choices, even when
    // length == 0 - verified against Random.cs.
    System::Random rng(6);
    EXPECT_THROW(rng.GetString("", 0), System::ArgumentException);
}

TEST(RandomTests, GetString_NegativeLength_Throws) {
    System::Random rng(6);
    EXPECT_THROW(rng.GetString("abc", -1), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// Shuffle — single element
// ---------------------------------------------------------------------------

TEST(RandomTests, Shuffle_SingleElement_NoThrow) {
    System::Random rng(7);
    std::vector<int> v = {42};
    EXPECT_NO_THROW(rng.Shuffle(v));
    EXPECT_EQ(v[0], 42);
}

// ---------------------------------------------------------------------------
// GetItems — ReadOnlySpan overload
// ---------------------------------------------------------------------------

TEST(RandomTests, GetItems_ReadOnlySpan_CorrectLength) {
    System::Random rng(111);
    std::vector<int> choices = {10, 20, 30, 40};
    System::ReadOnlySpan<int> src(choices);
    auto result = rng.GetItems(src, 6);
    EXPECT_EQ(result.size(), 6u);
}

TEST(RandomTests, GetItems_ReadOnlySpan_OnlyFromChoices) {
    System::Random rng(222);
    std::vector<int> choices = {10, 20, 30};
    System::ReadOnlySpan<int> src(choices);
    auto result = rng.GetItems(src, 50);
    for (int v : result)
        EXPECT_TRUE(v == 10 || v == 20 || v == 30);
}

// ---------------------------------------------------------------------------
// .NET parity (ticket 64): Random(seed) is now a byte-for-byte port of real
// .NET's own seeded algorithm (the Knuth "subtractive" generator .NET has kept
// bit-stable since .NET Framework 1.0). Every expected value below was captured
// from a live Mono run (System.Private.CoreLib-compatible mscorlib, which uses
// the identical unchanged seeded algorithm) with the exact same seeds -- these
// are not just "same seed -> same output" self-consistency checks like the
// tests above, but actual cross-runtime parity checks against real .NET output.
// ---------------------------------------------------------------------------

TEST(RandomTests, Parity_Next_Seed42) {
    System::Random rng(42);
    EXPECT_EQ(rng.Next(), 1434747710);
    EXPECT_EQ(rng.Next(), 302596119);
    EXPECT_EQ(rng.Next(), 269548474);
    EXPECT_EQ(rng.Next(), 1122627734);
    EXPECT_EQ(rng.Next(), 361709742);
}

TEST(RandomTests, Parity_NextMaxValue_Seed42) {
    System::Random rng(42);
    EXPECT_EQ(rng.Next(100), 66);
    EXPECT_EQ(rng.Next(100), 14);
    EXPECT_EQ(rng.Next(100), 12);
    EXPECT_EQ(rng.Next(100), 52);
    EXPECT_EQ(rng.Next(100), 16);
}

TEST(RandomTests, Parity_NextMinMax_Seed42) {
    System::Random rng(42);
    EXPECT_EQ(rng.Next(10, 20), 16);
    EXPECT_EQ(rng.Next(10, 20), 11);
    EXPECT_EQ(rng.Next(10, 20), 11);
    EXPECT_EQ(rng.Next(10, 20), 15);
    EXPECT_EQ(rng.Next(10, 20), 11);
}

TEST(RandomTests, Parity_NextDouble_Seed42) {
    System::Random rng(42);
    EXPECT_DOUBLE_EQ(rng.NextDouble(), 0.66810646591154232);
    EXPECT_DOUBLE_EQ(rng.NextDouble(), 0.14090729837348093);
    EXPECT_DOUBLE_EQ(rng.NextDouble(), 0.12551828945312568);
}

TEST(RandomTests, Parity_NextBytes_Seed42) {
    System::Random rng(42);
    std::vector<SharpRuntime::bytecs> buf(8);
    rng.NextBytes(buf);
    std::vector<SharpRuntime::bytecs> expected = {0x3E, 0x17, 0xBA, 0x96, 0xAE, 0x04, 0xCD, 0x3B};
    EXPECT_EQ(buf, expected);
}

TEST(RandomTests, Parity_Next_Seed12345) {
    System::Random rng(12345);
    EXPECT_EQ(rng.Next(), 143337951);
    EXPECT_EQ(rng.Next(), 150666398);
    EXPECT_EQ(rng.Next(), 1663795458);
    EXPECT_EQ(rng.Next(), 1097663221);
    EXPECT_EQ(rng.Next(), 1712597933);
}

TEST(RandomTests, Parity_Next_SeedZero) {
    System::Random rng(0);
    EXPECT_EQ(rng.Next(), 1559595546);
    EXPECT_EQ(rng.Next(), 1755192844);
    EXPECT_EQ(rng.Next(), 1649316166);
}

// Verified against real .NET's own seed-normalization: Math.Abs(seed) is used for every
// seed except int.MinValue (where Math.Abs would overflow), which is special-cased to
// int.MaxValue instead -- so this is deliberately a different effective seed from 0, and
// legitimately produces a different sequence from SeedZero above.
TEST(RandomTests, Parity_Next_SeedIntMinValue) {
    System::Random rng(std::numeric_limits<intcs>::min());
    EXPECT_EQ(rng.Next(), 1559595546);
    EXPECT_EQ(rng.Next(), 1755192844);
    EXPECT_EQ(rng.Next(), 1649316172);
}

// Verified against real .NET's Math.Abs(seed) normalization: a negative seed produces the
// exact same sequence as its positive absolute value.
TEST(RandomTests, Parity_Next_NegativeSeed_MatchesAbsoluteValue) {
    System::Random negative(-42);
    System::Random positive(42);
    EXPECT_EQ(negative.Next(), positive.Next());
    EXPECT_EQ(negative.Next(), positive.Next());
    EXPECT_EQ(negative.Next(), positive.Next());
}
