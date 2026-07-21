// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Buffers/SpanAction.hpp"

using System::Buffers::SpanAction;
using System::Span;

TEST(SpanActionTest, Invoke) {
    int sum = 0;
    SpanAction<int, int*> action = [](Span<int> span, int* out) {
        for (int i = 0; i < span.getLengthProperty(); ++i)
            *out += span[i];
    };
    std::vector<int> data = {10, 20, 30};
    Span<int> span(data.data(), static_cast<int>(data.size()));
    action(span, &sum);
    EXPECT_EQ(sum, 60);
}

TEST(SpanActionTest, ModifiesSpan) {
    SpanAction<int, int> action = [](Span<int> span, int fill) {
        for (int i = 0; i < span.getLengthProperty(); ++i)
            span[i] = fill;
    };
    std::vector<int> data(3, 0);
    Span<int> span(data.data(), 3);
    action(span, 7);
    EXPECT_EQ(data[0], 7);
    EXPECT_EQ(data[2], 7);
}

TEST(SpanActionTest, DefaultIsEmpty) {
    SpanAction<int, int> action;
    EXPECT_FALSE(static_cast<bool>(action));
}
