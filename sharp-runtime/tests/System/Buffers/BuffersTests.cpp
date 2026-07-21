// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/Buffers/ArrayPool.hpp"
#include "System/Buffers/OperationStatus.hpp"
#include "System/Buffers/StandardFormat.hpp"

using System::ArgumentOutOfRangeException;
using System::FormatException;
using System::Buffers::ArrayPool;
using System::Buffers::OperationStatus;
using System::Buffers::StandardFormat;

// ===========================================================================
// ArrayPool
// ===========================================================================

TEST(ArrayPoolTests, Shared_ReturnsSameInstance) {
    ArrayPool<int>& a = ArrayPool<int>::Shared();
    ArrayPool<int>& b = ArrayPool<int>::Shared();
    EXPECT_EQ(&a, &b);
}

TEST(ArrayPoolTests, Rent_ReturnsVectorOfRequestedSize) {
    auto v = ArrayPool<int>::Shared().Rent(10);
    EXPECT_EQ(static_cast<int>(v.size()), 10);
}

TEST(ArrayPoolTests, Rent_ZeroSize_ReturnsEmptyVector) {
    auto v = ArrayPool<int>::Shared().Rent(0);
    EXPECT_TRUE(v.empty());
}

TEST(ArrayPoolTests, Rent_ElementsDefaultInitialized) {
    auto v = ArrayPool<int>::Shared().Rent(5);
    for (int x : v) EXPECT_EQ(x, 0);
}

TEST(ArrayPoolTests, Rent_BytePool_ReturnsCorrectSize) {
    auto v = ArrayPool<uint8_t>::Shared().Rent(64);
    EXPECT_EQ(static_cast<int>(v.size()), 64);
}

TEST(ArrayPoolTests, Return_ClearArrayFalse_PreservesValues) {
    auto v = ArrayPool<int>::Shared().Rent(3);
    v[0] = 1; v[1] = 2; v[2] = 3;
    ArrayPool<int>::Shared().Return(v, false);
    // Values not cleared — still readable
    EXPECT_EQ(v[0], 1);
}

TEST(ArrayPoolTests, Return_ClearArrayTrue_ZeroesElements) {
    auto v = ArrayPool<int>::Shared().Rent(3);
    v[0] = 9; v[1] = 8; v[2] = 7;
    ArrayPool<int>::Shared().Return(v, true);
    for (int x : v) EXPECT_EQ(x, 0);
}

// ===========================================================================
// OperationStatus
// ===========================================================================

TEST(OperationStatusTests, Done_ValueIsZero) {
    EXPECT_EQ(static_cast<int>(OperationStatus::Done), 0);
}

TEST(OperationStatusTests, DestinationTooSmall_ValueIsOne) {
    EXPECT_EQ(static_cast<int>(OperationStatus::DestinationTooSmall), 1);
}

TEST(OperationStatusTests, NeedMoreData_ValueIsTwo) {
    EXPECT_EQ(static_cast<int>(OperationStatus::NeedMoreData), 2);
}

TEST(OperationStatusTests, InvalidData_ValueIsThree) {
    EXPECT_EQ(static_cast<int>(OperationStatus::InvalidData), 3);
}

TEST(OperationStatusTests, Equality_SameValue) {
    OperationStatus a = OperationStatus::Done;
    OperationStatus b = OperationStatus::Done;
    EXPECT_EQ(a, b);
}

TEST(OperationStatusTests, Inequality_DifferentValues) {
    EXPECT_NE(OperationStatus::Done, OperationStatus::InvalidData);
}

// ===========================================================================
// StandardFormat
// ===========================================================================

TEST(StandardFormatTests, DefaultConstructor_MatchesNetZeroInit) {
    // .NET's default(StandardFormat) is the struct's true zero-initialized value
    // (Symbol='\0', Precision==0), NOT NoPrecision - so HasPrecision is actually
    // true and Precision==0, and IsDefault reports true for exactly this state.
    StandardFormat sf;
    EXPECT_EQ(sf.getPrecisionProperty(), 0);
    EXPECT_TRUE(sf.getHasPrecisionProperty());
    EXPECT_TRUE(sf.getIsDefaultProperty());
}

TEST(StandardFormatTests, Constructor_SymbolOnly) {
    StandardFormat sf('G');
    EXPECT_EQ(sf.getSymbolProperty(), 'G');
    EXPECT_FALSE(sf.getHasPrecisionProperty());
}

TEST(StandardFormatTests, Constructor_SymbolAndPrecision) {
    StandardFormat sf('F', 2);
    EXPECT_EQ(sf.getSymbolProperty(), 'F');
    EXPECT_EQ(sf.getPrecisionProperty(), 2);
    EXPECT_TRUE(sf.getHasPrecisionProperty());
}

