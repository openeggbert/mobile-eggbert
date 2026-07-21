// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for: KeyValuePair, SortedDictionary, SortedList,
//            BitArray, Collections::Queue, Collections::Stack.
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "System/Collections/Generic/KeyValuePair.hpp"
#include "System/Collections/Generic/SortedDictionary.hpp"
#include "System/Collections/Generic/SortedList.hpp"
#include "System/Collections/BitArray.hpp"
#include "System/Collections/Queue.hpp"
#include "System/Collections/Stack.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Collections::Generic::KeyValuePair;
using System::Collections::Generic::SortedDictionary;
using System::Collections::Generic::SortedList;
using System::Collections::BitArray;
using System::Collections::Queue;
using System::Collections::Stack;

// ===========================================================================
// KeyValuePair<TKey, TValue>
// ===========================================================================

TEST(KeyValuePairTests, Constructor_StoresKeyAndValue) {
    KeyValuePair<std::string, int> kv("answer", 42);
    EXPECT_EQ(kv.Key, "answer");
    EXPECT_EQ(kv.Value, 42);
}
TEST(KeyValuePairTests, Equality_SameKeyValue) {
    KeyValuePair<int, int> a(1, 2);
    KeyValuePair<int, int> b(1, 2);
    EXPECT_TRUE(a == b);
}
TEST(KeyValuePairTests, Inequality_DifferentValue) {
    KeyValuePair<int, int> a(1, 2);
    KeyValuePair<int, int> b(1, 3);
    EXPECT_TRUE(a != b);
}
TEST(KeyValuePairTests, Create_InfersTypesFromArguments) {
    auto kv = System::Collections::Generic::Create(std::string("answer"), 42);
    EXPECT_EQ(kv.Key, "answer");
    EXPECT_EQ(kv.Value, 42);
}
TEST(KeyValuePairTests, ToString_FormatsAsBracketedPair) {
    KeyValuePair<std::string, int> kv("answer", 42);
    EXPECT_EQ(kv.ToString(), "[answer, 42]");
}

// ===========================================================================
// SortedDictionary<TKey, TValue>
// ===========================================================================

TEST(SortedDictionaryTests, DefaultCtor_CountZero) {
    SortedDictionary<std::string, int> sd;
    EXPECT_EQ(sd.getCountProperty(), 0);
}
TEST(SortedDictionaryTests, Add_IncreasesCount) {
    SortedDictionary<std::string, int> sd;
    sd.Add("b", 2);
    sd.Add("a", 1);
    EXPECT_EQ(sd.getCountProperty(), 2);
}
TEST(SortedDictionaryTests, Add_DuplicateKey_Throws) {
    SortedDictionary<std::string, int> sd;
    sd.Add("x", 1);
    EXPECT_THROW(sd.Add("x", 2), System::ArgumentException);
}
TEST(SortedDictionaryTests, ContainsKey_True_False) {
    SortedDictionary<std::string, int> sd;
    sd.Add("k", 10);
    EXPECT_TRUE(sd.ContainsKey("k"));
    EXPECT_FALSE(sd.ContainsKey("missing"));
}
TEST(SortedDictionaryTests, OperatorBracket_ReadWrite) {
    SortedDictionary<std::string, int> sd;
    sd["a"] = 100;
    EXPECT_EQ(sd["a"], 100);
}

// Regression test for POST_STABILIZATION_AUDIT.md finding #3 / ticket 1712: the non-const
// operator[] getter previously silently inserted a default-constructed value on a missing key
// (std::map::operator[]'s convention) instead of throwing, unlike real .NET's
// SortedDictionary<TKey,TValue> indexer, which throws KeyNotFoundException on the getter
// regardless of const/non-const.
TEST(SortedDictionaryTests, OperatorBracket_NonConst_ThrowsIfMissing) {
    SortedDictionary<std::string, int> sd;
    EXPECT_THROW((void)static_cast<int>(sd["missing"]), System::Collections::Generic::KeyNotFoundException);
    EXPECT_FALSE(sd.ContainsKey("missing"));
    EXPECT_EQ(sd.getCountProperty(), 0);
}

TEST(SortedDictionaryTests, OperatorBracket_NonConst_SetMissingStillInserts) {
    SortedDictionary<std::string, int> sd;
    sd["newkey"] = 42;
    EXPECT_TRUE(sd.ContainsKey("newkey"));
    EXPECT_EQ(static_cast<int>(sd["newkey"]), 42);
}

