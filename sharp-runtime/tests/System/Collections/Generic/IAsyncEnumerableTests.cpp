// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/IAsyncEnumerable.hpp"
#include "System/Collections/Generic/IAsyncEnumerator.hpp"
#include <vector>
#include <memory>

using System::Collections::Generic::IAsyncEnumerable;
using System::Collections::Generic::IAsyncEnumerator;

namespace {

// Concrete enumerator over a vector
class VectorAsyncEnumerator : public IAsyncEnumerator<int> {
    const std::vector<int>& data_;
    int index_ = -1;
public:
    explicit VectorAsyncEnumerator(const std::vector<int>& data) : data_(data) {}
    bool MoveNextAsync() override {
        ++index_;
        return index_ < static_cast<int>(data_.size());
    }
    const int& getCurrentProperty() const override { return data_[static_cast<std::size_t>(index_)]; }
};

// Concrete enumerable backed by a vector
class VectorAsyncEnumerable : public IAsyncEnumerable<int> {
    std::vector<int> data_;
public:
    explicit VectorAsyncEnumerable(std::vector<int> data) : data_(std::move(data)) {}
    std::shared_ptr<IAsyncEnumerator<int>> GetAsyncEnumerator(
        System::Threading::CancellationToken) override {
        return std::make_shared<VectorAsyncEnumerator>(data_);
    }
};

} // namespace

TEST(IAsyncEnumeratorTest, MoveNextAndCurrent) {
    std::vector<int> data{10, 20, 30};
    VectorAsyncEnumerator e(data);
    EXPECT_TRUE(e.MoveNextAsync());
    EXPECT_EQ(e.getCurrentProperty(), 10);
    EXPECT_TRUE(e.MoveNextAsync());
    EXPECT_EQ(e.getCurrentProperty(), 20);
    EXPECT_TRUE(e.MoveNextAsync());
    EXPECT_EQ(e.getCurrentProperty(), 30);
    EXPECT_FALSE(e.MoveNextAsync());
}

TEST(IAsyncEnumeratorTest, EmptyCollection) {
    std::vector<int> data;
    VectorAsyncEnumerator e(data);
    EXPECT_FALSE(e.MoveNextAsync());
}

TEST(IAsyncEnumeratorTest, DisposeAsyncIsNoOp) {
    std::vector<int> data{1};
    VectorAsyncEnumerator e(data);
    e.DisposeAsync();
    EXPECT_TRUE(e.MoveNextAsync());
}

TEST(IAsyncEnumerableTest, GetAsyncEnumeratorNotNull) {
    VectorAsyncEnumerable col({1, 2, 3});
    auto e = col.GetAsyncEnumerator({});
    EXPECT_NE(e, nullptr);
}

TEST(IAsyncEnumerableTest, IterateAllElements) {
    VectorAsyncEnumerable col({5, 10, 15});
    auto e = col.GetAsyncEnumerator({});
    int sum = 0;
    while (e->MoveNextAsync()) sum += e->getCurrentProperty();
    EXPECT_EQ(sum, 30);
}

TEST(IAsyncEnumerableTest, EmptyEnumerable) {
    VectorAsyncEnumerable col({});
    auto e = col.GetAsyncEnumerator({});
    EXPECT_FALSE(e->MoveNextAsync());
}
