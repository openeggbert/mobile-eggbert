// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/LinkedList.hpp"
#include "System/Collections/Generic/SortedSet.hpp"
#include "System/InvalidOperationException.hpp"

using System::Collections::Generic::LinkedList;
using System::Collections::Generic::SortedSet;

// ---------------------------------------------------------------------------
// LinkedList<T>
// ---------------------------------------------------------------------------

TEST(LinkedListTests, DefaultCtorIsEmpty) {
    LinkedList<int> ll;
    EXPECT_EQ(ll.getCountProperty(), 0);
}

TEST(LinkedListTests, AddLastIncreasesCount) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2);
    EXPECT_EQ(ll.getCountProperty(), 2);
}

TEST(LinkedListTests, AddFirstPrepends) {
    LinkedList<int> ll;
    ll.AddLast(2);
    ll.AddFirst(1);
    EXPECT_EQ(ll.getFirstProperty(), 1);
    EXPECT_EQ(ll.getLastProperty(),  2);
}

TEST(LinkedListTests, AddLastAppends) {
    LinkedList<int> ll;
    ll.AddFirst(1);
    ll.AddLast(2);
    EXPECT_EQ(ll.getFirstProperty(), 1);
    EXPECT_EQ(ll.getLastProperty(),  2);
}

TEST(LinkedListTests, GetFirstProperty) {
    LinkedList<int> ll;
    ll.AddLast(10); ll.AddLast(20); ll.AddLast(30);
    EXPECT_EQ(ll.getFirstProperty(), 10);
}

TEST(LinkedListTests, GetLastProperty) {
    LinkedList<int> ll;
    ll.AddLast(10); ll.AddLast(20); ll.AddLast(30);
    EXPECT_EQ(ll.getLastProperty(), 30);
}

TEST(LinkedListTests, RemoveFirst) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    ll.RemoveFirst();
    EXPECT_EQ(ll.getCountProperty(), 2);
    EXPECT_EQ(ll.getFirstProperty(), 2);
}

TEST(LinkedListTests, RemoveLast) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    ll.RemoveLast();
    EXPECT_EQ(ll.getCountProperty(), 2);
    EXPECT_EQ(ll.getLastProperty(), 2);
}

TEST(LinkedListTests, RemoveFirstOnEmptyThrowsInvalidOperationException) {
    LinkedList<int> ll;
    EXPECT_THROW(ll.RemoveFirst(), System::InvalidOperationException);
    EXPECT_EQ(ll.getCountProperty(), 0);
}

TEST(LinkedListTests, RemoveLastOnEmptyThrowsInvalidOperationException) {
    LinkedList<int> ll;
    EXPECT_THROW(ll.RemoveLast(), System::InvalidOperationException);
    EXPECT_EQ(ll.getCountProperty(), 0);
}

TEST(LinkedListTests, RemoveByValue) {
    LinkedList<int> ll;
    ll.AddLast(10); ll.AddLast(20); ll.AddLast(30);
    EXPECT_TRUE(ll.Remove(20));
    EXPECT_EQ(ll.getCountProperty(), 2);
    EXPECT_EQ(ll.getFirstProperty(), 10);
    EXPECT_EQ(ll.getLastProperty(),  30);
}

TEST(LinkedListTests, RemoveByValueNotFound) {
    LinkedList<int> ll;
    ll.AddLast(1);
    EXPECT_FALSE(ll.Remove(99));
    EXPECT_EQ(ll.getCountProperty(), 1);
}

TEST(LinkedListTests, ContainsFound) {
    LinkedList<int> ll;
    ll.AddLast(5); ll.AddLast(10);
    EXPECT_TRUE(ll.Contains(10));
}

TEST(LinkedListTests, ContainsNotFound) {
    LinkedList<int> ll;
    ll.AddLast(5);
    EXPECT_FALSE(ll.Contains(99));
}

TEST(LinkedListTests, ClearResetsCount) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    ll.Clear();
    EXPECT_EQ(ll.getCountProperty(), 0);
    EXPECT_FALSE(ll.Contains(1));
}

TEST(LinkedListTests, RangeForIteration) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    std::vector<int> collected;
    for (int x : ll) collected.push_back(x);
    ASSERT_EQ(collected.size(), 3u);
    EXPECT_EQ(collected[0], 1);
    EXPECT_EQ(collected[1], 2);
    EXPECT_EQ(collected[2], 3);
}

