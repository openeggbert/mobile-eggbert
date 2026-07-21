// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "System/SpanSplitEnumerator.hpp"

using System::SpanSplitEnumerator;
using System::ReadOnlySpan;

// helper: collect segments from enumerator into a vector of vectors
template<typename T>
static std::vector<std::vector<T>> collect(ReadOnlySpan<T> src, const T& sep) {
    SpanSplitEnumerator<T> e(src, sep);
    std::vector<std::vector<T>> result;
    for (auto seg : e) result.push_back(std::vector<T>(seg.begin(), seg.end()));
    return result;
}

template<typename T>
static std::vector<std::vector<T>> collectAny(ReadOnlySpan<T> src, const std::vector<T>& seps) {
    SpanSplitEnumerator<T> e(src, seps, true);
    std::vector<std::vector<T>> result;
    for (auto seg : e) result.push_back(std::vector<T>(seg.begin(), seg.end()));
    return result;
}

// ---------------------------------------------------------------------------
// Single-element separator
// ---------------------------------------------------------------------------

TEST(SpanSplitEnumeratorTests, SingleSep_TwoSegments) {
    std::vector<int> v = {1, 0, 2};
    auto res = collect(ReadOnlySpan<int>(v), 0);
    ASSERT_EQ(res.size(), 2u);
    EXPECT_EQ(res[0], (std::vector<int>{1}));
    EXPECT_EQ(res[1], (std::vector<int>{2}));
}

TEST(SpanSplitEnumeratorTests, SingleSep_NoSeparatorFound) {
    std::vector<int> v = {1, 2, 3};
    auto res = collect(ReadOnlySpan<int>(v), 9);
    ASSERT_EQ(res.size(), 1u);
    EXPECT_EQ(res[0], (std::vector<int>{1, 2, 3}));
}

TEST(SpanSplitEnumeratorTests, SingleSep_LeadingSeparator) {
    std::vector<int> v = {0, 1, 2};
    auto res = collect(ReadOnlySpan<int>(v), 0);
    ASSERT_EQ(res.size(), 2u);
    EXPECT_TRUE(res[0].empty());
    EXPECT_EQ(res[1], (std::vector<int>{1, 2}));
}

TEST(SpanSplitEnumeratorTests, SingleSep_TrailingSeparator) {
    std::vector<int> v = {1, 2, 0};
    auto res = collect(ReadOnlySpan<int>(v), 0);
    ASSERT_EQ(res.size(), 2u);
    EXPECT_EQ(res[0], (std::vector<int>{1, 2}));
    EXPECT_TRUE(res[1].empty());
}

TEST(SpanSplitEnumeratorTests, SingleSep_ConsecutiveSeparators) {
    std::vector<int> v = {1, 0, 0, 2};
    auto res = collect(ReadOnlySpan<int>(v), 0);
    ASSERT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0], (std::vector<int>{1}));
    EXPECT_TRUE(res[1].empty());
    EXPECT_EQ(res[2], (std::vector<int>{2}));
}

TEST(SpanSplitEnumeratorTests, SingleSep_EmptySource) {
    std::vector<int> v;
    auto res = collect(ReadOnlySpan<int>(v), 0);
    ASSERT_EQ(res.size(), 1u);
    EXPECT_TRUE(res[0].empty());
}

TEST(SpanSplitEnumeratorTests, SingleSep_ThreeSegments) {
    std::vector<int> v = {1, 0, 2, 0, 3};
    auto res = collect(ReadOnlySpan<int>(v), 0);
    ASSERT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0][0], 1);
    EXPECT_EQ(res[1][0], 2);
    EXPECT_EQ(res[2][0], 3);
}

// ---------------------------------------------------------------------------
// AnyOf separator
// ---------------------------------------------------------------------------

TEST(SpanSplitEnumeratorTests, AnyOf_TwoSeparators) {
    std::vector<int> v = {1, 0, 2, 9, 3};
    auto res = collectAny(ReadOnlySpan<int>(v), {0, 9});
    ASSERT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0], (std::vector<int>{1}));
    EXPECT_EQ(res[1], (std::vector<int>{2}));
    EXPECT_EQ(res[2], (std::vector<int>{3}));
}

TEST(SpanSplitEnumeratorTests, AnyOf_NoMatch) {
    std::vector<int> v = {1, 2, 3};
    auto res = collectAny(ReadOnlySpan<int>(v), {7, 8});
    ASSERT_EQ(res.size(), 1u);
    EXPECT_EQ(res[0], (std::vector<int>{1, 2, 3}));
}

// ---------------------------------------------------------------------------
// Sequence separator
// ---------------------------------------------------------------------------

TEST(SpanSplitEnumeratorTests, Sequence_TwoSegments) {
    std::vector<int> v = {1, 2, 9, 9, 3, 4};
    SpanSplitEnumerator<int> e(ReadOnlySpan<int>(v), std::vector<int>{9, 9}, false);
    std::vector<std::vector<int>> res;
    for (auto seg : e) res.push_back(std::vector<int>(seg.begin(), seg.end()));
    ASSERT_EQ(res.size(), 2u);
    EXPECT_EQ(res[0], (std::vector<int>{1, 2}));
    EXPECT_EQ(res[1], (std::vector<int>{3, 4}));
}

// ---------------------------------------------------------------------------
// MoveNext explicit usage
// ---------------------------------------------------------------------------

TEST(SpanSplitEnumeratorTests, MoveNext_Explicit) {
    std::vector<int> v = {10, 0, 20};
    SpanSplitEnumerator<int> e(ReadOnlySpan<int>(v), 0);
    EXPECT_TRUE(e.MoveNext());
    auto s0 = e.getCurrentSpan();
    EXPECT_EQ(s0.getLengthProperty(), 1);
    EXPECT_EQ(s0[0], 10);
    EXPECT_TRUE(e.MoveNext());
    auto s1 = e.getCurrentSpan();
    EXPECT_EQ(s1[0], 20);
    EXPECT_FALSE(e.MoveNext());
}
