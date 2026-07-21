// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/Collections/ObjectModel/Collection.hpp"
#include "System/Collections/ObjectModel/ReadOnlyCollection.hpp"
#include "System/Collections/ObjectModel/ReadOnlyDictionary.hpp"
#include "System/Collections/ObjectModel/ObservableCollection.hpp"
#include "System/NotSupportedException.hpp"

using SharpRuntime::intcs;
using System::Collections::ObjectModel::Collection;
using System::Collections::ObjectModel::ReadOnlyCollection;
using System::Collections::ObjectModel::ReadOnlyDictionary;
using System::Collections::ObjectModel::ObservableCollection;
using System::Collections::ObjectModel::NotifyCollectionChangedAction;
using System::Collections::ObjectModel::NotifyCollectionChangedEventArgs;

// ===========================================================================
// Collection<T>
// ===========================================================================

TEST(CollectionTests, DefaultCtor_IsEmpty) {
    Collection<int> c;
    EXPECT_EQ(c.getCountProperty(), 0);
    EXPECT_FALSE(c.getIsReadOnlyProperty());
}

TEST(CollectionTests, Add_IncreasesCount) {
    Collection<int> c;
    c.Add(10);
    c.Add(20);
    EXPECT_EQ(c.getCountProperty(), 2);
}

TEST(CollectionTests, Contains_True_False) {
    Collection<std::string> c;
    c.Add("hello");
    EXPECT_TRUE(c.Contains("hello"));
    EXPECT_FALSE(c.Contains("world"));
}

TEST(CollectionTests, IndexOf_Found) {
    Collection<int> c;
    c.Add(1);
    c.Add(2);
    c.Add(3);
    EXPECT_EQ(c.IndexOf(2), 1);
}

TEST(CollectionTests, IndexOf_NotFound_ReturnsMinusOne) {
    Collection<int> c;
    c.Add(1);
    EXPECT_EQ(c.IndexOf(99), -1);
}

TEST(CollectionTests, OperatorBracket_AccessByIndex) {
    Collection<int> c;
    c.Add(7);
    c.Add(8);
    EXPECT_EQ(c[0], 7);
    EXPECT_EQ(c[1], 8);
}

TEST(CollectionTests, Remove_RemovesFirstMatch) {
    Collection<int> c;
    c.Add(5);
    c.Add(10);
    EXPECT_TRUE(c.Remove(5));
    EXPECT_EQ(c.getCountProperty(), 1);
    EXPECT_EQ(c[0], 10);
}

TEST(CollectionTests, Remove_NotFound_ReturnsFalse) {
    Collection<int> c;
    c.Add(1);
    EXPECT_FALSE(c.Remove(99));
}

TEST(CollectionTests, Clear_EmptiesCollection) {
    Collection<int> c;
    c.Add(1);
    c.Add(2);
    c.Clear();
    EXPECT_EQ(c.getCountProperty(), 0);
}

TEST(CollectionTests, Insert_AtIndex) {
    Collection<int> c;
    c.Add(1);
    c.Add(3);
    c.Insert(1, 2);
    EXPECT_EQ(c.getCountProperty(), 3);
    EXPECT_EQ(c[1], 2);
}

TEST(CollectionTests, RemoveAt_RemovesByIndex) {
    Collection<int> c;
    c.Add(10);
    c.Add(20);
    c.Add(30);
    c.RemoveAt(1);
    EXPECT_EQ(c.getCountProperty(), 2);
    EXPECT_EQ(c[1], 30);
}

TEST(CollectionTests, CopyTo_CopiesToVector) {
    Collection<int> c;
    c.Add(1); c.Add(2); c.Add(3);
    std::vector<int> dest(5, 0);
    c.CopyTo(dest, 1);
    EXPECT_EQ(dest[0], 0);
    EXPECT_EQ(dest[1], 1);
    EXPECT_EQ(dest[2], 2);
    EXPECT_EQ(dest[3], 3);
    EXPECT_EQ(dest[4], 0);
}

