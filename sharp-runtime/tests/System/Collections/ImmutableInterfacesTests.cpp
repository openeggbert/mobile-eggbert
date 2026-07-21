// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Immutable/IImmutableDictionary.hpp"
#include "System/Collections/Immutable/IImmutableList.hpp"
#include "System/Collections/Immutable/IImmutableQueue.hpp"
#include "System/Collections/Immutable/IImmutableSet.hpp"
#include "System/Collections/Immutable/IImmutableStack.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include <algorithm>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace System::Collections::Immutable;
using SharpRuntime::intcs;
using System::Collections::Generic::IEqualityComparer;
using System::Collections::Generic::IEnumerator;
using System::Collections::Generic::KeyValuePair;

namespace {

// ---------------------------------------------------------------------------
// Minimal IImmutableDictionary implementation
// ---------------------------------------------------------------------------
class ConcreteImmDict
    : public IImmutableDictionary<std::string, int>,
      public std::enable_shared_from_this<ConcreteImmDict> {

    std::unordered_map<std::string, int> data_;
    explicit ConcreteImmDict(std::unordered_map<std::string, int> d) : data_(std::move(d)) {}

public:
    ConcreteImmDict() = default;
    static std::shared_ptr<ConcreteImmDict> make(std::unordered_map<std::string, int> d = {}) {
        return std::shared_ptr<ConcreteImmDict>(new ConcreteImmDict(std::move(d)));
    }

    [[nodiscard]] int getCountProperty() const override {
        return static_cast<int>(data_.size());
    }
    [[nodiscard]] bool ContainsKey(const std::string& key) const override {
        return data_.count(key) > 0;
    }
    [[nodiscard]] const int& operator[](const std::string& key) const override {
        return data_.at(key);
    }
    bool TryGetValue(const std::string& key, int& value) const override {
        auto it = data_.find(key);
        if (it == data_.end()) return false;
        value = it->second;
        return true;
    }
    IEnumerator<KeyValuePair<std::string, int>>* GetEnumerator() override { return nullptr; }
    IEnumerable<std::string>* getKeysProperty() const override { return nullptr; }
    IEnumerable<int>* getValuesProperty() const override { return nullptr; }

    std::shared_ptr<IImmutableDictionary<std::string, int>> Clear() const override {
        return ConcreteImmDict::make();
    }
    std::shared_ptr<IImmutableDictionary<std::string, int>>
        Add(const std::string& key, const int& value) const override {
        auto d = data_; d[key] = value;
        return ConcreteImmDict::make(std::move(d));
    }
    std::shared_ptr<IImmutableDictionary<std::string, int>>
        AddRange(const std::vector<KeyValuePair<std::string, int>>& pairs) const override {
        auto d = data_;
        for (auto& p : pairs) d[p.Key] = p.Value;
        return ConcreteImmDict::make(std::move(d));
    }
    std::shared_ptr<IImmutableDictionary<std::string, int>>
        SetItem(const std::string& key, const int& value) const override {
        auto d = data_; d[key] = value;
        return ConcreteImmDict::make(std::move(d));
    }
    std::shared_ptr<IImmutableDictionary<std::string, int>>
        SetItems(const std::vector<KeyValuePair<std::string, int>>& items) const override {
        auto d = data_;
        for (auto& p : items) d[p.Key] = p.Value;
        return ConcreteImmDict::make(std::move(d));
    }
    std::shared_ptr<IImmutableDictionary<std::string, int>>
        RemoveRange(const std::vector<std::string>& keys) const override {
        auto d = data_;
        for (auto& k : keys) d.erase(k);
        return ConcreteImmDict::make(std::move(d));
    }
    std::shared_ptr<IImmutableDictionary<std::string, int>>
        Remove(const std::string& key) const override {
        auto d = data_; d.erase(key);
        return ConcreteImmDict::make(std::move(d));
    }
    bool Contains(const KeyValuePair<std::string, int>& pair) const override {
        auto it = data_.find(pair.Key);
        return it != data_.end() && it->second == pair.Value;
    }
    bool TryGetKey(const std::string& equalKey, std::string& actualKey) const override {
        if (data_.count(equalKey)) { actualKey = equalKey; return true; }
        return false;
    }
};

// ---------------------------------------------------------------------------
// Minimal IImmutableList implementation
// ---------------------------------------------------------------------------
class ConcreteImmList
    : public IImmutableList<int>,
      public std::enable_shared_from_this<ConcreteImmList> {

    std::vector<int> data_;
    explicit ConcreteImmList(std::vector<int> d) : data_(std::move(d)) {}

    static std::shared_ptr<ConcreteImmList> wrap(std::vector<int> d) {
        return std::shared_ptr<ConcreteImmList>(new ConcreteImmList(std::move(d)));
    }
public:
    ConcreteImmList() = default;
    static std::shared_ptr<ConcreteImmList> make(std::vector<int> d = {}) { return wrap(std::move(d)); }

    [[nodiscard]] int getCountProperty() const override { return static_cast<int>(data_.size()); }
    [[nodiscard]] const int& operator[](int idx) const override { return data_[static_cast<size_t>(idx)]; }
    IEnumerator<int>* GetEnumerator() override { return nullptr; }

    std::shared_ptr<IImmutableList<int>> Clear() const override { return wrap({}); }
    std::shared_ptr<IImmutableList<int>> Add(const int& v) const override {
        auto d = data_; d.push_back(v); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> AddRange(const std::vector<int>& items) const override {
        auto d = data_; for (auto x : items) d.push_back(x); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> Insert(SharpRuntime::intcs idx, const int& v) const override {
        auto d = data_; d.insert(d.begin() + idx, v); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> InsertRange(SharpRuntime::intcs idx, const std::vector<int>& items) const override {
        auto d = data_; d.insert(d.begin() + idx, items.begin(), items.end()); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> Remove(const int& v, IEqualityComparer<int>*) const override {
        auto d = data_;
        auto it = std::find(d.begin(), d.end(), v);
        if (it != d.end()) d.erase(it);
        return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> RemoveAll(std::function<bool(const int&)> match) const override {
        auto d = data_;
        d.erase(std::remove_if(d.begin(), d.end(), match), d.end());
        return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> RemoveRange(const std::vector<int>& items, IEqualityComparer<int>*) const override {
        auto d = data_;
        for (auto x : items) { auto it = std::find(d.begin(), d.end(), x); if (it != d.end()) d.erase(it); }
        return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> RemoveRange(SharpRuntime::intcs idx, SharpRuntime::intcs count) const override {
        auto d = data_; d.erase(d.begin() + idx, d.begin() + idx + count); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> RemoveAt(SharpRuntime::intcs idx) const override {
        auto d = data_; d.erase(d.begin() + idx); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> SetItem(SharpRuntime::intcs idx, const int& v) const override {
        auto d = data_; d[static_cast<size_t>(idx)] = v; return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableList<int>> Replace(const int& oldV, const int& newV, IEqualityComparer<int>*) const override {
        auto d = data_;
        auto it = std::find(d.begin(), d.end(), oldV);
        if (it != d.end()) *it = newV;
        return wrap(std::move(d));
    }
    SharpRuntime::intcs IndexOf(const int& item, SharpRuntime::intcs index, SharpRuntime::intcs count,
                                IEqualityComparer<int>*) const override {
        for (auto i = index; i < index + count && i < getCountProperty(); ++i)
            if (data_[static_cast<size_t>(i)] == item) return i;
        return -1;
    }
    SharpRuntime::intcs LastIndexOf(const int& item, SharpRuntime::intcs index, SharpRuntime::intcs count,
                                    IEqualityComparer<int>*) const override {
        for (auto i = index; i > index - count && i >= 0; --i)
            if (data_[static_cast<size_t>(i)] == item) return i;
        return -1;
    }
};

// ---------------------------------------------------------------------------
// Minimal IImmutableQueue implementation
// ---------------------------------------------------------------------------
class ConcreteImmQueue
    : public IImmutableQueue<int> {

    std::vector<int> data_;
    explicit ConcreteImmQueue(std::vector<int> d) : data_(std::move(d)) {}
    static std::shared_ptr<ConcreteImmQueue> wrap(std::vector<int> d) {
        return std::shared_ptr<ConcreteImmQueue>(new ConcreteImmQueue(std::move(d)));
    }
public:
    ConcreteImmQueue() = default;
    static std::shared_ptr<ConcreteImmQueue> make(std::vector<int> d = {}) { return wrap(std::move(d)); }

    bool getIsEmptyProperty() const override { return data_.empty(); }
    IEnumerator<int>* GetEnumerator() override { return nullptr; }

    std::shared_ptr<IImmutableQueue<int>> Clear() const override { return wrap({}); }
    const int& Peek() const override {
        if (data_.empty()) throw std::out_of_range("Queue is empty.");
        return data_.front();
    }
    std::shared_ptr<IImmutableQueue<int>> Enqueue(const int& v) const override {
        auto d = data_; d.push_back(v); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableQueue<int>> Dequeue() const override {
        if (data_.empty()) throw std::out_of_range("Queue is empty.");
        auto d = data_; d.erase(d.begin()); return wrap(std::move(d));
    }
};

// ---------------------------------------------------------------------------
// Minimal IImmutableSet implementation
// ---------------------------------------------------------------------------
class ConcreteImmSet
    : public IImmutableSet<int> {

    std::set<int> data_;
    explicit ConcreteImmSet(std::set<int> d) : data_(std::move(d)) {}
    static std::shared_ptr<ConcreteImmSet> wrap(std::set<int> d) {
        return std::shared_ptr<ConcreteImmSet>(new ConcreteImmSet(std::move(d)));
    }
public:
    ConcreteImmSet() = default;
    static std::shared_ptr<ConcreteImmSet> make(std::set<int> d = {}) { return wrap(std::move(d)); }

    int getCountProperty() const override { return static_cast<int>(data_.size()); }
    IEnumerator<int>* GetEnumerator() override { return nullptr; }

    std::shared_ptr<IImmutableSet<int>> Clear() const override { return wrap({}); }
    bool Contains(const int& v) const override { return data_.count(v) > 0; }
    std::shared_ptr<IImmutableSet<int>> Add(const int& v) const override {
        auto d = data_; d.insert(v); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableSet<int>> Remove(const int& v) const override {
        auto d = data_; d.erase(v); return wrap(std::move(d));
    }
    bool TryGetValue(const int& eq, int& actual) const override {
        auto it = data_.find(eq);
        if (it == data_.end()) return false;
        actual = *it; return true;
    }
    std::shared_ptr<IImmutableSet<int>> Intersect(const std::vector<int>& other) const override {
        std::set<int> s(other.begin(), other.end());
        std::set<int> r;
        for (auto x : data_) if (s.count(x)) r.insert(x);
        return wrap(std::move(r));
    }
    std::shared_ptr<IImmutableSet<int>> Except(const std::vector<int>& other) const override {
        auto d = data_;
        for (auto x : other) d.erase(x);
        return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableSet<int>> SymmetricExcept(const std::vector<int>& other) const override {
        auto d = data_;
        for (auto x : other) { if (d.count(x)) d.erase(x); else d.insert(x); }
        return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableSet<int>> Union(const std::vector<int>& other) const override {
        auto d = data_;
        for (auto x : other) d.insert(x);
        return wrap(std::move(d));
    }
    bool SetEquals(const std::vector<int>& other) const override {
        std::set<int> s(other.begin(), other.end());
        return data_ == s;
    }
    bool IsProperSubsetOf(const std::vector<int>& other) const override {
        std::set<int> s(other.begin(), other.end());
        return std::includes(s.begin(), s.end(), data_.begin(), data_.end()) && data_ != s;
    }
    bool IsProperSupersetOf(const std::vector<int>& other) const override {
        std::set<int> s(other.begin(), other.end());
        return std::includes(data_.begin(), data_.end(), s.begin(), s.end()) && data_ != s;
    }
    bool IsSubsetOf(const std::vector<int>& other) const override {
        std::set<int> s(other.begin(), other.end());
        return std::includes(s.begin(), s.end(), data_.begin(), data_.end());
    }
    bool IsSupersetOf(const std::vector<int>& other) const override {
        std::set<int> s(other.begin(), other.end());
        return std::includes(data_.begin(), data_.end(), s.begin(), s.end());
    }
    bool Overlaps(const std::vector<int>& other) const override {
        for (auto x : other) if (data_.count(x)) return true;
        return false;
    }
};

// ---------------------------------------------------------------------------
// Minimal IImmutableStack implementation
// ---------------------------------------------------------------------------
class ConcreteImmStack
    : public IImmutableStack<int> {

    std::vector<int> data_; // back = top
    explicit ConcreteImmStack(std::vector<int> d) : data_(std::move(d)) {}
    static std::shared_ptr<ConcreteImmStack> wrap(std::vector<int> d) {
        return std::shared_ptr<ConcreteImmStack>(new ConcreteImmStack(std::move(d)));
    }
public:
    ConcreteImmStack() = default;
    static std::shared_ptr<ConcreteImmStack> make(std::vector<int> d = {}) { return wrap(std::move(d)); }

    bool getIsEmptyProperty() const override { return data_.empty(); }
    IEnumerator<int>* GetEnumerator() override { return nullptr; }

    std::shared_ptr<IImmutableStack<int>> Clear() const override { return wrap({}); }
    std::shared_ptr<IImmutableStack<int>> Push(const int& v) const override {
        auto d = data_; d.push_back(v); return wrap(std::move(d));
    }
    std::shared_ptr<IImmutableStack<int>> Pop() const override {
        if (data_.empty()) throw std::out_of_range("Stack is empty.");
        auto d = data_; d.pop_back(); return wrap(std::move(d));
    }
    const int& Peek() const override {
        if (data_.empty()) throw std::out_of_range("Stack is empty.");
        return data_.back();
    }
};

} // anonymous namespace

// ===========================================================================
// IImmutableDictionary tests
// ===========================================================================

TEST(ImmDictInterfaceTest, AddAndContainsKey) {
    auto d = ConcreteImmDict::make();
    auto d2 = d->Add("a", 1);
    EXPECT_FALSE(d->ContainsKey("a"));
    EXPECT_TRUE(d2->ContainsKey("a"));
    EXPECT_EQ((*d2)["a"], 1);
}

TEST(ImmDictInterfaceTest, RemoveReturnsCopy) {
    auto d = ConcreteImmDict::make();
    auto d2 = d->Add("x", 10)->Remove("x");
    EXPECT_FALSE(d2->ContainsKey("x"));
}

TEST(ImmDictInterfaceTest, SetItemUpdates) {
    auto d = ConcreteImmDict::make();
    auto d2 = d->Add("k", 1)->SetItem("k", 99);
    EXPECT_EQ((*d2)["k"], 99);
}

TEST(ImmDictInterfaceTest, AddRangeAddsMultiple) {
    auto d = ConcreteImmDict::make();
    std::vector<KeyValuePair<std::string, int>> pairs;
    KeyValuePair<std::string, int> p1; p1.Key = "a"; p1.Value = 1;
    KeyValuePair<std::string, int> p2; p2.Key = "b"; p2.Value = 2;
    pairs.push_back(p1); pairs.push_back(p2);
    auto d2 = d->AddRange(pairs);
    EXPECT_EQ(d2->getCountProperty(), 2);
}

TEST(ImmDictInterfaceTest, ClearReturnsEmpty) {
    auto d = ConcreteImmDict::make()->Add("a", 1)->Clear();
    EXPECT_EQ(d->getCountProperty(), 0);
}

TEST(ImmDictInterfaceTest, ContainsKvp) {
    auto d = ConcreteImmDict::make()->Add("z", 42);
    KeyValuePair<std::string, int> p; p.Key = "z"; p.Value = 42;
    EXPECT_TRUE(d->Contains(p));
    p.Value = 0;
    EXPECT_FALSE(d->Contains(p));
}

TEST(ImmDictInterfaceTest, TryGetKey) {
    auto d = ConcreteImmDict::make()->Add("hello", 1);
    std::string actual;
    EXPECT_TRUE(d->TryGetKey("hello", actual));
    EXPECT_EQ(actual, "hello");
    EXPECT_FALSE(d->TryGetKey("missing", actual));
}

TEST(ImmDictInterfaceTest, RemoveRange) {
    auto d = ConcreteImmDict::make()->Add("a", 1)->Add("b", 2)->Add("c", 3);
    auto d2 = d->RemoveRange({"a", "c"});
    EXPECT_EQ(d2->getCountProperty(), 1);
    EXPECT_TRUE(d2->ContainsKey("b"));
}

// ===========================================================================
// IImmutableList tests
// ===========================================================================

TEST(ImmListInterfaceTest, AddPreservesOriginal) {
    auto lst = ConcreteImmList::make();
    auto lst2 = lst->Add(10);
    EXPECT_EQ(lst->getCountProperty(), 0);
    EXPECT_EQ(lst2->getCountProperty(), 1);
    EXPECT_EQ((*lst2)[0], 10);
}

TEST(ImmListInterfaceTest, RemoveAt) {
    auto lst = ConcreteImmList::make({1, 2, 3})->RemoveAt(1);
    EXPECT_EQ(lst->getCountProperty(), 2);
    EXPECT_EQ((*lst)[0], 1);
    EXPECT_EQ((*lst)[1], 3);
}

TEST(ImmListInterfaceTest, Insert) {
    auto lst = ConcreteImmList::make({1, 3})->Insert(1, 2);
    EXPECT_EQ(lst->getCountProperty(), 3);
    EXPECT_EQ((*lst)[1], 2);
}

TEST(ImmListInterfaceTest, SetItem) {
    auto lst = ConcreteImmList::make({1, 2, 3})->SetItem(1, 99);
    EXPECT_EQ((*lst)[1], 99);
}

TEST(ImmListInterfaceTest, Replace) {
    auto lst = ConcreteImmList::make({1, 2, 3})->Replace(2, 20, nullptr);
    EXPECT_EQ((*lst)[1], 20);
}

TEST(ImmListInterfaceTest, RemoveAll) {
    auto lst = ConcreteImmList::make({1, 2, 3, 4})->RemoveAll([](const int& x){ return x % 2 == 0; });
    EXPECT_EQ(lst->getCountProperty(), 2);
    EXPECT_EQ((*lst)[0], 1);
    EXPECT_EQ((*lst)[1], 3);
}

TEST(ImmListInterfaceTest, IndexOf) {
    auto lst = ConcreteImmList::make({10, 20, 30, 20});
    EXPECT_EQ(lst->IndexOf(20, 0, 4, nullptr), 1);
    EXPECT_EQ(lst->IndexOf(99, 0, 4, nullptr), -1);
}

TEST(ImmListInterfaceTest, ClearEmpty) {
    auto lst = ConcreteImmList::make({1, 2})->Clear();
    EXPECT_EQ(lst->getCountProperty(), 0);
}

// ===========================================================================
// IImmutableQueue tests
// ===========================================================================

TEST(ImmQueueInterfaceTest, EnqueueAndPeek) {
    auto q = ConcreteImmQueue::make();
    EXPECT_TRUE(q->getIsEmptyProperty());
    auto q2 = q->Enqueue(5);
    EXPECT_FALSE(q2->getIsEmptyProperty());
    EXPECT_EQ(q2->Peek(), 5);
}

TEST(ImmQueueInterfaceTest, DequeueRemovesFront) {
    auto q = ConcreteImmQueue::make()->Enqueue(1)->Enqueue(2);
    auto q2 = q->Dequeue();
    EXPECT_EQ(q2->Peek(), 2);
}

TEST(ImmQueueInterfaceTest, OriginalUnchangedAfterDequeue) {
    auto q = ConcreteImmQueue::make()->Enqueue(1)->Enqueue(2);
    auto q2 = q->Dequeue();
    EXPECT_EQ(q->Peek(), 1);
    EXPECT_EQ(q2->Peek(), 2);
}

TEST(ImmQueueInterfaceTest, ClearReturnsEmpty) {
    auto q = ConcreteImmQueue::make()->Enqueue(1)->Clear();
    EXPECT_TRUE(q->getIsEmptyProperty());
}

TEST(ImmQueueInterfaceTest, DequeueEmptyThrows) {
    auto q = ConcreteImmQueue::make();
    EXPECT_THROW(q->Dequeue(), std::out_of_range);
}

// ===========================================================================
// IImmutableSet tests
// ===========================================================================

TEST(ImmSetInterfaceTest, AddContains) {
    auto s = ConcreteImmSet::make();
    auto s2 = s->Add(7);
    EXPECT_FALSE(s->Contains(7));
    EXPECT_TRUE(s2->Contains(7));
}

TEST(ImmSetInterfaceTest, Remove) {
    auto s = ConcreteImmSet::make({1, 2, 3})->Remove(2);
    EXPECT_FALSE(s->Contains(2));
    EXPECT_EQ(s->getCountProperty(), 2);
}

TEST(ImmSetInterfaceTest, Union) {
    auto s = ConcreteImmSet::make({1, 2})->Union({3, 4});
    EXPECT_EQ(s->getCountProperty(), 4);
}

TEST(ImmSetInterfaceTest, Intersect) {
    auto s = ConcreteImmSet::make({1, 2, 3})->Intersect({2, 3, 4});
    EXPECT_EQ(s->getCountProperty(), 2);
    EXPECT_TRUE(s->Contains(2));
    EXPECT_TRUE(s->Contains(3));
}

TEST(ImmSetInterfaceTest, Except) {
    auto s = ConcreteImmSet::make({1, 2, 3})->Except({2});
    EXPECT_EQ(s->getCountProperty(), 2);
    EXPECT_FALSE(s->Contains(2));
}

TEST(ImmSetInterfaceTest, SetEquals) {
    auto s = ConcreteImmSet::make({1, 2, 3});
    EXPECT_TRUE(s->SetEquals({1, 2, 3}));
    EXPECT_FALSE(s->SetEquals({1, 2}));
}

TEST(ImmSetInterfaceTest, IsSubsetAndSuperset) {
    auto s = ConcreteImmSet::make({2, 3});
    EXPECT_TRUE(s->IsSubsetOf({1, 2, 3}));
    EXPECT_FALSE(s->IsSupersetOf({1, 2, 3}));
}

TEST(ImmSetInterfaceTest, Overlaps) {
    auto s = ConcreteImmSet::make({1, 2});
    EXPECT_TRUE(s->Overlaps({2, 3}));
    EXPECT_FALSE(s->Overlaps({5, 6}));
}

TEST(ImmSetInterfaceTest, TryGetValue) {
    auto s = ConcreteImmSet::make({42});
    int val = 0;
    EXPECT_TRUE(s->TryGetValue(42, val));
    EXPECT_EQ(val, 42);
    EXPECT_FALSE(s->TryGetValue(99, val));
}

TEST(ImmSetInterfaceTest, ClearEmpty) {
    auto s = ConcreteImmSet::make({1, 2})->Clear();
    EXPECT_EQ(s->getCountProperty(), 0);
}

// ===========================================================================
// IImmutableStack tests
// ===========================================================================

TEST(ImmStackInterfaceTest, PushAndPeek) {
    auto s = ConcreteImmStack::make();
    EXPECT_TRUE(s->getIsEmptyProperty());
    auto s2 = s->Push(10);
    EXPECT_FALSE(s2->getIsEmptyProperty());
    EXPECT_EQ(s2->Peek(), 10);
}

TEST(ImmStackInterfaceTest, PopRemovesTop) {
    auto s = ConcreteImmStack::make()->Push(1)->Push(2);
    auto s2 = s->Pop();
    EXPECT_EQ(s2->Peek(), 1);
}

TEST(ImmStackInterfaceTest, OriginalUnchangedAfterPop) {
    auto s = ConcreteImmStack::make()->Push(1)->Push(2);
    auto s2 = s->Pop();
    EXPECT_EQ(s->Peek(), 2);
    EXPECT_EQ(s2->Peek(), 1);
}

TEST(ImmStackInterfaceTest, ClearReturnsEmpty) {
    auto s = ConcreteImmStack::make()->Push(1)->Clear();
    EXPECT_TRUE(s->getIsEmptyProperty());
}

TEST(ImmStackInterfaceTest, PopEmptyThrows) {
    auto s = ConcreteImmStack::make();
    EXPECT_THROW(s->Pop(), std::out_of_range);
}

TEST(ImmStackInterfaceTest, PeekEmptyThrows) {
    auto s = ConcreteImmStack::make();
    EXPECT_THROW(s->Peek(), std::out_of_range);
}