TEST(LinkedListTests, AddFirstThenIterateIsReversed) {
    LinkedList<int> ll;
    ll.AddFirst(3); ll.AddFirst(2); ll.AddFirst(1);
    std::vector<int> collected;
    for (int x : ll) collected.push_back(x);
    EXPECT_EQ(collected[0], 1);
    EXPECT_EQ(collected[1], 2);
    EXPECT_EQ(collected[2], 3);
}

TEST(LinkedListTests, StringLinkedList) {
    LinkedList<std::string> ll;
    ll.AddLast(std::string("a"));
    ll.AddLast(std::string("b"));
    ll.AddFirst(std::string("z"));
    EXPECT_EQ(ll.getFirstProperty(), "z");
    EXPECT_EQ(ll.getLastProperty(),  "b");
    EXPECT_EQ(ll.getCountProperty(), 3);
    EXPECT_TRUE(ll.Contains(std::string("a")));
    ll.Remove(std::string("a"));
    EXPECT_FALSE(ll.Contains(std::string("a")));
}

// ---------------------------------------------------------------------------
// LinkedListNode-based API
// ---------------------------------------------------------------------------

TEST(LinkedListTests, AddLast_ReturnsNode_ValueCorrect) {
    LinkedList<int> ll;
    auto node = ll.AddLast(42);
    EXPECT_TRUE(static_cast<bool>(node));
    EXPECT_EQ(node.getValueProperty(), 42);
}

TEST(LinkedListTests, AddFirst_ReturnsNode_ValueCorrect) {
    LinkedList<int> ll;
    ll.AddLast(2);
    auto node = ll.AddFirst(1);
    EXPECT_EQ(node.getValueProperty(), 1);
    EXPECT_EQ(ll.getCountProperty(), 2);
}

TEST(LinkedListTests, GetFirstProperty_ReturnsNode) {
    LinkedList<int> ll;
    ll.AddLast(10); ll.AddLast(20);
    auto n = ll.getFirstProperty();
    EXPECT_TRUE(static_cast<bool>(n));
    EXPECT_EQ(n.getValueProperty(), 10);
}

TEST(LinkedListTests, GetLastProperty_ReturnsNode) {
    LinkedList<int> ll;
    ll.AddLast(10); ll.AddLast(20);
    auto n = ll.getLastProperty();
    EXPECT_EQ(n.getValueProperty(), 20);
}

TEST(LinkedListTests, GetFirstProperty_EmptyList_NullNode) {
    LinkedList<int> ll;
    auto n = ll.getFirstProperty();
    EXPECT_FALSE(static_cast<bool>(n));
    EXPECT_EQ(n, nullptr);
}

TEST(LinkedListTests, Find_ExistingValue_ReturnsNode) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    auto n = ll.Find(2);
    EXPECT_TRUE(static_cast<bool>(n));
    EXPECT_EQ(n.getValueProperty(), 2);
}

TEST(LinkedListTests, Find_MissingValue_NullNode) {
    LinkedList<int> ll;
    ll.AddLast(1);
    auto n = ll.Find(99);
    EXPECT_FALSE(static_cast<bool>(n));
}

TEST(LinkedListTests, FindLast_ReturnsLastOccurrence) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(1);
    auto n = ll.FindLast(1);
    EXPECT_TRUE(static_cast<bool>(n));
    EXPECT_EQ(n.getValueProperty(), 1);
    // Verify it's the last by checking count after removal
    ll.Remove(n);
    EXPECT_EQ(ll.getCountProperty(), 2);
    EXPECT_EQ(ll.getLastProperty().getValueProperty(), 2);
}

TEST(LinkedListTests, AddBefore_InsertsBeforeNode) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(3);
    auto n3 = ll.Find(3);
    ll.AddBefore(n3, 2);
    std::vector<int> v;
    for (int x : ll) v.push_back(x);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
}

TEST(LinkedListTests, AddAfter_InsertsAfterNode) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(3);
    auto n1 = ll.Find(1);
    ll.AddAfter(n1, 2);
    std::vector<int> v;
    for (int x : ll) v.push_back(x);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
}

