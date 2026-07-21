// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/ICollection.hpp"
#include "System/Collections/Generic/IComparer.hpp"
#include "System/Collections/Generic/IDictionary.hpp"
#include "System/Collections/Generic/IEnumerable.hpp"
#include "System/Collections/Generic/IEnumerator.hpp"
#include <vector>
#include <unordered_map>

using namespace System::Collections::Generic;

namespace {

// ---- IEnumerator<T> concrete implementation ----
class VecEnumerator : public IEnumerator<int> {
    const std::vector<int>& data_;
    int idx_ = -1;
public:
    explicit VecEnumerator(const std::vector<int>& d) : data_(d) {}
    bool MoveNext() override { return ++idx_ < static_cast<int>(data_.size()); }
    void Reset()    override { idx_ = -1; }
    const int& Current() const override { return data_[static_cast<std::size_t>(idx_)]; }
};

// ---- IEnumerable<T> concrete implementation ----
class VecEnumerable : public IEnumerable<int> {
    std::vector<int> data_;
public:
    explicit VecEnumerable(std::vector<int> d) : data_(std::move(d)) {}
    IEnumerator<int>* GetEnumerator() override { return new VecEnumerator(data_); }
};

// ---- ICollection<T> concrete implementation ----
class VecCollection : public ICollection<int> {
    std::vector<int> data_;
public:
    IEnumerator<int>* GetEnumerator() override { return new VecEnumerator(data_); }
    int  getCountProperty()      const override { return static_cast<int>(data_.size()); }
    bool getIsReadOnlyProperty() const override { return false; }
    void Add(const int& item)          override { data_.push_back(item); }
    void Clear()                       override { data_.clear(); }
    bool Contains(const int& item) const override {
        for (const auto& v : data_) if (v == item) return true;
        return false;
    }
    bool Remove(const int& item) override {
        for (auto it = data_.begin(); it != data_.end(); ++it) {
            if (*it == item) { data_.erase(it); return true; }
        }
        return false;
    }
};

// ---- IComparer<T> concrete implementation ----
class IntComparer : public IComparer<int> {
public:
    int Compare(const int& x, const int& y) const override {
        return (x < y) ? -1 : (x > y) ? 1 : 0;
    }
};

// ---- IDictionary<TKey,TValue> concrete implementation ----
class IntStringDict : public IDictionary<int, std::string> {
    std::unordered_map<int, std::string> map_;
public:
    IEnumerator<std::pair<int,std::string>>* GetEnumerator() override { return nullptr; }
    int getCountProperty() const override { return static_cast<int>(map_.size()); }
    std::string& operator[](const int& k)       override { return map_[k]; }
    const std::string& operator[](const int& k) const override { return map_.at(k); }
    bool ContainsKey(const int& k)         const override { return map_.count(k) > 0; }
    void Add(const int& k, const std::string& v) override { map_[k] = v; }
    bool Remove(const int& k)                    override { return map_.erase(k) > 0; }
    bool TryGetValue(const int& k, std::string& v) const override {
        auto it = map_.find(k);
        if (it == map_.end()) return false;
        v = it->second;
        return true;
    }
    void Clear() override { map_.clear(); }
};

} // namespace

// ---- IEnumerator tests ----
TEST(GenericIEnumeratorTest, MoveNextAndCurrent) {
    std::vector<int> data{1, 2, 3};
    VecEnumerator e(data);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.Current(), 1);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.Current(), 2);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.Current(), 3);
    EXPECT_FALSE(e.MoveNext());
}

TEST(GenericIEnumeratorTest, ResetRestarts) {
    std::vector<int> data{10, 20};
    VecEnumerator e(data);
    e.MoveNext(); e.MoveNext();
    e.Reset();
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.Current(), 10);
}

TEST(GenericIEnumeratorTest, GetCurrentVoidPtr) {
    std::vector<int> data{42};
    VecEnumerator e(data);
    e.MoveNext();
    void* p = e.getCurrentProperty();
    EXPECT_EQ(*static_cast<int*>(p), 42);
}

// ---- IEnumerable tests ----
TEST(GenericIEnumerableTest, GetEnumeratorNotNull) {
    VecEnumerable col({1, 2, 3});
    auto* e = col.GetEnumerator();
    EXPECT_NE(e, nullptr);
    delete e;
}

TEST(GenericIEnumerableTest, IterateAllElements) {
    VecEnumerable col({5, 10, 15});
    auto* e = col.GetEnumerator();
    int sum = 0;
    while (e->MoveNext()) sum += e->Current();
    EXPECT_EQ(sum, 30);
    delete e;
}

// ---- ICollection tests ----
TEST(GenericICollectionTest, AddAndCount) {
    VecCollection c;
    c.Add(1); c.Add(2);
    EXPECT_EQ(c.getCountProperty(), 2);
}

TEST(GenericICollectionTest, Contains) {
    VecCollection c;
    c.Add(7);
    EXPECT_TRUE(c.Contains(7));
    EXPECT_FALSE(c.Contains(99));
}

TEST(GenericICollectionTest, Remove) {
    VecCollection c;
    c.Add(3);
    EXPECT_TRUE(c.Remove(3));
    EXPECT_EQ(c.getCountProperty(), 0);
}

TEST(GenericICollectionTest, Clear) {
    VecCollection c;
    c.Add(1); c.Add(2);
    c.Clear();
    EXPECT_EQ(c.getCountProperty(), 0);
}

TEST(GenericICollectionTest, IsReadOnly) {
    VecCollection c;
    EXPECT_FALSE(c.getIsReadOnlyProperty());
}

// ---- IComparer tests ----
TEST(GenericIComparerTest, LessThan) {
    IntComparer cmp;
    EXPECT_LT(cmp.Compare(1, 2), 0);
}

TEST(GenericIComparerTest, Equal) {
    IntComparer cmp;
    EXPECT_EQ(cmp.Compare(5, 5), 0);
}

TEST(GenericIComparerTest, GreaterThan) {
    IntComparer cmp;
    EXPECT_GT(cmp.Compare(10, 3), 0);
}

// ---- IDictionary tests ----
TEST(GenericIDictionaryTest, AddAndContainsKey) {
    IntStringDict d;
    d.Add(1, "one");
    EXPECT_TRUE(d.ContainsKey(1));
    EXPECT_FALSE(d.ContainsKey(2));
}

TEST(GenericIDictionaryTest, TryGetValue) {
    IntStringDict d;
    d.Add(42, "answer");
    std::string v;
    EXPECT_TRUE(d.TryGetValue(42, v));
    EXPECT_EQ(v, "answer");
}

TEST(GenericIDictionaryTest, Remove) {
    IntStringDict d;
    d.Add(1, "a");
    EXPECT_TRUE(d.Remove(1));
    EXPECT_FALSE(d.ContainsKey(1));
}

TEST(GenericIDictionaryTest, Count) {
    IntStringDict d;
    d.Add(1, "a"); d.Add(2, "b");
    EXPECT_EQ(d.getCountProperty(), 2);
}

TEST(GenericIDictionaryTest, Clear) {
    IntStringDict d;
    d.Add(1, "a");
    d.Clear();
    EXPECT_EQ(d.getCountProperty(), 0);
}
