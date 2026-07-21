// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 21:
//   ListDictionary:    all new properties, set(), Keys, Values
//   NameValueCollection: copy ctors, capacity ctors, all methods
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Specialized/ListDictionary.hpp"
#include "System/Collections/Specialized/NameValueCollection.hpp"
#include <any>
#include <string>
#include <vector>

using System::Collections::Specialized::ListDictionary;
using System::Collections::Specialized::NameValueCollection;

// ===========================================================================
// ListDictionary
// ===========================================================================

TEST(ListDictionaryBatch21Test, DefaultCtor_Empty) {
    ListDictionary d;
    EXPECT_EQ(d.getCountProperty(), 0);
    EXPECT_TRUE(d.getIsEmptyProperty());
}

TEST(ListDictionaryBatch21Test, NullptrCtor) {
    ListDictionary d(nullptr);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ListDictionaryBatch21Test, Properties) {
    ListDictionary d;
    EXPECT_FALSE(d.getIsFixedSizeProperty());
    EXPECT_FALSE(d.getIsReadOnlyProperty());
    EXPECT_FALSE(d.getIsSynchronizedProperty());
    EXPECT_NE(d.getSyncRootProperty(), nullptr);
}

TEST(ListDictionaryBatch21Test, AddAndCount) {
    ListDictionary d;
    d.Add("a", std::any(1));
    d.Add("b", std::any(2));
    EXPECT_EQ(d.getCountProperty(), 2);
}

TEST(ListDictionaryBatch21Test, Add_DuplicateThrows) {
    ListDictionary d;
    d.Add("k", std::any(1));
    EXPECT_THROW(d.Add("k", std::any(2)), System::ArgumentException);
}

TEST(ListDictionaryBatch21Test, Contains_TrueAndFalse) {
    ListDictionary d;
    d.Add("x", std::any(42));
    EXPECT_TRUE(d.Contains("x"));
    EXPECT_FALSE(d.Contains("y"));
}

TEST(ListDictionaryBatch21Test, Indexer_Const_Found) {
    ListDictionary d;
    d.Add("val", std::any(99));
    const ListDictionary& cd = d;
    EXPECT_EQ(std::any_cast<int>(cd["val"]), 99);
}

TEST(ListDictionaryBatch21Test, Indexer_Const_NotFound_ReturnsEmpty) {
    ListDictionary d;
    const ListDictionary& cd = d;
    EXPECT_FALSE(cd["missing"].has_value());
}

TEST(ListDictionaryBatch21Test, Indexer_Mutable_Inserts) {
    ListDictionary d;
    // operator[] is read-only (matches .NET's getter, which never inserts on a miss); use
    // set() for dict[key]=value-style writes.
    d.set("newkey", std::any(7));
    EXPECT_TRUE(d.Contains("newkey"));
    EXPECT_EQ(std::any_cast<int>(d["newkey"]), 7);
}

TEST(ListDictionaryBatch21Test, OperatorBracket_MissingKey_DoesNotInsert) {
    ListDictionary d;
    auto v = d["missing"];
    EXPECT_FALSE(v.has_value());
    EXPECT_FALSE(d.Contains("missing"));
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ListDictionaryBatch21Test, Set_InsertsAndOverwrites) {
    ListDictionary d;
    d.Add("k", std::any(1));
    d.set("k", std::any(2));
    EXPECT_EQ(std::any_cast<int>(d["k"]), 2);
    EXPECT_EQ(d.getCountProperty(), 1);
    d.set("new", std::any(5));
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_TRUE(d.Contains("new"));
}

TEST(ListDictionaryBatch21Test, Remove) {
    ListDictionary d;
    d.Add("a", std::any(1));
    d.Add("b", std::any(2));
    d.Remove("a");
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.Contains("a"));
}

TEST(ListDictionaryBatch21Test, Remove_NotPresent_NoOp) {
    ListDictionary d;
    d.Add("x", std::any(1));
    d.Remove("missing");
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ListDictionaryBatch21Test, Clear) {
    ListDictionary d;
    d.Add("a", std::any(1));
    d.Clear();
    EXPECT_EQ(d.getCountProperty(), 0);
    EXPECT_TRUE(d.getIsEmptyProperty());
}

