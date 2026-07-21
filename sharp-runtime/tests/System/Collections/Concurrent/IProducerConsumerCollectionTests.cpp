// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Concurrent/IProducerConsumerCollection.hpp"
#include <vector>
#include <stdexcept>

using namespace System::Collections::Concurrent;
using System::Collections::Generic::IEnumerator;

namespace {

// Minimal concrete implementation for testing the interface contract
class VectorCollection : public IProducerConsumerCollection<int> {
    std::vector<int> data_;
public:
    bool TryAdd(const int& item) override { data_.push_back(item); return true; }
    bool TryTake(int& item) override {
        if (data_.empty()) return false;
        item = data_.back();
        data_.pop_back();
        return true;
    }
    SharpRuntime::intcs getCountProperty() const override {
        return static_cast<SharpRuntime::intcs>(data_.size());
    }
    bool getIsEmptyProperty() const override { return data_.empty(); }
    void CopyTo(std::vector<int>& array, SharpRuntime::intcs index) override {
        for (std::size_t i = 0; i < data_.size(); ++i) {
            if (static_cast<std::size_t>(index) + i < array.size()) {
                array[static_cast<std::size_t>(index) + i] = data_[i];
            } else {
                array.push_back(data_[i]);
            }
        }
    }
    std::vector<int> ToArray() const override { return data_; }
    IEnumerator<int>* GetEnumerator() override { return nullptr; }
};

} // namespace

TEST(IProducerConsumerCollectionTest, TryAddAndCount) {
    VectorCollection c;
    EXPECT_TRUE(c.TryAdd(1));
    EXPECT_TRUE(c.TryAdd(2));
    EXPECT_EQ(c.getCountProperty(), 2);
}

TEST(IProducerConsumerCollectionTest, TryTake) {
    VectorCollection c;
    c.TryAdd(42);
    int v = 0;
    EXPECT_TRUE(c.TryTake(v));
    EXPECT_EQ(v, 42);
}

TEST(IProducerConsumerCollectionTest, TryTakeEmptyReturnsFalse) {
    VectorCollection c;
    int v = 0;
    EXPECT_FALSE(c.TryTake(v));
}

TEST(IProducerConsumerCollectionTest, IsEmpty) {
    VectorCollection c;
    EXPECT_TRUE(c.getIsEmptyProperty());
    c.TryAdd(1);
    EXPECT_FALSE(c.getIsEmptyProperty());
}

TEST(IProducerConsumerCollectionTest, ToArray) {
    VectorCollection c;
    c.TryAdd(10);
    c.TryAdd(20);
    auto arr = c.ToArray();
    ASSERT_EQ(static_cast<int>(arr.size()), 2);
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
}

TEST(IProducerConsumerCollectionTest, CopyTo) {
    VectorCollection c;
    c.TryAdd(7);
    c.TryAdd(8);
    std::vector<int> buf = {0, 0, 0};
    c.CopyTo(buf, 1);
    EXPECT_EQ(buf[1], 7);
    EXPECT_EQ(buf[2], 8);
}