TEST(SortedDictionaryTests, OperatorBracket_NonConst_SetExistingUpdates) {
    SortedDictionary<std::string, int> sd;
    sd.Add("k", 1);
    sd["k"] = 2;
    EXPECT_EQ(static_cast<int>(sd["k"]), 2);
    EXPECT_EQ(sd.getCountProperty(), 1);
}
TEST(SortedDictionaryTests, TryGetValue_Found) {
    SortedDictionary<std::string, int> sd;
    sd.Add("z", 99);
    int v = 0;
    EXPECT_TRUE(sd.TryGetValue("z", v));
    EXPECT_EQ(v, 99);
}
TEST(SortedDictionaryTests, TryGetValue_NotFound_ReturnsFalse) {
    SortedDictionary<std::string, int> sd;
    int v = 0;
    EXPECT_FALSE(sd.TryGetValue("missing", v));
}
TEST(SortedDictionaryTests, Remove_DecreaseCount) {
    SortedDictionary<std::string, int> sd;
    sd.Add("a", 1);
    sd.Add("b", 2);
    EXPECT_TRUE(sd.Remove("a"));
    EXPECT_EQ(sd.getCountProperty(), 1);
    EXPECT_FALSE(sd.ContainsKey("a"));
}
TEST(SortedDictionaryTests, Remove_NonExistent_ReturnsFalse) {
    SortedDictionary<std::string, int> sd;
    EXPECT_FALSE(sd.Remove("nope"));
}
TEST(SortedDictionaryTests, Clear_EmptiesMap) {
    SortedDictionary<std::string, int> sd;
    sd.Add("a", 1);
    sd.Clear();
    EXPECT_EQ(sd.getCountProperty(), 0);
}
TEST(SortedDictionaryTests, Keys_AreSorted) {
    SortedDictionary<std::string, int> sd;
    sd.Add("c", 3); sd.Add("a", 1); sd.Add("b", 2);
    auto keys = sd.Keys();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "a");
    EXPECT_EQ(keys[1], "b");
    EXPECT_EQ(keys[2], "c");
}
TEST(SortedDictionaryTests, Values_InKeyOrder) {
    SortedDictionary<std::string, int> sd;
    sd.Add("c", 3); sd.Add("a", 1); sd.Add("b", 2);
    auto vals = sd.Values();
    ASSERT_EQ(vals.size(), 3u);
    EXPECT_EQ(vals[0], 1);
    EXPECT_EQ(vals[2], 3);
}

// ===========================================================================
// SortedList<TKey, TValue>
// ===========================================================================

TEST(SortedListTests, DefaultCtor_CountZero) {
    SortedList<int, std::string> sl;
    EXPECT_EQ(sl.getCountProperty(), 0);
}
TEST(SortedListTests, Add_IncreasesCount) {
    SortedList<int, std::string> sl;
    sl.Add(2, "b");
    sl.Add(1, "a");
    EXPECT_EQ(sl.getCountProperty(), 2);
}
TEST(SortedListTests, Add_DuplicateKey_Throws) {
    SortedList<int, std::string> sl;
    sl.Add(1, "a");
    EXPECT_THROW(sl.Add(1, "b"), System::ArgumentException);
}
TEST(SortedListTests, ContainsKey_True_False) {
    SortedList<int, std::string> sl;
    sl.Add(5, "five");
    EXPECT_TRUE(sl.ContainsKey(5));
    EXPECT_FALSE(sl.ContainsKey(99));
}
TEST(SortedListTests, ContainsValue_True_False) {
    SortedList<int, std::string> sl;
    sl.Add(1, "hello");
    EXPECT_TRUE(sl.ContainsValue("hello"));
    EXPECT_FALSE(sl.ContainsValue("world"));
}
TEST(SortedListTests, IndexOfKey_Found_NotFound) {
    SortedList<int, std::string> sl;
    sl.Add(10, "a"); sl.Add(20, "b"); sl.Add(30, "c");
    EXPECT_EQ(sl.IndexOfKey(20), 1);
    EXPECT_EQ(sl.IndexOfKey(99), -1);
}
TEST(SortedListTests, RemoveAt_RemovesByIndex) {
    SortedList<int, std::string> sl;
    sl.Add(1, "a"); sl.Add(2, "b"); sl.Add(3, "c");
    sl.RemoveAt(1);
    EXPECT_EQ(sl.getCountProperty(), 2);
    EXPECT_FALSE(sl.ContainsKey(2));
}
TEST(SortedListTests, RemoveAt_OutOfRange_Throws) {
    SortedList<int, std::string> sl;
    EXPECT_THROW(sl.RemoveAt(0), System::ArgumentOutOfRangeException);
}
TEST(SortedListTests, TryGetValue_Found) {
    SortedList<int, std::string> sl;
    sl.Add(7, "seven");
    std::string v;
    EXPECT_TRUE(sl.TryGetValue(7, v));
    EXPECT_EQ(v, "seven");
}
TEST(SortedListTests, Clear_Empties) {
    SortedList<int, std::string> sl;
    sl.Add(1, "a");
    sl.Clear();
    EXPECT_EQ(sl.getCountProperty(), 0);
}
TEST(SortedListTests, Keys_AreSorted) {
    SortedList<int, std::string> sl;
    sl.Add(3, "c"); sl.Add(1, "a"); sl.Add(2, "b");
    auto keys = sl.Keys();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], 1);
    EXPECT_EQ(keys[2], 3);
}

