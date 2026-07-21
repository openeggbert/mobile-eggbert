// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/ICollection.hpp"
#include "System/Collections/IComparer.hpp"
#include "System/Collections/IDictionary.hpp"
#include "System/Collections/IDictionaryEnumerator.hpp"
#include "System/Collections/IEnumerable.hpp"
#include "System/Collections/DictionaryEntry.hpp"
#include "System/Collections/Hashtable.hpp"
#include <any>
#include <vector>

using namespace System::Collections;

// -----------------------------------------------------------------------
// Minimal concrete implementations for testing interfaces
// -----------------------------------------------------------------------

class MinimalCollection : public ICollection {
    std::vector<int> data_;
public:
    explicit MinimalCollection(std::vector<int> d) : data_(std::move(d)) {}
    int getCountProperty() const override { return static_cast<int>(data_.size()); }
    void CopyTo(void* array, int index) override {
        auto* dest = static_cast<int*>(array);
        for (size_t i = 0; i < data_.size(); ++i) dest[index + i] = data_[i];
    }
    IEnumerator* GetEnumerator() override { return nullptr; }
};

class IntComparer : public IComparer {
public:
    int Compare(const void* x, const void* y) const override {
        int a = *static_cast<const int*>(x);
        int b = *static_cast<const int*>(y);
        return (a < b) ? -1 : (a > b) ? 1 : 0;
    }
};

// -----------------------------------------------------------------------
// ICollection tests
// -----------------------------------------------------------------------

TEST(ICollectionTest, CountReturnsSize) {
    MinimalCollection c({1, 2, 3});
    EXPECT_EQ(c.getCountProperty(), 3);
}

TEST(ICollectionTest, CopyToFillsBuffer) {
    MinimalCollection c({10, 20, 30});
    int buf[5] = {};
    c.CopyTo(buf, 1);
    EXPECT_EQ(buf[1], 10);
    EXPECT_EQ(buf[3], 30);
}

TEST(ICollectionTest, IsSynchronizedDefault) {
    MinimalCollection c({});
    EXPECT_FALSE(c.getIsSynchronizedProperty());
}

TEST(ICollectionTest, SyncRootIsNonNull) {
    MinimalCollection c({});
    EXPECT_NE(c.getSyncRootProperty(), nullptr);
}

// -----------------------------------------------------------------------
// IComparer tests
// -----------------------------------------------------------------------

TEST(IComparerTest, LessThan) {
    IntComparer cmp;
    int a = 1, b = 2;
    EXPECT_LT(cmp.Compare(&a, &b), 0);
}

TEST(IComparerTest, GreaterThan) {
    IntComparer cmp;
    int a = 5, b = 3;
    EXPECT_GT(cmp.Compare(&a, &b), 0);
}

TEST(IComparerTest, Equal) {
    IntComparer cmp;
    int a = 7, b = 7;
    EXPECT_EQ(cmp.Compare(&a, &b), 0);
}

// -----------------------------------------------------------------------
// IDictionaryEnumerator — via DictionaryEntry (no concrete impl needed)
// -----------------------------------------------------------------------

TEST(IDictionaryEnumeratorTest, EntryPropertyRoundTrip) {
    DictionaryEntry entry(std::any(42), std::any(std::string("val")));
    EXPECT_EQ(std::any_cast<int>(entry.getKeyProperty()), 42);
    EXPECT_EQ(std::any_cast<std::string>(entry.getValueProperty()), "val");
}

// -----------------------------------------------------------------------
// IEnumerable tests
// -----------------------------------------------------------------------

TEST(IEnumerableTest, MinimalCollectionIsIEnumerable) {
    MinimalCollection c({1});
    IEnumerable& e = c;
    IEnumerator* en = e.GetEnumerator();
    EXPECT_EQ(en, nullptr);
}

// -----------------------------------------------------------------------
// IDictionary — tested via Hashtable concrete implementation
// -----------------------------------------------------------------------

TEST(IDictionaryTest, HashtableImplementsIDictionary) {
    Hashtable ht;
    IDictionary& d = ht;
    EXPECT_EQ(d.getCountProperty(), 0);
    EXPECT_FALSE(d.getIsReadOnlyProperty());
    EXPECT_FALSE(d.getIsFixedSizeProperty());
}

TEST(IDictionaryTest, HashtableAddAndContains) {
    Hashtable ht;
    ht.Add(std::string("key1"), std::any(42));
    EXPECT_TRUE(ht.ContainsKey("key1"));
    EXPECT_FALSE(ht.ContainsKey("key2"));
}

TEST(IDictionaryTest, HashtableClear) {
    Hashtable ht;
    ht.Add(std::string("a"), std::any(1));
    ht.Clear();
    EXPECT_EQ(ht.getCountProperty(), 0);
}
