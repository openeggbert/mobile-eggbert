// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Buffers/ReadOnlySpanAction.hpp"

using System::Buffers::ReadOnlySpanAction;
using System::ReadOnlySpan;

TEST(ReadOnlySpanActionTest, Invoke) {
    int sum = 0;
    ReadOnlySpanAction<int, int*> action = [](ReadOnlySpan<int> span, int* out) {
        for (int i = 0; i < span.getLengthProperty(); ++i)
            *out += span[i];
    };
    std::vector<int> data = {1, 2, 3};
    ReadOnlySpan<int> span(data.data(), static_cast<int>(data.size()));
    action(span, &sum);
    EXPECT_EQ(sum, 6);
}

TEST(ReadOnlySpanActionTest, IsCallable) {
    bool called = false;
    ReadOnlySpanAction<char, bool*> action = [](ReadOnlySpan<char>, bool* flag) {
        *flag = true;
    };
    std::vector<char> data = {'x'};
    ReadOnlySpan<char> span(data.data(), 1);
    action(span, &called);
    EXPECT_TRUE(called);
}

TEST(ReadOnlySpanActionTest, DefaultIsEmpty) {
    ReadOnlySpanAction<int, int> action;
    EXPECT_FALSE(static_cast<bool>(action));
}