TEST(StandardFormatTests, NoPrecision_ConstantIs255) {
    EXPECT_EQ(StandardFormat::NoPrecision, 255);
}

TEST(StandardFormatTests, MaxPrecision_ConstantIs99) {
    EXPECT_EQ(StandardFormat::MaxPrecision, 99);
}

TEST(StandardFormatTests, Equality_SameSymbolAndPrecision) {
    StandardFormat a('D', 3);
    StandardFormat b('D', 3);
    EXPECT_TRUE(a == b);
}

TEST(StandardFormatTests, Equality_DifferentSymbol) {
    StandardFormat a('D', 3);
    StandardFormat b('F', 3);
    EXPECT_FALSE(a == b);
}

TEST(StandardFormatTests, Equality_DifferentPrecision) {
    StandardFormat a('D', 3);
    StandardFormat b('D', 4);
    EXPECT_FALSE(a == b);
}

TEST(StandardFormatTests, Inequality_DifferentFormats) {
    StandardFormat a('G');
    StandardFormat b('F', 2);
    EXPECT_TRUE(a != b);
}

TEST(StandardFormatTests, ToString_SymbolOnly) {
    StandardFormat sf('G');
    EXPECT_EQ(sf.ToString(), "G");
}

TEST(StandardFormatTests, ToString_SymbolAndPrecision) {
    StandardFormat sf('F', 4);
    EXPECT_EQ(sf.ToString(), "F4");
}

TEST(StandardFormatTests, ToString_PrecisionTwoDigits) {
    StandardFormat sf('D', 10);
    EXPECT_EQ(sf.ToString(), "D10");
}

TEST(StandardFormatTests, Parse_SymbolOnly) {
    StandardFormat sf = StandardFormat::Parse("G");
    EXPECT_EQ(sf.getSymbolProperty(), 'G');
    EXPECT_FALSE(sf.getHasPrecisionProperty());
}

TEST(StandardFormatTests, Parse_SymbolAndPrecision) {
    StandardFormat sf = StandardFormat::Parse("F2");
    EXPECT_EQ(sf.getSymbolProperty(), 'F');
    EXPECT_EQ(sf.getPrecisionProperty(), 2);
}

TEST(StandardFormatTests, Parse_Empty_DefaultFormat) {
    // Matches .NET's ParseHelper: an empty format string yields the true
    // zero-initialized default(StandardFormat), not NoPrecision.
    StandardFormat sf = StandardFormat::Parse("");
    EXPECT_TRUE(sf.getIsDefaultProperty());
}

TEST(StandardFormatTests, Parse_ToString_RoundTrip) {
    StandardFormat original('X', 8);
    StandardFormat parsed = StandardFormat::Parse(original.ToString());
    EXPECT_EQ(parsed, original);
}

TEST(StandardFormatTests, Ctor_PrecisionTooLarge_Throws) {
    EXPECT_THROW(StandardFormat('D', 100), ArgumentOutOfRangeException);
}

TEST(StandardFormatTests, Ctor_MaxPrecision_Allowed) {
    EXPECT_NO_THROW(StandardFormat('D', StandardFormat::MaxPrecision));
}

TEST(StandardFormatTests, Parse_PrecisionTooLarge_Throws) {
    EXPECT_THROW(StandardFormat::Parse("D100"), FormatException);
}

TEST(StandardFormatTests, Parse_NonDigitPrecision_Throws) {
    EXPECT_THROW(StandardFormat::Parse("Dx"), FormatException);
}

TEST(StandardFormatTests, TryParse_Invalid_ReturnsFalse) {
    StandardFormat result;
    EXPECT_FALSE(StandardFormat::TryParse("D100", result));
}

TEST(StandardFormatTests, TryParse_Valid_ReturnsTrue) {
    StandardFormat result;
    EXPECT_TRUE(StandardFormat::TryParse("F2", result));
    EXPECT_EQ(result.getSymbolProperty(), 'F');
    EXPECT_EQ(result.getPrecisionProperty(), 2);
}

TEST(StandardFormatTests, GetHashCode_SameValues_SameHash) {
    StandardFormat a('G', 5);
    StandardFormat b('G', 5);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(StandardFormatTests, Equals_SameValues_True) {
    StandardFormat a('G', 5);
    StandardFormat b('G', 5);
    EXPECT_TRUE(a.Equals(b));
}

TEST(StandardFormatTests, IsDefault_NonDefault_False) {
    StandardFormat sf('G');
    EXPECT_FALSE(sf.getIsDefaultProperty());
}
