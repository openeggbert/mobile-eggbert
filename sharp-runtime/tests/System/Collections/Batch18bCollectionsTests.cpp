// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 18b gap-fill:
//   BitArray: Clone, GetEnumerator, IsReadOnly/IsSynchronized/SyncRoot
//   ArrayList: Clone, GetRange, IndexOf overloads, LastIndexOf overloads,
//              Sort(IComparer), BinarySearch, Repeat, ICollection-ctor
#include <gtest/gtest.h>
#include <any>
#include <vector>
#include "System/Collections/BitArray.hpp"
#include "System/Collections/ArrayList.hpp"
#include "System/Collections/Comparer.hpp"

using namespace System::Collections;

// -----------------------------------------------------------------------
// BitArray — Clone
// -----------------------------------------------------------------------

TEST(BitArrayGapFill, Clone_IsIndependent) {
    BitArray orig(4, false);
    orig.Set(1, true);
    BitArray copy = orig.Clone();
    copy.Set(0, true);
    EXPECT_FALSE(orig.Get(0));  // original unaffected
    EXPECT_TRUE(copy.Get(0));
    EXPECT_TRUE(copy.Get(1));
}

TEST(BitArrayGapFill, Clone_SameLength) {
    BitArray orig(7, true);
    BitArray copy = orig.Clone();
    EXPECT_EQ(copy.getLengthProperty(), 7);
    EXPECT_TRUE(copy.HasAllSet());
}

// -----------------------------------------------------------------------
// BitArray — GetEnumerator
// -----------------------------------------------------------------------

TEST(BitArrayGapFill, GetEnumerator_IteratesAllBits) {
    BitArray ba(4, false);
    ba.Set(0, true);
    ba.Set(2, true);
    IEnumerator* e = ba.GetEnumerator();
    ASSERT_NE(e, nullptr);
    std::vector<bool> got;
    while (e->MoveNext()) {
        bool v = *static_cast<bool*>(e->getCurrentProperty());
        got.push_back(v);
    }
    delete e;
    ASSERT_EQ(got.size(), 4u);
    EXPECT_TRUE(got[0]);
    EXPECT_FALSE(got[1]);
    EXPECT_TRUE(got[2]);
    EXPECT_FALSE(got[3]);
}

TEST(BitArrayGapFill, GetEnumerator_Reset_RestartsIteration) {
    BitArray ba(3, true);
    IEnumerator* e = ba.GetEnumerator();
    ASSERT_NE(e, nullptr);
    e->MoveNext();
    e->MoveNext();
    e->Reset();
    int count = 0;
    while (e->MoveNext()) ++count;
    delete e;
    EXPECT_EQ(count, 3);
}

TEST(BitArrayGapFill, GetEnumerator_Empty_NoMoveNext) {
    BitArray ba(0);
    IEnumerator* e = ba.GetEnumerator();
    ASSERT_NE(e, nullptr);
    EXPECT_FALSE(e->MoveNext());
    delete e;
}

// -----------------------------------------------------------------------
// BitArray — properties
// -----------------------------------------------------------------------

TEST(BitArrayGapFill, IsReadOnly_False) {
    BitArray ba(4, false);
    EXPECT_FALSE(ba.getIsReadOnlyProperty());
}

TEST(BitArrayGapFill, IsSynchronized_False) {
    BitArray ba(4, false);
    EXPECT_FALSE(ba.getIsSynchronizedProperty());
}

TEST(BitArrayGapFill, SyncRoot_NonNull_Stable) {
    BitArray ba(4, false);
    EXPECT_NE(ba.getSyncRootProperty(), nullptr);
    EXPECT_EQ(ba.getSyncRootProperty(), ba.getSyncRootProperty());
}

// -----------------------------------------------------------------------
// ArrayList — Clone
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, Clone_IsShallowCopy) {
    ArrayList al;
    al.Add(std::any(42));
    al.Add(std::any(99));
    ArrayList copy = al.Clone();
    EXPECT_EQ(copy.getCountProperty(), 2);
    al.Add(std::any(1));
    EXPECT_EQ(copy.getCountProperty(), 2);  // independent
}

// -----------------------------------------------------------------------
// ArrayList — GetRange
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, GetRange_ReturnsSubset) {
    ArrayList al;
    al.Add(std::any(10));
    al.Add(std::any(20));
    al.Add(std::any(30));
    al.Add(std::any(40));
    ArrayList sub = al.GetRange(1, 2);
    EXPECT_EQ(sub.getCountProperty(), 2);
}

TEST(ArrayListGapFill, GetRange_Empty) {
    ArrayList al;
    al.Add(std::any(1));
    ArrayList sub = al.GetRange(0, 0);
    EXPECT_EQ(sub.getCountProperty(), 0);
}

