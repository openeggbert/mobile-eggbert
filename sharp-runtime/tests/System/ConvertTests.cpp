// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/ArgumentException.hpp"
#include "System/Convert.hpp"

using System::Convert;

// ---------------------------------------------------------------------------
// ToInt32
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToInt32FromString) {
    // Explicit std::string to avoid const char* → bool implicit conversion
    EXPECT_EQ(Convert::ToInt32(std::string("42")),  42);
    EXPECT_EQ(Convert::ToInt32(std::string("-7")), -7);
    EXPECT_EQ(Convert::ToInt32(std::string("0")),   0);
}

TEST(ConvertTests, ToInt32FromStringInvalidThrows) {
    EXPECT_THROW(Convert::ToInt32(std::string("abc")),   std::exception);
    EXPECT_THROW(Convert::ToInt32(std::string("")),      std::exception);
    EXPECT_THROW(Convert::ToInt32(std::string("1.5")),   std::exception);
}

// Regression tests for ticket 328: a raw string-literal argument (no explicit std::string(...)
// wrapping -- exactly the form the tests above deliberately avoided) previously silently
// resolved to the ToInt32(bool) overload instead of ToInt32(const std::string&): C++ overload
// resolution always prefers a standard conversion (pointer-to-bool) over a user-defined one
// (const char* -> std::string), regardless of which one the caller obviously meant. Convert::
// ToInt32("42") returned 1 (true), not 42. Fixed by adding an explicit ToInt32(const char*)
// overload (an exact-match candidate that wins over both). This bug affected essentially every
// Convert::To* method with both a bool and a std::string overload (ToBoolean/ToByte/ToInt16/
// ToInt32/ToInt64/ToDouble/ToSingle/ToUInt32/ToUInt64/ToUInt16/ToSByte) -- ToBoolean had
// already been fixed; the other 10 had not.
TEST(ConvertTests, ToInt32FromRawStringLiteral_ResolvesToStringOverload) {
    EXPECT_EQ(Convert::ToInt32("42"), 42);
    EXPECT_EQ(Convert::ToInt32("-7"), -7);
}

// Regression test for ticket 328: real .NET's Convert.ToInt32(string) delegates to
// int.Parse(value), whose default NumberStyles.Integer tolerates leading AND trailing
// whitespace. The port previously reimplemented parsing via strtoll directly, which rejects
// any trailing character including whitespace -- Convert::ToInt32(" 42 ") threw
// FormatException where real .NET succeeds.
TEST(ConvertTests, ToInt32FromString_TrailingWhitespace_Tolerated) {
    EXPECT_EQ(Convert::ToInt32(std::string(" 42 ")), 42);
    EXPECT_EQ(Convert::ToInt32(std::string("\t-7\n")), -7);
}

TEST(ConvertTests, ToInt32FromBool) {
    EXPECT_EQ(Convert::ToInt32(true),  1);
    EXPECT_EQ(Convert::ToInt32(false), 0);
}

TEST(ConvertTests, ToInt32FromDouble) {
    // Real .NET rounds to nearest, ties to even (matching Math.Round's default), not truncation.
    EXPECT_EQ(Convert::ToInt32(3.7),  4);
    EXPECT_EQ(Convert::ToInt32(-2.9), -3);
    EXPECT_EQ(Convert::ToInt32(0.0),  0);
}

TEST(ConvertTests, ToInt32FromDouble_RoundsHalfToEven) {
    EXPECT_EQ(Convert::ToInt32(2.5), 2);
    EXPECT_EQ(Convert::ToInt32(3.5), 4);
    EXPECT_EQ(Convert::ToInt32(-2.5), -2);
    EXPECT_EQ(Convert::ToInt32(-3.5), -4);
}

TEST(ConvertTests, ToInt32FromDoubleOverflowThrows) {
    EXPECT_THROW(Convert::ToInt32(3e10), std::exception);
}

TEST(ConvertTests, ToInt32FromInt) {
    EXPECT_EQ(Convert::ToInt32(42), 42);
}

TEST(ConvertTests, ToInt32FromLong) {
    EXPECT_EQ(Convert::ToInt32(static_cast<int64_t>(100)), 100);
}

TEST(ConvertTests, ToInt32FromLongOverflowThrows) {
    EXPECT_THROW(Convert::ToInt32(static_cast<int64_t>(3000000000LL)), std::exception);
}

