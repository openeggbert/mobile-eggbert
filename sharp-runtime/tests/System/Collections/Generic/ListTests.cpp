// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/List.hpp"
#include "System/Collections/Generic/LinkedList.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include <string>
#include <vector>

using namespace System::Collections::Generic;

// ---- List<int> core IList interface ----
TEST(GenListTests, AddAndCount) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_EQ(lst.getCountProperty(), 3);
}

TEST(GenListTests, IndexAccess) {
    List<int> lst;
    lst.Add(10); lst.Add(20);
    EXPECT_EQ(lst[0], 10);
    EXPECT_EQ(lst[1], 20);
}

TEST(GenListTests, MutableIndexAccess) {
    List<int> lst;
    lst.Add(0);
    lst[0] = 42;
    EXPECT_EQ(lst[0], 42);
}

TEST(GenListTests, Contains) {
    List<int> lst;
    lst.Add(5);
    EXPECT_TRUE(lst.Contains(5));
    EXPECT_FALSE(lst.Contains(99));
}

TEST(GenListTests, Remove) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    EXPECT_TRUE(lst.Remove(1));
    EXPECT_EQ(lst.getCountProperty(), 1);
}

TEST(GenListTests, Clear) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    lst.Clear();
    EXPECT_EQ(lst.getCountProperty(), 0);
}

TEST(GenListTests, IndexOf) {
    List<int> lst;
    lst.Add(5); lst.Add(10); lst.Add(15);
    EXPECT_EQ(lst.IndexOf(10), 1);
    EXPECT_EQ(lst.IndexOf(99), -1);
}

// IndexOf(item, startIndex, count) / LastIndexOf(item, startIndex, count) were entirely missing
// from List<T> (only the 1-arg and 2-arg overloads existed) despite being part of real .NET's
// List<T> public surface; added to match List.cs exactly (ticket 261).
TEST(GenListTests, IndexOf_StartIndexCount_FindsWithinRange) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(1); lst.Add(2);
    EXPECT_EQ(lst.IndexOf(1, 1, 3), 2);
    EXPECT_EQ(lst.IndexOf(1, 0, 1), 0);
}

TEST(GenListTests, IndexOf_StartIndexCount_NotFoundOutsideRange) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(1);
    EXPECT_EQ(lst.IndexOf(1, 1, 1), -1); // only examines index 1 (value 2)
}

TEST(GenListTests, IndexOf_StartIndexCount_NegativeCount_Throws) {
    List<int> lst;
    lst.Add(1);
    EXPECT_THROW(lst.IndexOf(1, 0, -1), System::ArgumentOutOfRangeException);
}

TEST(GenListTests, IndexOf_StartIndexCount_RangePastEnd_Throws) {
    List<int> lst;
    lst.Add(1);
    EXPECT_THROW(lst.IndexOf(1, 0, 5), System::ArgumentOutOfRangeException);
}

TEST(GenListTests, LastIndexOf_StartIndexCount_FindsWithinRange) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(1); lst.Add(2);
    EXPECT_EQ(lst.LastIndexOf(1, 2, 3), 2);
    // Excluding index 2 from the search range (start at 1, examine indices {1,0}) finds the
    // earlier occurrence at index 0 instead.
    EXPECT_EQ(lst.LastIndexOf(1, 1, 2), 0);
}

TEST(GenListTests, LastIndexOf_StartIndexCount_OnEmptyList_ReturnsNegativeOneWithoutThrowing) {
    List<int> lst;
    EXPECT_EQ(lst.LastIndexOf(1, -1, 0), -1);
}

TEST(GenListTests, LastIndexOf_StartIndexCount_CountPastStart_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_THROW(lst.LastIndexOf(1, 1, 5), System::ArgumentOutOfRangeException);
}

TEST(GenListTests, Insert) {
    List<int> lst;
    lst.Add(1); lst.Add(3);
    lst.Insert(1, 2);
    EXPECT_EQ(lst[1], 2);
    EXPECT_EQ(lst.getCountProperty(), 3);
}

TEST(GenListTests, RemoveAt) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    lst.RemoveAt(1);
    EXPECT_EQ(lst.getCountProperty(), 2);
    EXPECT_EQ(lst[1], 3);
}

TEST(GenListTests, GetEnumerator) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    auto* e = lst.GetEnumerator();
    int sum = 0;
    while (e->MoveNext()) sum += e->Current();
    EXPECT_EQ(sum, 6);
    delete e;
}