TEST(CollectionTests, CopyTo_NegativeIndex_Throws) {
    Collection<int> c;
    c.Add(1);
    std::vector<int> dest(5, 0);
    EXPECT_THROW(c.CopyTo(dest, -1), System::ArgumentOutOfRangeException);
}

TEST(CollectionTests, CopyTo_DestTooSmall_Throws) {
    Collection<int> c;
    c.Add(1); c.Add(2); c.Add(3);
    std::vector<int> dest(2, 0);
    EXPECT_THROW(c.CopyTo(dest, 0), System::ArgumentException);
}

// Regression for ticket 339: the bounds check `index + getCountProperty() >
// destination.size()` is signed-integer-overflow UB for a large index (confirmed via UBSan) --
// and worse than UB in the abstract: the wraparound can make the check evaluate false when it
// should be true, silently skipping the throw and letting the copy loop write out of bounds.
// Rewritten as a subtraction-based check (matching List<T>.CopyTo's own `array.Length -
// arrayIndex < Count`), which cannot overflow since index and destination.size() are both
// already known non-negative at that point.
TEST(CollectionTests, CopyTo_LargeIndexPastDestSize_ThrowsInsteadOfOverflowing) {
    Collection<int> c;
    c.Add(1); c.Add(2); c.Add(3);
    std::vector<int> dest(5, 0);
    EXPECT_THROW(c.CopyTo(dest, std::numeric_limits<intcs>::max() - 2), System::ArgumentException);
}

// ===========================================================================
// ReadOnlyCollection<T>
// ===========================================================================

TEST(ReadOnlyCollectionTests, Constructor_FromVector) {
    ReadOnlyCollection<int> rc(std::vector<int>{1, 2, 3});
    EXPECT_EQ(rc.getCountProperty(), 3);
    EXPECT_TRUE(rc.getIsReadOnlyProperty());
}

TEST(ReadOnlyCollectionTests, OperatorBracket_ReadAccess) {
    const ReadOnlyCollection<std::string> rc(std::vector<std::string>{"a", "b", "c"});
    EXPECT_EQ(rc[0], "a");
    EXPECT_EQ(rc[2], "c");
}

TEST(ReadOnlyCollectionTests, Contains_True_False) {
    ReadOnlyCollection<int> rc(std::vector<int>{10, 20, 30});
    EXPECT_TRUE(rc.Contains(20));
    EXPECT_FALSE(rc.Contains(99));
}

TEST(ReadOnlyCollectionTests, IndexOf_Found) {
    ReadOnlyCollection<int> rc(std::vector<int>{5, 10, 15});
    EXPECT_EQ(rc.IndexOf(10), 1);
    EXPECT_EQ(rc.IndexOf(99), -1);
}

TEST(ReadOnlyCollectionTests, Add_Throws) {
    ReadOnlyCollection<int> rc(std::vector<int>{1});
    EXPECT_THROW(rc.Add(2), System::NotSupportedException);
}

TEST(ReadOnlyCollectionTests, Clear_Throws) {
    ReadOnlyCollection<int> rc(std::vector<int>{1});
    EXPECT_THROW(rc.Clear(), System::NotSupportedException);
}

TEST(ReadOnlyCollectionTests, Remove_Throws) {
    ReadOnlyCollection<int> rc(std::vector<int>{1});
    EXPECT_THROW(rc.Remove(1), System::NotSupportedException);
}

TEST(ReadOnlyCollectionTests, DefaultCtor_CountZero) {
    ReadOnlyCollection<int> rc;
    EXPECT_EQ(rc.getCountProperty(), 0);
}

TEST(ReadOnlyCollectionTests, SharedPtrCtor_IsLiveView_ReflectsMutationsToSource) {
    auto source = std::make_shared<std::vector<int>>(std::vector<int>{1, 2, 3});
    const ReadOnlyCollection<int> rc(source);
    EXPECT_EQ(rc.getCountProperty(), 3);
    source->push_back(4);
    EXPECT_EQ(rc.getCountProperty(), 4);
    EXPECT_EQ(rc[3], 4);
    (*source)[0] = 99;
    EXPECT_EQ(rc[0], 99);
}