TEST(LinkedListTests, Remove_ByNode_RemovesIt) {
    LinkedList<int> ll;
    ll.AddLast(10); ll.AddLast(20); ll.AddLast(30);
    auto n = ll.Find(20);
    ll.Remove(n);
    EXPECT_EQ(ll.getCountProperty(), 2);
    EXPECT_FALSE(ll.Contains(20));
}

TEST(LinkedListTests, Remove_NullNode_ThrowsArgumentNullException) {
    LinkedList<int> ll;
    ll.AddLast(1);
    System::Collections::Generic::LinkedListNode<int> nullNode;
    EXPECT_THROW(ll.Remove(nullNode), System::ArgumentNullException);
}

TEST(LinkedListTests, Remove_NodeFromOtherList_ThrowsInvalidOperationException) {
    LinkedList<int> ll1;
    ll1.AddLast(1);
    LinkedList<int> ll2;
    ll2.AddLast(1);
    auto foreignNode = ll2.Find(1);
    EXPECT_THROW(ll1.Remove(foreignNode), System::InvalidOperationException);
}

TEST(LinkedListTests, AddBefore_NullNode_ThrowsArgumentNullException) {
    LinkedList<int> ll;
    System::Collections::Generic::LinkedListNode<int> nullNode;
    EXPECT_THROW(ll.AddBefore(nullNode, 1), System::ArgumentNullException);
}

TEST(LinkedListTests, AddAfter_NodeFromOtherList_ThrowsInvalidOperationException) {
    LinkedList<int> ll1;
    ll1.AddLast(1);
    LinkedList<int> ll2;
    ll2.AddLast(1);
    auto foreignNode = ll2.Find(1);
    EXPECT_THROW(ll1.AddAfter(foreignNode, 2), System::InvalidOperationException);
}

TEST(LinkedListTests, CopyTo_Basic) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    std::vector<int> dest(5, 0);
    ll.CopyTo(dest, 1);
    EXPECT_EQ(dest[0], 0);
    EXPECT_EQ(dest[1], 1);
    EXPECT_EQ(dest[2], 2);
    EXPECT_EQ(dest[3], 3);
    EXPECT_EQ(dest[4], 0);
}

TEST(LinkedListTests, CopyTo_NegativeIndex_ThrowsArgumentOutOfRangeException) {
    LinkedList<int> ll;
    ll.AddLast(1);
    std::vector<int> dest(5, 0);
    EXPECT_THROW(ll.CopyTo(dest, -1), System::ArgumentOutOfRangeException);
}

TEST(LinkedListTests, CopyTo_IndexBeyondEmptyDest_ThrowsArgumentOutOfRangeException) {
    // Regression: real .NET's LinkedList<T>.CopyTo treats "index > array.Length" as a distinct
    // ArgumentOutOfRangeException, separate from the ArgumentException thrown for insufficient
    // remaining space -- an out-of-bounds index against an EMPTY source list previously threw
    // the wrong exception type (ArgumentException) here, since both cases were conflated into
    // one check.
    LinkedList<int> ll; // empty
    std::vector<int> dest(3, 0);
    EXPECT_THROW(ll.CopyTo(dest, 4), System::ArgumentOutOfRangeException);
}

TEST(LinkedListTests, CopyTo_InsufficientSpace_ThrowsArgumentException) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    std::vector<int> dest(2, 0);
    EXPECT_THROW(ll.CopyTo(dest, 0), System::ArgumentException);
}

// ---------------------------------------------------------------------------
// SortedSet<T>
// ---------------------------------------------------------------------------

TEST(SortedSetTests, DefaultCtorIsEmpty) {
    SortedSet<int> ss;
    EXPECT_EQ(ss.getCountProperty(), 0);
    EXPECT_TRUE(ss.getIsEmptyProperty());
}

TEST(SortedSetTests, InitializerListCtor) {
    SortedSet<int> ss{3, 1, 2};
    EXPECT_EQ(ss.getCountProperty(), 3);
    EXPECT_FALSE(ss.getIsEmptyProperty());
}

TEST(SortedSetTests, AddReturnsTrueFirstTime) {
    SortedSet<int> ss;
    EXPECT_TRUE(ss.Add(42));
    EXPECT_EQ(ss.getCountProperty(), 1);
}

