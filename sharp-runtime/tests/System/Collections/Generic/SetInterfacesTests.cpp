// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/IReadOnlySet.hpp"
#include "System/Collections/Generic/ISet.hpp"
#include "System/Collections/Generic/KeyValuePair.hpp"
#include <unordered_set>
#include <vector>

using namespace System::Collections::Generic;

namespace {

// ---- Minimal IEnumerable<int> backed by vector ----
class VecEnumInt : public IEnumerable<int> {
    std::vector<int> data_;
public:
    explicit VecEnumInt(std::vector<int> d) : data_(std::move(d)) {}
    IEnumerator<int>* GetEnumerator() override { return nullptr; }
    const std::vector<int>& data() const { return data_; }
};

// ---- IReadOnlySet<int> concrete backed by unordered_set ----
class RoSet : public IReadOnlySet<int> {
    std::unordered_set<int> s_;
public:
    explicit RoSet(std::initializer_list<int> il) : s_(il) {}
    IEnumerator<int>* GetEnumerator() override { return nullptr; }
    int getCountProperty() const override { return static_cast<int>(s_.size()); }
    bool Contains(const int& item) const override { return s_.count(item) > 0; }

    bool IsSubsetOf(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        for (const auto& e : s_) if (!os.count(e)) return false;
        return true;
    }
    bool IsSupersetOf(const IEnumerable<int>& other) const override {
        for (const auto& e : static_cast<const VecEnumInt&>(other).data())
            if (!s_.count(e)) return false;
        return true;
    }
    bool IsProperSubsetOf(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        return IsSubsetOf(other) && os.size() > s_.size();
    }
    bool IsProperSupersetOf(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        return IsSupersetOf(other) && s_.size() > os.size();
    }
    bool Overlaps(const IEnumerable<int>& other) const override {
        for (const auto& e : static_cast<const VecEnumInt&>(other).data())
            if (s_.count(e)) return true;
        return false;
    }
    bool SetEquals(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        return s_ == os;
    }
};

// ---- ISet<int> concrete backed by unordered_set ----
class MutSet : public ISet<int> {
    std::unordered_set<int> s_;
public:
    explicit MutSet(std::initializer_list<int> il) : s_(il) {}
    IEnumerator<int>* GetEnumerator() override { return nullptr; }
    int  getCountProperty()      const override { return static_cast<int>(s_.size()); }
    bool getIsReadOnlyProperty() const override { return false; }
    bool TryAdd(const int& item)       override { return s_.insert(item).second; }
    void Clear()                       override { s_.clear(); }
    bool Contains(const int& item) const override { return s_.count(item) > 0; }
    bool Remove(const int& item)       override { return s_.erase(item) > 0; }

    void UnionWith(const IEnumerable<int>& other) override {
        for (const auto& e : static_cast<const VecEnumInt&>(other).data()) s_.insert(e);
    }
    void IntersectWith(const IEnumerable<int>& other) override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        for (auto it = s_.begin(); it != s_.end(); )
            it = os.count(*it) ? std::next(it) : s_.erase(it);
    }
    void ExceptWith(const IEnumerable<int>& other) override {
        for (const auto& e : static_cast<const VecEnumInt&>(other).data()) s_.erase(e);
    }
    void SymmetricExceptWith(const IEnumerable<int>& other) override {
        for (const auto& e : static_cast<const VecEnumInt&>(other).data()) {
            if (s_.count(e)) s_.erase(e); else s_.insert(e);
        }
    }
    bool IsSubsetOf(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        for (const auto& e : s_) if (!os.count(e)) return false;
        return true;
    }
    bool IsSupersetOf(const IEnumerable<int>& other) const override {
        for (const auto& e : static_cast<const VecEnumInt&>(other).data())
            if (!s_.count(e)) return false;
        return true;
    }
    bool IsProperSubsetOf(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        return IsSubsetOf(other) && os.size() > s_.size();
    }
    bool IsProperSupersetOf(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        return IsSupersetOf(other) && s_.size() > os.size();
    }
    bool Overlaps(const IEnumerable<int>& other) const override {
        for (const auto& e : static_cast<const VecEnumInt&>(other).data())
            if (s_.count(e)) return true;
        return false;
    }
    bool SetEquals(const IEnumerable<int>& other) const override {
        const auto& v = static_cast<const VecEnumInt&>(other).data();
        std::unordered_set<int> os(v.begin(), v.end());
        return s_ == os;
    }
    bool hasElement(int e) const { return s_.count(e) > 0; }
    int size() const { return static_cast<int>(s_.size()); }
};

} // namespace

