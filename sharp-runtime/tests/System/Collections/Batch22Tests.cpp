// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 22:
//   OrderedDictionary:  new ctors, IsReadOnly/IsFixedSize/IsSynchronized/SyncRoot, AsReadOnly,
//                       getKeysProperty/ValuesProperty, mutation guard
//   StringCollection:   IsSynchronized, SyncRoot
//   StringDictionary:   getKeysProperty, getValuesProperty, getSyncRootProperty
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Collections/Specialized/OrderedDictionary.hpp"
#include "System/Collections/Specialized/StringCollection.hpp"
#include "System/Collections/Specialized/StringDictionary.hpp"
#include <string>
#include <vector>

using System::Collections::Specialized::OrderedDictionary;
using System::Collections::Specialized::StringCollection;
using System::Collections::Specialized::StringDictionary;

// ===========================================================================
// OrderedDictionary
// ===========================================================================

TEST(OrderedDictionaryBatch22Test, DefaultCtor) {
    OrderedDictionary d;
    EXPECT_EQ(d.getCountProperty(), 0);
    EXPECT_FALSE(d.getIsReadOnlyProperty());
}

TEST(OrderedDictionaryBatch22Test, NullptrCtor) {
    OrderedDictionary d(nullptr);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(OrderedDictionaryBatch22Test, IntCtor) {
    OrderedDictionary d(16);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(OrderedDictionaryBatch22Test, IntNullptrCtor) {
    OrderedDictionary d(16, nullptr);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(OrderedDictionaryBatch22Test, Properties) {
    OrderedDictionary d;
    EXPECT_FALSE(d.getIsFixedSizeProperty());
    EXPECT_FALSE(d.getIsSynchronizedProperty());
    EXPECT_NE(d.getSyncRootProperty(), nullptr);
}

TEST(OrderedDictionaryBatch22Test, AddAndContains) {
    OrderedDictionary d;
    d.Add("a", "1");
    d.Add("b", "2");
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_TRUE(d.Contains("a"));
    EXPECT_FALSE(d.Contains("c"));
}

TEST(OrderedDictionaryBatch22Test, Add_DuplicateThrows) {
    OrderedDictionary d;
    d.Add("k", "v");
    EXPECT_THROW(d.Add("k", "v2"), System::ArgumentException);
}

TEST(OrderedDictionaryBatch22Test, Indexer_ByKey) {
    OrderedDictionary d;
    d.Add("x", "hello");
    EXPECT_EQ(d["x"], "hello");
}

TEST(OrderedDictionaryBatch22Test, Indexer_Missing_ReturnsEmpty) {
    const OrderedDictionary d;
    EXPECT_EQ(d["missing"], "");
}

TEST(OrderedDictionaryBatch22Test, Indexer_Mutable_InsertsNewKey) {
    OrderedDictionary d;
    // operator[] is read-only (matches .NET's getter, which never inserts on a miss); use
    // set() for dict[key]=value-style writes.
    d.set("newkey", "value");
    EXPECT_TRUE(d.Contains("newkey"));
    EXPECT_EQ(d["newkey"], "value");
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(OrderedDictionaryBatch22Test, Indexer_MissingKey_DoesNotInsert) {
    OrderedDictionary d;
    EXPECT_EQ(d["missing"], "");
    EXPECT_FALSE(d.Contains("missing"));
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(OrderedDictionaryBatch22Test, GetByIndex) {
    OrderedDictionary d;
    d.Add("first", "A");
    d.Add("second", "B");
    EXPECT_EQ(d.GetByIndex(0), "A");
    EXPECT_EQ(d.GetByIndex(1), "B");
    EXPECT_EQ(d.GetKey(0), "first");
    EXPECT_EQ(d.GetKey(1), "second");
}

TEST(OrderedDictionaryBatch22Test, Insert_PreservesOrder) {
    OrderedDictionary d;
    d.Add("a", "1");
    d.Add("c", "3");
    d.Insert(1, "b", "2");
    EXPECT_EQ(d.GetKey(0), "a");
    EXPECT_EQ(d.GetKey(1), "b");
    EXPECT_EQ(d.GetKey(2), "c");
}

TEST(OrderedDictionaryBatch22Test, Remove_ByKey) {
    OrderedDictionary d;
    d.Add("a", "1");
    d.Add("b", "2");
    d.Remove("a");
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.Contains("a"));
    EXPECT_EQ(d.GetKey(0), "b");
}

TEST(OrderedDictionaryBatch22Test, RemoveAt) {
    OrderedDictionary d;
    d.Add("x", "1");
    d.Add("y", "2");
    d.RemoveAt(0);
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_EQ(d.GetKey(0), "y");
}

TEST(OrderedDictionaryBatch22Test, Clear) {
    OrderedDictionary d;
    d.Add("a", "1");
    d.Clear();
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(OrderedDictionaryBatch22Test, KeysProperty_InsertionOrder) {
    OrderedDictionary d;
    d.Add("z", "1");
    d.Add("a", "2");
    d.Add("m", "3");
    const auto& keys = d.getKeysProperty();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "z");
    EXPECT_EQ(keys[1], "a");
    EXPECT_EQ(keys[2], "m");
}

TEST(OrderedDictionaryBatch22Test, ValuesProperty_InsertionOrder) {
    OrderedDictionary d;
    d.Add("a", "alpha");
    d.Add("b", "beta");
    const auto& vals = d.getValuesProperty();
    ASSERT_EQ(vals.size(), 2u);
    EXPECT_EQ(vals[0], "alpha");
    EXPECT_EQ(vals[1], "beta");
}

TEST(OrderedDictionaryBatch22Test, AsReadOnly_WrapperIsReadOnly_OriginalStaysMutable) {
    OrderedDictionary d;
    d.Add("k", "v");
    OrderedDictionary view = d.AsReadOnly();

    // The wrapper is read-only...
    EXPECT_TRUE(view.getIsReadOnlyProperty());
    EXPECT_THROW(view.Add("x", "y"), System::NotSupportedException);
    EXPECT_THROW(view.Remove("k"), System::NotSupportedException);
    EXPECT_THROW(view.Clear(), System::NotSupportedException);

    // ...but the original is NOT: AsReadOnly() must not mutate `d` itself.
    EXPECT_FALSE(d.getIsReadOnlyProperty());
    d.Add("x", "y");
    EXPECT_EQ(d.getCountProperty(), 2);

    // The wrapper shares storage with the original: it's a live view, not a snapshot.
    EXPECT_EQ(view.getCountProperty(), 2);
    EXPECT_TRUE(view.Contains("x"));
}

// ===========================================================================
// StringCollection
// ===========================================================================

TEST(StringCollectionBatch22Test, IsSynchronized_SyncRoot) {
    StringCollection sc;
    EXPECT_FALSE(sc.getIsSynchronizedProperty());
    EXPECT_NE(sc.getSyncRootProperty(), nullptr);
}

TEST(StringCollectionBatch22Test, AddAndCount) {
    StringCollection sc;
    int idx = sc.Add("hello");
    EXPECT_EQ(idx, 0);
    sc.Add("world");
    EXPECT_EQ(sc.getCountProperty(), 2);
}

TEST(StringCollectionBatch22Test, Indexer_GetSet) {
    StringCollection sc;
    sc.Add("a");
    EXPECT_EQ(sc[0], "a");
    sc[0] = "b";
    EXPECT_EQ(sc[0], "b");
}

TEST(StringCollectionBatch22Test, Contains_IndexOf) {
    StringCollection sc;
    sc.Add("x"); sc.Add("y");
    EXPECT_TRUE(sc.Contains("x"));
    EXPECT_EQ(sc.IndexOf("y"), 1);
    EXPECT_EQ(sc.IndexOf("z"), -1);
}

TEST(StringCollectionBatch22Test, AddRange) {
    StringCollection sc;
    sc.AddRange({"a", "b", "c"});
    EXPECT_EQ(sc.getCountProperty(), 3);
}

TEST(StringCollectionBatch22Test, Remove_RemoveAt_Clear) {
    StringCollection sc;
    sc.Add("a"); sc.Add("b"); sc.Add("c");
    sc.Remove("b");
    EXPECT_EQ(sc.getCountProperty(), 2);
    sc.RemoveAt(0);
    EXPECT_EQ(sc[0], "c");
    sc.Clear();
    EXPECT_EQ(sc.getCountProperty(), 0);
}

TEST(StringCollectionBatch22Test, CopyTo) {
    StringCollection sc;
    sc.Add("p"); sc.Add("q");
    std::vector<std::string> dest(4, "");
    sc.CopyTo(dest, 1);
    EXPECT_EQ(dest[1], "p");
    EXPECT_EQ(dest[2], "q");
}

TEST(StringCollectionBatch22Test, CopyTo_NegativeIndex_Throws) {
    StringCollection sc;
    sc.Add("p");
    std::vector<std::string> dest(4, "");
    EXPECT_THROW(sc.CopyTo(dest, -1), System::ArgumentOutOfRangeException);
}

TEST(StringCollectionBatch22Test, CopyTo_DestTooSmall_Throws) {
    StringCollection sc;
    sc.Add("p"); sc.Add("q");
    std::vector<std::string> dest(2, "");
    EXPECT_THROW(sc.CopyTo(dest, 1), System::ArgumentException);
}

TEST(StringCollectionBatch22Test, Insert_OutOfRange_Throws) {
    StringCollection sc;
    sc.Add("a");
    EXPECT_THROW(sc.Insert(-1, "x"), System::ArgumentOutOfRangeException);
    EXPECT_THROW(sc.Insert(2, "x"), System::ArgumentOutOfRangeException);
    sc.Insert(1, "b"); // index == Count is valid (append)
    EXPECT_EQ(sc.getCountProperty(), 2);
}

TEST(StringCollectionBatch22Test, Indexer_OutOfRange_Throws) {
    StringCollection sc;
    sc.Add("a");
    EXPECT_THROW((void)sc[-1], System::ArgumentOutOfRangeException);
    EXPECT_THROW((void)sc[1], System::ArgumentOutOfRangeException);
}

TEST(StringCollectionBatch22Test, RemoveAt_OutOfRange_Throws) {
    StringCollection sc;
    sc.Add("a");
    EXPECT_THROW(sc.RemoveAt(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(sc.RemoveAt(1), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// StringDictionary
// ===========================================================================

TEST(StringDictionaryBatch22Test, SyncRoot) {
    StringDictionary sd;
    EXPECT_NE(sd.getSyncRootProperty(), nullptr);
    EXPECT_FALSE(sd.getIsSynchronizedProperty());
}

TEST(StringDictionaryBatch22Test, AddAndCount) {
    StringDictionary sd;
    sd.Add("KEY", "val");
    EXPECT_EQ(sd.getCountProperty(), 1);
    EXPECT_TRUE(sd.ContainsKey("key")); // case-insensitive
}

TEST(StringDictionaryBatch22Test, Add_DuplicateKey_CaseInsensitive_Throws) {
    StringDictionary sd;
    sd.Add("Key", "Value");
    EXPECT_THROW(sd.Add("Key", "value"), System::ArgumentException);
    EXPECT_THROW(sd.Add("KEY", "value"), System::ArgumentException);
    EXPECT_THROW(sd.Add("key", "value"), System::ArgumentException);
    EXPECT_EQ(sd.getCountProperty(), 1);
    EXPECT_EQ(sd.GetValue("key"), "Value");
}

TEST(StringDictionaryBatch22Test, GetKeysProperty) {
    StringDictionary sd;
    sd.Add("Alpha", "1");
    sd.Add("Beta", "2");
    auto keys = sd.getKeysProperty();
    EXPECT_EQ(keys.size(), 2u);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "alpha") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "beta") != keys.end());
}

TEST(StringDictionaryBatch22Test, GetValuesProperty) {
    StringDictionary sd;
    sd.Add("k1", "v1");
    sd.Add("k2", "v2");
    auto vals = sd.getValuesProperty();
    EXPECT_EQ(vals.size(), 2u);
    EXPECT_TRUE(std::find(vals.begin(), vals.end(), "v1") != vals.end());
    EXPECT_TRUE(std::find(vals.begin(), vals.end(), "v2") != vals.end());
}

TEST(StringDictionaryBatch22Test, ContainsValue) {
    StringDictionary sd;
    sd.Add("k", "hello");
    EXPECT_TRUE(sd.ContainsValue("hello"));
    EXPECT_FALSE(sd.ContainsValue("world"));
}

TEST(StringDictionaryBatch22Test, GetValue_MissingReturnsEmpty) {
    StringDictionary sd;
    EXPECT_EQ(sd.GetValue("missing"), "");
    sd.Add("present", "found");
    EXPECT_EQ(sd.GetValue("PRESENT"), "found");
}

// operator[] used to be a single non-const `data_[lower(key)]` overload, so even a read-only
// access via operator[] silently inserted an empty entry, growing Count -- the same
// phantom-insert-on-read bug fixed in ListDictionary/OrderedDictionary/ConcurrentDictionary.
TEST(StringDictionaryBatch22Test, OperatorBracket_MissingKey_DoesNotInsert) {
    StringDictionary sd;
    EXPECT_EQ(sd["missing"], "");
    EXPECT_FALSE(sd.ContainsKey("missing"));
    EXPECT_EQ(sd.getCountProperty(), 0);
}

TEST(StringDictionaryBatch22Test, Set_InsertsAndOverwrites) {
    StringDictionary sd;
    sd.set("k", "v1");
    EXPECT_EQ(sd["k"], "v1");
    sd.set("K", "v2"); // case-insensitive key
    EXPECT_EQ(sd["k"], "v2");
    EXPECT_EQ(sd.getCountProperty(), 1);
}

// A key with a UTF-8 multi-byte sequence (high-bit bytes) must not crash or behave as UB.
// ::tolower(int) is UB for a negative argument other than EOF; passing a signed char with the
// high bit set previously sign-extended to a negative int before this fix.
TEST(StringDictionaryBatch22Test, Key_WithHighBitBytes_DoesNotCrash) {
    StringDictionary sd;
    std::string key = "caf\xC3\xA9"; // "café"
    sd.set(key, "value");
    EXPECT_EQ(sd[key], "value");
    EXPECT_TRUE(sd.ContainsKey(key));
}

TEST(StringDictionaryBatch22Test, Remove_Clear) {
    StringDictionary sd;
    sd.Add("a", "1");
    sd.Add("b", "2");
    sd.Remove("A");
    EXPECT_EQ(sd.getCountProperty(), 1);
    sd.Clear();
    EXPECT_EQ(sd.getCountProperty(), 0);
}
