// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for System::String static helpers.
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/String.hpp"

using System::String;

// --- Split (multi-char / string delimiter) ---

TEST(StringTests, Split_MultiChar_TwoDelimiters) {
    auto r = String::Split("a,b;c", std::vector<char>{',', ';'});
    ASSERT_EQ(r.size(), 3u);
    EXPECT_EQ(r[0], "a");
    EXPECT_EQ(r[1], "b");
    EXPECT_EQ(r[2], "c");
}

TEST(StringTests, Split_MultiChar_NoDelimiterFound) {
    auto r = String::Split("hello", std::vector<char>{','});
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0], "hello");
}

TEST(StringTests, Split_StringDelimiter) {
    auto r = String::Split("one||two||three", "||");
    ASSERT_EQ(r.size(), 3u);
    EXPECT_EQ(r[0], "one");
    EXPECT_EQ(r[1], "two");
    EXPECT_EQ(r[2], "three");
}

TEST(StringTests, Split_StringDelimiter_EmptyDelimiter) {
    auto r = String::Split("hello", "");
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0], "hello");
}

TEST(StringTests, Split_StringDelimiter_NoMatch) {
    auto r = String::Split("hello", "||");
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0], "hello");
}

// --- Split (single-char delimiter) ---
// Regression coverage for the 2026-07-13 performance pass (stringstream -> manual find/substr
// scan), which also fixed a pre-existing correctness bug -- see String::Split(value, char)'s
// doc-comment and String.cpp's own comment for the .NET-reference verification.

TEST(StringTests, Split_Char_EmptyValue_ReturnsSingleEmptyString) {
    // Matches real .NET's "".Split(',') == {""} (String.Manipulation.cs's
    // CreateSplitArrayOfThisAsSoleValue short-circuit for a zero-length input) -- the pre-fix
    // stringstream-based implementation incorrectly returned an empty vector here.
    auto r = String::Split("", ',');
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0], "");
}

TEST(StringTests, Split_Char_NoDelimiterFound) {
    auto r = String::Split("hello", ',');
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0], "hello");
}

TEST(StringTests, Split_Char_MultipleDelimiters) {
    auto r = String::Split("a,b,c", ',');
    ASSERT_EQ(r.size(), 3u);
    EXPECT_EQ(r[0], "a");
    EXPECT_EQ(r[1], "b");
    EXPECT_EQ(r[2], "c");
}

TEST(StringTests, Split_Char_TrailingDelimiter_YieldsTrailingEmptyString) {
    auto r = String::Split("a,b,", ',');
    ASSERT_EQ(r.size(), 3u);
    EXPECT_EQ(r[0], "a");
    EXPECT_EQ(r[1], "b");
    EXPECT_EQ(r[2], "");
}

TEST(StringTests, Split_Char_LeadingDelimiter_YieldsLeadingEmptyString) {
    auto r = String::Split(",a,b", ',');
    ASSERT_EQ(r.size(), 3u);
    EXPECT_EQ(r[0], "");
    EXPECT_EQ(r[1], "a");
    EXPECT_EQ(r[2], "b");
}

TEST(StringTests, Split_Char_ConsecutiveDelimiters_YieldEmptyStringsBetween) {
    auto r = String::Split("a,,b", ',');
    ASSERT_EQ(r.size(), 3u);
    EXPECT_EQ(r[0], "a");
    EXPECT_EQ(r[1], "");
    EXPECT_EQ(r[2], "b");
}

// --- IsNullOrWhiteSpace ---

TEST(StringTests, IsNullOrWhiteSpace_Empty) {
    EXPECT_TRUE(String::IsNullOrWhiteSpace(""));
}
TEST(StringTests, IsNullOrWhiteSpace_Spaces) {
    EXPECT_TRUE(String::IsNullOrWhiteSpace("   "));
}
TEST(StringTests, IsNullOrWhiteSpace_Tabs) {
    EXPECT_TRUE(String::IsNullOrWhiteSpace("\t\n\r"));
}
TEST(StringTests, IsNullOrWhiteSpace_NotWhitespace) {
    EXPECT_FALSE(String::IsNullOrWhiteSpace("hello"));
}
TEST(StringTests, IsNullOrWhiteSpace_LeadingSpace) {
    EXPECT_FALSE(String::IsNullOrWhiteSpace("  x"));
}

// --- EndsWith ---

TEST(StringTests, EndsWith_True) {
    EXPECT_TRUE(String::EndsWith("hello world", "world"));
}
TEST(StringTests, EndsWith_False) {
    EXPECT_FALSE(String::EndsWith("hello world", "hello"));
}
TEST(StringTests, EndsWith_EmptySuffix) {
    EXPECT_TRUE(String::EndsWith("hello", ""));
}
TEST(StringTests, EndsWith_SuffixLonger) {
    EXPECT_FALSE(String::EndsWith("hi", "hello"));
}
TEST(StringTests, EndsWith_ExactMatch) {
    EXPECT_TRUE(String::EndsWith("abc", "abc"));
}

// --- Contains ---

TEST(StringTests, Contains_True) {
    EXPECT_TRUE(String::Contains("hello world", "world"));
}
TEST(StringTests, Contains_False) {
    EXPECT_FALSE(String::Contains("hello", "xyz"));
}
TEST(StringTests, Contains_EmptySubstr) {
    EXPECT_TRUE(String::Contains("hello", ""));
}

