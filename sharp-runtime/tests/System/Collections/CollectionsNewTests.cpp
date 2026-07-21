// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include <gtest/gtest.h>
#include "System/Collections/ArrayList.hpp"
#include "System/Collections/Hashtable.hpp"
#include "System/Text/Ascii.hpp"
#include "System/IO/RandomAccess.hpp"

using namespace System::Collections;

// --- ArrayList ---
TEST(ArrayListTests, AddAndCount) {
    ArrayList list;
    list.Add(std::any(42));
    list.Add(std::any(std::string("hello")));
    EXPECT_EQ(list.getCountProperty(), 2);
}
TEST(ArrayListTests, RemoveAt) {
    ArrayList list;
    list.Add(std::any(1));
    list.Add(std::any(2));
    list.RemoveAt(0);
    EXPECT_EQ(list.getCountProperty(), 1);
}
TEST(ArrayListTests, Clear) {
    ArrayList list;
    list.Add(std::any(1));
    list.Clear();
    EXPECT_EQ(list.getCountProperty(), 0);
}
TEST(ArrayListTests, Reverse) {
    ArrayList list;
    list.Add(std::any(1));
    list.Add(std::any(2));
    list.Add(std::any(3));
    list.Reverse();
    EXPECT_EQ(std::any_cast<int>(list[0]), 3);
}
TEST(ArrayListTests, Insert) {
    ArrayList list;
    list.Add(std::any(10));
    list.Add(std::any(30));
    list.Insert(1, std::any(20));
    EXPECT_EQ(list.getCountProperty(), 3);
    EXPECT_EQ(std::any_cast<int>(list[1]), 20);
}
TEST(ArrayListTests, ToArray) {
    ArrayList list;
    list.Add(std::any(7));
    auto arr = list.ToArray();
    EXPECT_EQ(arr.size(), 1u);
    EXPECT_EQ(std::any_cast<int>(arr[0]), 7);
}

// --- Hashtable ---
TEST(HashtableTests, AddAndContains) {
    Hashtable ht;
    ht.Add("key1", std::any(42));
    EXPECT_TRUE(ht.ContainsKey("key1"));
    EXPECT_FALSE(ht.ContainsKey("key2"));
}
TEST(HashtableTests, Count) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    ht.Add("b", std::any(2));
    EXPECT_EQ(ht.getCountProperty(), 2);
}
TEST(HashtableTests, Remove) {
    Hashtable ht;
    ht.Add("x", std::any(99));
    ht.Remove("x");
    EXPECT_FALSE(ht.ContainsKey("x"));
}
TEST(HashtableTests, Clear) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    ht.Clear();
    EXPECT_EQ(ht.getCountProperty(), 0);
}

// Regression test for a wave-3 audit finding: Add() threw std::invalid_argument (an unrelated
// std:: exception type invisible to code catching System::Exception&) instead of
// System::ArgumentException, which is what real .NET's Hashtable.Add throws for a duplicate
// key (SR.Argument_AddingDuplicate__: "Item has already been added. Key in dictionary...").
TEST(HashtableTests, Add_DuplicateKey_Throws) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    EXPECT_THROW(ht.Add("a", std::any(2)), System::ArgumentException);
}

// Regression test: GetEnumerator() previously returned nullptr unconditionally (a stub),
// violating IDictionary's contract that GetEnumerator() always returns a working enumerator.
TEST(HashtableTests, GetEnumerator_IteratesAllEntries) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    ht.Add("b", std::any(2));
    ht.Add("c", std::any(3));

    IDictionaryEnumerator* e = ht.GetEnumerator();
    ASSERT_NE(e, nullptr);
    int count = 0;
    int sum = 0;
    while (e->MoveNext()) {
        DictionaryEntry entry = e->getEntryProperty();
        sum += std::any_cast<int>(entry.getValueProperty());
        ++count;
    }
    EXPECT_EQ(count, 3);
    EXPECT_EQ(sum, 6);
    delete e;
}

TEST(HashtableTests, GetEnumerator_CurrentBeforeMoveNext_Throws) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    IDictionaryEnumerator* e = ht.GetEnumerator();
    EXPECT_THROW(e->getEntryProperty(), System::InvalidOperationException);
    delete e;
}