// -----------------------------------------------------------------------
// ArrayList — IndexOf overloads
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, IndexOf_StartIndex_FindsFromOffset) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(1));
    // value 1 occurs at index 0 and 2; searching from 1 should skip index 0 and find index 2
    EXPECT_EQ(al.IndexOf(std::any(1), 1), 2);
}

TEST(ArrayListGapFill, IndexOf_StartIndex_NotFound) {
    ArrayList al;
    al.Add(std::any(std::string("x")));
    EXPECT_EQ(al.IndexOf(std::any(42), 0), -1);
}

TEST(ArrayListGapFill, IndexOf_StartIndex_Count) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(3));
    // search 1 element starting at index 2 — value 3 is there
    EXPECT_EQ(al.IndexOf(std::any(3), 2, 1), 2);
    // search 1 element starting at index 0 — value 3 is out of the searched window
    EXPECT_EQ(al.IndexOf(std::any(3), 0, 1), -1);
}

TEST(ArrayListGapFill, IndexOf_ComparesByValueNotJustType) {
    // Regression test: an earlier version compared only std::any::type(), so searching a list
    // of ints for a value not present would incorrectly "find" the first int of any value.
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(3));
    EXPECT_EQ(al.IndexOf(std::any(2)), 1);
    EXPECT_EQ(al.IndexOf(std::any(99)), -1);
    EXPECT_TRUE(al.Contains(std::any(3)));
    EXPECT_FALSE(al.Contains(std::any(42)));
}

// -----------------------------------------------------------------------
// ArrayList — LastIndexOf overloads
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, LastIndexOf_Basic) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(1));
    // value 1 occurs at index 0 and 2; LastIndexOf finds the last occurrence, index 2
    EXPECT_EQ(al.LastIndexOf(std::any(1)), 2);
}

TEST(ArrayListGapFill, LastIndexOf_StartIndex) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(1));
    // search backwards from index 1 — value 1 not in [0,1] search window except index 0
    EXPECT_EQ(al.LastIndexOf(std::any(1), 1), 0);
}

TEST(ArrayListGapFill, LastIndexOf_StartIndex_Count) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(3));
    // search 2 elements ending at index 2 (indices 1,2): value 3 is at index 2
    EXPECT_EQ(al.LastIndexOf(std::any(3), 2, 2), 2);
    // search 1 element ending at index 0: value 3 is not there
    EXPECT_EQ(al.LastIndexOf(std::any(3), 0, 1), -1);
}

// -----------------------------------------------------------------------
// ArrayList — Sort(IComparer)
// -----------------------------------------------------------------------

// Comparer that compares std::any* containing int values
struct IntAnyComparer : public IComparer {
    int Compare(const void* x, const void* y) const override {
        int a = std::any_cast<int>(*static_cast<const std::any*>(x));
        int b = std::any_cast<int>(*static_cast<const std::any*>(y));
        return (a < b) ? -1 : (a > b) ? 1 : 0;
    }
};

TEST(ArrayListGapFill, Sort_FullList) {
    ArrayList al;
    al.Add(std::any(3));
    al.Add(std::any(1));
    al.Add(std::any(2));
    IntAnyComparer cmp;
    al.Sort(cmp);
    EXPECT_EQ(std::any_cast<int>(al[0]), 1);
    EXPECT_EQ(std::any_cast<int>(al[1]), 2);
    EXPECT_EQ(std::any_cast<int>(al[2]), 3);
}

TEST(ArrayListGapFill, Sort_Range) {
    ArrayList al;
    al.Add(std::any(10));
    al.Add(std::any(3));
    al.Add(std::any(1));
    al.Add(std::any(2));
    IntAnyComparer cmp;
    // sort indices 1..3
    al.Sort(1, 3, cmp);
    EXPECT_EQ(std::any_cast<int>(al[0]), 10); // unchanged
    EXPECT_EQ(std::any_cast<int>(al[1]), 1);
    EXPECT_EQ(std::any_cast<int>(al[2]), 2);
    EXPECT_EQ(std::any_cast<int>(al[3]), 3);
}

// -----------------------------------------------------------------------
// ArrayList — BinarySearch
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, BinarySearch_Found) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(3));
    al.Add(std::any(4));
    IntAnyComparer cmp;
    int idx = al.BinarySearch(std::any(3), cmp);
    EXPECT_EQ(idx, 2);
}

TEST(ArrayListGapFill, BinarySearch_NotFound_ReturnsComplement) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(3));
    al.Add(std::any(5));
    IntAnyComparer cmp;
    int idx = al.BinarySearch(std::any(4), cmp);
    EXPECT_LT(idx, 0);  // not found → negative (bitwise complement)
    EXPECT_EQ(~idx, 2); // insertion point is 2
}