TEST(ReadOnlyCollectionTests, SharedPtrCtor_NullSource_ThrowsArgumentNullException) {
    std::shared_ptr<std::vector<int>> null_source;
    EXPECT_THROW(ReadOnlyCollection<int> rc(null_source), System::ArgumentNullException);
}

TEST(ReadOnlyCollectionTests, VectorCtor_IsCopy_DoesNotReflectMutationsToSource) {
    std::vector<int> source{1, 2, 3};
    ReadOnlyCollection<int> rc(source);
    source.push_back(4);
    EXPECT_EQ(rc.getCountProperty(), 3);
}

// ===========================================================================
// ReadOnlyDictionary<K,V>
// ===========================================================================

TEST(ReadOnlyDictionaryTests, Constructor_FromSharedMap) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["a"] = 1;
    (*m)["b"] = 2;
    ReadOnlyDictionary<std::string, int> rd(m);
    EXPECT_EQ(rd.getCountProperty(), 2);
}

TEST(ReadOnlyDictionaryTests, ContainsKey_True_False) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["x"] = 10;
    ReadOnlyDictionary<std::string, int> rd(m);
    EXPECT_TRUE(rd.ContainsKey("x"));
    EXPECT_FALSE(rd.ContainsKey("y"));
}

TEST(ReadOnlyDictionaryTests, OperatorBracket_ReturnsValue) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["key"] = 42;
    ReadOnlyDictionary<std::string, int> rd(m);
    EXPECT_EQ(rd["key"], 42);
}

TEST(ReadOnlyDictionaryTests, TryGetValue_Found) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["k"] = 99;
    ReadOnlyDictionary<std::string, int> rd(m);
    int v = 0;
    EXPECT_TRUE(rd.TryGetValue("k", v));
    EXPECT_EQ(v, 99);
}

TEST(ReadOnlyDictionaryTests, TryGetValue_NotFound_ReturnsFalse) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    ReadOnlyDictionary<std::string, int> rd(m);
    int v = 0;
    EXPECT_FALSE(rd.TryGetValue("missing", v));
}

TEST(ReadOnlyDictionaryTests, Keys_HasAllKeys) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["a"] = 1;
    (*m)["b"] = 2;
    ReadOnlyDictionary<std::string, int> rd(m);
    auto keys = rd.getKeysProperty();
    EXPECT_EQ(keys.size(), 2u);
}

TEST(ReadOnlyDictionaryTests, Values_HasAllValues) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["a"] = 10;
    ReadOnlyDictionary<std::string, int> rd(m);
    auto vals = rd.getValuesProperty();
    EXPECT_EQ(vals.size(), 1u);
    EXPECT_EQ(vals[0], 10);
}

TEST(ReadOnlyDictionaryTests, OperatorBracket_Missing_ThrowsKeyNotFoundException) {
    auto m = std::make_shared<std::unordered_map<std::string, int>>();
    (*m)["key"] = 42;
    ReadOnlyDictionary<std::string, int> rd(m);
    EXPECT_THROW(rd["missing"], System::Collections::Generic::KeyNotFoundException);
}

TEST(ReadOnlyDictionaryTests, NullDictionary_ThrowsArgumentNullException) {
    EXPECT_THROW(
        (ReadOnlyDictionary<std::string, int>(std::shared_ptr<std::unordered_map<std::string, int>>())),
        System::ArgumentNullException);
}

TEST(ReadOnlyDictionaryTests, Empty_IsEmptyAndCached) {
    auto& empty1 = ReadOnlyDictionary<std::string, int>::Empty();
    auto& empty2 = ReadOnlyDictionary<std::string, int>::Empty();
    EXPECT_EQ(empty1.getCountProperty(), 0);
    EXPECT_EQ(&empty1, &empty2);
}

// ===========================================================================
// NotifyCollectionChangedAction enum
// ===========================================================================

TEST(NotifyCollectionChangedActionTests, Add_IsZero) {
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Add), 0);
}

