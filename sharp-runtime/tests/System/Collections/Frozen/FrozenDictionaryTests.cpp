// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Collections/Frozen/FrozenDictionary.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include <string>

using System::Collections::Frozen::FrozenDictionary;
using System::Collections::Frozen::ToFrozenDictionary;

TEST(FrozenDictionaryTest, CreateFromVector) {
    auto d = FrozenDictionary<std::string, int>::Create({{"a", 1}, {"b", 2}});
    EXPECT_EQ(d.getCountProperty(), 2);
}

TEST(FrozenDictionaryTest, EmptySingleton) {
    const auto& e = FrozenDictionary<std::string, int>::getEmptyProperty();
    EXPECT_EQ(e.getCountProperty(), 0);
}

TEST(FrozenDictionaryTest, ContainsKeyTrue) {
    auto d = FrozenDictionary<std::string, int>::Create({{"x", 10}});
    EXPECT_TRUE(d.ContainsKey("x"));
}

TEST(FrozenDictionaryTest, ContainsKeyFalse) {
    auto d = FrozenDictionary<std::string, int>::Create({{"x", 10}});
    EXPECT_FALSE(d.ContainsKey("y"));
}

TEST(FrozenDictionaryTest, GetItemFound) {
    auto d = FrozenDictionary<std::string, int>::Create({{"k", 42}});
    EXPECT_EQ(d.getItem("k"), 42);
}

TEST(FrozenDictionaryTest, GetItemNotFoundThrows) {
    auto d = FrozenDictionary<std::string, int>::Create({{"k", 42}});
    EXPECT_THROW(d.getItem("missing"), System::Collections::Generic::KeyNotFoundException);
}

TEST(FrozenDictionaryTest, TryGetValueFound) {
    auto d = FrozenDictionary<std::string, int>::Create({{"p", 99}});
    int v = 0;
    EXPECT_TRUE(d.TryGetValue("p", v));
    EXPECT_EQ(v, 99);
}

TEST(FrozenDictionaryTest, TryGetValueNotFound) {
    auto d = FrozenDictionary<std::string, int>::Create({{"p", 99}});
    int v = 0;
    EXPECT_FALSE(d.TryGetValue("missing", v));
}

TEST(FrozenDictionaryTest, DuplicateKeyLastWins) {
    auto d = FrozenDictionary<std::string, int>::Create({{"k", 1}, {"k", 2}});
    EXPECT_EQ(d.getItem("k"), 2);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(FrozenDictionaryTest, GetKeysAndValues) {
    auto d = FrozenDictionary<std::string, int>::Create({{"a", 1}, {"b", 2}});
    auto keys = d.getKeysProperty();
    auto vals = d.getValuesProperty();
    EXPECT_EQ(static_cast<int>(keys.size()), 2);
    EXPECT_EQ(static_cast<int>(vals.size()), 2);
}

TEST(FrozenDictionaryTest, CopyTo) {
    auto d = FrozenDictionary<std::string, int>::Create({{"a", 1}});
    std::vector<std::pair<std::string, int>> buf(1);
    d.CopyTo(buf, 0);
    ASSERT_EQ(static_cast<int>(buf.size()), 1);
    EXPECT_EQ(buf[0].first, "a");
    EXPECT_EQ(buf[0].second, 1);
}

TEST(FrozenDictionaryTest, CopyTo_TooSmall_Throws) {
    auto d = FrozenDictionary<std::string, int>::Create({{"a", 1}, {"b", 2}});
    std::vector<std::pair<std::string, int>> buf(1);
    EXPECT_THROW(d.CopyTo(buf, 0), System::ArgumentException);
}

TEST(FrozenDictionaryTest, RangeBasedFor) {
    auto d = FrozenDictionary<std::string, int>::Create({{"a", 10}, {"b", 20}});
    int sum = 0;
    for (const auto& kv : d) sum += kv.second;
    EXPECT_EQ(sum, 30);
}

TEST(FrozenDictionaryTest, ToFrozenDictionaryHelper) {
    std::vector<std::pair<std::string, int>> src = {{"x", 5}, {"y", 6}};
    auto d = ToFrozenDictionary(src);
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_EQ(d.getItem("x"), 5);
}

TEST(FrozenDictionaryTest, CreateFromUnorderedMap) {
    std::unordered_map<std::string, int> m = {{"m", 100}};
    auto d = FrozenDictionary<std::string, int>::CreateFromMap(m);
    EXPECT_EQ(d.getItem("m"), 100);
}