TEST(SortedSetTests, AddReturnsFalseIfDuplicate) {
    SortedSet<int> ss;
    ss.Add(7);
    EXPECT_FALSE(ss.Add(7));
    EXPECT_EQ(ss.getCountProperty(), 1);
}

TEST(SortedSetTests, ContainsAfterAdd) {
    SortedSet<int> ss;
    ss.Add(5);
    EXPECT_TRUE(ss.Contains(5));
}

TEST(SortedSetTests, ContainsNotFound) {
    SortedSet<int> ss;
    ss.Add(1);
    EXPECT_FALSE(ss.Contains(99));
}

TEST(SortedSetTests, RemoveReturnsTrueIfFound) {
    SortedSet<int> ss;
    ss.Add(10);
    EXPECT_TRUE(ss.Remove(10));
    EXPECT_EQ(ss.getCountProperty(), 0);
}

TEST(SortedSetTests, RemoveReturnsFalseIfAbsent) {
    SortedSet<int> ss;
    EXPECT_FALSE(ss.Remove(99));
}

TEST(SortedSetTests, MinProperty) {
    SortedSet<int> ss{5, 1, 3, 9, 2};
    EXPECT_EQ(ss.getMinProperty(), 1);
}

TEST(SortedSetTests, MaxProperty) {
    SortedSet<int> ss{5, 1, 3, 9, 2};
    EXPECT_EQ(ss.getMaxProperty(), 9);
}

TEST(SortedSetTests, MinMaxProperty_Empty_ReturnsDefault) {
    SortedSet<int> ss;
    EXPECT_EQ(ss.getMinProperty(), 0);
    EXPECT_EQ(ss.getMaxProperty(), 0);
}

TEST(SortedSetTests, IterationIsSorted) {
    SortedSet<int> ss;
    ss.Add(30); ss.Add(10); ss.Add(20);
    std::vector<int> v;
    for (int x : ss) v.push_back(x);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
}

TEST(SortedSetTests, ToVectorIsSorted) {
    SortedSet<int> ss{5, 3, 1, 4, 2};
    std::vector<int> v = ss.ToVector();
    ASSERT_EQ(v.size(), 5u);
    for (int i = 0; i < 4; ++i) EXPECT_LT(v[i], v[i+1]);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[4], 5);
}

TEST(SortedSetTests, ClearResetsCount) {
    SortedSet<int> ss{1, 2, 3};
    ss.Clear();
    EXPECT_EQ(ss.getCountProperty(), 0);
    EXPECT_TRUE(ss.getIsEmptyProperty());
}

TEST(SortedSetTests, UnionWith) {
    SortedSet<int> a{1, 2, 3};
    SortedSet<int> b{2, 3, 4};
    a.UnionWith(b);
    EXPECT_EQ(a.getCountProperty(), 4);
    EXPECT_TRUE(a.Contains(1));
    EXPECT_TRUE(a.Contains(4));
}

TEST(SortedSetTests, IntersectWith) {
    SortedSet<int> a{1, 2, 3, 4};
    SortedSet<int> b{2, 4, 6};
    a.IntersectWith(b);
    EXPECT_EQ(a.getCountProperty(), 2);
    EXPECT_FALSE(a.Contains(1));
    EXPECT_TRUE(a.Contains(2));
    EXPECT_FALSE(a.Contains(3));
    EXPECT_TRUE(a.Contains(4));
}

TEST(SortedSetTests, ExceptWith) {
    SortedSet<int> a{1, 2, 3, 4};
    SortedSet<int> b{2, 4};
    a.ExceptWith(b);
    EXPECT_EQ(a.getCountProperty(), 2);
    EXPECT_TRUE(a.Contains(1));
    EXPECT_FALSE(a.Contains(2));
    EXPECT_TRUE(a.Contains(3));
    EXPECT_FALSE(a.Contains(4));
}

TEST(SortedSetTests, IsSubsetOf) {
    SortedSet<int> sub{2, 3};
    SortedSet<int> sup{1, 2, 3, 4};
    EXPECT_TRUE(sub.IsSubsetOf(sup));
    EXPECT_FALSE(sup.IsSubsetOf(sub));
}