TEST(ListDictionaryBatch21Test, Keys_InInsertionOrder) {
    ListDictionary d;
    d.Add("z", std::any(1));
    d.Add("a", std::any(2));
    d.Add("m", std::any(3));
    auto keys = d.getKeysProperty();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "z");
    EXPECT_EQ(keys[1], "a");
    EXPECT_EQ(keys[2], "m");
}

TEST(ListDictionaryBatch21Test, Values_MatchInsertion) {
    ListDictionary d;
    d.Add("a", std::any(10));
    d.Add("b", std::any(20));
    auto vals = d.getValuesProperty();
    ASSERT_EQ(vals.size(), 2u);
    EXPECT_EQ(std::any_cast<int>(vals[0]), 10);
    EXPECT_EQ(std::any_cast<int>(vals[1]), 20);
}

TEST(ListDictionaryBatch21Test, STLIteration) {
    ListDictionary d;
    d.Add("x", std::any(5));
    d.Add("y", std::any(10));
    int count = 0;
    for (const auto& [k, v] : d) { (void)k; (void)v; ++count; }
    EXPECT_EQ(count, 2);
}

// ===========================================================================
// NameValueCollection
// ===========================================================================

TEST(NameValueCollectionBatch21Test, DefaultCtor) {
    NameValueCollection c;
    EXPECT_EQ(c.getCountProperty(), 0);
    EXPECT_FALSE(c.HasKeys());
}

TEST(NameValueCollectionBatch21Test, CapacityCtor) {
    NameValueCollection c(10);
    EXPECT_EQ(c.getCountProperty(), 0);
}

TEST(NameValueCollectionBatch21Test, CopyCtor) {
    NameValueCollection src;
    src.Add("k", "v1");
    src.Add("k", "v2");
    NameValueCollection copy(src);
    EXPECT_EQ(copy.getCountProperty(), 1);
    EXPECT_EQ(copy.Get("k"), "v1,v2");
    src.Add("x", "y");
    EXPECT_EQ(copy.getCountProperty(), 1); // independent copy
}

TEST(NameValueCollectionBatch21Test, CapacityAndCollectionCtor) {
    NameValueCollection src;
    src.Add("a", "1");
    NameValueCollection copy(5, src);
    EXPECT_EQ(copy.getCountProperty(), 1);
    EXPECT_EQ(copy.Get("a"), "1");
}

TEST(NameValueCollectionBatch21Test, AddAndGetSingleValue) {
    NameValueCollection c;
    c.Add("name", "Alice");
    EXPECT_EQ(c.Get("name"), "Alice");
    EXPECT_EQ(c.getCountProperty(), 1);
}

TEST(NameValueCollectionBatch21Test, AddMultipleValues_CommaJoined) {
    NameValueCollection c;
    c.Add("colors", "red");
    c.Add("colors", "green");
    c.Add("colors", "blue");
    EXPECT_EQ(c.Get("colors"), "red,green,blue");
    EXPECT_EQ(c.getCountProperty(), 1); // one key
}

TEST(NameValueCollectionBatch21Test, Set_ReplacesExisting) {
    NameValueCollection c;
    c.Add("k", "old1");
    c.Add("k", "old2");
    c.Set("k", "new");
    EXPECT_EQ(c.Get("k"), "new");
    EXPECT_EQ(c.GetValues("k").size(), 1u);
}

TEST(NameValueCollectionBatch21Test, Remove) {
    NameValueCollection c;
    c.Add("a", "1");
    c.Add("b", "2");
    c.Remove("a");
    EXPECT_EQ(c.getCountProperty(), 1);
    EXPECT_EQ(c.Get("a"), "");
}

TEST(NameValueCollectionBatch21Test, Clear) {
    NameValueCollection c;
    c.Add("x", "y");
    c.Clear();
    EXPECT_EQ(c.getCountProperty(), 0);
    EXPECT_FALSE(c.HasKeys());
}

TEST(NameValueCollectionBatch21Test, GetByIndex) {
    NameValueCollection c;
    c.Add("first", "f");
    c.Add("second", "s");
    EXPECT_EQ(c.Get(0), "f");
    EXPECT_EQ(c.Get(1), "s");
}