TEST(ConvertTests, ToInt32FromByte) {
    EXPECT_EQ(Convert::ToInt32(static_cast<uint8_t>(200)), 200);
}

// ---------------------------------------------------------------------------
// ToInt32 with base
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToInt32Base16) {
    EXPECT_EQ(Convert::ToInt32("ff", 16), 255);
    EXPECT_EQ(Convert::ToInt32("FF", 16), 255);
    EXPECT_EQ(Convert::ToInt32("10", 16), 16);
}

TEST(ConvertTests, ToInt32Base2) {
    EXPECT_EQ(Convert::ToInt32("1010", 2), 10);
    EXPECT_EQ(Convert::ToInt32("11111111", 2), 255);
}

TEST(ConvertTests, ToInt32Base8) {
    EXPECT_EQ(Convert::ToInt32("17",  8), 15);
    EXPECT_EQ(Convert::ToInt32("377", 8), 255);
}

TEST(ConvertTests, ToInt32Base10) {
    EXPECT_EQ(Convert::ToInt32("42", 10), 42);
}

// Regression test for ticket 328: real .NET's Convert.ToInt32(string, int fromBase) restricts
// fromBase to {2, 8, 10, 16}, throwing ArgumentException for anything else (ParseNumbers.
// StringToLong). The port previously accepted any base strtoll itself supports (e.g. base 5),
// silently producing a result real .NET would reject outright.
TEST(ConvertTests, ToInt32InvalidBase_ThrowsArgumentException) {
    EXPECT_THROW(Convert::ToInt32("42", 5), System::ArgumentException);
    EXPECT_THROW(Convert::ToInt32("42", 3), System::ArgumentException);
}

// ---------------------------------------------------------------------------
// ToInt64
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToInt64FromString) {
    EXPECT_EQ(Convert::ToInt64(std::string("1000000000000")), 1000000000000LL);
    EXPECT_EQ(Convert::ToInt64(std::string("-1")),            -1LL);
}

// Regression tests for ticket 328 -- see the identical ToInt32 tests above for the full
// rationale (raw string literal overload resolution, and trailing-whitespace tolerance).
TEST(ConvertTests, ToInt64FromRawStringLiteral_ResolvesToStringOverload) {
    EXPECT_EQ(Convert::ToInt64("100"), 100LL);
}
TEST(ConvertTests, ToInt64FromString_TrailingWhitespace_Tolerated) {
    EXPECT_EQ(Convert::ToInt64(std::string("  100  ")), 100LL);
}

TEST(ConvertTests, ToInt64FromStringInvalidThrows) {
    EXPECT_THROW(Convert::ToInt64(std::string("abc")), std::exception);
    EXPECT_THROW(Convert::ToInt64(std::string("")),    std::exception);
}

TEST(ConvertTests, ToInt64FromInt) {
    EXPECT_EQ(Convert::ToInt64(42), 42LL);
}

TEST(ConvertTests, ToInt64FromDouble) {
    // Real .NET: checked((long)Math.Round(value)) -- rounds to nearest, ties to even.
    EXPECT_EQ(Convert::ToInt64(3.7), 4LL);
}

TEST(ConvertTests, ToInt64FromDouble_OverflowThrows) {
    EXPECT_THROW(Convert::ToInt64(1e20), System::OverflowException);
}

// ---------------------------------------------------------------------------
// ToInt16
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToInt16FromString) {
    EXPECT_EQ(Convert::ToInt16(std::string("100")),    100);
    EXPECT_EQ(Convert::ToInt16(std::string("-32768")), -32768);
}

TEST(ConvertTests, ToInt16FromInt) {
    EXPECT_EQ(Convert::ToInt16(32767), 32767);
}

