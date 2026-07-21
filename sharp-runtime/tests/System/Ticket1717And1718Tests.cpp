// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Regression tests for ticket 1717 (NumberStyles-aware Parse/TryParse for the 8 integer
// primitive types) and ticket 1718 (List<T>/StringBuilder CopyTo), both from
// POST_STABILIZATION_AUDIT.md findings #8/#9.
#include <gtest/gtest.h>
#include "System/Byte.hpp"
#include "System/Int16.hpp"
#include "System/Int32.hpp"
#include "System/Int64.hpp"
#include "System/SByte.hpp"
#include "System/UInt16.hpp"
#include "System/UInt32.hpp"
#include "System/UInt64.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/OverflowException.hpp"
#include "System/Collections/Generic/List.hpp"
#include "System/Text/StringBuilder.hpp"

using System::Byte;
using System::Int16;
using System::Int32;
using System::Int64;
using System::SByte;
using System::UInt16;
using System::UInt32;
using System::UInt64;
using System::Globalization::NumberStyles;

// -----------------------------------------------------------------------
// Ticket 1717 -- NumberStyles-aware Parse/TryParse
// -----------------------------------------------------------------------

TEST(Ticket1717Tests, Int32_HexNumberStyle_RoundTrips) {
    EXPECT_EQ(Int32::Parse("FF", NumberStyles::HexNumber, nullptr), 255);
    EXPECT_EQ(Int32::Parse("ff", NumberStyles::HexNumber, nullptr), 255);
    EXPECT_EQ(Int32::Parse("FFFFFFFF", NumberStyles::HexNumber, nullptr), -1);
    EXPECT_EQ(Int32::Parse(" FF ", NumberStyles::HexNumber, nullptr), 255);
}

TEST(Ticket1717Tests, Int32_HexNumberStyle_TooManyDigitsOverflows) {
    EXPECT_THROW(Int32::Parse("1FFFFFFFF", NumberStyles::HexNumber, nullptr), System::OverflowException);
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse("1FFFFFFFF", NumberStyles::HexNumber, nullptr, out));
}

TEST(Ticket1717Tests, Int32_IntegerStyle_WhitespaceAndSign) {
    EXPECT_EQ(Int32::Parse("  -42  ", NumberStyles::Integer, nullptr), -42);
    EXPECT_EQ(Int32::Parse("+42", NumberStyles::Integer, nullptr), 42);
}

TEST(Ticket1717Tests, Int32_IntegerStyle_FormatAndOverflow) {
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse("not a number", NumberStyles::Integer, nullptr, out));
    EXPECT_THROW(Int32::Parse("99999999999", NumberStyles::Integer, nullptr), System::OverflowException);
    EXPECT_THROW(Int32::Parse("-99999999999", NumberStyles::Integer, nullptr), System::OverflowException);
}

TEST(Ticket1717Tests, Int16_HexAndInteger) {
    EXPECT_EQ(Int16::Parse("FFFF", NumberStyles::HexNumber, nullptr), -1);
    EXPECT_EQ(Int16::Parse("7FFF", NumberStyles::HexNumber, nullptr), 32767);
    EXPECT_THROW(Int16::Parse("40000", NumberStyles::Integer, nullptr), System::OverflowException);
}

TEST(Ticket1717Tests, Int64_HexAndInteger) {
    EXPECT_EQ(Int64::Parse("FFFFFFFFFFFFFFFF", NumberStyles::HexNumber, nullptr), -1);
    EXPECT_EQ(Int64::Parse("  123  ", NumberStyles::Integer, nullptr), 123);
    EXPECT_THROW(Int64::Parse("99999999999999999999", NumberStyles::Integer, nullptr), System::OverflowException);
}

TEST(Ticket1717Tests, SByte_HexAndInteger) {
    EXPECT_EQ(SByte::Parse("FF", NumberStyles::HexNumber, nullptr), -1);
    EXPECT_EQ(SByte::Parse("7F", NumberStyles::HexNumber, nullptr), 127);
    EXPECT_THROW(SByte::Parse("200", NumberStyles::Integer, nullptr), System::OverflowException);
}

TEST(Ticket1717Tests, UInt16_HexAndInteger) {
    EXPECT_EQ(UInt16::Parse("FFFF", NumberStyles::HexNumber, nullptr), 65535);
    EXPECT_EQ(UInt16::Parse("+42", NumberStyles::Integer, nullptr), 42);
    EXPECT_THROW(UInt16::Parse("-1", NumberStyles::Integer, nullptr), System::FormatException);
}

