// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Text/StringBuilder.hpp"

using System::Text::StringBuilder;

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, DefaultCtorIsEmpty) {
    StringBuilder sb;
    EXPECT_EQ(sb.ToString(), "");
    EXPECT_EQ(sb.getLengthProperty(), 0);
    EXPECT_TRUE(sb.Empty());
}

TEST(StringBuilderTests, CtorWithInitialValue) {
    StringBuilder sb(std::string("hello"));
    EXPECT_EQ(sb.ToString(), "hello");
    EXPECT_EQ(sb.getLengthProperty(), 5);
    EXPECT_FALSE(sb.Empty());
}

// ---------------------------------------------------------------------------
// Append (string overloads)
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, AppendString) {
    StringBuilder sb;
    sb.Append(std::string("foo"));
    EXPECT_EQ(sb.ToString(), "foo");
}

TEST(StringBuilderTests, AppendCString) {
    StringBuilder sb;
    sb.Append("bar");
    EXPECT_EQ(sb.ToString(), "bar");
}

TEST(StringBuilderTests, AppendNullptrIsNoop) {
    StringBuilder sb;
    sb.Append("x");
    sb.Append(nullptr);
    EXPECT_EQ(sb.ToString(), "x");
}

TEST(StringBuilderTests, AppendChar) {
    StringBuilder sb;
    sb.Append('A').Append('B').Append('C');
    EXPECT_EQ(sb.ToString(), "ABC");
}

TEST(StringBuilderTests, AppendInt) {
    StringBuilder sb;
    sb.Append(42);
    EXPECT_EQ(sb.ToString(), "42");
}

TEST(StringBuilderTests, AppendNegativeInt) {
    StringBuilder sb;
    sb.Append(-7);
    EXPECT_EQ(sb.ToString(), "-7");
}

TEST(StringBuilderTests, AppendDouble) {
    StringBuilder sb;
    sb.Append(3.14);
    // Just check it starts with "3.14" (implementation uses std::to_string)
    EXPECT_EQ(sb.ToString().substr(0, 4), "3.14");
}

TEST(StringBuilderTests, AppendBoolTrue) {
    StringBuilder sb;
    sb.Append(true);
    EXPECT_EQ(sb.ToString(), "True");
}

TEST(StringBuilderTests, AppendBoolFalse) {
    StringBuilder sb;
    sb.Append(false);
    EXPECT_EQ(sb.ToString(), "False");
}

// ---------------------------------------------------------------------------
// AppendLine
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, AppendLineNoArg) {
    StringBuilder sb;
    sb.AppendLine();
    EXPECT_EQ(sb.ToString(), "\n");
    EXPECT_EQ(sb.getLengthProperty(), 1);
}

TEST(StringBuilderTests, AppendLineWithString) {
    StringBuilder sb;
    sb.AppendLine(std::string("hello"));
    EXPECT_EQ(sb.ToString(), "hello\n");
}

TEST(StringBuilderTests, AppendLineTwice) {
    StringBuilder sb;
    sb.AppendLine(std::string("line1"));
    sb.AppendLine(std::string("line2"));
    EXPECT_EQ(sb.ToString(), "line1\nline2\n");
}

// ---------------------------------------------------------------------------
// Chaining (fluent API)
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, ChainingAppend) {
    StringBuilder sb;
    sb.Append(std::string("Hello")).Append(", ").Append(std::string("world")).Append('!');
    EXPECT_EQ(sb.ToString(), "Hello, world!");
}

TEST(StringBuilderTests, ChainingMixedTypes) {
    StringBuilder sb;
    sb.Append("n=").Append(42).Append(" ok=").Append(true);
    EXPECT_EQ(sb.ToString(), "n=42 ok=True");
}

// ---------------------------------------------------------------------------
// Length and Empty
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, LengthAfterAppend) {
    StringBuilder sb;
    sb.Append("abc");
    EXPECT_EQ(sb.getLengthProperty(), 3);
}

TEST(StringBuilderTests, LengthAccumulates) {
    StringBuilder sb;
    sb.Append("ab").Append("cde");
    EXPECT_EQ(sb.getLengthProperty(), 5);
}