TEST(ConvertTests, ToInt16OverflowThrows) {
    EXPECT_THROW(Convert::ToInt16(32768), std::exception);
    EXPECT_THROW(Convert::ToInt16(std::string("40000")), std::exception);
}
TEST(ConvertTests, ToInt16FromLong) {
    EXPECT_EQ(Convert::ToInt16(static_cast<SharpRuntime::longcs>(1000LL)), 1000);
}
TEST(ConvertTests, ToInt16FromLongOverflowThrows) {
    EXPECT_THROW(Convert::ToInt16(static_cast<SharpRuntime::longcs>(40000LL)), std::exception);
}
TEST(ConvertTests, ToInt16FromDouble) {
    // Rounds to nearest (ties to even), matching real .NET -- not truncation.
    EXPECT_EQ(Convert::ToInt16(3.7), static_cast<SharpRuntime::shortcs>(4));
}
TEST(ConvertTests, ToInt16FromFloat) {
    EXPECT_EQ(Convert::ToInt16(5.9f), static_cast<SharpRuntime::shortcs>(6));
}
TEST(ConvertTests, ToInt16FromBool_True)  { EXPECT_EQ(Convert::ToInt16(true),  static_cast<SharpRuntime::shortcs>(1)); }
TEST(ConvertTests, ToInt16FromBool_False) { EXPECT_EQ(Convert::ToInt16(false), static_cast<SharpRuntime::shortcs>(0)); }
TEST(ConvertTests, ToInt16FromByte) {
    EXPECT_EQ(Convert::ToInt16(static_cast<SharpRuntime::bytecs>(200)), static_cast<SharpRuntime::shortcs>(200));
}

// ---------------------------------------------------------------------------
// ToDouble
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToDoubleFromString) {
    EXPECT_NEAR(Convert::ToDouble(std::string("3.14")), 3.14, 1e-10);
    EXPECT_NEAR(Convert::ToDouble(std::string("-1.5")), -1.5, 1e-10);
    EXPECT_NEAR(Convert::ToDouble(std::string("0")),    0.0,  1e-15);
}

TEST(ConvertTests, ToDoubleFromStringInvalidThrows) {
    EXPECT_THROW(Convert::ToDouble(std::string("abc")), std::exception);
    EXPECT_THROW(Convert::ToDouble(std::string("")),    std::exception);
}

TEST(ConvertTests, ToDoubleFromInt) {
    EXPECT_NEAR(Convert::ToDouble(42), 42.0, 1e-15);
}

TEST(ConvertTests, ToDoubleFromLong) {
    EXPECT_NEAR(Convert::ToDouble(static_cast<int64_t>(1000000000000LL)), 1e12, 1.0);
}

// ---------------------------------------------------------------------------
// ToSingle
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToSingleFromString) {
    EXPECT_NEAR(static_cast<double>(Convert::ToSingle(std::string("3.14"))), 3.14, 1e-5);
}

TEST(ConvertTests, ToSingleFromDouble) {
    EXPECT_NEAR(static_cast<double>(Convert::ToSingle(3.14)), 3.14, 1e-5);
}

TEST(ConvertTests, ToSingleFromInt) {
    EXPECT_NEAR(static_cast<double>(Convert::ToSingle(42)), 42.0, 1e-5);
}

// ---------------------------------------------------------------------------
// ToByte
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToByteFromInt) {
    EXPECT_EQ(Convert::ToByte(200), 200u);
    EXPECT_EQ(Convert::ToByte(0),   0u);
    EXPECT_EQ(Convert::ToByte(255), 255u);
}

TEST(ConvertTests, ToByteFromIntOverflowThrows) {
    EXPECT_THROW(Convert::ToByte(256),  std::exception);
    EXPECT_THROW(Convert::ToByte(-1),   std::exception);
}

TEST(ConvertTests, ToByteFromString) {
    EXPECT_EQ(Convert::ToByte(std::string("200")), 200u);
}

// ---------------------------------------------------------------------------
// ToBoolean
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToBooleanFromInt) {
    EXPECT_TRUE(Convert::ToBoolean(1));
    EXPECT_TRUE(Convert::ToBoolean(-1));
    EXPECT_FALSE(Convert::ToBoolean(0));
}

TEST(ConvertTests, ToBooleanFromString) {
    EXPECT_TRUE(Convert::ToBoolean("True"));
    EXPECT_TRUE(Convert::ToBoolean("true"));
    EXPECT_FALSE(Convert::ToBoolean("False"));
    EXPECT_FALSE(Convert::ToBoolean("false"));
}

