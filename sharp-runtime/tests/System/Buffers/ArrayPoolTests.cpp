// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Buffers/ArrayPool.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Buffers::ArrayPool;
using System::ArgumentOutOfRangeException;

TEST(ArrayPoolTest, SharedNotNull) {
    auto& pool = ArrayPool<int>::Shared();
    (void)pool;
}

TEST(ArrayPoolTest, RentMinimumLength) {
    auto& pool = ArrayPool<int>::Shared();
    auto buf = pool.Rent(10);
    EXPECT_GE(static_cast<int>(buf.size()), 10);
}

TEST(ArrayPoolTest, ReturnWithClear) {
    auto& pool = ArrayPool<int>::Shared();
    auto buf = pool.Rent(4);
    buf[0] = 42;
    pool.Return(buf, true);
    for (auto v : buf) EXPECT_EQ(v, 0);
}

TEST(ArrayPoolTest, CreateReturnsPool) {
    auto pool = ArrayPool<double>::Create();
    EXPECT_NE(pool, nullptr);
    auto buf = pool->Rent(5);
    EXPECT_GE(static_cast<int>(buf.size()), 5);
}

TEST(ArrayPoolTest, CreateWithCapacity) {
    auto pool = ArrayPool<int>::Create(1024, 10);
    EXPECT_NE(pool, nullptr);
}

TEST(ArrayPoolTest, Rent_ZeroLength_ReturnsEmpty) {
    auto& pool = ArrayPool<int>::Shared();
    auto buf = pool.Rent(0);
    EXPECT_EQ(buf.size(), 0u);
}

TEST(ArrayPoolTest, Rent_NegativeLength_Throws) {
    auto& pool = ArrayPool<int>::Shared();
    EXPECT_THROW(pool.Rent(-1), ArgumentOutOfRangeException);
}