// --- Replace (string) ---

TEST(StringTests, Replace_String_Single) {
    EXPECT_EQ(String::Replace("hello world", "world", "there"), "hello there");
}
TEST(StringTests, Replace_String_Multiple) {
    EXPECT_EQ(String::Replace("aaa", "a", "b"), "bbb");
}
TEST(StringTests, Replace_String_NoMatch) {
    EXPECT_EQ(String::Replace("hello", "xyz", "abc"), "hello");
}
TEST(StringTests, Replace_String_EmptyOld) {
    EXPECT_EQ(String::Replace("hello", "", "x"), "hello");
}

// --- Replace (char) ---

TEST(StringTests, Replace_Char) {
    EXPECT_EQ(String::Replace("hello", 'l', 'r'), "herro");
}
TEST(StringTests, Replace_Char_NoMatch) {
    EXPECT_EQ(String::Replace("hello", 'z', 'x'), "hello");
}

// --- Substring ---

TEST(StringTests, Substring_FromIndex) {
    EXPECT_EQ(String::Substring("hello world", 6), "world");
}
TEST(StringTests, Substring_FromZero) {
    EXPECT_EQ(String::Substring("hello", 0), "hello");
}
TEST(StringTests, Substring_WithLength) {
    EXPECT_EQ(String::Substring("hello world", 0, 5), "hello");
}
TEST(StringTests, Substring_MiddleWithLength) {
    EXPECT_EQ(String::Substring("hello world", 6, 5), "world");
}
TEST(StringTests, Substring_LengthTooLong_Throws) {
    EXPECT_THROW(String::Substring("abcde", 2, 100), System::ArgumentOutOfRangeException);
}
TEST(StringTests, Substring_NegativeLength_Throws) {
    EXPECT_THROW(String::Substring("abcde", 0, -1), System::ArgumentOutOfRangeException);
}

// Regression tests for a code-audit finding (ticket 237): the single-arg Substring(string,
// startIndex) had no validation at all, unlike its two-arg sibling above -- an
// out-of-bounds startIndex fell straight into std::string::substr(), which throws
// std::out_of_range (an unrelated std:: exception type invisible to code catching
// System::Exception&) instead of the System::ArgumentOutOfRangeException real .NET's
// String.Substring(int) throws for the same input (verified against
// String.Manipulation.cs's `(uint)startIndex > (uint)Length` check).
TEST(StringTests, Substring_SingleArg_NegativeStartIndex_Throws) {
    EXPECT_THROW(String::Substring("abcde", -1), System::ArgumentOutOfRangeException);
}
TEST(StringTests, Substring_SingleArg_StartIndexTooLarge_Throws) {
    EXPECT_THROW(String::Substring("abcde", 100), System::ArgumentOutOfRangeException);
}
TEST(StringTests, Substring_SingleArg_StartIndexEqualsLength_ReturnsEmpty) {
    EXPECT_EQ(String::Substring("abcde", 5), "");
}

// --- Trim ---

TEST(StringTests, Trim_Both) {
    EXPECT_EQ(String::Trim("  hello  "), "hello");
}
TEST(StringTests, Trim_None) {
    EXPECT_EQ(String::Trim("hello"), "hello");
}
TEST(StringTests, Trim_AllWhitespace) {
    EXPECT_EQ(String::Trim("   "), "");
}
TEST(StringTests, TrimStart) {
    EXPECT_EQ(String::TrimStart("  hello  "), "hello  ");
}
TEST(StringTests, TrimEnd) {
    EXPECT_EQ(String::TrimEnd("  hello  "), "  hello");
}

// --- Concat ---

TEST(StringTests, Concat_Two) {
    EXPECT_EQ(String::Concat("foo", "bar"), "foobar");
}
TEST(StringTests, Concat_Three) {
    EXPECT_EQ(String::Concat("a", "b", "c"), "abc");
}
TEST(StringTests, Concat_Four) {
    EXPECT_EQ(String::Concat("a", "b", "c", "d"), "abcd");
}
TEST(StringTests, Concat_Vector) {
    EXPECT_EQ(String::Concat(std::vector<std::string>{"x", "y", "z"}), "xyz");
}
TEST(StringTests, Concat_VectorEmpty) {
    EXPECT_EQ(String::Concat(std::vector<std::string>{}), "");
}

// --- Join ---

TEST(StringTests, Join_Comma) {
    EXPECT_EQ(String::Join(", ", {"a", "b", "c"}), "a, b, c");
}
TEST(StringTests, Join_Empty) {
    EXPECT_EQ(String::Join(",", {}), "");
}
TEST(StringTests, Join_Single) {
    EXPECT_EQ(String::Join(",", {"only"}), "only");
}
TEST(StringTests, Join_EmptySeparator) {
    EXPECT_EQ(String::Join("", {"a", "b", "c"}), "abc");
}

// --- ToUpper / ToLower ---

