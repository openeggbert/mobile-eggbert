// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include "System/Lazy.hpp"

using System::Lazy;
using System::LazyThreadSafetyMode;

TEST(LazyTests, DefaultCtor_NotCreated) {
    Lazy<int> lz;
    EXPECT_FALSE(lz.getIsValueCreatedProperty());
}

TEST(LazyTests, DefaultCtor_AccessCreatesDefault) {
    Lazy<int> lz;
    EXPECT_EQ(lz.getValueProperty(), 0);
    EXPECT_TRUE(lz.getIsValueCreatedProperty());
}

TEST(LazyTests, PreInitCtor_IsValueCreatedTrue) {
    Lazy<int> lz(42);
    EXPECT_TRUE(lz.getIsValueCreatedProperty());
}

TEST(LazyTests, PreInitCtor_ValueCorrect) {
    Lazy<int> lz(42);
    EXPECT_EQ(lz.getValueProperty(), 42);
}

TEST(LazyTests, FactoryCtor_NotCreatedBeforeAccess) {
    int calls = 0;
    Lazy<int> lz([&] { ++calls; return 99; });
    EXPECT_EQ(calls, 0);
    EXPECT_FALSE(lz.getIsValueCreatedProperty());
}

TEST(LazyTests, FactoryCtor_ValueCorrectOnAccess) {
    Lazy<int> lz([] { return 7; });
    EXPECT_EQ(lz.getValueProperty(), 7);
}

TEST(LazyTests, FactoryCtor_FactoryCalledOnce) {
    int calls = 0;
    Lazy<int> lz([&] { ++calls; return 1; });
    lz.getValueProperty();
    lz.getValueProperty();
    EXPECT_EQ(calls, 1);
}

TEST(LazyTests, BoolCtor_ThreadSafe_DefaultConstruct) {
    Lazy<std::string> lz(true);
    EXPECT_FALSE(lz.getIsValueCreatedProperty());
    EXPECT_EQ(lz.getValueProperty(), "");
}

TEST(LazyTests, ModeCtor_DefaultConstruct) {
    Lazy<double> lz(LazyThreadSafetyMode::None);
    EXPECT_EQ(lz.getValueProperty(), 0.0);
}

TEST(LazyTests, FactoryBoolCtor_WorksCorrectly) {
    Lazy<int> lz([] { return 55; }, false);
    EXPECT_EQ(lz.getValueProperty(), 55);
}

TEST(LazyTests, FactoryModeCtor_WorksCorrectly) {
    Lazy<int> lz([] { return 33; }, LazyThreadSafetyMode::None);
    EXPECT_EQ(lz.getValueProperty(), 33);
}

TEST(LazyTests, IsValueCreated_AfterAccess_True) {
    Lazy<int> lz([] { return 1; });
    lz.getValueProperty();
    EXPECT_TRUE(lz.getIsValueCreatedProperty());
}

TEST(LazyTests, Value_Alias_SameAsGetValueProperty) {
    Lazy<int> lz([] { return 21; });
    EXPECT_EQ(lz.Value(), lz.getValueProperty());
}

TEST(LazyTests, ToString_BeforeAccess) {
    Lazy<int> lz([] { return 0; });
    EXPECT_NE(lz.ToString().find("not"), std::string::npos);
}

TEST(LazyTests, ToString_AfterAccess) {
    Lazy<int> lz([] { return 0; });
    lz.getValueProperty();
    EXPECT_EQ(lz.ToString().find("not"), std::string::npos);
}

TEST(LazyTests, GetMode_DefaultIsSafe) {
    Lazy<int> lz;
    EXPECT_EQ(lz.getModeProperty(), LazyThreadSafetyMode::ExecutionAndPublication);
}

// --- Fault-caching semantics (matches .NET's LazyHelper: None and
// ExecutionAndPublication cache and rethrow the same exception on every later
// access; PublicationOnly does not cache and retries the factory instead) ---

TEST(LazyTests, ExecutionAndPublication_FaultIsCached_SameExceptionRethrown) {
    int calls = 0;
    Lazy<int> lz([&]() -> int { ++calls; throw std::runtime_error("boom"); }, LazyThreadSafetyMode::ExecutionAndPublication);
    EXPECT_THROW(lz.getValueProperty(), std::runtime_error);
    EXPECT_THROW(lz.getValueProperty(), std::runtime_error);
    EXPECT_EQ(calls, 1); // factory not retried - same fault rethrown
    EXPECT_FALSE(lz.getIsValueCreatedProperty());
}

TEST(LazyTests, None_FaultIsCached_SameExceptionRethrown) {
    int calls = 0;
    Lazy<int> lz([&]() -> int { ++calls; throw std::runtime_error("boom"); }, LazyThreadSafetyMode::None);
    EXPECT_THROW(lz.getValueProperty(), std::runtime_error);
    EXPECT_THROW(lz.getValueProperty(), std::runtime_error);
    EXPECT_EQ(calls, 1);
}

TEST(LazyTests, PublicationOnly_FaultNotCached_FactoryRetried) {
    int calls = 0;
    Lazy<int> lz([&]() -> int {
        ++calls;
        if (calls < 3) throw std::runtime_error("not yet");
        return 42;
    }, LazyThreadSafetyMode::PublicationOnly);
    EXPECT_THROW(lz.getValueProperty(), std::runtime_error);
    EXPECT_THROW(lz.getValueProperty(), std::runtime_error);
    EXPECT_EQ(lz.getValueProperty(), 42); // third attempt succeeds
    EXPECT_EQ(calls, 3);
    EXPECT_TRUE(lz.getIsValueCreatedProperty());
}

TEST(LazyTests, ExecutionAndPublication_SucceedsAfterNoFault) {
    Lazy<int> lz([]() { return 7; }, LazyThreadSafetyMode::ExecutionAndPublication);
    EXPECT_EQ(lz.getValueProperty(), 7);
    EXPECT_EQ(lz.getValueProperty(), 7);
}

// --- Recursive Value() access from within the factory ---

TEST(LazyTests, RecursiveValueAccess_ThrowsInvalidOperationException) {
    Lazy<int>* self = nullptr;
    Lazy<int> lz([&]() -> int { return self->getValueProperty(); });
    self = &lz;
    EXPECT_THROW(lz.getValueProperty(), System::InvalidOperationException);
}

// --- ToString() forwards to the value's own string representation ---

TEST(LazyTests, ToString_ForwardsToValueToString_ForTypesThatHaveIt) {
    struct Widget {
        [[nodiscard]] std::string ToString() const { return "a-widget"; }
    };
    Lazy<Widget> lz([] { return Widget{}; });
    lz.getValueProperty();
    EXPECT_EQ(lz.ToString(), "a-widget");
}
