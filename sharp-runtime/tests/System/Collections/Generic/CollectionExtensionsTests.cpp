// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/CollectionExtensions.hpp"
#include <string>

using System::Collections::Generic::CollectionExtensions;

TEST(CollectionExtensionsTest, GetValueOrDefaultKeyPresent) {
    std::unordered_map<std::string, int> m{{"a", 42}};
    EXPECT_EQ(CollectionExtensions::GetValueOrDefault(m, std::string("a")), 42);
}

TEST(CollectionExtensionsTest, GetValueOrDefaultKeyAbsent) {
    std::unordered_map<std::string, int> m;
    EXPECT_EQ(CollectionExtensions::GetValueOrDefault(m, std::string("x")), 0);
}

TEST(CollectionExtensionsTest, GetValueOrDefaultWithFallback) {
    std::unordered_map<std::string, int> m;
    EXPECT_EQ(CollectionExtensions::GetValueOrDefault(m, std::string("x"), 99), 99);
}

TEST(CollectionExtensionsTest, TryAddSucceeds) {
    std::unordered_map<std::string, int> m;
    EXPECT_TRUE(CollectionExtensions::TryAdd(m, std::string("k"), 1));
    EXPECT_EQ(m.at("k"), 1);
}

TEST(CollectionExtensionsTest, TryAddDuplicateReturnsFalse) {
    std::unordered_map<std::string, int> m{{"k", 1}};
    EXPECT_FALSE(CollectionExtensions::TryAdd(m, std::string("k"), 2));
    EXPECT_EQ(m.at("k"), 1);
}

TEST(CollectionExtensionsTest, RemoveFound) {
    std::unordered_map<std::string, int> m{{"r", 77}};
    int v = 0;
    EXPECT_TRUE(CollectionExtensions::Remove(m, std::string("r"), v));
    EXPECT_EQ(v, 77);
    EXPECT_EQ(static_cast<int>(m.size()), 0);
}

TEST(CollectionExtensionsTest, RemoveNotFound) {
    std::unordered_map<std::string, int> m;
    int v = 0;
    EXPECT_FALSE(CollectionExtensions::Remove(m, std::string("missing"), v));
}

TEST(CollectionExtensionsTest, AsReadOnly) {
    std::vector<int> v{1, 2, 3};
    const auto& ro = CollectionExtensions::AsReadOnly(v);
    EXPECT_EQ(ro.size(), 3u);
    EXPECT_EQ(&ro, &v);
}

TEST(CollectionExtensionsTest, AddRange) {
    std::vector<int> list{1, 2};
    CollectionExtensions::AddRange(list, std::vector<int>{3, 4});
    ASSERT_EQ(static_cast<int>(list.size()), 4);
    EXPECT_EQ(list[2], 3);
    EXPECT_EQ(list[3], 4);
}

TEST(CollectionExtensionsTest, InsertRange) {
    std::vector<int> list{1, 4};
    CollectionExtensions::InsertRange(list, 1, std::vector<int>{2, 3});
    ASSERT_EQ(static_cast<int>(list.size()), 4);
    EXPECT_EQ(list[0], 1);
    EXPECT_EQ(list[1], 2);
    EXPECT_EQ(list[2], 3);
    EXPECT_EQ(list[3], 4);
}