TEST(StringTests, ToUpper_Basic) {
    EXPECT_EQ(String::ToUpper("hello"), "HELLO");
}
TEST(StringTests, ToUpper_AlreadyUpper) {
    EXPECT_EQ(String::ToUpper("WORLD"), "WORLD");
}
TEST(StringTests, ToLower_Basic) {
    EXPECT_EQ(String::ToLower("HELLO"), "hello");
}
TEST(StringTests, ToLower_Mixed) {
    EXPECT_EQ(String::ToLower("HeLLo"), "hello");
}

// --- IndexOf ---

TEST(StringTests, IndexOf_String_Found) {
    EXPECT_EQ(String::IndexOf("hello world", "world"), 6);
}
TEST(StringTests, IndexOf_String_NotFound) {
    EXPECT_EQ(String::IndexOf("hello", "xyz"), -1);
}
TEST(StringTests, IndexOf_Char_Found) {
    EXPECT_EQ(String::IndexOf("hello", 'l'), 2);
}
TEST(StringTests, IndexOf_Char_NotFound) {
    EXPECT_EQ(String::IndexOf("hello", 'z'), -1);
}

// --- LastIndexOf ---

TEST(StringTests, LastIndexOf_String_Found) {
    EXPECT_EQ(String::LastIndexOf("abcabc", "bc"), 4);
}
TEST(StringTests, LastIndexOf_String_NotFound) {
    EXPECT_EQ(String::LastIndexOf("hello", "xyz"), -1);
}
TEST(StringTests, LastIndexOf_Char_Found) {
    EXPECT_EQ(String::LastIndexOf("hello", 'l'), 3);
}
TEST(StringTests, LastIndexOf_Char_NotFound) {
    EXPECT_EQ(String::LastIndexOf("hello", 'z'), -1);
}

// Real .NET's CompareInfo.LastIndexOf special-cases startIndex == source.Length as valid (an
// off-by-one compat fixup, treated as startIndex == length - 1) rather than throwing -- a natural
// "search the whole string backward" idiom using str.Length as the start index must not throw.
TEST(StringTests, LastIndexOf_String_StartIndexEqualsLength_DoesNotThrow) {
    EXPECT_EQ(String::LastIndexOf("abcabc", "bc", 6), 4);
}
TEST(StringTests, LastIndexOf_Char_StartIndexEqualsLength_DoesNotThrow) {
    EXPECT_EQ(String::LastIndexOf("hello", 'l', 5), 3);
}
TEST(StringTests, LastIndexOf_String_StartIndexGreaterThanLength_StillThrows) {
    EXPECT_THROW(String::LastIndexOf("hello", "l", 6), System::ArgumentOutOfRangeException);
}
TEST(StringTests, LastIndexOf_Char_StartIndexGreaterThanLength_StillThrows) {
    EXPECT_THROW(String::LastIndexOf("hello", 'l', 6), System::ArgumentOutOfRangeException);
}
TEST(StringTests, LastIndexOf_CharWithCount_StartIndexEqualsLength_DoesNotThrow) {
    // startIndex==5 (length) fixes up to startIndex=4, count=3-1=2 -> examines indices 3,4 ("lo").
    EXPECT_EQ(String::LastIndexOf("hello", 'l', 5, 3), 3);
}
TEST(StringTests, LastIndexOf_StringWithCount_StartIndexEqualsLength_DoesNotThrow) {
    EXPECT_EQ(String::LastIndexOf("abcabc", "bc", 6, 4), 4);
}

// --- PadLeft ---

TEST(StringTests, PadLeft_WithSpaces) {
    EXPECT_EQ(String::PadLeft("42", 5), "   42");
}
TEST(StringTests, PadLeft_WithChar) {
    EXPECT_EQ(String::PadLeft("42", 5, '0'), "00042");
}
TEST(StringTests, PadLeft_AlreadyWide) {
    EXPECT_EQ(String::PadLeft("hello", 3), "hello");
}

// Regression test for a code-audit finding (ticket 237): negative totalWidth was silently
// treated as "already wide enough" instead of throwing, unlike real .NET's PadLeft(int,
// char), which calls ArgumentOutOfRangeException.ThrowIfNegative(totalWidth).
TEST(StringTests, PadLeft_NegativeTotalWidth_Throws) {
    EXPECT_THROW(String::PadLeft("hello", -1), System::ArgumentOutOfRangeException);
}

// --- PadRight ---

TEST(StringTests, PadRight_WithSpaces) {
    EXPECT_EQ(String::PadRight("42", 5), "42   ");
}
TEST(StringTests, PadRight_WithChar) {
    EXPECT_EQ(String::PadRight("hi", 5, '-'), "hi---");
}
TEST(StringTests, PadRight_AlreadyWide) {
    EXPECT_EQ(String::PadRight("hello", 3), "hello");
}

// Regression test for a code-audit finding (ticket 237): same negative-totalWidth gap as
// PadLeft above.
TEST(StringTests, PadRight_NegativeTotalWidth_Throws) {
    EXPECT_THROW(String::PadRight("hello", -1), System::ArgumentOutOfRangeException);
}

// --- Format specifiers ---