TEST(HashtableTests, GetEnumerator_ModifiedDuringEnumeration_Throws) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    ht.Add("b", std::any(2));
    IDictionaryEnumerator* e = ht.GetEnumerator();
    ASSERT_TRUE(e->MoveNext());
    ht.Add("c", std::any(3));
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

// Regression test: CopyTo() was previously a no-op stub; now copies DictionaryEntry elements.
TEST(HashtableTests, CopyTo_CopiesAllEntriesAsDictionaryEntry) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    ht.Add("b", std::any(2));
    std::vector<DictionaryEntry> dest(2);
    ht.CopyTo(dest.data(), 0);
    int sum = 0;
    for (const auto& entry : dest) sum += std::any_cast<int>(entry.getValueProperty());
    EXPECT_EQ(sum, 3);
}

// Regression test: ContainsValue() previously compared only std::any::type(), so it would
// report a false positive for any value of a type already present in the table.
TEST(HashtableTests, ContainsValue_ComparesByValueNotJustType) {
    Hashtable ht;
    ht.Add("a", std::any(1));
    ht.Add("b", std::any(2));
    EXPECT_TRUE(ht.ContainsValue(std::any(2)));
    EXPECT_FALSE(ht.ContainsValue(std::any(99)));
}

// --- Ascii ---
TEST(AsciiTests, IsValid) {
    EXPECT_TRUE(System::Text::Ascii::IsValid("hello"));
    EXPECT_TRUE(System::Text::Ascii::IsValid(static_cast<uint8_t>(65)));
    EXPECT_FALSE(System::Text::Ascii::IsValid(static_cast<uint8_t>(200)));
}
TEST(AsciiTests, ToUpperInPlace) {
    std::string s = "hello";
    int n = 0;
    System::Text::Ascii::ToUpperInPlace(s, n);
    EXPECT_EQ(s, "HELLO");
    EXPECT_EQ(n, 5);
}
TEST(AsciiTests, ToLowerInPlace) {
    std::string s = "WORLD";
    int n = 0;
    System::Text::Ascii::ToLowerInPlace(s, n);
    EXPECT_EQ(s, "world");
}
TEST(AsciiTests, Trim) {
    EXPECT_EQ(System::Text::Ascii::Trim("  hello  "), "hello");
    EXPECT_EQ(System::Text::Ascii::TrimStart("  hello"), "hello");
    EXPECT_EQ(System::Text::Ascii::TrimEnd("hello  "), "hello");
}
// `char` is signed on this platform: a byte with the high bit set (e.g. a UTF-8 continuation
// byte, 0x80-0xFF) becomes negative when read as `char`, and a naive `value[i] <= 32` check
// would then treat it as "whitespace" and strip it. Real .NET's Ascii.Trim family always
// treats the byte as unsigned (Ascii.Trimming.cs's `uint.CreateTruncating`), so a high-bit
// byte is never trimmed.
TEST(AsciiTests, Trim_DoesNotStripHighBitBytes) {
    std::string s = "\x80hello\xFF";
    EXPECT_EQ(System::Text::Ascii::Trim(s), s);
    EXPECT_EQ(System::Text::Ascii::TrimStart(s), s);
    EXPECT_EQ(System::Text::Ascii::TrimEnd(s), s);
}
// Real .NET's TrimMask covers only TAB/LF/VT/FF/CR/space (0x09,0x0A,0x0B,0x0C,0x0D,0x20) --
// NOT every byte <= 32. NUL and other C0 controls outside that set must not be trimmed.
TEST(AsciiTests, Trim_DoesNotStripOtherControlChars) {
    std::string s = std::string("\x01hello\x1B", 7);
    EXPECT_EQ(System::Text::Ascii::Trim(s), s);
}
TEST(AsciiTests, Trim_StripsAllSixWhitespaceBytes) {
    std::string s = "\x09\x0A\x0B\x0C\x0D hello";
    EXPECT_EQ(System::Text::Ascii::Trim(s), "hello");
}
TEST(AsciiTests, EqualsIgnoreCase) {
    EXPECT_TRUE(System::Text::Ascii::EqualsIgnoreCase("Hello", "hello"));
    EXPECT_FALSE(System::Text::Ascii::EqualsIgnoreCase("Hello", "world"));
}