TEST(StringBuilderTests, EmptyAfterDefault) {
    EXPECT_TRUE(StringBuilder().Empty());
}

TEST(StringBuilderTests, NotEmptyAfterAppend) {
    StringBuilder sb;
    sb.Append("x");
    EXPECT_FALSE(sb.Empty());
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, ClearResetsBuffer) {
    StringBuilder sb(std::string("hello"));
    sb.Clear();
    EXPECT_EQ(sb.ToString(), "");
    EXPECT_EQ(sb.getLengthProperty(), 0);
    EXPECT_TRUE(sb.Empty());
}

TEST(StringBuilderTests, AppendAfterClear) {
    StringBuilder sb(std::string("old"));
    sb.Clear();
    sb.Append("new");
    EXPECT_EQ(sb.ToString(), "new");
}

// ---------------------------------------------------------------------------
// ToString
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, ToStringEmpty) {
    EXPECT_EQ(StringBuilder().ToString(), "");
}

TEST(StringBuilderTests, ToStringMultipleAppends) {
    StringBuilder sb;
    for (int i = 0; i < 5; ++i) sb.Append(std::to_string(i));
    EXPECT_EQ(sb.ToString(), "01234");
}

TEST(StringBuilderTests, ToStringDoesNotMutate) {
    StringBuilder sb(std::string("immutable"));
    std::string s1 = sb.ToString();
    std::string s2 = sb.ToString();
    EXPECT_EQ(s1, s2);
    EXPECT_EQ(sb.getLengthProperty(), 9);
}

// ---------------------------------------------------------------------------
// Large input stress
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, AppendManyTimes) {
    StringBuilder sb;
    for (int i = 0; i < 1000; ++i) sb.Append('x');
    EXPECT_EQ(sb.getLengthProperty(), 1000);
    EXPECT_EQ(sb.ToString(), std::string(1000, 'x'));
}

TEST(StringBuilderTests, AppendLargeString) {
    std::string big(10000, 'a');
    StringBuilder sb;
    sb.Append(big);
    EXPECT_EQ(sb.getLengthProperty(), 10000);
    EXPECT_EQ(sb.ToString(), big);
}

// ---------------------------------------------------------------------------
// Append(longcs)
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, AppendLong_Positive) {
    StringBuilder sb;
    sb.Append(int64_t(9876543210LL));
    EXPECT_EQ(sb.ToString(), "9876543210");
}

TEST(StringBuilderTests, AppendLong_Negative) {
    StringBuilder sb;
    sb.Append(int64_t(-1LL));
    EXPECT_EQ(sb.ToString(), "-1");
}

TEST(StringBuilderTests, AppendLong_Zero) {
    StringBuilder sb;
    sb.Append(int64_t(0LL));
    EXPECT_EQ(sb.ToString(), "0");
}

// ---------------------------------------------------------------------------
// Insert
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, Insert_AtStart) {
    StringBuilder sb("world");
    sb.Insert(0, std::string("hello "));
    EXPECT_EQ(sb.ToString(), "hello world");
}

TEST(StringBuilderTests, Insert_AtEnd) {
    StringBuilder sb("hello");
    sb.Insert(5, std::string("!"));
    EXPECT_EQ(sb.ToString(), "hello!");
}

TEST(StringBuilderTests, Insert_InMiddle) {
    StringBuilder sb("helo");
    sb.Insert(3, std::string("l"));
    EXPECT_EQ(sb.ToString(), "hello");
}

TEST(StringBuilderTests, Insert_EmptyString_NoChange) {
    StringBuilder sb("abc");
    sb.Insert(1, std::string(""));
    EXPECT_EQ(sb.ToString(), "abc");
}

// Regression tests for ticket 329: Insert never validated `index` against [0, Length], letting
// an out-of-range index reach std::string::insert directly, which throws a raw
// std::out_of_range instead of the System::ArgumentOutOfRangeException StringBuilder.cs's
// Insert(int, string) throws for the identical condition.
TEST(StringBuilderTests, Insert_NegativeIndex_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb("abc");
    EXPECT_THROW(sb.Insert(-1, "x"), System::ArgumentOutOfRangeException);
}
TEST(StringBuilderTests, Insert_IndexPastEnd_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb("abc");
    EXPECT_THROW(sb.Insert(4, "x"), System::ArgumentOutOfRangeException);
}
TEST(StringBuilderTests, Insert_IndexAtLength_Succeeds) {
    StringBuilder sb("abc");
    sb.Insert(3, "x");
    EXPECT_EQ(sb.ToString(), "abcx");
}