TEST(StringTests, Format_Int_Plain) {
    EXPECT_EQ(String::Format("{0}", 42), "42");
}
TEST(StringTests, Format_Int_HexUpper) {
    EXPECT_EQ(String::Format("{0:X}", 255), "FF");
}
TEST(StringTests, Format_Int_HexLower) {
    EXPECT_EQ(String::Format("{0:x}", 255), "ff");
}
TEST(StringTests, Format_Int_HexPadded) {
    EXPECT_EQ(String::Format("{0:X4}", 255), "00FF");
}
TEST(StringTests, Format_Int_DecimalPadded) {
    EXPECT_EQ(String::Format("{0:D3}", 7), "007");
}
TEST(StringTests, Format_Int_DecimalPadded_Negative) {
    EXPECT_EQ(String::Format("{0:D3}", -7), "-007");
}
TEST(StringTests, Format_Double_Fixed) {
    EXPECT_EQ(String::Format("{0:F2}", 3.14159), "3.14");
}
TEST(StringTests, Format_Double_Fixed_Zero) {
    EXPECT_EQ(String::Format("{0:F0}", 3.7), "4");
}
TEST(StringTests, Format_Double_Plain) {
    EXPECT_EQ(String::Format("{0}", 1.5), "1.5");
}

// --- Format multi-arg ---

TEST(StringTests, Format_TwoInts) {
    EXPECT_EQ(String::Format("{0} / {1}", 10, 20), "10 / 20");
}
TEST(StringTests, Format_TwoInts_WithSpecs) {
    EXPECT_EQ(String::Format("{0:D2}:{1:D2}", 3, 5), "03:05");
}
TEST(StringTests, Format_IntAndString) {
    EXPECT_EQ(String::Format("HP: {0}/{1}", 80, std::string("100")), "HP: 80/100");
}
TEST(StringTests, Format_StringAndInt) {
    EXPECT_EQ(String::Format("{0}={1}", std::string("x"), 7), "x=7");
}
TEST(StringTests, Format_TwoStrings) {
    EXPECT_EQ(String::Format("{0} {1}", std::string("hello"), std::string("world")), "hello world");
}
TEST(StringTests, Format_TwoDoubles) {
    EXPECT_EQ(String::Format("({0:F1},{1:F1})", 1.25, 3.75), "(1.2,3.8)");
}
TEST(StringTests, Format_BoolTrue) {
    EXPECT_EQ(String::Format("{0}", true), "True");
}
TEST(StringTests, Format_BoolFalse) {
    EXPECT_EQ(String::Format("{0}", false), "False");
}
TEST(StringTests, Format_BoolInSentence) {
    EXPECT_EQ(String::Format("value={0}", true), "value=True");
}
TEST(StringTests, Format_Char) {
    EXPECT_EQ(String::Format("{0}", 'A'), "A");
}
TEST(StringTests, Format_CharInSentence) {
    EXPECT_EQ(String::Format("key={0}", 'Z'), "key=Z");
}

// --- Format validation (2026-07-13 fix: previously silently left unresolved placeholders in
// the output instead of throwing, matching real .NET's actual FormatException behavior) ---

TEST(StringTests, Format_OutOfRangeIndex_Throws) {
    EXPECT_THROW(String::Format("{5}", 42), System::FormatException);
}

TEST(StringTests, Format_OutOfRangeIndex_TwoArgOverload_Throws) {
    EXPECT_THROW(String::Format("{0} {2}", 1, 2), System::FormatException);
}

TEST(StringTests, Format_UnclosedBrace_Throws) {
    EXPECT_THROW(String::Format("value={0", 42), System::FormatException);
}

TEST(StringTests, Format_MalformedBrace_NoDigit_Throws) {
    EXPECT_THROW(String::Format("{abc}", 42), System::FormatException);
}

TEST(StringTests, Format_TwoDigitIndexDoesNotAliasSingleDigitIndex) {
    // Regression for a prefix-matching bug in replaceArg: naively find()-ing "{1" would also
    // match inside "{10}" (a different, out-of-range index for a 2-arg call), silently
    // corrupting the output instead of leaving "{10}" for FinalizeFormat to reject.
    EXPECT_THROW(String::Format("{1} and {10}", std::string("a"), std::string("b")), System::FormatException);
}

TEST(StringTests, Format_ValidPlaceholders_DoNotThrow) {
    EXPECT_NO_THROW(String::Format("{0} and {1}", std::string("a"), std::string("b")));
}

// --- IndexOfAny ---
TEST(StringTests, IndexOfAny_Found_FirstChar) {
    EXPECT_EQ(String::IndexOfAny("hello,world", {',', '.', '!'}), 5);
}
TEST(StringTests, IndexOfAny_Found_SecondInList) {
    EXPECT_EQ(String::IndexOfAny("hello world", {'x', ' '}), 5);
}
TEST(StringTests, IndexOfAny_NotFound) {
    EXPECT_EQ(String::IndexOfAny("hello", {',', '.'}), -1);
}
TEST(StringTests, IndexOfAny_Empty) {
    EXPECT_EQ(String::IndexOfAny("", {'a'}), -1);
}

// --- LastIndexOfAny ---
TEST(StringTests, LastIndexOfAny_Found) {
    EXPECT_EQ(String::LastIndexOfAny("hello,world,end", {','}), 11);
}
TEST(StringTests, LastIndexOfAny_MultipleChars) {
    EXPECT_EQ(String::LastIndexOfAny("abc.def/ghi", {'.', '/'}), 7);
}
TEST(StringTests, LastIndexOfAny_NotFound) {
    EXPECT_EQ(String::LastIndexOfAny("hello", {'x', 'z'}), -1);
}

