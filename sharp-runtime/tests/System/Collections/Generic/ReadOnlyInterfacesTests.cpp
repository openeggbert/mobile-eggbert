// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/IEqualityComparer.hpp"
#include "System/Collections/Generic/IList.hpp"
#include "System/Collections/Generic/IReadOnlyCollection.hpp"
#include "System/Collections/Generic/IReadOnlyDictionary.hpp"
#include "System/Collections/Generic/IReadOnlyList.hpp"
#include <vector>
#include <unordered_map>
#include <functional>

using namespace System::Collections::Generic;

namespace {

// ---- IEqualityComparer<int> concrete ----
class IntEqCmp : public IEqualityComparer<int> {
public:
    bool Equals(const int& x, const int& y) const override { return x == y; }
    int GetHashCode(const int& obj) const override { return static_cast<int>(std::hash<int>{}(obj)); }
};

// ---- IList<int> concrete ----
class IntList : public IList<int> {
    std::vector<int> data_;
public:
    IEnumerator<int>* GetEnumerator() override { return nullptr; }
    int  getCountProperty()       const override { return static_cast<int>(data_.size()); }
    bool getIsReadOnlyProperty()  const override { return false; }
    void Add(const int& item)           override { data_.push_back(item); }
    void Clear()                        override { data_.clear(); }
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
    const int& operator[](int i) const override { return data_[static_cast<std::size_t>(i)]; }
    int&       operator[](int i)       override { return data_[static_cast<std::size_t>(i)]; }
    int IndexOf(const int& item) const override {
        for (int i = 0; i < static_cast<int>(data_.size()); ++i)
            if (data_[static_cast<std::size_t>(i)] == item) return i;
        return -1;
    }
    void Insert(int index, const int& item) override {
        data_.insert(data_.begin() + index, item);
    }
    void RemoveAt(int index) override {
        data_.erase(data_.begin() + index);
    }
};

// ---- IReadOnlyCollection<int> concrete ----
class RoCollection : public IReadOnlyCollection<int> {
    std::vector<int> data_;
public:
    explicit RoCollection(std::vector<int> d) : data_(std::move(d)) {}
    IEnumerator<int>* GetEnumerator() override { return nullptr; }
    int getCountProperty() const override { return static_cast<int>(data_.size()); }
};

// ---- IReadOnlyList<int> concrete ----
class RoList : public IReadOnlyList<int> {
    std::vector<int> data_;
public:
    explicit RoList(std::vector<int> d) : data_(std::move(d)) {}
    IEnumerator<int>* GetEnumerator() override { return nullptr; }
    int getCountProperty() const override { return static_cast<int>(data_.size()); }
    const int& operator[](int i) const override { return data_[static_cast<std::size_t>(i)]; }
};

// ---- IReadOnlyDictionary<int,std::string> concrete ----
class RoDict : public IReadOnlyDictionary<int, std::string> {
    std::unordered_map<int, std::string> map_;
public:
    explicit RoDict(std::unordered_map<int, std::string> m) : map_(std::move(m)) {}
    IEnumerator<KeyValuePair<int,std::string>>* GetEnumerator() override { return nullptr; }
    int getCountProperty() const override { return static_cast<int>(map_.size()); }
    const std::string& operator[](const int& k) const override { return map_.at(k); }
    IEnumerable<int>* getKeysProperty() const override { return nullptr; }
    IEnumerable<std::string>* getValuesProperty() const override { return nullptr; }
    bool ContainsKey(const int& k) const override { return map_.count(k) > 0; }
    bool TryGetValue(const int& k, std::string& v) const override {
        auto it = map_.find(k);
        if (it == map_.end()) return false;
        v = it->second;
        return true;
    }
};

} // namespace

// ---- IEqualityComparer tests ----
TEST(GenIEqualityComparerTest, EqualValues) {
    IntEqCmp cmp;
    EXPECT_TRUE(cmp.Equals(5, 5));
}

TEST(GenIEqualityComparerTest, UnequalValues) {
    IntEqCmp cmp;
    EXPECT_FALSE(cmp.Equals(3, 7));
}

TEST(GenIEqualityComparerTest, HashCodeConsistent) {
    IntEqCmp cmp;
    EXPECT_EQ(cmp.GetHashCode(42), cmp.GetHashCode(42));
}

TEST(GenIEqualityComparerTest, EqualObjectsSameHash) {
    IntEqCmp cmp;
    EXPECT_EQ(cmp.GetHashCode(10), cmp.GetHashCode(10));
}

// ---- IList tests ----
TEST(GenIListTest, AddAndIndex) {
    IntList lst;
    lst.Add(10); lst.Add(20);
    EXPECT_EQ(lst[0], 10);
    EXPECT_EQ(lst[1], 20);
}

TEST(GenIListTest, IndexOf) {
    IntList lst;
    lst.Add(5); lst.Add(15);
    EXPECT_EQ(lst.IndexOf(15), 1);
    EXPECT_EQ(lst.IndexOf(99), -1);
}

TEST(GenIListTest, Insert) {
    IntList lst;
    lst.Add(1); lst.Add(3);
    lst.Insert(1, 2);
    EXPECT_EQ(lst[1], 2);
    EXPECT_EQ(lst.getCountProperty(), 3);
}

TEST(GenIListTest, RemoveAt) {
    IntList lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    lst.RemoveAt(1);
    EXPECT_EQ(lst.getCountProperty(), 2);
    EXPECT_EQ(lst[1], 3);
}

TEST(GenIListTest, MutableIndex) {
    IntList lst;
    lst.Add(0);
    lst[0] = 99;
    EXPECT_EQ(lst[0], 99);
}

// ---- IReadOnlyCollection tests ----
TEST(GenIReadOnlyCollectionTest, Count) {
    RoCollection c({1, 2, 3});
    EXPECT_EQ(c.getCountProperty(), 3);
}

TEST(GenIReadOnlyCollectionTest, EmptyCount) {
    RoCollection c({});
    EXPECT_EQ(c.getCountProperty(), 0);
}

// ---- IReadOnlyList tests ----
TEST(GenIReadOnlyListTest, IndexAccess) {
    RoList lst({10, 20, 30});
    EXPECT_EQ(lst[0], 10);
    EXPECT_EQ(lst[2], 30);
}

TEST(GenIReadOnlyListTest, Count) {
    RoList lst({1, 2});
    EXPECT_EQ(lst.getCountProperty(), 2);
}

// ---- IReadOnlyDictionary tests ----
TEST(GenIReadOnlyDictionaryTest, ContainsKey) {
    RoDict d({{1, "one"}, {2, "two"}});
    EXPECT_TRUE(d.ContainsKey(1));
    EXPECT_FALSE(d.ContainsKey(99));
}

TEST(GenIReadOnlyDictionaryTest, TryGetValueFound) {
    RoDict d({{7, "seven"}});
    std::string v;
    EXPECT_TRUE(d.TryGetValue(7, v));
    EXPECT_EQ(v, "seven");
}

TEST(GenIReadOnlyDictionaryTest, TryGetValueMissing) {
    RoDict d({});
    std::string v;
    EXPECT_FALSE(d.TryGetValue(1, v));
}

TEST(GenIReadOnlyDictionaryTest, Indexer) {
    RoDict d({{3, "three"}});
    EXPECT_EQ(d[3], "three");
}

TEST(GenIReadOnlyDictionaryTest, Count) {
    RoDict d({{1, "a"}, {2, "b"}, {3, "c"}});
    EXPECT_EQ(d.getCountProperty(), 3);
}
