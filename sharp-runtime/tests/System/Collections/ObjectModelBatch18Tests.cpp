// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for methods added in Batch 18:
//   KeyedCollection:     TryGetValue
//   ObservableCollection: Move
//   ReadOnlyCollection:  CopyTo
//   ReadOnlyDictionary:  getIsEmptyProperty, ContainsKey, TryGetValue, Keys, Values
#include <gtest/gtest.h>
#include "System/Collections/ObjectModel/KeyedCollection.hpp"
#include "System/Collections/ObjectModel/ObservableCollection.hpp"
#include "System/Collections/ObjectModel/ReadOnlyCollection.hpp"
#include "System/Collections/ObjectModel/ReadOnlyDictionary.hpp"
#include "System/ArgumentException.hpp"
#include "System/NotSupportedException.hpp"
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using System::Collections::ObjectModel::KeyedCollection;
using System::Collections::ObjectModel::ObservableCollection;
using System::Collections::ObjectModel::NotifyCollectionChangedAction;
using System::Collections::ObjectModel::ReadOnlyCollection;
using System::Collections::ObjectModel::ReadOnlyDictionary;

// ===========================================================================
// KeyedCollection — TryGetValue
// ===========================================================================

namespace {

struct NamedItem {
    std::string name;
    int value;
    bool operator==(const NamedItem& o) const { return name == o.name && value == o.value; }
};

class NamedCollection : public KeyedCollection<std::string, NamedItem> {
protected:
    std::string GetKeyForItem(const NamedItem& item) const override { return item.name; }
};

} // namespace

TEST(KeyedCollectionBatch18Test, TryGetValue_Found) {
    NamedCollection col;
    col.Add({"alpha", 1});
    col.Add({"beta", 2});
    NamedItem out{};
    EXPECT_TRUE(col.TryGetValue("alpha", out));
    EXPECT_EQ(out.name, "alpha");
    EXPECT_EQ(out.value, 1);
}

TEST(KeyedCollectionBatch18Test, TryGetValue_NotFound) {
    NamedCollection col;
    col.Add({"alpha", 1});
    NamedItem out{};
    EXPECT_FALSE(col.TryGetValue("gamma", out));
}

TEST(KeyedCollectionBatch18Test, Contains_Key) {
    NamedCollection col;
    col.Add({"x", 10});
    EXPECT_TRUE(col.Contains("x"));
    EXPECT_FALSE(col.Contains("y"));
}

TEST(KeyedCollectionBatch18Test, Remove_ByKey) {
    NamedCollection col;
    col.Add({"a", 1});
    col.Add({"b", 2});
    EXPECT_TRUE(col.Remove("a"));
    EXPECT_EQ(col.getCountProperty(), 1);
    EXPECT_FALSE(col.Contains("a"));
}

// ===========================================================================
// ObservableCollection — Move
// ===========================================================================

TEST(ObservableCollectionBatch18Test, Move_ChangesOrder) {
    ObservableCollection<int> col({1, 2, 3, 4});
    col.Move(0, 2); // move 1 to index 2 → [2, 3, 1, 4]
    EXPECT_EQ(col[0], 2);
    EXPECT_EQ(col[1], 3);
    EXPECT_EQ(col[2], 1);
    EXPECT_EQ(col[3], 4);
}

TEST(ObservableCollectionBatch18Test, Move_FiresEvent) {
    ObservableCollection<int> col({10, 20, 30});
    bool fired = false;
    int capturedOld = -1, capturedNew = -1;
    col.CollectionChanged.push_back([&](void*, const auto& args) {
        fired = true;
        capturedOld = args.OldStartingIndex;
        capturedNew = args.NewStartingIndex;
    });
    col.Move(2, 0);
    EXPECT_TRUE(fired);
    EXPECT_EQ(capturedOld, 2);
    EXPECT_EQ(capturedNew, 0);
}

TEST(ObservableCollectionBatch18Test, Move_EventActionIsMove) {
    ObservableCollection<std::string> col({"a", "b", "c"});
    NotifyCollectionChangedAction capturedAction = NotifyCollectionChangedAction::Reset;
    col.CollectionChanged.push_back([&](void*, const auto& args) {
        capturedAction = args.Action;
    });
    col.Move(1, 0);
    EXPECT_EQ(capturedAction, NotifyCollectionChangedAction::Move);
}

TEST(ObservableCollectionBatch18Test, Move_SameIndex_NoReorder) {
    ObservableCollection<int> col({5, 10, 15});
    col.Move(1, 1);
    EXPECT_EQ(col[0], 5);
    EXPECT_EQ(col[1], 10);
    EXPECT_EQ(col[2], 15);
}

// ===========================================================================
// ReadOnlyCollection — CopyTo
// ===========================================================================

TEST(ReadOnlyCollectionBatch18Test, CopyTo_CopiesToVector) {
    std::vector<int> src = {1, 2, 3};
    ReadOnlyCollection<int> col(src);
    std::vector<int> dest(5, 0);
    col.CopyTo(dest, 1);
    EXPECT_EQ(dest[0], 0);
    EXPECT_EQ(dest[1], 1);
    EXPECT_EQ(dest[2], 2);
    EXPECT_EQ(dest[3], 3);
    EXPECT_EQ(dest[4], 0);
}