// ---- IReadOnlySet tests ----
TEST(GenIReadOnlySetTest, Contains) {
    RoSet s{1, 2, 3};
    EXPECT_TRUE(s.Contains(2));
    EXPECT_FALSE(s.Contains(99));
}

TEST(GenIReadOnlySetTest, Count) {
    RoSet s{4, 5, 6};
    EXPECT_EQ(s.getCountProperty(), 3);
}

TEST(GenIReadOnlySetTest, IsSubsetOf) {
    RoSet s{1, 2};
    VecEnumInt other({1, 2, 3});
    EXPECT_TRUE(s.IsSubsetOf(other));
}

TEST(GenIReadOnlySetTest, IsSupersetOf) {
    RoSet s{1, 2, 3};
    VecEnumInt other({1, 2});
    EXPECT_TRUE(s.IsSupersetOf(other));
}

TEST(GenIReadOnlySetTest, IsProperSubsetOf) {
    RoSet s{1, 2};
    VecEnumInt other({1, 2, 3});
    EXPECT_TRUE(s.IsProperSubsetOf(other));
    VecEnumInt same({1, 2});
    EXPECT_FALSE(s.IsProperSubsetOf(same));
}

TEST(GenIReadOnlySetTest, Overlaps) {
    RoSet s{1, 2};
    VecEnumInt overlap({2, 3});
    EXPECT_TRUE(s.Overlaps(overlap));
    VecEnumInt noOverlap({5, 6});
    EXPECT_FALSE(s.Overlaps(noOverlap));
}

TEST(GenIReadOnlySetTest, SetEquals) {
    RoSet s{1, 2, 3};
    VecEnumInt same({3, 1, 2});
    EXPECT_TRUE(s.SetEquals(same));
    VecEnumInt diff({1, 2});
    EXPECT_FALSE(s.SetEquals(diff));
}

// ---- ISet tests ----
TEST(GenISetTest, TryAddReturnsBool) {
    MutSet s{};
    EXPECT_TRUE(s.TryAdd(10));
    EXPECT_FALSE(s.TryAdd(10));
}

TEST(GenISetTest, UnionWith) {
    MutSet s{1, 2};
    VecEnumInt other({2, 3});
    s.UnionWith(other);
    EXPECT_EQ(s.size(), 3);
    EXPECT_TRUE(s.hasElement(3));
}

TEST(GenISetTest, IntersectWith) {
    MutSet s{1, 2, 3};
    VecEnumInt other({2, 3, 4});
    s.IntersectWith(other);
    EXPECT_EQ(s.size(), 2);
    EXPECT_FALSE(s.hasElement(1));
}

TEST(GenISetTest, ExceptWith) {
    MutSet s{1, 2, 3};
    VecEnumInt other({2, 3});
    s.ExceptWith(other);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(s.hasElement(1));
}

TEST(GenISetTest, SymmetricExceptWith) {
    MutSet s{1, 2, 3};
    VecEnumInt other({3, 4});
    s.SymmetricExceptWith(other);
    EXPECT_TRUE(s.hasElement(1));
    EXPECT_TRUE(s.hasElement(2));
    EXPECT_FALSE(s.hasElement(3));
    EXPECT_TRUE(s.hasElement(4));
}

TEST(GenISetTest, SetEquals) {
    MutSet s{1, 2};
    VecEnumInt same({2, 1});
    EXPECT_TRUE(s.SetEquals(same));
}

// ---- KeyValuePair tests ----
TEST(KeyValuePairTest, ConstructAndAccess) {
    KeyValuePair<int, std::string> kv(1, "one");
    EXPECT_EQ(kv.Key, 1);
    EXPECT_EQ(kv.Value, "one");
}

TEST(KeyValuePairTest, EqualityTrue) {
    KeyValuePair<int, int> a(1, 2), b(1, 2);
    EXPECT_TRUE(a == b);
}

TEST(KeyValuePairTest, EqualityFalse) {
    KeyValuePair<int, int> a(1, 2), b(1, 3);
    EXPECT_TRUE(a != b);
}

TEST(KeyValuePairTest, DefaultConstruct) {
    KeyValuePair<int, int> kv;
    EXPECT_EQ(kv.Key, 0);
    EXPECT_EQ(kv.Value, 0);
}

TEST(KeyValuePairTest, MoveConstruct) {
    std::string s = "hello";
    KeyValuePair<int, std::string> kv(42, std::move(s));
    EXPECT_EQ(kv.Key, 42);
    EXPECT_EQ(kv.Value, "hello");
}