// --- Contains(char) ---
TEST(StringTests, Contains_Char_Found) {
    EXPECT_TRUE(String::Contains("hello", 'e'));
}
TEST(StringTests, Contains_Char_NotFound) {
    EXPECT_FALSE(String::Contains("hello", 'z'));
}
TEST(StringTests, Contains_Char_Empty) {
    EXPECT_FALSE(String::Contains("", 'a'));
}

// --- IndexOf with startIndex ---
TEST(StringTests, IndexOf_StringStartIndex_Found) {
    EXPECT_EQ(String::IndexOf("abcabc", "bc", 2), 4);
}
TEST(StringTests, IndexOf_StringStartIndex_NotFound) {
    EXPECT_EQ(String::IndexOf("abcabc", "bc", 5), -1);
}
TEST(StringTests, IndexOf_CharStartIndex_Found) {
    EXPECT_EQ(String::IndexOf("hello world", 'o', 5), 7);
}
TEST(StringTests, IndexOf_CharStartIndex_NotFound) {
    EXPECT_EQ(String::IndexOf("hello", 'z', 0), -1);
}

// --- LastIndexOf with startIndex ---
TEST(StringTests, LastIndexOf_StringStartIndex_Found) {
    EXPECT_EQ(String::LastIndexOf("abcabc", "ab", 4), 3);
}
TEST(StringTests, LastIndexOf_StringStartIndex_BeforeFirst) {
    EXPECT_EQ(String::LastIndexOf("abcabc", "ab", 1), 0);
}
TEST(StringTests, LastIndexOf_CharStartIndex_Found) {
    EXPECT_EQ(String::LastIndexOf("hello world", 'l', 5), 3);
}
TEST(StringTests, LastIndexOf_CharStartIndex_NotFound) {
    EXPECT_EQ(String::LastIndexOf("hello", 'z', 4), -1);
}

// Regression tests for a code-audit finding (ticket 237): all four of these startIndex-only
// (no count) overloads silently returned -1 for an out-of-bounds startIndex instead of
// throwing, unlike their startIndex+count siblings tested further down in this file (e.g.
// IndexOf_CharCount_StartIndexOutOfRange_Throws) -- an out-of-range std::string::find/rfind
// pos is well-defined to just return npos rather than throw, so nothing crashed, but real
// .NET's String.IndexOf(string/char, int startIndex) and LastIndexOf(...) both throw
// ArgumentOutOfRangeException for the same input, making the old behavior "silently wrong"
// rather than merely differently-typed.
TEST(StringTests, IndexOf_StringStartIndex_NegativeStartIndex_Throws) {
    EXPECT_THROW(String::IndexOf("abc", "b", -1), System::ArgumentOutOfRangeException);
}
TEST(StringTests, IndexOf_CharStartIndex_StartIndexTooLarge_Throws) {
    EXPECT_THROW(String::IndexOf("abc", 'b', 100), System::ArgumentOutOfRangeException);
}
TEST(StringTests, LastIndexOf_StringStartIndex_NegativeStartIndex_Throws) {
    EXPECT_THROW(String::LastIndexOf("abc", "b", -1), System::ArgumentOutOfRangeException);
}
TEST(StringTests, LastIndexOf_CharStartIndex_StartIndexTooLarge_Throws) {
    EXPECT_THROW(String::LastIndexOf("abc", 'b', 100), System::ArgumentOutOfRangeException);
}

// --- String::Create ---
TEST(StringTests, Create_RepeatsChar) {
    EXPECT_EQ(String::Create(5, 'x'), "xxxxx");
}
TEST(StringTests, Create_ZeroCount) {
    EXPECT_EQ(String::Create(0, 'a'), "");
}

// Regression test for a code-audit finding (ticket 237): a negative count wrapped to a huge
// size_t and fell straight into the std::string(size_t, char) constructor, which throws
// std::length_error (an unrelated std:: exception type) or attempts a massive allocation,
// instead of the System::ArgumentOutOfRangeException real .NET's `new string(char, int)`
// throws for a negative count (String.cs's Ctor(char, int):
// ArgumentOutOfRangeException.ThrowIfNegative(count)).
TEST(StringTests, Create_NegativeCount_Throws) {
    EXPECT_THROW(String::Create(-1, 'x'), System::ArgumentOutOfRangeException);
}

// --- String::Compare ---
TEST(StringTests, Compare_Equal_ReturnsZero) {
    EXPECT_EQ(String::Compare("abc", "abc"), 0);
}
TEST(StringTests, Compare_Less_ReturnsNegative) {
    EXPECT_LT(String::Compare("abc", "abd"), 0);
}
TEST(StringTests, Compare_Greater_ReturnsPositive) {
    EXPECT_GT(String::Compare("abd", "abc"), 0);
}
TEST(StringTests, Compare_IgnoreCase_Equal) {
    EXPECT_EQ(String::Compare("Hello", "hello", true), 0);
}
TEST(StringTests, Compare_IgnoreCase_Different) {
    EXPECT_NE(String::Compare("Hello", "world", true), 0);
}