TEST(SortedSetTests, IsSupersetOf) {
    SortedSet<int> sup{1, 2, 3, 4};
    SortedSet<int> sub{2, 3};
    EXPECT_TRUE(sup.IsSupersetOf(sub));
    EXPECT_FALSE(sub.IsSupersetOf(sup));
}

TEST(SortedSetTests, GetViewBetween) {
    SortedSet<int> ss{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    SortedSet<int> view = ss.GetViewBetween(3, 7);
    EXPECT_EQ(view.getCountProperty(), 5);
    EXPECT_EQ(view.getMinProperty(), 3);
    EXPECT_EQ(view.getMaxProperty(), 7);
    EXPECT_FALSE(view.Contains(2));
    EXPECT_FALSE(view.Contains(8));
}

TEST(SortedSetTests, StringSortedSet) {
    SortedSet<std::string> ss;
    ss.Add(std::string("banana"));
    ss.Add(std::string("apple"));
    ss.Add(std::string("cherry"));
    EXPECT_EQ(ss.getCountProperty(), 3);
    std::vector<std::string> v = ss.ToVector();
    EXPECT_EQ(v[0], "apple");
    EXPECT_EQ(v[1], "banana");
    EXPECT_EQ(v[2], "cherry");
}

// ---------------------------------------------------------------------------
// SortedSet — set-algebra parity with HashSet
// ---------------------------------------------------------------------------

TEST(SortedSetTests, SetEquals_SameSets_True) {
    SortedSet<int> a{1, 2, 3};
    SortedSet<int> b{1, 2, 3};
    EXPECT_TRUE(a.SetEquals(b));
}
TEST(SortedSetTests, SetEquals_DifferentSets_False) {
    SortedSet<int> a{1, 2, 3};
    SortedSet<int> b{1, 2, 4};
    EXPECT_FALSE(a.SetEquals(b));
}
TEST(SortedSetTests, SymmetricExceptWith_BasicXor) {
    SortedSet<int> a{1, 2, 3};
    SortedSet<int> b{2, 3, 4};
    a.SymmetricExceptWith(b);
    std::vector<int> v = a.ToVector();
    EXPECT_EQ(v, (std::vector<int>{1, 4}));
}
TEST(SortedSetTests, SymmetricExceptWith_EmptyOther_Unchanged) {
    SortedSet<int> a{1, 2};
    SortedSet<int> b;
    a.SymmetricExceptWith(b);
    EXPECT_EQ(a.getCountProperty(), 2);
}
// Regression tests: ExceptWith/SymmetricExceptWith previously erased from data_ while
// iterating other.data_ with no guard for `other` aliasing `this` -- real UB (an iterator to
// an erased element, used again by the range-for loop), matching the identical bug just found
// and fixed in HashSet<T> (ticket 324). Real .NET's SortedSet<T> explicitly special-cases
// `other == this` in both methods for exactly this reason.
TEST(SortedSetTests, ExceptWith_Self_ClearsSet) {
    SortedSet<int> a{1, 2, 3, 4, 5};
    a.ExceptWith(a);
    EXPECT_EQ(a.getCountProperty(), 0);
}
TEST(SortedSetTests, SymmetricExceptWith_Self_ClearsSet) {
    SortedSet<int> a{1, 2, 3, 4, 5};
    a.SymmetricExceptWith(a);
    EXPECT_EQ(a.getCountProperty(), 0);
}
TEST(SortedSetTests, IsProperSubsetOf_True) {
    SortedSet<int> a{1, 2};
    SortedSet<int> b{1, 2, 3};
    EXPECT_TRUE(a.IsProperSubsetOf(b));
}
TEST(SortedSetTests, IsProperSubsetOf_EqualSets_False) {
    SortedSet<int> a{1, 2, 3};
    SortedSet<int> b{1, 2, 3};
    EXPECT_FALSE(a.IsProperSubsetOf(b));
}
TEST(SortedSetTests, IsProperSupersetOf_True) {
    SortedSet<int> a{1, 2, 3};
    SortedSet<int> b{1, 2};
    EXPECT_TRUE(a.IsProperSupersetOf(b));
}
TEST(SortedSetTests, IsProperSupersetOf_EqualSets_False) {
    SortedSet<int> a{1, 2};
    SortedSet<int> b{1, 2};
    EXPECT_FALSE(a.IsProperSupersetOf(b));
}
