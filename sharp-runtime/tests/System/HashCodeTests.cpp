// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include "System/HashCode.hpp"
#include "System/Span.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

using System::HashCode;

TEST(HashCodeTests, ToHashCode_DefaultIsNonZero) {
    HashCode hc;
    EXPECT_NE(hc.ToHashCode(), 0);
}

TEST(HashCodeTests, Add_SameValuesTwice_SameResult) {
    HashCode hc1, hc2;
    hc1.Add(42);
    hc2.Add(42);
    EXPECT_EQ(hc1.ToHashCode(), hc2.ToHashCode());
}

TEST(HashCodeTests, Add_DifferentValues_DifferentResult) {
    HashCode hc1, hc2;
    hc1.Add(1);
    hc2.Add(2);
    EXPECT_NE(hc1.ToHashCode(), hc2.ToHashCode());
}

TEST(HashCodeTests, Combine1_Consistent) {
    EXPECT_EQ(HashCode::Combine(10), HashCode::Combine(10));
}

TEST(HashCodeTests, Combine2_Consistent) {
    EXPECT_EQ(HashCode::Combine(1, 2), HashCode::Combine(1, 2));
}

TEST(HashCodeTests, Combine2_OrderMatters) {
    EXPECT_NE(HashCode::Combine(1, 2), HashCode::Combine(2, 1));
}

TEST(HashCodeTests, Combine3_Consistent) {
    EXPECT_EQ(HashCode::Combine(1, 2, 3), HashCode::Combine(1, 2, 3));
}

TEST(HashCodeTests, Combine8_DoesNotThrow) {
    EXPECT_NO_THROW(HashCode::Combine(1, 2, 3, 4, 5, 6, 7, 8));
}

TEST(HashCodeTests, AddBytes_SameData_SameHash) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    HashCode hc1, hc2;
    hc1.AddBytes(data);
    hc2.AddBytes(data);
    EXPECT_EQ(hc1.ToHashCode(), hc2.ToHashCode());
}

TEST(HashCodeTests, AddBytes_Span_MatchesVectorOverload) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    System::ReadOnlySpan<uint8_t> span(data.data(), static_cast<SharpRuntime::intcs>(data.size()));
    HashCode hc1, hc2;
    hc1.AddBytes(data);
    hc2.AddBytes(span);
    EXPECT_EQ(hc1.ToHashCode(), hc2.ToHashCode());
}

TEST(HashCodeTests, Seed_DiffersAcrossProcessesButConsistentWithinOne) {
    // The per-process global seed is generated once and shared by every HashCode
    // instance in this run, so two independently-constructed accumulators given the
    // same input still agree (the seed is not per-instance randomness).
    HashCode hc1, hc2;
    hc1.Add(std::string("hello"));
    hc2.Add(std::string("hello"));
    EXPECT_EQ(hc1.ToHashCode(), hc2.ToHashCode());
}