// Regression tests for a code-audit finding (ticket 248): verified against Convert.cs's
// ToBoolean(string) (delegates to bool.Parse) and Boolean.cs's TryParse -- real .NET's
// Convert.ToBoolean(string) is fully case-insensitive for "true"/"false" (not just the two
// exact casings previously matched here) and does NOT accept "1"/"0" at all (those throw
// FormatException). Convert::ToBoolean("1")/("0") previously returned true/false instead of
// throwing.
TEST(ConvertTests, ToBooleanFromString_CaseInsensitive) {
    EXPECT_TRUE(Convert::ToBoolean("TRUE"));
    EXPECT_TRUE(Convert::ToBoolean("TrUe"));
    EXPECT_FALSE(Convert::ToBoolean("FALSE"));
    EXPECT_FALSE(Convert::ToBoolean("FaLsE"));
}

TEST(ConvertTests, ToBooleanFromString_OneAndZero_Throws) {
    EXPECT_THROW(Convert::ToBoolean("1"), std::exception);
    EXPECT_THROW(Convert::ToBoolean("0"), std::exception);
}

TEST(ConvertTests, ToBooleanFromStringInvalidThrows) {
    EXPECT_THROW(Convert::ToBoolean("yes"),  std::exception);
    EXPECT_THROW(Convert::ToBoolean(""),     std::exception);
}

// ---------------------------------------------------------------------------
// ToString
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToStringFromInt) {
    EXPECT_EQ(Convert::ToString(42),   "42");
    EXPECT_EQ(Convert::ToString(-7),   "-7");
    EXPECT_EQ(Convert::ToString(0),    "0");
}

TEST(ConvertTests, ToStringFromLong) {
    EXPECT_EQ(Convert::ToString(static_cast<int64_t>(1000000000000LL)), "1000000000000");
}

TEST(ConvertTests, ToStringFromBool) {
    EXPECT_EQ(Convert::ToString(true),  "True");
    EXPECT_EQ(Convert::ToString(false), "False");
}

TEST(ConvertTests, ToStringFromChar) {
    EXPECT_EQ(Convert::ToString('A'), "A");
    EXPECT_EQ(Convert::ToString('z'), "z");
}

TEST(ConvertTests, ToStringFromByte) {
    EXPECT_EQ(Convert::ToString(static_cast<uint8_t>(200)), "200");
}

// ---------------------------------------------------------------------------
// ToString with base
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToStringBase16) {
    EXPECT_EQ(Convert::ToString(255, 16), "ff");
    EXPECT_EQ(Convert::ToString(0,   16), "0");
    EXPECT_EQ(Convert::ToString(16,  16), "10");
}

TEST(ConvertTests, ToStringBase2) {
    EXPECT_EQ(Convert::ToString(10,  2), "1010");
    EXPECT_EQ(Convert::ToString(255, 2), "11111111");
    EXPECT_EQ(Convert::ToString(0,   2), "0");
}

TEST(ConvertTests, ToStringBase8) {
    EXPECT_EQ(Convert::ToString(255, 8), "377");
    EXPECT_EQ(Convert::ToString(15,  8), "17");
}

TEST(ConvertTests, ToStringBase10) {
    EXPECT_EQ(Convert::ToString(42, 10), "42");
}

TEST(ConvertTests, ToStringInvalidBaseThrows) {
    EXPECT_THROW(Convert::ToString(42, 3), std::exception);
}

// ---------------------------------------------------------------------------
// ToHexString / FromHexString
//
// Regression coverage for a code-audit finding (ticket 248): verified against Convert.cs,
// Convert.ToHexString produces UPPERCASE (HexConverter.Casing.Upper) -- this port previously
// had the casing backwards (ToHexString was lowercase, and the uppercase implementation was
// under the non-existent-in-.NET name ToHexStringUpper, since renamed to ToHexStringLower).
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToHexString_Empty) {
    EXPECT_EQ(Convert::ToHexString({}), "");
}

TEST(ConvertTests, ToHexString_SingleByte) {
    EXPECT_EQ(Convert::ToHexString({0xFF}), "FF");
}

TEST(ConvertTests, ToHexString_MultipleBytes) {
    EXPECT_EQ(Convert::ToHexString({0x00, 0xAB, 0xCD, 0xEF}), "00ABCDEF");
}

TEST(ConvertTests, ToHexString_AllZeros) {
    EXPECT_EQ(Convert::ToHexString({0x00, 0x00}), "0000");
}

TEST(ConvertTests, FromHexString_Empty) {
    EXPECT_EQ(Convert::FromHexString("").size(), 0u);
}