TEST(GenListTests, RangeForLoop) {
    List<int> lst;
    lst.Add(4); lst.Add(5); lst.Add(6);
    int sum = 0;
    for (int v : lst) sum += v;
    EXPECT_EQ(sum, 15);
}

// ---- List<int> higher-level methods ----
TEST(GenListTests, Sort) {
    List<int> lst;
    lst.Add(3); lst.Add(1); lst.Add(2);
    lst.Sort();
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[2], 3);
}

TEST(GenListTests, SortWithComparison) {
    List<int> lst;
    lst.Add(1); lst.Add(3); lst.Add(2);
    lst.Sort([](const int& a, const int& b) { return b - a; }); // descending
    EXPECT_EQ(lst[0], 3);
    EXPECT_EQ(lst[2], 1);
}

TEST(GenListTests, Reverse) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    lst.Reverse();
    EXPECT_EQ(lst[0], 3);
    EXPECT_EQ(lst[2], 1);
}

TEST(GenListTests, Find) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_EQ(lst.Find([](const int& x) { return x > 1; }), 2);
}

TEST(GenListTests, FindAll) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4);
    auto evens = lst.FindAll([](const int& x) { return x % 2 == 0; });
    EXPECT_EQ(evens.getCountProperty(), 2);
}

TEST(GenListTests, FindIndex) {
    List<int> lst;
    lst.Add(10); lst.Add(20); lst.Add(30);
    EXPECT_EQ(lst.FindIndex([](const int& x) { return x > 15; }), 1);
}

TEST(GenListTests, RemoveAll) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4);
    int removed = lst.RemoveAll([](const int& x) { return x % 2 == 0; });
    EXPECT_EQ(removed, 2);
    EXPECT_EQ(lst.getCountProperty(), 2);
}

TEST(GenListTests, ForEach) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    int sum = 0;
    lst.ForEach([&sum](const int& x) { sum += x; });
    EXPECT_EQ(sum, 6);
}

TEST(GenListTests, Exists) {
    List<int> lst;
    lst.Add(5); lst.Add(10);
    EXPECT_TRUE(lst.Exists([](const int& x) { return x == 10; }));
    EXPECT_FALSE(lst.Exists([](const int& x) { return x == 99; }));
}

TEST(GenListTests, TrueForAll) {
    List<int> lst;
    lst.Add(2); lst.Add(4); lst.Add(6);
    EXPECT_TRUE(lst.TrueForAll([](const int& x) { return x % 2 == 0; }));
    lst.Add(3);
    EXPECT_FALSE(lst.TrueForAll([](const int& x) { return x % 2 == 0; }));
}

TEST(GenListTests, BinarySearch) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4);
    EXPECT_EQ(lst.BinarySearch(3), 2);
    EXPECT_LT(lst.BinarySearch(99), 0); // bitwise complement
}

TEST(GenListTests, AddRange) {
    List<int> lst;
    lst.Add(1);
    std::vector<int> v{2, 3, 4};
    lst.AddRange(v);
    EXPECT_EQ(lst.getCountProperty(), 4);
}

TEST(GenListTests, GetRange) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4);
    auto sub = lst.GetRange(1, 2);
    EXPECT_EQ(sub.getCountProperty(), 2);
    EXPECT_EQ(sub[0], 2);
}

TEST(GenListTests, ToArray) {
    List<int> lst;
    lst.Add(7); lst.Add(8);
    auto v = lst.ToArray();
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 7);
}

TEST(GenListTests, EnsureCapacityAndTrimExcess) {
    List<int> lst;
    lst.EnsureCapacity(100);
    EXPECT_GE(lst.getCapacityProperty(), 100);
    lst.Add(1);
    lst.TrimExcess();
    EXPECT_EQ(lst.getCapacityProperty(), 1);
}

// Regression: EnsureCapacity previously returned void, but real .NET's
// List<T>.EnsureCapacity(int) returns the resulting capacity -- added to match that signature.
TEST(GenListTests, EnsureCapacity_ReturnsResultingCapacity) {
    List<int> lst;
    SharpRuntime::intcs cap = lst.EnsureCapacity(100);
    EXPECT_GE(cap, 100);
}