// ---------------------------------------------------------------------------
// Remove
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, Remove_FromStart) {
    StringBuilder sb("hello world");
    sb.Remove(0, 6);
    EXPECT_EQ(sb.ToString(), "world");
}

TEST(StringBuilderTests, Remove_FromMiddle) {
    StringBuilder sb("hello world");
    sb.Remove(5, 6);
    EXPECT_EQ(sb.ToString(), "hello");
}

TEST(StringBuilderTests, Remove_SingleChar) {
    StringBuilder sb("abc");
    sb.Remove(1, 1);
    EXPECT_EQ(sb.ToString(), "ac");
}

TEST(StringBuilderTests, Remove_ZeroCount_NoChange) {
    StringBuilder sb("abc");
    sb.Remove(1, 0);
    EXPECT_EQ(sb.ToString(), "abc");
}

// Regression tests for ticket 329: Remove never validated startIndex/count, letting a negative
// value reach std::string::erase directly (raw std::out_of_range instead of System::
// ArgumentOutOfRangeException), and -- more subtly -- an over-large but non-negative count was
// previously silently CLAMPED by std::string::erase instead of throwing, so
// Remove(0, 100) on a 3-char buffer removed only 3 characters with no error at all where real
// .NET's StringBuilder.Remove(int, int) throws ArgumentOutOfRangeException outright.
TEST(StringBuilderTests, Remove_NegativeStartIndex_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb("abc");
    EXPECT_THROW(sb.Remove(-1, 1), System::ArgumentOutOfRangeException);
}
TEST(StringBuilderTests, Remove_NegativeCount_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb("abc");
    EXPECT_THROW(sb.Remove(0, -1), System::ArgumentOutOfRangeException);
}
TEST(StringBuilderTests, Remove_CountExceedsRemainingLength_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb("abc");
    EXPECT_THROW(sb.Remove(0, 100), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// Replace
// ---------------------------------------------------------------------------

TEST(StringBuilderTests, Replace_SingleOccurrence) {
    StringBuilder sb("hello world");
    sb.Replace("world", "C++");
    EXPECT_EQ(sb.ToString(), "hello C++");
}

TEST(StringBuilderTests, Replace_MultipleOccurrences) {
    StringBuilder sb("aXbXcX");
    sb.Replace("X", "-");
    EXPECT_EQ(sb.ToString(), "a-b-c-");
}

TEST(StringBuilderTests, Replace_NoOccurrence_NoChange) {
    StringBuilder sb("hello");
    sb.Replace("xyz", "abc");
    EXPECT_EQ(sb.ToString(), "hello");
}

TEST(StringBuilderTests, Replace_WithEmptyString_RemovesAll) {
    StringBuilder sb("abcabc");
    sb.Replace("b", "");
    EXPECT_EQ(sb.ToString(), "acac");
}

TEST(StringBuilderTests, Replace_EmptyOldValue_Throws) {
    // Regression: previously hung forever (or ran until OOM if newValue was non-empty) --
    // buffer.find("", pos) always matches at pos, so pos+=newValue.size() would leave
    // buffer.size()-pos constant, and the loop's std::string::npos exit condition was
    // never reached. .NET throws ArgumentException for an empty oldValue
    // (StringBuilder.Replace -> ArgumentException.ThrowIfNullOrEmpty).
    StringBuilder sb("abc");
    EXPECT_THROW(sb.Replace("", "x"), System::ArgumentException);
}

TEST(StringBuilderTests, Replace_EmptyOldValueAndEmptyNewValue_Throws) {
    StringBuilder sb("abc");
    EXPECT_THROW(sb.Replace("", ""), System::ArgumentException);
}

TEST(StringBuilderTests, Replace_ChainedWithAppend) {
    StringBuilder sb;
    sb.Append("foo bar foo").Replace("foo", "baz");
    EXPECT_EQ(sb.ToString(), "baz bar baz");
}

// --- AppendFormat ---

TEST(StringBuilderTests, AppendFormat_Int) {
    StringBuilder sb;
    sb.AppendFormat("Score: {0}", 42);
    EXPECT_EQ(sb.ToString(), "Score: 42");
}

TEST(StringBuilderTests, AppendFormat_Double_Fixed) {
    StringBuilder sb;
    sb.AppendFormat("{0:F2}", 3.14159);
    EXPECT_EQ(sb.ToString(), "3.14");
}

TEST(StringBuilderTests, AppendFormat_String) {
    StringBuilder sb;
    sb.AppendFormat("Hello, {0}!", std::string("world"));
    EXPECT_EQ(sb.ToString(), "Hello, world!");
}

TEST(StringBuilderTests, AppendFormat_TwoInts) {
    StringBuilder sb;
    sb.AppendFormat("{0}/{1}", 3, 10);
    EXPECT_EQ(sb.ToString(), "3/10");
}

TEST(StringBuilderTests, AppendFormat_IntAndString) {
    StringBuilder sb;
    sb.AppendFormat("{0}: {1}", 1, std::string("alpha"));
    EXPECT_EQ(sb.ToString(), "1: alpha");
}

TEST(StringBuilderTests, AppendFormat_TwoStrings) {
    StringBuilder sb;
    sb.AppendFormat("{0} {1}", std::string("foo"), std::string("bar"));
    EXPECT_EQ(sb.ToString(), "foo bar");
}

TEST(StringBuilderTests, AppendFormat_TwoDoubles) {
    StringBuilder sb;
    sb.AppendFormat("({0:F1},{1:F1})", 1.5, 2.5);
    EXPECT_EQ(sb.ToString(), "(1.5,2.5)");
}

TEST(StringBuilderTests, AppendFormat_Chained) {
    StringBuilder sb;
    sb.AppendFormat("x={0}", 1)
      .AppendFormat(" y={0}", 2);
    EXPECT_EQ(sb.ToString(), "x=1 y=2");
}

// --- AppendJoin ---
TEST(StringBuilderTests, AppendJoin_CommaSeparated) {
    StringBuilder sb;
    sb.AppendJoin(", ", {"a", "b", "c"});
    EXPECT_EQ(sb.ToString(), "a, b, c");
}
TEST(StringBuilderTests, AppendJoin_SingleElement) {
    StringBuilder sb;
    sb.AppendJoin("-", {"only"});
    EXPECT_EQ(sb.ToString(), "only");
}
TEST(StringBuilderTests, AppendJoin_EmptyList) {
    StringBuilder sb;
    sb.AppendJoin(",", {});
    EXPECT_EQ(sb.ToString(), "");
}
TEST(StringBuilderTests, AppendJoin_Chained) {
    StringBuilder sb;
    sb.Append("ids: ").AppendJoin("|", {"1", "2", "3"});
    EXPECT_EQ(sb.ToString(), "ids: 1|2|3");
}

// --- AppendFormat(longcs) ---
TEST(StringBuilderTests, AppendFormat_LongArg) {
    StringBuilder sb;
    SharpRuntime::longcs big = 1234567890123LL;
    sb.AppendFormat("n={0}", big);
    EXPECT_EQ(sb.ToString(), "n=1234567890123");
}

// --- setLengthProperty ---
TEST(StringBuilderTests, SetLength_Truncate) {
    StringBuilder sb("hello world");
    sb.setLengthProperty(5);
    EXPECT_EQ(sb.ToString(), "hello");
    EXPECT_EQ(sb.getLengthProperty(), 5);
}
TEST(StringBuilderTests, SetLength_Zero_ClearsBuffer) {
    StringBuilder sb("abc");
    sb.setLengthProperty(0);
    EXPECT_EQ(sb.ToString(), "");
}
TEST(StringBuilderTests, SetLength_Extend_PadsNulls) {
    StringBuilder sb("hi");
    sb.setLengthProperty(4);
    EXPECT_EQ(sb.getLengthProperty(), 4);
}
// Regression test for ticket 329: a negative length previously reached std::string::resize
// directly (wrapping to a huge size_t, throwing a raw std::length_error/std::bad_alloc) instead
// of the System::ArgumentOutOfRangeException StringBuilder.cs's Length setter throws for the
// identical condition.
TEST(StringBuilderTests, SetLength_Negative_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb("abc");
    EXPECT_THROW(sb.setLengthProperty(-1), System::ArgumentOutOfRangeException);
}

// Regression test for ticket 329: Append(char, int repeatCount) never validated repeatCount,
// letting a negative value reach std::string::append directly (raw std::length_error) instead
// of the System::ArgumentOutOfRangeException StringBuilder.cs's Append(char, int) throws.
TEST(StringBuilderTests, AppendCharRepeated_Negative_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb;
    EXPECT_THROW(sb.Append('x', -1), System::ArgumentOutOfRangeException);
}

// --- Append(float) ---
TEST(StringBuilderTests, Append_Float_Basic) {
    StringBuilder sb;
    sb.Append(1.5f);
    EXPECT_FALSE(sb.ToString().empty());
}
TEST(StringBuilderTests, Append_Float_Chained) {
    StringBuilder sb;
    sb.Append("x=").Append(2.5f);
    EXPECT_TRUE(sb.ToString().rfind("x=", 0) == 0);
}

// --- Append(char, repeatCount) ---
TEST(StringBuilderTests, Append_CharRepeat_Basic) {
    StringBuilder sb;
    sb.Append('x', 5);
    EXPECT_EQ(sb.ToString(), "xxxxx");
    EXPECT_EQ(sb.getLengthProperty(), 5);
}
TEST(StringBuilderTests, Append_CharRepeat_Zero) {
    StringBuilder sb("hi");
    sb.Append('-', 0);
    EXPECT_EQ(sb.ToString(), "hi");
}
TEST(StringBuilderTests, Append_CharRepeat_Chained) {
    StringBuilder sb;
    sb.Append('a', 3).Append('b', 2);
    EXPECT_EQ(sb.ToString(), "aaabb");
}

// --- AppendFormat 3-arg ---
TEST(StringBuilderTests, AppendFormat_ThreeInts) {
    StringBuilder sb;
    sb.AppendFormat("{0}-{1}-{2}", 1, 2, 3);
    EXPECT_EQ(sb.ToString(), "1-2-3");
}
TEST(StringBuilderTests, AppendFormat_ThreeStrings) {
    StringBuilder sb;
    sb.AppendFormat("{0}/{1}/{2}", std::string("a"), std::string("b"), std::string("c"));
    EXPECT_EQ(sb.ToString(), "a/b/c");
}

// --- Append(StringBuilder) / Equals ---
TEST(StringBuilderTests, Append_StringBuilder) {
    StringBuilder a("hello");
    StringBuilder b(" world");
    a.Append(b);
    EXPECT_EQ(a.ToString(), "hello world");
}
TEST(StringBuilderTests, Equals_True) {
    StringBuilder a("abc");
    StringBuilder b("abc");
    EXPECT_TRUE(a.Equals(b));
}
TEST(StringBuilderTests, Equals_False) {
    StringBuilder a("abc");
    StringBuilder b("xyz");
    EXPECT_FALSE(a.Equals(b));
}

// --- getCapacityProperty / EnsureCapacity ---
TEST(StringBuilderTests, EnsureCapacity_ReservesSpace) {
    StringBuilder sb;
    sb.EnsureCapacity(500);
    EXPECT_GE(sb.getCapacityProperty(), 500);
}
// Regression test for ticket 329: EnsureCapacity never validated capacity, letting a negative
// value reach std::string::reserve directly (raw std::length_error) instead of the System::
// ArgumentOutOfRangeException StringBuilder.cs's EnsureCapacity(int) throws.
TEST(StringBuilderTests, EnsureCapacity_Negative_ThrowsArgumentOutOfRangeException) {
    StringBuilder sb;
    EXPECT_THROW(sb.EnsureCapacity(-1), System::ArgumentOutOfRangeException);
}
TEST(StringBuilderTests, EnsureCapacity_DoesNotTruncate) {
    StringBuilder sb("hello");
    sb.EnsureCapacity(1000);
    EXPECT_EQ(sb.ToString(), "hello");
}