TEST(ConvertTests, FromHexString_LowerCase) {
    auto v = Convert::FromHexString("ff");
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 0xFF);
}

TEST(ConvertTests, FromHexString_UpperCase) {
    auto v = Convert::FromHexString("ABCD");
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 0xAB);
    EXPECT_EQ(v[1], 0xCD);
}

TEST(ConvertTests, FromHexString_MultipleBytes) {
    auto v = Convert::FromHexString("00abcdef");
    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 0x00);
    EXPECT_EQ(v[1], 0xAB);
    EXPECT_EQ(v[2], 0xCD);
    EXPECT_EQ(v[3], 0xEF);
}

TEST(ConvertTests, FromHexString_OddLengthThrows) {
    EXPECT_THROW(Convert::FromHexString("abc"), std::exception);
}

TEST(ConvertTests, FromHexString_InvalidCharThrows) {
    EXPECT_THROW(Convert::FromHexString("zz"), std::exception);
}

TEST(ConvertTests, HexRoundTrip) {
    std::vector<uint8_t> original{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    auto hex = Convert::ToHexString(original);
    auto restored = Convert::FromHexString(hex);
    EXPECT_EQ(original, restored);
}

// ---------------------------------------------------------------------------
// ToBase64String / FromBase64String
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToBase64String_KnownValue) {
    // "Man" → "TWFu" per RFC 4648
    std::vector<uint8_t> bytes = {'M', 'a', 'n'};
    EXPECT_EQ(Convert::ToBase64String(bytes), "TWFu");
}

TEST(ConvertTests, ToBase64String_Empty) {
    EXPECT_EQ(Convert::ToBase64String({}), "");
}

TEST(ConvertTests, ToBase64String_OneByte_HasPadding) {
    std::vector<uint8_t> bytes = {0x01};
    std::string b64 = Convert::ToBase64String(bytes);
    EXPECT_EQ(b64.size() % 4, 0u);
    EXPECT_EQ(b64.back(), '=');
}

TEST(ConvertTests, ToBase64String_TwoBytes_HasOnePad) {
    std::vector<uint8_t> bytes = {0x01, 0x02};
    std::string b64 = Convert::ToBase64String(bytes);
    EXPECT_EQ(b64.size() % 4, 0u);
    EXPECT_EQ(b64.back(), '=');
}

TEST(ConvertTests, Base64RoundTrip_Short) {
    std::vector<uint8_t> original = {0xDE, 0xAD, 0xBE, 0xEF};
    auto b64 = Convert::ToBase64String(original);
    auto restored = Convert::FromBase64String(b64);
    EXPECT_EQ(original, restored);
}

TEST(ConvertTests, Base64RoundTrip_Long) {
    std::vector<uint8_t> original;
    for (int i = 0; i < 50; ++i) original.push_back(static_cast<uint8_t>(i));
    auto b64 = Convert::ToBase64String(original);
    auto restored = Convert::FromBase64String(b64);
    EXPECT_EQ(original, restored);
}

TEST(ConvertTests, FromBase64String_KnownValue) {
    auto bytes = Convert::FromBase64String("TWFu");
    EXPECT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], 'M');
    EXPECT_EQ(bytes[1], 'a');
    EXPECT_EQ(bytes[2], 'n');
}

TEST(ConvertTests, FromBase64String_InvalidLength_Throws) {
    EXPECT_THROW(Convert::FromBase64String("AB"), std::exception);
}

TEST(ConvertTests, FromBase64String_InvalidChar_Throws) {
    EXPECT_THROW(Convert::FromBase64String("TW!u"), std::exception);
}

// ---------------------------------------------------------------------------
// ToUInt32 / ToUInt64 / ToChar
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToUInt32_FromInt_Positive) {
    EXPECT_EQ(Convert::ToUInt32(static_cast<SharpRuntime::intcs>(42)), 42u);
}
TEST(ConvertTests, ToUInt32_FromLong) {
    EXPECT_EQ(Convert::ToUInt32(static_cast<SharpRuntime::longcs>(1000LL)), 1000u);
}
TEST(ConvertTests, ToUInt32_FromString) {
    EXPECT_EQ(Convert::ToUInt32(std::string("4294967295")), 4294967295u);
}