TEST(GenListTests, ConvertAll) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    auto strs = lst.ConvertAll<std::string>([](const int& x) { return std::to_string(x); });
    EXPECT_EQ(strs[0], "1");
    EXPECT_EQ(strs[2], "3");
}

// ---- Bounds validation (real .NET List<T> throws ArgumentOutOfRangeException/ArgumentException) ----

TEST(GenListTests, IndexerOutOfRange_Throws) {
    List<int> lst;
    lst.Add(1);
    EXPECT_THROW((void)lst[1], System::ArgumentOutOfRangeException);
    EXPECT_THROW((void)lst[-1], System::ArgumentOutOfRangeException);
}

TEST(GenListTests, ConstIndexerOutOfRange_Throws) {
    const List<int> lst;
    EXPECT_THROW((void)lst[0], System::ArgumentOutOfRangeException);
}

TEST(GenListTests, InsertOutOfRange_Throws) {
    List<int> lst;
    lst.Add(1);
    EXPECT_THROW(lst.Insert(-1, 5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(lst.Insert(2, 5), System::ArgumentOutOfRangeException);
    EXPECT_NO_THROW(lst.Insert(1, 5)); // inserting at Count (append) is valid
}

TEST(GenListTests, RemoveAtOutOfRange_Throws) {
    List<int> lst;
    lst.Add(1);
    EXPECT_THROW(lst.RemoveAt(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(lst.RemoveAt(1), System::ArgumentOutOfRangeException);
}

TEST(GenListTests, GetRangeInvalid_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_THROW(lst.GetRange(-1, 2), System::ArgumentOutOfRangeException);
    EXPECT_THROW(lst.GetRange(0, -1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(lst.GetRange(2, 5), System::ArgumentException);
}

TEST(GenListTests, RemoveRangeInvalid_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_THROW(lst.RemoveRange(2, 5), System::ArgumentException);
    lst.RemoveRange(1, 2);
    EXPECT_EQ(lst.getCountProperty(), 1);
}

TEST(GenListTests, ReverseRangeInvalid_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_THROW(lst.Reverse(2, 5), System::ArgumentException);
    lst.Reverse(0, 2);
    EXPECT_EQ(lst[0], 2);
    EXPECT_EQ(lst[1], 1);
    EXPECT_EQ(lst[2], 3);
}

TEST(GenListTests, InsertRangeOutOfRange_Throws) {
    List<int> lst;
    lst.Add(1);
    std::vector<int> v{2, 3};
    EXPECT_THROW(lst.InsertRange(-1, v), System::ArgumentOutOfRangeException);
    EXPECT_THROW(lst.InsertRange(2, v), System::ArgumentOutOfRangeException);
}

// ---- GetEnumerator() version-tracking (ticket 1713, post-stabilization-audit finding #4) ----
TEST(GenListTests, GetEnumerator_NoModification_DoesNotThrow) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    auto* e = lst.GetEnumerator();
    int count = 0;
    while (e->MoveNext()) { (void)e->Current(); count++; }
    EXPECT_EQ(count, 3);
    delete e;
}

TEST(GenListTests, GetEnumerator_AddDuringIteration_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    auto* e = lst.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    lst.Add(3);
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenListTests, GetEnumerator_RemoveDuringIteration_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    auto* e = lst.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    lst.Remove(2);
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenListTests, GetEnumerator_ClearDuringIteration_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    auto* e = lst.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    lst.Clear();
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenListTests, GetEnumerator_SortDuringIteration_Throws) {
    List<int> lst;
    lst.Add(3); lst.Add(1); lst.Add(2);
    auto* e = lst.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    lst.Sort();
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenListTests, GetEnumerator_ReverseDuringIteration_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    auto* e = lst.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    lst.Reverse();
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenListTests, GetEnumerator_InsertDuringIteration_Throws) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    auto* e = lst.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    lst.Insert(0, 99);
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenListTests, AddRange_Empty_DoesNotBumpVersion) {
    // Matches real .NET's List<T>.AddRange: adding an empty collection is a no-op that does
    // NOT bump _version, so an in-progress enumeration must not be invalidated by it.
    List<int> lst;
    lst.Add(1);
    auto* e = lst.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    std::vector<int> empty;
    lst.AddRange(empty);
    EXPECT_NO_THROW(e->MoveNext());
    delete e;
}

// ---- LinkedListNode Next/Previous ----
TEST(GenLinkedListNodeTests, NextProperty) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    auto n = ll.getFirstProperty();
    ASSERT_TRUE(static_cast<bool>(n));
    auto next = n.getNextProperty();
    EXPECT_TRUE(static_cast<bool>(next));
    EXPECT_EQ(next.getValueProperty(), 2);
}

TEST(GenLinkedListNodeTests, PreviousProperty) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    auto n = ll.getLastProperty();
    auto prev = n.getPreviousProperty();
    EXPECT_TRUE(static_cast<bool>(prev));
    EXPECT_EQ(prev.getValueProperty(), 2);
}

TEST(GenLinkedListNodeTests, NextOfLastIsNull) {
    LinkedList<int> ll;
    ll.AddLast(1);
    auto n = ll.getLastProperty();
    EXPECT_FALSE(static_cast<bool>(n.getNextProperty()));
}

TEST(GenLinkedListNodeTests, PreviousOfFirstIsNull) {
    LinkedList<int> ll;
    ll.AddLast(1);
    auto n = ll.getFirstProperty();
    EXPECT_FALSE(static_cast<bool>(n.getPreviousProperty()));
}

// ---- LinkedList CopyTo and GetEnumerator ----
TEST(GenLinkedListTests, CopyTo) {
    LinkedList<int> ll;
    ll.AddLast(10); ll.AddLast(20); ll.AddLast(30);
    std::vector<int> dest(3);
    ll.CopyTo(dest, 0);
    EXPECT_EQ(dest[0], 10);
    EXPECT_EQ(dest[2], 30);
}

TEST(GenLinkedListTests, CopyTo_TooSmall_Throws) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2);
    std::vector<int> dest(1);
    EXPECT_THROW(ll.CopyTo(dest, 0), System::ArgumentException);
}