TEST(NotifyCollectionChangedActionTests, Remove_IsOne) {
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Remove), 1);
}

TEST(NotifyCollectionChangedActionTests, Reset_IsFour) {
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Reset), 4);
}

// ===========================================================================
// ObservableCollection<T>
// ===========================================================================

TEST(ObservableCollectionTests, DefaultCtor_IsEmpty) {
    ObservableCollection<int> oc;
    EXPECT_EQ(oc.getCountProperty(), 0);
}

TEST(ObservableCollectionTests, VectorCtor_InitializesItems) {
    ObservableCollection<int> oc(std::vector<int>{1, 2, 3});
    EXPECT_EQ(oc.getCountProperty(), 3);
}

TEST(ObservableCollectionTests, Add_TriggersCollectionChanged) {
    ObservableCollection<int> oc;
    int eventCount = 0;
    NotifyCollectionChangedAction lastAction = NotifyCollectionChangedAction::Reset;
    oc.CollectionChanged.push_back([&](void*, const NotifyCollectionChangedEventArgs<int>& args) {
        ++eventCount;
        lastAction = args.Action;
    });
    oc.Add(42);
    EXPECT_EQ(eventCount, 1);
    EXPECT_EQ(lastAction, NotifyCollectionChangedAction::Add);
}

TEST(ObservableCollectionTests, Remove_TriggersCollectionChanged) {
    ObservableCollection<int> oc(std::vector<int>{1, 2, 3});
    NotifyCollectionChangedAction lastAction = NotifyCollectionChangedAction::Add;
    oc.CollectionChanged.push_back([&](void*, const NotifyCollectionChangedEventArgs<int>& args) {
        lastAction = args.Action;
    });
    oc.Remove(2);
    EXPECT_EQ(lastAction, NotifyCollectionChangedAction::Remove);
    EXPECT_EQ(oc.getCountProperty(), 2);
}

TEST(ObservableCollectionTests, Clear_TriggersResetEvent) {
    ObservableCollection<int> oc(std::vector<int>{1, 2});
    NotifyCollectionChangedAction lastAction = NotifyCollectionChangedAction::Add;
    oc.CollectionChanged.push_back([&](void*, const NotifyCollectionChangedEventArgs<int>& args) {
        lastAction = args.Action;
    });
    oc.Clear();
    EXPECT_EQ(lastAction, NotifyCollectionChangedAction::Reset);
    EXPECT_EQ(oc.getCountProperty(), 0);
}

TEST(ObservableCollectionTests, NoHandlers_AddDoesNotThrow) {
    ObservableCollection<int> oc;
    EXPECT_NO_THROW(oc.Add(1));
}

TEST(ObservableCollectionTests, VectorCtor_DoesNotFireEvents) {
    // .NET's collection-copying constructors populate the base list directly with no
    // CollectionChanged/PropertyChanged notifications.
    bool collectionFired = false, propertyFired = false;
    ObservableCollection<int> oc(std::vector<int>{1, 2, 3});
    oc.CollectionChanged.push_back([&](void*, const auto&) { collectionFired = true; });
    oc.PropertyChanged.push_back([&](void*, const auto&) { propertyFired = true; });
    EXPECT_EQ(oc.getCountProperty(), 3);
    EXPECT_FALSE(collectionFired);
    EXPECT_FALSE(propertyFired);
}

TEST(ObservableCollectionTests, Insert_TriggersCollectionChanged) {
    // Previously Insert()/RemoveAt() silently bypassed CollectionChanged because only the
    // public Add/Remove/Clear were overridden instead of the protected InsertItem/RemoveItem hooks.
    ObservableCollection<int> oc(std::vector<int>{1, 3});
    NotifyCollectionChangedAction lastAction = NotifyCollectionChangedAction::Reset;
    intcs lastIndex = -1;
    oc.CollectionChanged.push_back([&](void*, const NotifyCollectionChangedEventArgs<int>& args) {
        lastAction = args.Action;
        lastIndex = args.NewStartingIndex;
    });
    oc.Insert(1, 2);
    EXPECT_EQ(lastAction, NotifyCollectionChangedAction::Add);
    EXPECT_EQ(lastIndex, 1);
    EXPECT_EQ(oc[0], 1);
    EXPECT_EQ(oc[1], 2);
    EXPECT_EQ(oc[2], 3);
}

