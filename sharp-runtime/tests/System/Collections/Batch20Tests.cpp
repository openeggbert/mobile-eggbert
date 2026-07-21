// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 20: HybridDictionary
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Collections/Specialized/HybridDictionary.hpp"
#include <any>
#include <string>
#include <vector>

using System::Collections::Specialized::HybridDictionary;

TEST(HybridDictionaryBatch20Test, DefaultCtor_Empty) {
    HybridDictionary d;
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(HybridDictionaryBatch20Test, Ctor_Bool) {
    HybridDictionary d(true);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(HybridDictionaryBatch20Test, Ctor_Int) {
    HybridDictionary d(10);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(HybridDictionaryBatch20Test, Ctor_IntBool) {
    HybridDictionary d(10, false);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(HybridDictionaryBatch20Test, AddAndCount) {
    HybridDictionary d;
    d.Add("key1", std::any(42));
    d.Add("key2", std::any(std::string("hello")));
    EXPECT_EQ(d.getCountProperty(), 2);
}

TEST(HybridDictionaryBatch20Test, Contains_TrueAndFalse) {
    HybridDictionary d;
    d.Add("present", std::any(1));
    EXPECT_TRUE(d.Contains("present"));
    EXPECT_FALSE(d.Contains("absent"));
}

TEST(HybridDictionaryBatch20Test, Indexer_Found) {
    HybridDictionary d;
    d.Add("x", std::any(99));
    auto v = d["x"];
    EXPECT_TRUE(v.has_value());
    EXPECT_EQ(std::any_cast<int>(v), 99);
}

TEST(HybridDictionaryBatch20Test, Indexer_Missing_ReturnsEmpty) {
    HybridDictionary d;
    auto v = d["missing"];
    EXPECT_FALSE(v.has_value());
}

TEST(HybridDictionaryBatch20Test, SetOverwrites) {
    HybridDictionary d;
    d.Add("k", std::any(1));
    d.set("k", std::any(2));
    EXPECT_EQ(std::any_cast<int>(d["k"]), 2);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(HybridDictionaryBatch20Test, Remove) {
    HybridDictionary d;
    d.Add("a", std::any(1));
    d.Add("b", std::any(2));
    d.Remove("a");
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.Contains("a"));
    EXPECT_TRUE(d.Contains("b"));
}

TEST(HybridDictionaryBatch20Test, Clear) {
    HybridDictionary d;
    d.Add("a", std::any(1));
    d.Clear();
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(HybridDictionaryBatch20Test, Properties_Fixed_ReadOnly_Sync) {
    HybridDictionary d;
    EXPECT_FALSE(d.getIsFixedSizeProperty());
    EXPECT_FALSE(d.getIsReadOnlyProperty());
    EXPECT_FALSE(d.getIsSynchronizedProperty());
    EXPECT_NE(d.getSyncRootProperty(), nullptr);
}

TEST(HybridDictionaryBatch20Test, Keys_ContainsAll) {
    HybridDictionary d;
    d.Add("alpha", std::any(1));
    d.Add("beta", std::any(2));
    auto keys = d.getKeysProperty();
    EXPECT_EQ(keys.size(), 2u);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "alpha") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "beta") != keys.end());
}

TEST(HybridDictionaryBatch20Test, Values_ContainsAll) {
    HybridDictionary d;
    d.Add("a", std::any(10));
    d.Add("b", std::any(20));
    auto vals = d.getValuesProperty();
    EXPECT_EQ(vals.size(), 2u);
}

TEST(HybridDictionaryBatch20Test, STLIteration) {
    HybridDictionary d;
    d.Add("x", std::any(5));
    d.Add("y", std::any(10));
    int count = 0;
    for (const auto& [k, v] : d) { (void)k; (void)v; ++count; }
    EXPECT_EQ(count, 2);
}

TEST(HybridDictionaryBatch20Test, Add_DuplicateKey_ThrowsArgumentException) {
    HybridDictionary d;
    d.Add("key", std::any(1));
    EXPECT_THROW(d.Add("key", std::any(2)), System::ArgumentException);
    EXPECT_EQ(std::any_cast<int>(d["key"]), 1);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(HybridDictionaryBatch20Test, CaseInsensitive_TreatsKeysAsEqual) {
    HybridDictionary d(true);
    d.Add("Key", std::any(std::string("value")));
    EXPECT_TRUE(d.Contains("KEY"));
    EXPECT_TRUE(d.Contains("key"));
    EXPECT_EQ(std::any_cast<std::string>(d["KEY"]), "value");
    EXPECT_THROW(d.Add("kEy", std::any(std::string("other"))), System::ArgumentException);

    d.set("kEy", std::any(std::string("updated")));
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_EQ(std::any_cast<std::string>(d["Key"]), "updated");

    d.Remove("KEY");
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(HybridDictionaryBatch20Test, CaseSensitive_TreatsKeysAsDistinct) {
    HybridDictionary d(false);
    d.Add("Key", std::any(1));
    d.Add("KEY", std::any(2));
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_EQ(std::any_cast<int>(d["Key"]), 1);
    EXPECT_EQ(std::any_cast<int>(d["KEY"]), 2);
}
