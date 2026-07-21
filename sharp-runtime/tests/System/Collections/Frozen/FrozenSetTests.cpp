// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Collections/Frozen/FrozenSet.hpp"
#include <string>

using System::Collections::Frozen::FrozenSet;
using System::Collections::Frozen::ToFrozenSet;

TEST(FrozenSetTest, CreateFromVector) {
    auto s = FrozenSet<int>::Create({1, 2, 3});
    EXPECT_EQ(s.getCountProperty(), 3);
}

TEST(FrozenSetTest, EmptySingleton) {
    const auto& e = FrozenSet<int>::getEmptyProperty();
    EXPECT_EQ(e.getCountProperty(), 0);
}

TEST(FrozenSetTest, ContainsTrue) {
    auto s = FrozenSet<int>::Create({10, 20, 30});
    EXPECT_TRUE(s.Contains(20));
}

TEST(FrozenSetTest, ContainsFalse) {
    auto s = FrozenSet<int>::Create({10, 20, 30});
    EXPECT_FALSE(s.Contains(99));
}

TEST(FrozenSetTest, DuplicatesIgnored) {
    auto s = FrozenSet<int>::Create({5, 5, 5});
    EXPECT_EQ(s.getCountProperty(), 1);
}

TEST(FrozenSetTest, TryGetValueFound) {
    auto s = FrozenSet<std::string>::Create({"hello", "world"});
    std::string out;
    EXPECT_TRUE(s.TryGetValue("hello", out));
    EXPECT_EQ(out, "hello");
}

TEST(FrozenSetTest, TryGetValueNotFound) {
    auto s = FrozenSet<std::string>::Create({"hello"});
    std::string out;
    EXPECT_FALSE(s.TryGetValue("missing", out));
}

TEST(FrozenSetTest, GetItems) {
    auto s = FrozenSet<int>::Create({1, 2, 3});
    auto items = s.getItemsProperty();
    EXPECT_EQ(static_cast<int>(items.size()), 3);
}

TEST(FrozenSetTest, CopyTo) {
    auto s = FrozenSet<int>::Create({7});
    std::vector<int> buf(1);
    s.CopyTo(buf, 0);
    ASSERT_EQ(static_cast<int>(buf.size()), 1);
    EXPECT_EQ(buf[0], 7);
}

TEST(FrozenSetTest, CopyTo_TooSmall_Throws) {
    auto s = FrozenSet<int>::Create({1, 2});
    std::vector<int> buf(1);
    EXPECT_THROW(s.CopyTo(buf, 0), System::ArgumentException);
}

TEST(FrozenSetTest, RangeBasedFor) {
    auto s = FrozenSet<int>::Create({1, 2, 3, 4});
    int sum = 0;
    for (const auto& v : s) sum += v;
    EXPECT_EQ(sum, 10);
}

TEST(FrozenSetTest, CreateFromSet) {
    std::unordered_set<int> src = {100, 200};
    auto s = FrozenSet<int>::CreateFromSet(src);
    EXPECT_EQ(s.getCountProperty(), 2);
    EXPECT_TRUE(s.Contains(100));
}

TEST(FrozenSetTest, ToFrozenSetHelper) {
    auto s = ToFrozenSet(std::vector<std::string>{"a", "b"});
    EXPECT_EQ(s.getCountProperty(), 2);
    EXPECT_TRUE(s.Contains("a"));
}