// --- String::Equals (static) ---
TEST(StringTests, Equals_SameString_True) {
    EXPECT_TRUE(String::Equals("foo", "foo"));
}
TEST(StringTests, Equals_DifferentString_False) {
    EXPECT_FALSE(String::Equals("foo", "bar"));
}
TEST(StringTests, Equals_IgnoreCase_True) {
    EXPECT_TRUE(String::Equals("Hello", "HELLO", true));
}
TEST(StringTests, Equals_IgnoreCase_False) {
    EXPECT_FALSE(String::Equals("foo", "bar", true));
}

// --- String::Format 3-arg ---
TEST(StringTests, Format_ThreeInts) {
    EXPECT_EQ(String::Format("{0}/{1}/{2}", 1, 2, 3), "1/2/3");
}
TEST(StringTests, Format_ThreeStrings) {
    EXPECT_EQ(String::Format("{0}-{1}-{2}", std::string("a"), std::string("b"), std::string("c")), "a-b-c");
}

// --- String::Format longcs ---
TEST(StringTests, Format_LongArg) {
    SharpRuntime::longcs big = 9876543210LL;
    EXPECT_EQ(String::Format("val={0}", big), "val=9876543210");
}

// --- String::StartsWith(char) / EndsWith(char) ---
TEST(StringTests, StartsWithChar_True)  { EXPECT_TRUE(String::StartsWith("hello", 'h')); }
TEST(StringTests, StartsWithChar_False) { EXPECT_FALSE(String::StartsWith("hello", 'e')); }
TEST(StringTests, StartsWithChar_Empty) { EXPECT_FALSE(String::StartsWith("", 'h')); }
TEST(StringTests, EndsWithChar_True)    { EXPECT_TRUE(String::EndsWith("hello", 'o')); }
TEST(StringTests, EndsWithChar_False)   { EXPECT_FALSE(String::EndsWith("hello", 'l')); }
TEST(StringTests, EndsWithChar_Empty)   { EXPECT_FALSE(String::EndsWith("", 'o')); }

// --- String::Remove ---
TEST(StringTests, Remove_FromIndex) {
    EXPECT_EQ(String::Remove("hello world", 5), "hello");
}
TEST(StringTests, Remove_StartIndex_Count) {
    EXPECT_EQ(String::Remove("hello world", 5, 6), "hello");
}
TEST(StringTests, Remove_Count_Zero) {
    EXPECT_EQ(String::Remove("hello", 2, 0), "hello");
}
TEST(StringTests, Remove_CountTooLong_Throws) {
    EXPECT_THROW(String::Remove("hello world", 2, 1000), System::ArgumentOutOfRangeException);
}
TEST(StringTests, Remove_StartIndexTooLarge_Throws) {
    EXPECT_THROW(String::Remove("hello", 100), System::ArgumentOutOfRangeException);
}

// --- String::Insert ---
TEST(StringTests, Insert_AtStart) {
    EXPECT_EQ(String::Insert("world", 0, "hello "), "hello world");
}
TEST(StringTests, Insert_AtEnd) {
    EXPECT_EQ(String::Insert("hello", 5, "!"), "hello!");
}
TEST(StringTests, Insert_InMiddle) {
    EXPECT_EQ(String::Insert("helo", 3, "l"), "hello");
}

// Regression tests for a code-audit finding (ticket 237): Insert had no validation at all,
// unlike Remove/Substring(int,int) elsewhere in this file -- an out-of-bounds startIndex fell
// straight into std::string::insert(), which throws std::out_of_range (an unrelated std::
// exception type) instead of the System::ArgumentOutOfRangeException real .NET's
// String.Insert(int, string) throws for the same input (verified against
// String.Manipulation.cs's ArgumentOutOfRangeException.ThrowIfGreaterThan((uint)startIndex,
// (uint)Length, ...)).
TEST(StringTests, Insert_NegativeStartIndex_Throws) {
    EXPECT_THROW(String::Insert("hello", -1, "X"), System::ArgumentOutOfRangeException);
}
TEST(StringTests, Insert_StartIndexTooLarge_Throws) {
    EXPECT_THROW(String::Insert("hello", 100, "X"), System::ArgumentOutOfRangeException);
}
TEST(StringTests, Insert_StartIndexEqualsLength_AppendsAtEnd) {
    EXPECT_EQ(String::Insert("hello", 5, "!"), "hello!");
}

// --- String::Join(intcs vector) ---
TEST(StringTests, Join_IntVector) {
    std::vector<SharpRuntime::intcs> v = {1, 2, 3};
    EXPECT_EQ(String::Join(", ", v), "1, 2, 3");
}
TEST(StringTests, Join_IntVector_Empty) {
    std::vector<SharpRuntime::intcs> v;
    EXPECT_EQ(String::Join(",", v), "");
}

// --- String::Join(double vector) ---
TEST(StringTests, Join_DoubleVector_Single) {
    std::vector<double> v = {3.14};
    EXPECT_FALSE(String::Join(",", v).empty());
}

// --- String::ToCharArray ---
TEST(StringTests, ToCharArray_Basic) {
    auto v = String::ToCharArray("abc");
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 'a');
    EXPECT_EQ(v[1], 'b');
    EXPECT_EQ(v[2], 'c');
}
TEST(StringTests, ToCharArray_Empty) {
    EXPECT_TRUE(String::ToCharArray("").empty());
}

