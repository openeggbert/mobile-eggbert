// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/DictionaryEntry.hpp"
#include <any>
#include <string>

using System::Collections::DictionaryEntry;

TEST(DictionaryEntryTest, DefaultCtor) {
    DictionaryEntry de;
    EXPECT_FALSE(de.getKeyProperty().has_value());
    EXPECT_FALSE(de.getValueProperty().has_value());
}

TEST(DictionaryEntryTest, CtorSetsKeyAndValue) {
    DictionaryEntry de(std::any(42), std::any(std::string("hello")));
    EXPECT_EQ(std::any_cast<int>(de.getKeyProperty()), 42);
    EXPECT_EQ(std::any_cast<std::string>(de.getValueProperty()), "hello");
}

TEST(DictionaryEntryTest, SetKeyProperty) {
    DictionaryEntry de;
    de.setKeyProperty(std::any(std::string("key1")));
    EXPECT_EQ(std::any_cast<std::string>(de.getKeyProperty()), "key1");
}

TEST(DictionaryEntryTest, SetValueProperty) {
    DictionaryEntry de;
    de.setValueProperty(std::any(99));
    EXPECT_EQ(std::any_cast<int>(de.getValueProperty()), 99);
}

TEST(DictionaryEntryTest, Deconstruct) {
    DictionaryEntry de(std::any(7), std::any(std::string("world")));
    std::any k, v;
    de.Deconstruct(k, v);
    EXPECT_EQ(std::any_cast<int>(k), 7);
    EXPECT_EQ(std::any_cast<std::string>(v), "world");
}

TEST(DictionaryEntryTest, ToStringNotEmpty) {
    DictionaryEntry de(std::any(1), std::any(std::string("x")));
    EXPECT_FALSE(de.ToString().empty());
}

TEST(DictionaryEntryTest, OverwriteValue) {
    DictionaryEntry de(std::any(1), std::any(10));
    de.setValueProperty(std::any(20));
    EXPECT_EQ(std::any_cast<int>(de.getValueProperty()), 20);
}