// ===========================================================================
// BitArray
// ===========================================================================

TEST(BitArrayTests, LengthCtor_AllFalseByDefault) {
    BitArray ba(8);
    EXPECT_EQ(ba.getLengthProperty(), 8);
    for (int i = 0; i < 8; ++i) EXPECT_FALSE(ba.Get(i));
}
TEST(BitArrayTests, LengthCtor_DefaultTrue) {
    BitArray ba(4, true);
    for (int i = 0; i < 4; ++i) EXPECT_TRUE(ba.Get(i));
}
TEST(BitArrayTests, Set_And_Get) {
    BitArray ba(8);
    ba.Set(3, true);
    EXPECT_TRUE(ba.Get(3));
    EXPECT_FALSE(ba.Get(2));
}
TEST(BitArrayTests, OperatorBracket_ReadsBit) {
    BitArray ba(4, false);
    ba.Set(1, true);
    EXPECT_TRUE(ba[1]);
    EXPECT_FALSE(ba[0]);
}
TEST(BitArrayTests, SetAll_SetsEverything) {
    BitArray ba(5, false);
    ba.SetAll(true);
    for (int i = 0; i < 5; ++i) EXPECT_TRUE(ba.Get(i));
}
TEST(BitArrayTests, And_Operation) {
    BitArray a(4, true);
    BitArray b(4, false);
    b.Set(2, true);
    a.And(b);
    EXPECT_FALSE(a.Get(0));
    EXPECT_TRUE(a.Get(2));
}
TEST(BitArrayTests, Or_Operation) {
    BitArray a(4, false);
    BitArray b(4, false);
    b.Set(1, true);
    a.Or(b);
    EXPECT_TRUE(a.Get(1));
    EXPECT_FALSE(a.Get(0));
}
TEST(BitArrayTests, Xor_Operation) {
    BitArray a(4, true);
    BitArray b(4, true);
    a.Xor(b);
    for (int i = 0; i < 4; ++i) EXPECT_FALSE(a.Get(i));
}
TEST(BitArrayTests, Not_FlipsBits) {
    BitArray ba(3, false);
    ba.Not();
    for (int i = 0; i < 3; ++i) EXPECT_TRUE(ba.Get(i));
}
TEST(BitArrayTests, BytesCtor_ExpandsToBits) {
    std::vector<uint8_t> bytes = {0b00000001, 0b10000000};
    BitArray ba(bytes);
    EXPECT_EQ(ba.getLengthProperty(), 16);
    EXPECT_TRUE(ba.Get(0));
    EXPECT_FALSE(ba.Get(1));
    EXPECT_TRUE(ba.Get(15));
}
TEST(BitArrayTests, HasAllSet_AllTrue) {
    BitArray ba(4, true);
    EXPECT_TRUE(ba.HasAllSet());
}
TEST(BitArrayTests, HasAllSet_SomeFalse) {
    BitArray ba(4, true);
    ba.Set(2, false);
    EXPECT_FALSE(ba.HasAllSet());
}
TEST(BitArrayTests, HasAnySet_OneBitSet) {
    BitArray ba(4, false);
    ba.Set(1, true);
    EXPECT_TRUE(ba.HasAnySet());
}
TEST(BitArrayTests, HasAnySet_AllFalse) {
    BitArray ba(4, false);
    EXPECT_FALSE(ba.HasAnySet());
}
TEST(BitArrayTests, CopyTo_BoolVector) {
    BitArray ba(3, false);
    ba.Set(0, true); ba.Set(2, true);
    std::vector<bool> v;
    ba.CopyTo(v);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_TRUE(v[0]); EXPECT_FALSE(v[1]); EXPECT_TRUE(v[2]);
}
TEST(BitArrayTests, CopyTo_ByteVector) {
    BitArray ba(8, false);
    ba.Set(0, true); ba.Set(1, true); // byte 0 = 0b00000011
    std::vector<uint8_t> v;
    ba.CopyTo(v);
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 0b00000011u);
}