// --- String::Trim(chars) / TrimStart(chars) / TrimEnd(chars) ---
TEST(StringTests, TrimChars_BothEnds) {
    EXPECT_EQ(String::Trim("..hello..", {'.'}), "hello");
}
TEST(StringTests, TrimChars_MultipleChars) {
    EXPECT_EQ(String::Trim("*-hi-*", {'*', '-'}), "hi");
}
TEST(StringTests, TrimStartChars) {
    EXPECT_EQ(String::TrimStart("000123", {'0'}), "123");
}
TEST(StringTests, TrimEndChars) {
    EXPECT_EQ(String::TrimEnd("hello!!!", {'!'}), "hello");
}

// --- String::Split with StringSplitOptions ---
TEST(StringTests, Split_RemoveEmptyEntries_Char) {
    auto v = String::Split("a,,b,,c", ',', System::StringSplitOptions::RemoveEmptyEntries);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "a");
    EXPECT_EQ(v[1], "b");
    EXPECT_EQ(v[2], "c");
}
TEST(StringTests, Split_TrimEntries_Char) {
    auto v = String::Split("a , b , c", ',', System::StringSplitOptions::TrimEntries);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "a");
    EXPECT_EQ(v[1], "b");
    EXPECT_EQ(v[2], "c");
}
TEST(StringTests, Split_RemoveEmptyEntries_String) {
    auto v = String::Split("a::b::c", "::", System::StringSplitOptions::RemoveEmptyEntries);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "a");
}
TEST(StringTests, Split_None_KeepsEmpty) {
    auto v = String::Split("a,,b", ',', System::StringSplitOptions::None);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[1], "");
}

// --- String::Format mixed-type overloads ---
TEST(StringTests, Format_DoubleAndString) {
    EXPECT_EQ(String::Format("{0} {1}", 3.14, std::string("hi")), "3.14 hi");
}
TEST(StringTests, Format_StringAndDouble) {
    EXPECT_EQ(String::Format("{0}={1}", std::string("pi"), 3.14), "pi=3.14");
}
TEST(StringTests, Format_LongAndInt) {
    SharpRuntime::longcs big = 9999999999LL;
    EXPECT_EQ(String::Format("{0}/{1}", big, static_cast<SharpRuntime::intcs>(42)), "9999999999/42");
}
TEST(StringTests, Format_IntAndLong) {
    SharpRuntime::longcs big = 1234567890123LL;
    EXPECT_EQ(String::Format("{0}+{1}", static_cast<SharpRuntime::intcs>(1), big), "1+1234567890123");
}
TEST(StringTests, Format_IntAndDouble) {
    EXPECT_EQ(String::Format("{0}: {1:F2}", static_cast<SharpRuntime::intcs>(3), 3.14159), "3: 3.14");
}
TEST(StringTests, Format_DoubleAndInt) {
    EXPECT_EQ(String::Format("{0:F1}/{1}", 2.5, static_cast<SharpRuntime::intcs>(10)), "2.5/10");
}

// ---------------------------------------------------------------------------
// String::Empty
// ---------------------------------------------------------------------------
TEST(StringTests, Empty_IsEmpty) { EXPECT_EQ(String::Empty, ""); }
TEST(StringTests, Empty_IsNotNullOrEmpty) { EXPECT_TRUE(String::IsNullOrEmpty(String::Empty)); }

// ---------------------------------------------------------------------------
// Format(float) / Format(longcs, longcs) / Format(4 strings)
// ---------------------------------------------------------------------------
TEST(StringTests, Format_Float) {
    std::string r = String::Format("{0:F2}", 3.14f);
    EXPECT_EQ(r, "3.14");
}
TEST(StringTests, Format_LongAndLong) {
    SharpRuntime::longcs a = 100000000000LL, b = 200000000000LL;
    EXPECT_EQ(String::Format("{0}/{1}", a, b), "100000000000/200000000000");
}
TEST(StringTests, Format_FourStrings) {
    EXPECT_EQ(String::Format("{0}-{1}-{2}-{3}",
        std::string("a"), std::string("b"), std::string("c"), std::string("d")),
        "a-b-c-d");
}

// ---------------------------------------------------------------------------
// ToUpperInvariant / ToLowerInvariant
// ---------------------------------------------------------------------------
TEST(StringTests, ToUpperInvariant_Basic) { EXPECT_EQ(String::ToUpperInvariant("hello"), "HELLO"); }
TEST(StringTests, ToLowerInvariant_Basic) { EXPECT_EQ(String::ToLowerInvariant("WORLD"), "world"); }

// ---------------------------------------------------------------------------
// CompareOrdinal
// ---------------------------------------------------------------------------
TEST(StringTests, CompareOrdinal_Equal)  { EXPECT_EQ(String::CompareOrdinal("abc", "abc"), 0); }
TEST(StringTests, CompareOrdinal_Less)   { EXPECT_LT(String::CompareOrdinal("abc", "abd"), 0); }
TEST(StringTests, CompareOrdinal_Greater){ EXPECT_GT(String::CompareOrdinal("abd", "abc"), 0); }

// ---------------------------------------------------------------------------
// IndexOf / LastIndexOf with count — new overloads
// ---------------------------------------------------------------------------