TEST(ArrayListGapFill, BinarySearch_Range) {
    ArrayList al;
    al.Add(std::any(10));
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(3));
    IntAnyComparer cmp;
    int idx = al.BinarySearch(1, 3, std::any(2), cmp);
    EXPECT_EQ(idx, 2);
}

// -----------------------------------------------------------------------
// ArrayList — Repeat
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, Repeat_CreatesNcopies) {
    ArrayList al = ArrayList::Repeat(std::any(42), 5);
    EXPECT_EQ(al.getCountProperty(), 5);
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(std::any_cast<int>(al[i]), 42);
}

TEST(ArrayListGapFill, Repeat_ZeroCount) {
    ArrayList al = ArrayList::Repeat(std::any(1), 0);
    EXPECT_EQ(al.getCountProperty(), 0);
}

// -----------------------------------------------------------------------
// ArrayList — bounds validation (ticket 255: these methods previously did zero validation,
// so an out-of-range index/count reached std::vector iterator arithmetic directly -- undefined
// behavior, not a clean exception like real ArrayList.cs raises).
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, Constructor_NegativeCapacity_Throws) {
    EXPECT_THROW(ArrayList al(-1), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, SetCapacity_BelowCount_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    EXPECT_THROW(al.setCapacityProperty(1), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, Insert_NegativeIndex_Throws) {
    ArrayList al;
    EXPECT_THROW(al.Insert(-1, std::any(1)), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, Insert_IndexPastEnd_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.Insert(2, std::any(2)), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, Insert_IndexEqualsCount_Legal) {
    // Inserting exactly at the end (index == count) is legal, matching real .NET.
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_NO_THROW(al.Insert(1, std::any(2)));
    EXPECT_EQ(al.getCountProperty(), 2);
}

TEST(ArrayListGapFill, InsertRange_IndexPastEnd_Throws) {
    ArrayList al;
    std::vector<std::any> items{std::any(1)};
    EXPECT_THROW(al.InsertRange(5, items), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, RemoveRange_NegativeIndex_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.RemoveRange(-1, 1), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, RemoveRange_PastEnd_ThrowsArgumentException) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.RemoveRange(0, 5), System::ArgumentException);
}

TEST(ArrayListGapFill, GetRange_NegativeCount_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.GetRange(0, -1), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, GetRange_PastEnd_ThrowsArgumentException) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.GetRange(0, 5), System::ArgumentException);
}

TEST(ArrayListGapFill, Reverse_PastEnd_ThrowsArgumentException) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.Reverse(0, 5), System::ArgumentException);
}

TEST(ArrayListGapFill, Sort_Range_PastEnd_ThrowsArgumentException) {
    ArrayList al;
    al.Add(std::any(1));
    IntAnyComparer cmp;
    EXPECT_THROW(al.Sort(0, 5, cmp), System::ArgumentException);
}

TEST(ArrayListGapFill, BinarySearch_Range_PastEnd_ThrowsArgumentException) {
    ArrayList al;
    al.Add(std::any(1));
    IntAnyComparer cmp;
    EXPECT_THROW(al.BinarySearch(0, 5, std::any(1), cmp), System::ArgumentException);
}

TEST(ArrayListGapFill, SetRange_PastEnd_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    std::vector<std::any> items{std::any(1), std::any(2), std::any(3)};
    EXPECT_THROW(al.SetRange(0, items), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, IndexOf_StartIndexPastEnd_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.IndexOf(std::any(0), 5), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, IndexOf_StartIndexCount_NegativeCount_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.IndexOf(std::any(0), 0, -1), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, LastIndexOf_StartIndexPastEnd_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.LastIndexOf(std::any(0), 5), System::ArgumentOutOfRangeException);
}

TEST(ArrayListGapFill, LastIndexOf_OnEmptyList_ReturnsNegativeOneWithoutThrowing) {
    // Matches .NET's own special case: LastIndexOf(value) on an empty list delegates to
    // LastIndexOf(value, -1, 0), which skips the negative-startIndex validation entirely
    // when Count == 0 and returns -1 directly.
    ArrayList al;
    EXPECT_EQ(al.LastIndexOf(std::any(0)), -1);
}

TEST(ArrayListGapFill, Repeat_NegativeCount_Throws) {
    EXPECT_THROW(ArrayList::Repeat(std::any(1), -1), System::ArgumentOutOfRangeException);
}

// -----------------------------------------------------------------------
// ArrayList — ICollection constructor
// -----------------------------------------------------------------------