TEST(ReadOnlyCollectionBatch18Test, CopyTo_OutOfRange_Throws) {
    ReadOnlyCollection<int>(std::vector<int>{1, 2, 3});
    ReadOnlyCollection<int> col(std::vector<int>{1, 2, 3});
    std::vector<int> dest(2, 0); // too small
    EXPECT_THROW(col.CopyTo(dest, 0), System::ArgumentException);
}

// Regression for ticket 339: same overflow-prone-check fix as Collection<T>::CopyTo (see its
// test of the same name for the full rationale) -- `index + getCountProperty() >
// destination.size()` is signed-integer-overflow UB for a large index, and the wraparound could
// make the check wrongly evaluate false, skipping the bounds throw entirely.
TEST(ReadOnlyCollectionBatch18Test, CopyTo_LargeIndexPastDestSize_ThrowsInsteadOfOverflowing) {
    ReadOnlyCollection<int> col(std::vector<int>{1, 2, 3});
    std::vector<int> dest(5, 0);
    EXPECT_THROW(col.CopyTo(dest, std::numeric_limits<SharpRuntime::intcs>::max() - 2), System::ArgumentException);
}

TEST(ReadOnlyCollectionBatch18Test, CountAndIndexer) {
    const ReadOnlyCollection<std::string> col(std::vector<std::string>{"x", "y"});
    EXPECT_EQ(col.getCountProperty(), 2);
    EXPECT_EQ(col[0], "x");
    EXPECT_EQ(col[1], "y");
}

TEST(ReadOnlyCollectionBatch18Test, Contains_IndexOf) {
    ReadOnlyCollection<int> col(std::vector<int>{10, 20, 30});
    EXPECT_TRUE(col.Contains(20));
    EXPECT_FALSE(col.Contains(99));
    EXPECT_EQ(col.IndexOf(30), 2);
    EXPECT_EQ(col.IndexOf(99), -1);
}

TEST(ReadOnlyCollectionBatch18Test, MutationThrows) {
    ReadOnlyCollection<int> col(std::vector<int>{1});
    EXPECT_THROW(col.Add(2), System::NotSupportedException);
    EXPECT_THROW(col.Clear(), System::NotSupportedException);
    EXPECT_THROW(col.Remove(1), System::NotSupportedException);
}

// ===========================================================================
// ReadOnlyDictionary — getIsEmptyProperty, TryGetValue, Keys, Values
// ===========================================================================

TEST(ReadOnlyDictionaryBatch18Test, IsEmpty_True) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    ReadOnlyDictionary<std::string, int> rod(m);
    EXPECT_TRUE(rod.getIsEmptyProperty());
    EXPECT_EQ(rod.getCountProperty(), 0);
}

TEST(ReadOnlyDictionaryBatch18Test, IsEmpty_False) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["a"] = 1;
    ReadOnlyDictionary<std::string, int> rod(m);
    EXPECT_FALSE(rod.getIsEmptyProperty());
    EXPECT_EQ(rod.getCountProperty(), 1);
}

TEST(ReadOnlyDictionaryBatch18Test, ContainsKey) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["key"] = 42;
    ReadOnlyDictionary<std::string, int> rod(m);
    EXPECT_TRUE(rod.ContainsKey("key"));
    EXPECT_FALSE(rod.ContainsKey("missing"));
}

TEST(ReadOnlyDictionaryBatch18Test, Indexer_Found) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["x"] = 99;
    ReadOnlyDictionary<std::string, int> rod(m);
    EXPECT_EQ(rod["x"], 99);
}

TEST(ReadOnlyDictionaryBatch18Test, TryGetValue_FoundAndMissing) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["foo"] = 7;
    ReadOnlyDictionary<std::string, int> rod(m);
    int val = 0;
    EXPECT_TRUE(rod.TryGetValue("foo", val));
    EXPECT_EQ(val, 7);
    EXPECT_FALSE(rod.TryGetValue("bar", val));
}

TEST(ReadOnlyDictionaryBatch18Test, Keys_ContainsExpectedKey) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["a"] = 1;
    (*m)["b"] = 2;
    ReadOnlyDictionary<std::string, int> rod(m);
    auto keys = rod.getKeysProperty();
    EXPECT_EQ(keys.size(), 2u);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "a") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "b") != keys.end());
}

TEST(ReadOnlyDictionaryBatch18Test, Values_ContainsExpectedValues) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["a"] = 10;
    (*m)["b"] = 20;
    ReadOnlyDictionary<std::string, int> rod(m);
    auto vals = rod.getValuesProperty();
    EXPECT_EQ(vals.size(), 2u);
    EXPECT_TRUE(std::find(vals.begin(), vals.end(), 10) != vals.end());
    EXPECT_TRUE(std::find(vals.begin(), vals.end(), 20) != vals.end());
}