TEST(StringTests, IndexOf_CharCount_Found) {
    EXPECT_EQ(String::IndexOf("abcabc", 'c', 0, 4), 2);
}
TEST(StringTests, IndexOf_CharCount_OutOfWindow) {
    EXPECT_EQ(String::IndexOf("abcabc", 'c', 0, 2), -1);
}
TEST(StringTests, IndexOf_StringCount_Found) {
    EXPECT_EQ(String::IndexOf("abcabc", std::string("bc"), 0, 4), 1);
}
TEST(StringTests, IndexOf_StringCount_OutOfWindow) {
    // "hello world": "world" starts at 6, outside window [0..4]
    EXPECT_EQ(String::IndexOf("hello world", std::string("world"), 0, 5), -1);
}
TEST(StringTests, LastIndexOf_CharCount_Found) {
    EXPECT_EQ(String::LastIndexOf("abcabc", 'c', 5, 3), 5);
}
TEST(StringTests, LastIndexOf_CharCount_OutOfWindow) {
    EXPECT_EQ(String::LastIndexOf("abcabc", 'c', 1, 1), -1);
}
TEST(StringTests, LastIndexOf_StringCount_Found) {
    EXPECT_EQ(String::LastIndexOf("abcabc", std::string("ab"), 5, 4), 3);
}
TEST(StringTests, LastIndexOf_StringCount_OutOfWindow) {
    EXPECT_EQ(String::LastIndexOf("abcabc", std::string("ab"), 1, 1), -1);
}
TEST(StringTests, IndexOf_CharCount_StartIndexOutOfRange_Throws) {
    EXPECT_THROW(String::IndexOf("abc", 'a', 10, 1), System::ArgumentOutOfRangeException);
}
TEST(StringTests, IndexOf_CharCount_CountOutOfRange_Throws) {
    EXPECT_THROW(String::IndexOf("abc", 'a', 0, 10), System::ArgumentOutOfRangeException);
}
TEST(StringTests, LastIndexOf_CharCount_StartIndexOutOfRange_Throws) {
    EXPECT_THROW(String::LastIndexOf("abc", 'a', 10, 1), System::ArgumentOutOfRangeException);
}
TEST(StringTests, LastIndexOf_CharCount_CountOutOfRange_Throws) {
    EXPECT_THROW(String::LastIndexOf("abc", 'a', 2, 10), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// ToCharArray with range
// ---------------------------------------------------------------------------

TEST(StringTests, ToCharArray_Range_Basic) {
    auto v = String::ToCharArray("hello", 1, 3);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 'e');
    EXPECT_EQ(v[1], 'l');
    EXPECT_EQ(v[2], 'l');
}
TEST(StringTests, ToCharArray_Range_OutOfBounds_Throws) {
    EXPECT_THROW(String::ToCharArray("hi", 1, 5), System::ArgumentOutOfRangeException);
}
// Regression test (ticket 1487): startIndex+length used to be computed directly in intcs
// (int32) arithmetic, which overflows for large inputs and silently bypasses the bounds check
// instead of throwing -- same bug class as Span<T>::Slice (ticket 265).
TEST(StringTests, ToCharArray_Range_StartIndexPlusLengthOverflow_Throws) {
    EXPECT_THROW(String::ToCharArray("hi", 2147483647, 10), System::ArgumentOutOfRangeException);
}
TEST(StringTests, ToCharArray_Range_ZeroLength) {
    auto v = String::ToCharArray("hello", 2, 0);
    EXPECT_TRUE(v.empty());
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(StringTests, GetHashCode_Consistent) {
    EXPECT_EQ(String::GetHashCode("hello"), String::GetHashCode("hello"));
}
TEST(StringTests, GetHashCode_DifferentStrings) {
    EXPECT_NE(String::GetHashCode("abc"), String::GetHashCode("xyz"));
}
TEST(StringTests, GetHashCode_EmptyString) {
    EXPECT_EQ(String::GetHashCode(""), String::GetHashCode(""));
}

// ---------------------------------------------------------------------------
// StringNormalizationExtensions
// ---------------------------------------------------------------------------
#include "System/StringNormalizationExtensions.hpp"

TEST(StringNormalizationExtensionsTests, IsNormalized_Default_True) {
    EXPECT_TRUE(System::StringNormalizationExtensions::IsNormalized("hello"));
}
TEST(StringNormalizationExtensionsTests, IsNormalized_FormD_True) {
    EXPECT_TRUE(System::StringNormalizationExtensions::IsNormalized(
        "hello", System::Text::NormalizationForm::FormD));
}
TEST(StringNormalizationExtensionsTests, Normalize_Default_ReturnsInput) {
    EXPECT_EQ(System::StringNormalizationExtensions::Normalize("hello"), "hello");
}
TEST(StringNormalizationExtensionsTests, Normalize_FormKC_ReturnsInput) {
    EXPECT_EQ(System::StringNormalizationExtensions::Normalize(
        "abc", System::Text::NormalizationForm::FormKC), "abc");
}
TEST(StringNormalizationExtensionsTests, Normalize_EmptyString) {
    EXPECT_EQ(System::StringNormalizationExtensions::Normalize(""), "");
}