// ===========================================================================
// Collections::Queue (void*)
// ===========================================================================

TEST(CollectionsQueueTests, DefaultCtor_CountZero) {
    Queue q;
    EXPECT_EQ(q.getCountProperty(), 0);
}
TEST(CollectionsQueueTests, Enqueue_IncreasesCount) {
    Queue q;
    int x = 1;
    q.Enqueue(&x);
    EXPECT_EQ(q.getCountProperty(), 1);
}
TEST(CollectionsQueueTests, Dequeue_FIFO_Order) {
    Queue q;
    int a = 1, b = 2;
    q.Enqueue(&a);
    q.Enqueue(&b);
    EXPECT_EQ(q.Dequeue(), &a);
    EXPECT_EQ(q.Dequeue(), &b);
}
TEST(CollectionsQueueTests, Peek_ReturnsFront_DoesNotRemove) {
    Queue q;
    int x = 5;
    q.Enqueue(&x);
    EXPECT_EQ(q.Peek(), &x);
    EXPECT_EQ(q.getCountProperty(), 1);
}
TEST(CollectionsQueueTests, Dequeue_Empty_Throws) {
    Queue q;
    EXPECT_THROW(q.Dequeue(), System::InvalidOperationException);
}
TEST(CollectionsQueueTests, Peek_Empty_Throws) {
    Queue q;
    EXPECT_THROW(q.Peek(), System::InvalidOperationException);
}
TEST(CollectionsQueueTests, Contains_True_False) {
    Queue q;
    int x = 1;
    q.Enqueue(&x);
    EXPECT_TRUE(q.Contains(&x));
    int y = 2;
    EXPECT_FALSE(q.Contains(&y));
}
TEST(CollectionsQueueTests, Clear_EmptiesQueue) {
    Queue q;
    int x = 1;
    q.Enqueue(&x);
    q.Clear();
    EXPECT_EQ(q.getCountProperty(), 0);
}

// ===========================================================================
// Collections::Stack (void*)
// ===========================================================================

TEST(CollectionsStackTests, DefaultCtor_CountZero) {
    Stack s;
    EXPECT_EQ(s.getCountProperty(), 0);
}
TEST(CollectionsStackTests, Push_IncreasesCount) {
    Stack s;
    int x = 1;
    s.Push(&x);
    EXPECT_EQ(s.getCountProperty(), 1);
}
TEST(CollectionsStackTests, Pop_LIFO_Order) {
    Stack s;
    int a = 1, b = 2;
    s.Push(&a);
    s.Push(&b);
    EXPECT_EQ(s.Pop(), &b);
    EXPECT_EQ(s.Pop(), &a);
}
TEST(CollectionsStackTests, Peek_ReturnsTop_DoesNotRemove) {
    Stack s;
    int x = 9;
    s.Push(&x);
    EXPECT_EQ(s.Peek(), &x);
    EXPECT_EQ(s.getCountProperty(), 1);
}
TEST(CollectionsStackTests, Pop_Empty_Throws) {
    Stack s;
    EXPECT_THROW(s.Pop(), System::InvalidOperationException);
}
TEST(CollectionsStackTests, Peek_Empty_Throws) {
    Stack s;
    EXPECT_THROW(s.Peek(), System::InvalidOperationException);
}
TEST(CollectionsStackTests, Contains_True_False) {
    Stack s;
    int x = 42;
    s.Push(&x);
    EXPECT_TRUE(s.Contains(&x));
    int y = 0;
    EXPECT_FALSE(s.Contains(&y));
}
TEST(CollectionsStackTests, Clear_EmptiesStack) {
    Stack s;
    int x = 1;
    s.Push(&x);
    s.Clear();
    EXPECT_EQ(s.getCountProperty(), 0);
}