// .NET's Get(int)/GetValues(int)/GetKey(int)/this[int] delegate through
// NameObjectCollectionBase's internal ArrayList indexer, which throws
// ArgumentOutOfRangeException for an out-of-range index -- verified against
// NameValueCollection.cs/NameObjectCollectionBase.cs. This port previously returned ""/{}
// silently instead.
TEST(NameValueCollectionBatch21Test, GetByIndex_OutOfRange_Throws) {
    NameValueCollection c;
    c.Add("first", "f");
    EXPECT_THROW(c.Get(99), System::ArgumentOutOfRangeException);
    EXPECT_THROW(c.Get(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(c.GetValues(99), System::ArgumentOutOfRangeException);
    EXPECT_THROW(c.GetKey(99), System::ArgumentOutOfRangeException);
    EXPECT_THROW(c[99], System::ArgumentOutOfRangeException);
}

TEST(NameValueCollectionBatch21Test, GetKey) {
    NameValueCollection c;
    c.Add("alpha", "a");
    c.Add("beta", "b");
    EXPECT_EQ(c.GetKey(0), "alpha");
    EXPECT_EQ(c.GetKey(1), "beta");
}

TEST(NameValueCollectionBatch21Test, GetValues_ByName) {
    NameValueCollection c;
    c.Add("k", "x");
    c.Add("k", "y");
    auto vals = c.GetValues("k");
    ASSERT_EQ(vals.size(), 2u);
    EXPECT_EQ(vals[0], "x");
    EXPECT_EQ(vals[1], "y");
}

TEST(NameValueCollectionBatch21Test, GetValues_ByIndex) {
    NameValueCollection c;
    c.Add("p", "1");
    c.Add("p", "2");
    auto vals = c.GetValues(0);
    ASSERT_EQ(vals.size(), 2u);
}

TEST(NameValueCollectionBatch21Test, AllKeys_OrderPreserved) {
    NameValueCollection c;
    c.Add("z", "1");
    c.Add("a", "2");
    c.Add("m", "3");
    auto& keys = c.AllKeys();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "z");
    EXPECT_EQ(keys[1], "a");
    EXPECT_EQ(keys[2], "m");
}

TEST(NameValueCollectionBatch21Test, AddCollection) {
    NameValueCollection src;
    src.Add("x", "1");
    src.Add("y", "2");
    NameValueCollection dst;
    dst.Add(src);
    EXPECT_EQ(dst.getCountProperty(), 2);
    EXPECT_EQ(dst.Get("x"), "1");
}

TEST(NameValueCollectionBatch21Test, OperatorBracket_ByName) {
    NameValueCollection c;
    c.Add("n", "v");
    EXPECT_EQ(c["n"], "v");
    EXPECT_EQ(c["missing"], "");
}

TEST(NameValueCollectionBatch21Test, OperatorBracket_ByIndex) {
    NameValueCollection c;
    c.Add("k", "val");
    EXPECT_EQ(c[0], "val");
}

TEST(NameValueCollectionBatch21Test, CaseInsensitive_LookupAndAdd) {
    NameValueCollection c;
    c.Add("Name_0", "Value_0");

    EXPECT_EQ(c["NAME_0"], "Value_0");
    EXPECT_EQ(c["name_0"], "Value_0");
    EXPECT_EQ(c.Get("NAME_0"), "Value_0");
    std::vector<std::string> expected{"Value_0"};
    EXPECT_EQ(c.GetValues("NAME_0"), expected);

    // Second Add() under a different casing appends to the SAME key, not a new one.
    c.Add("NAME_0", "Value_1");
    EXPECT_EQ(c.getCountProperty(), 1);
    EXPECT_EQ(c.Get("Name_0"), "Value_0,Value_1");

    const auto& keys = c.AllKeys();
    ASSERT_EQ(keys.size(), 1u);
    EXPECT_EQ(keys[0], "Name_0"); // original casing preserved
}

TEST(NameValueCollectionBatch21Test, CaseInsensitive_SetAndRemove) {
    NameValueCollection c;
    c.Add("Key", "value");
    c.Set("KEY", "new-value");
    EXPECT_EQ(c.getCountProperty(), 1);
    EXPECT_EQ(c.Get("key"), "new-value");

    c.Remove("kEy");
    EXPECT_EQ(c.getCountProperty(), 0);
    EXPECT_EQ(c.Get("Key"), "");
}