TEST(GenLinkedListTests, CopyTo_NegativeIndex_Throws) {
    LinkedList<int> ll;
    ll.AddLast(1);
    std::vector<int> dest(1);
    EXPECT_THROW(ll.CopyTo(dest, -1), System::ArgumentOutOfRangeException);
}

TEST(GenLinkedListTests, GetEnumerator) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    auto* e = ll.GetEnumerator();
    int sum = 0;
    while (e->MoveNext()) sum += e->Current();
    EXPECT_EQ(sum, 6);
    delete e;
}

// post-stabilization-audit finding #4 / ticket 1713: GetEnumerator()'s Enumerator must detect
// structural modification during iteration and throw InvalidOperationException, matching real
// .NET's fail-fast contract.
TEST(GenLinkedListTests, GetEnumerator_NoModification_DoesNotThrow) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2);
    auto* e = ll.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    EXPECT_TRUE(e->MoveNext());
    EXPECT_FALSE(e->MoveNext());
    delete e;
}

TEST(GenLinkedListTests, GetEnumerator_AddLastDuringIteration_Throws) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2);
    auto* e = ll.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    ll.AddLast(3);
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenLinkedListTests, GetEnumerator_RemoveFirstDuringIteration_Throws) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2); ll.AddLast(3);
    auto* e = ll.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    ll.RemoveFirst();
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

TEST(GenLinkedListTests, GetEnumerator_ClearDuringIteration_Throws) {
    LinkedList<int> ll;
    ll.AddLast(1); ll.AddLast(2);
    auto* e = ll.GetEnumerator();
    EXPECT_TRUE(e->MoveNext());
    ll.Clear();
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
    delete e;
}

// ---- KeyNotFoundException ----
TEST(GenKeyNotFoundExceptionTests, DefaultMessage) {
    try { throw KeyNotFoundException(); }
    catch (const KeyNotFoundException& ex) {
        EXPECT_NE(std::string(ex.what()).find("key"), std::string::npos);
    }
}

TEST(GenKeyNotFoundExceptionTests, CustomMessage) {
    try { throw KeyNotFoundException("no such key"); }
    catch (const KeyNotFoundException& ex) {
        EXPECT_EQ(std::string(ex.what()), "no such key");
    }
}

TEST(GenKeyNotFoundExceptionTests, WithInnerException) {
    auto inner = std::make_exception_ptr(std::runtime_error("inner"));
    try { throw KeyNotFoundException("outer", inner); }
    catch (const KeyNotFoundException& ex) {
        EXPECT_EQ(std::string(ex.what()), "outer");
    }
}