TEST(Ticket1717Tests, UInt32_HexAndInteger) {
    EXPECT_EQ(UInt32::Parse("FFFFFFFF", NumberStyles::HexNumber, nullptr), 4294967295u);
    EXPECT_THROW(UInt32::Parse("99999999999", NumberStyles::Integer, nullptr), System::OverflowException);
}

TEST(Ticket1717Tests, UInt64_HexAndInteger) {
    EXPECT_EQ(UInt64::Parse("FFFFFFFFFFFFFFFF", NumberStyles::HexNumber, nullptr), 18446744073709551615ull);
    SharpRuntime::ulongcs out;
    EXPECT_TRUE(UInt64::TryParse("123", NumberStyles::Integer, nullptr, out));
    EXPECT_EQ(out, 123u);
}

TEST(Ticket1717Tests, Byte_HexAndInteger) {
    EXPECT_EQ(Byte::Parse("FF", NumberStyles::HexNumber, nullptr), 255);
    EXPECT_THROW(Byte::Parse("-1", NumberStyles::Integer, nullptr), System::FormatException);
    EXPECT_THROW(Byte::Parse("256", NumberStyles::Integer, nullptr), System::OverflowException);
}

// -----------------------------------------------------------------------
// Ticket 1718 -- List<T>/StringBuilder CopyTo
// -----------------------------------------------------------------------

TEST(Ticket1718Tests, List_CopyTo_RawArray_Succeeds) {
    System::Collections::Generic::List<int> list;
    list.Add(1); list.Add(2); list.Add(3);
    int dest[5] = {0, 0, 0, 0, 0};
    list.CopyTo(dest, 5, 1);
    EXPECT_EQ(dest[0], 0);
    EXPECT_EQ(dest[1], 1);
    EXPECT_EQ(dest[2], 2);
    EXPECT_EQ(dest[3], 3);
    EXPECT_EQ(dest[4], 0);
}

TEST(Ticket1718Tests, List_CopyTo_NullArrayThrows) {
    System::Collections::Generic::List<int> list;
    list.Add(1);
    EXPECT_THROW(list.CopyTo(static_cast<int*>(nullptr), 5, 0), System::ArgumentNullException);
}

TEST(Ticket1718Tests, List_CopyTo_InsufficientCapacityThrows) {
    System::Collections::Generic::List<int> list;
    list.Add(1); list.Add(2); list.Add(3);
    int dest[2] = {0, 0};
    EXPECT_THROW(list.CopyTo(dest, 2, 0), System::ArgumentException);
}

TEST(Ticket1718Tests, List_CopyTo_NegativeIndexThrows) {
    System::Collections::Generic::List<int> list;
    list.Add(1);
    int dest[3] = {0, 0, 0};
    EXPECT_THROW(list.CopyTo(dest, 3, -1), System::ArgumentOutOfRangeException);
}

TEST(Ticket1718Tests, List_CopyTo_VectorOverload) {
    System::Collections::Generic::List<int> list;
    list.Add(10); list.Add(20);
    std::vector<int> dest(2, 0);
    list.CopyTo(dest, 0);
    EXPECT_EQ(dest[0], 10);
    EXPECT_EQ(dest[1], 20);
}

TEST(Ticket1718Tests, StringBuilder_CopyTo_Succeeds) {
    System::Text::StringBuilder sb("Hello, world!");
    char dest[8] = {0};
    sb.CopyTo(7, dest, 8, 0, 5);
    EXPECT_EQ(std::string(dest, 5), "world");
}

TEST(Ticket1718Tests, StringBuilder_CopyTo_NullDestinationThrows) {
    System::Text::StringBuilder sb("abc");
    EXPECT_THROW(sb.CopyTo(0, nullptr, 5, 0, 3), System::ArgumentNullException);
}

TEST(Ticket1718Tests, StringBuilder_CopyTo_SourceIndexTooLargeThrows) {
    System::Text::StringBuilder sb("abc");
    char dest[5] = {0};
    EXPECT_THROW(sb.CopyTo(10, dest, 5, 0, 1), System::ArgumentOutOfRangeException);
}

TEST(Ticket1718Tests, StringBuilder_CopyTo_CountExceedsSourceThrows) {
    System::Text::StringBuilder sb("abc");
    char dest[5] = {0};
    EXPECT_THROW(sb.CopyTo(0, dest, 5, 0, 10), System::ArgumentException);
}

TEST(Ticket1718Tests, StringBuilder_CopyTo_DestinationTooSmallThrows) {
    System::Text::StringBuilder sb("abcdef");
    char dest[3] = {0};
    EXPECT_THROW(sb.CopyTo(0, dest, 3, 0, 5), System::ArgumentException);
}