TEST(ObservableCollectionTests, RemoveAt_TriggersCollectionChanged) {
    ObservableCollection<int> oc(std::vector<int>{1, 2, 3});
    NotifyCollectionChangedAction lastAction = NotifyCollectionChangedAction::Add;
    oc.CollectionChanged.push_back([&](void*, const NotifyCollectionChangedEventArgs<int>& args) {
        lastAction = args.Action;
    });
    oc.RemoveAt(1);
    EXPECT_EQ(lastAction, NotifyCollectionChangedAction::Remove);
    EXPECT_EQ(oc.getCountProperty(), 2);
    EXPECT_EQ(oc[1], 3);
}

TEST(ObservableCollectionTests, Add_And_Remove_FirePropertyChanged) {
    ObservableCollection<int> oc;
    std::vector<std::string> names;
    oc.PropertyChanged.push_back([&](void*, const auto& args) { names.push_back(args.PropertyName); });
    oc.Add(1);
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "Count");
    EXPECT_EQ(names[1], "Item[]");
    names.clear();
    oc.Remove(1);
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "Count");
    EXPECT_EQ(names[1], "Item[]");
}

TEST(ObservableCollectionTests, Move_OutOfRange_Throws) {
    ObservableCollection<int> oc(std::vector<int>{1, 2, 3});
    EXPECT_THROW(oc.Move(0, 5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(oc.Move(5, 0), System::ArgumentOutOfRangeException);
}

namespace {
    // Re-exposes the protected SetItem hook for direct testing.
    template<typename T>
    class ExposedObservableCollection : public ObservableCollection<T> {
    public:
        using ObservableCollection<T>::ObservableCollection;
        using ObservableCollection<T>::SetItem;
    };
}

TEST(ObservableCollectionTests, SetItem_TriggersReplaceEvent) {
    ExposedObservableCollection<int> oc(std::vector<int>{1, 2, 3});
    NotifyCollectionChangedAction lastAction = NotifyCollectionChangedAction::Add;
    int oldItem = -1, newItem = -1;
    oc.CollectionChanged.push_back([&](void*, const NotifyCollectionChangedEventArgs<int>& args) {
        lastAction = args.Action;
        oldItem = args.OldItems.at(0);
        newItem = args.NewItems.at(0);
    });
    oc.SetItem(1, 99);
    EXPECT_EQ(lastAction, NotifyCollectionChangedAction::Replace);
    EXPECT_EQ(oldItem, 2);
    EXPECT_EQ(newItem, 99);
    EXPECT_EQ(oc[1], 99);
}

TEST(ObservableCollectionTests, Reentrancy_MultipleSubscribers_Throws) {
    ObservableCollection<int> oc(std::vector<int>{1, 2});
    oc.CollectionChanged.push_back([&](void*, const auto&) {
        EXPECT_THROW(oc.Add(99), System::InvalidOperationException);
    });
    oc.CollectionChanged.push_back([](void*, const auto&) {});
    oc.Add(3);
}

TEST(ObservableCollectionTests, Reentrancy_SingleSubscriber_Allowed) {
    // .NET's documented compatibility carve-out: a single CollectionChanged subscriber may
    // reentrantly mutate the collection from within its own handler.
    ObservableCollection<int> oc(std::vector<int>{1});
    bool reentrantAddSucceeded = false;
    oc.CollectionChanged.push_back([&](void*, const NotifyCollectionChangedEventArgs<int>& args) {
        if (args.Action == NotifyCollectionChangedAction::Add && args.NewItems.at(0) == 2) {
            oc.Add(3);
            reentrantAddSucceeded = true;
        }
    });
    oc.Add(2);
    EXPECT_TRUE(reentrantAddSucceeded);
    EXPECT_EQ(oc.getCountProperty(), 3);
}