// Regression test for a wave-3 audit finding: ArrayList::GetEnumerator() unconditionally
// returned nullptr, so this ICollection-copying constructor's `if (e) { ... }` guard always
// skipped the copy loop entirely -- silently producing an empty ArrayList regardless of the
// source's contents, instead of either a working enumerator or a clear failure. Now that
// GetEnumerator() returns a real enumerator, the constructor actually copies elements.
TEST(ArrayListGapFill, ICollectionCtor_CopiesElements) {
    ArrayList src;
    src.Add(std::any(7));
    ArrayList dst(static_cast<ICollection&>(src));
    EXPECT_EQ(dst.getCountProperty(), 1);
}

// -----------------------------------------------------------------------
// ArrayList — GetEnumerator
// -----------------------------------------------------------------------

TEST(ArrayListGapFill, GetEnumerator_IteratesAllElementsInOrder) {
    ArrayList al;
    al.Add(std::any(10));
    al.Add(std::any(20));
    al.Add(std::any(30));

    std::unique_ptr<IEnumerator> e(al.GetEnumerator());
    ASSERT_NE(e, nullptr);
    std::vector<int> seen;
    while (e->MoveNext())
        seen.push_back(std::any_cast<int>(*static_cast<std::any*>(e->getCurrentProperty())));
    EXPECT_EQ(seen, (std::vector<int>{10, 20, 30}));
    EXPECT_FALSE(e->MoveNext());
}

TEST(ArrayListGapFill, GetEnumerator_EmptyList_MoveNextFalseImmediately) {
    ArrayList al;
    std::unique_ptr<IEnumerator> e(al.GetEnumerator());
    ASSERT_NE(e, nullptr);
    EXPECT_FALSE(e->MoveNext());
}

TEST(ArrayListGapFill, GetEnumerator_GetCurrent_BeforeMoveNext_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    std::unique_ptr<IEnumerator> e(al.GetEnumerator());
    EXPECT_THROW(e->getCurrentProperty(), System::InvalidOperationException);
}

TEST(ArrayListGapFill, GetEnumerator_GetCurrent_AfterExhaustion_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    std::unique_ptr<IEnumerator> e(al.GetEnumerator());
    e->MoveNext();
    EXPECT_FALSE(e->MoveNext());
    EXPECT_THROW(e->getCurrentProperty(), System::InvalidOperationException);
}

TEST(ArrayListGapFill, GetEnumerator_Reset_AllowsReiteration) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    std::unique_ptr<IEnumerator> e(al.GetEnumerator());
    e->MoveNext();
    e->MoveNext();
    e->Reset();
    ASSERT_TRUE(e->MoveNext());
    EXPECT_EQ(std::any_cast<int>(*static_cast<std::any*>(e->getCurrentProperty())), 1);
}

// Regression test: real .NET's ArrayList enumerator is fail-fast -- a structural
// modification (Add/Insert/Remove/Clear/...) after the enumerator is created invalidates it,
// throwing InvalidOperationException on the next MoveNext()/Reset()/getCurrentProperty() call.
TEST(ArrayListGapFill, GetEnumerator_ModifiedDuringEnumeration_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    std::unique_ptr<IEnumerator> e(al.GetEnumerator());
    e->MoveNext();
    al.Add(std::any(3));
    EXPECT_THROW(e->MoveNext(), System::InvalidOperationException);
}

TEST(ArrayListGapFill, GetEnumerator_TwoArgOverload_IteratesOnlyTheRequestedRange) {
    ArrayList al;
    al.Add(std::any(1));
    al.Add(std::any(2));
    al.Add(std::any(3));
    al.Add(std::any(4));

    std::unique_ptr<IEnumerator> e(al.GetEnumerator(1, 2));
    ASSERT_NE(e, nullptr);
    std::vector<int> seen;
    while (e->MoveNext())
        seen.push_back(std::any_cast<int>(*static_cast<std::any*>(e->getCurrentProperty())));
    EXPECT_EQ(seen, (std::vector<int>{2, 3}));
}

TEST(ArrayListGapFill, GetEnumerator_TwoArgOverload_OutOfBoundsRange_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.GetEnumerator(0, 5), System::ArgumentException);
    EXPECT_THROW(al.GetEnumerator(-1, 1), System::ArgumentOutOfRangeException);
}

// Regression test for a wave-3 audit finding: RemoveAt() threw std::out_of_range (an unrelated
// std:: exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's ArrayList.RemoveAt throws
// for an out-of-range index.
TEST(ArrayListGapFill, RemoveAt_OutOfRange_Throws) {
    ArrayList al;
    al.Add(std::any(1));
    EXPECT_THROW(al.RemoveAt(5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(al.RemoveAt(-1), System::ArgumentOutOfRangeException);
}