TEST(ConvertTests, ToUInt64_FromInt) {
    EXPECT_EQ(Convert::ToUInt64(static_cast<SharpRuntime::intcs>(99)), 99ull);
}
TEST(ConvertTests, ToUInt64_FromLong) {
    EXPECT_EQ(Convert::ToUInt64(static_cast<SharpRuntime::longcs>(9999999999LL)), 9999999999ull);
}
TEST(ConvertTests, ToUInt64_FromString) {
    EXPECT_EQ(Convert::ToUInt64(std::string("18446744073709551615")), 18446744073709551615ull);
}

TEST(ConvertTests, ToChar_FromInt) {
    EXPECT_EQ(Convert::ToChar(static_cast<SharpRuntime::intcs>(65)), 'A');
}
TEST(ConvertTests, ToChar_Zero) {
    EXPECT_EQ(Convert::ToChar(static_cast<SharpRuntime::intcs>(0)), '\0');
}

// ---------------------------------------------------------------------------
// Convert — additional overloads (Task 99)
// ---------------------------------------------------------------------------

TEST(ConvertTests, ToInt64_FromFloat) {
    EXPECT_EQ(Convert::ToInt64(2.9f), static_cast<SharpRuntime::longcs>(3));
}
TEST(ConvertTests, ToInt64_FromBool_True) {
    EXPECT_EQ(Convert::ToInt64(true), 1LL);
}
TEST(ConvertTests, ToInt64_FromBool_False) {
    EXPECT_EQ(Convert::ToInt64(false), 0LL);
}

TEST(ConvertTests, ToDouble_FromFloat) {
    EXPECT_NEAR(Convert::ToDouble(1.5f), 1.5, 1e-6);
}
TEST(ConvertTests, ToDouble_FromBool_True) {
    EXPECT_EQ(Convert::ToDouble(true), 1.0);
}
TEST(ConvertTests, ToDouble_FromBool_False) {
    EXPECT_EQ(Convert::ToDouble(false), 0.0);
}

TEST(ConvertTests, ToSingle_FromLong) {
    EXPECT_EQ(Convert::ToSingle(static_cast<SharpRuntime::longcs>(4LL)), 4.0f);
}
TEST(ConvertTests, ToSingle_FromBool_True)  { EXPECT_EQ(Convert::ToSingle(true),  1.0f); }
TEST(ConvertTests, ToSingle_FromBool_False) { EXPECT_EQ(Convert::ToSingle(false), 0.0f); }

TEST(ConvertTests, ToByte_FromLong) {
    EXPECT_EQ(Convert::ToByte(static_cast<SharpRuntime::longcs>(200LL)), SharpRuntime::bytecs(200));
}
TEST(ConvertTests, ToByte_FromBool_True)  { EXPECT_EQ(Convert::ToByte(true),  SharpRuntime::bytecs(1)); }
TEST(ConvertTests, ToByte_FromBool_False) { EXPECT_EQ(Convert::ToByte(false), SharpRuntime::bytecs(0)); }
TEST(ConvertTests, ToByte_FromDouble) {
    EXPECT_EQ(Convert::ToByte(65.9), SharpRuntime::bytecs(66));
}
TEST(ConvertTests, ToByte_FromFloat) {
    EXPECT_EQ(Convert::ToByte(10.5f), SharpRuntime::bytecs(10));
}

TEST(ConvertTests, ToBoolean_FromLong_NonZero) { EXPECT_TRUE(Convert::ToBoolean(static_cast<SharpRuntime::longcs>(1LL))); }
TEST(ConvertTests, ToBoolean_FromLong_Zero)    { EXPECT_FALSE(Convert::ToBoolean(static_cast<SharpRuntime::longcs>(0LL))); }
TEST(ConvertTests, ToBoolean_FromDouble_NonZero) { EXPECT_TRUE(Convert::ToBoolean(3.14)); }
TEST(ConvertTests, ToBoolean_FromDouble_Zero)    { EXPECT_FALSE(Convert::ToBoolean(0.0)); }
TEST(ConvertTests, ToBoolean_FromFloat_NonZero)  { EXPECT_TRUE(Convert::ToBoolean(0.1f)); }
TEST(ConvertTests, ToBoolean_FromFloat_Zero)     { EXPECT_FALSE(Convert::ToBoolean(0.0f)); }
