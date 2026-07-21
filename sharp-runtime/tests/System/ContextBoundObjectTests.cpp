// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <memory>
#include "System/ContextBoundObject.hpp"
#include "System/MarshalByRefObject.hpp"

namespace {
    // Minimal concrete subclass for testing
    class ConcreteContext : public System::ContextBoundObject {
    public:
        ConcreteContext() = default;
        int value = 42;
    };
}

// ---------------------------------------------------------------------------
// Instantiation
// ---------------------------------------------------------------------------

TEST(ContextBoundObjectTests, Subclass_CanBeInstantiated) {
    ConcreteContext obj;
    EXPECT_EQ(obj.value, 42);
}

TEST(ContextBoundObjectTests, Subclass_CanBeHeapAllocated) {
    auto obj = std::make_unique<ConcreteContext>();
    EXPECT_NE(obj, nullptr);
}

// ---------------------------------------------------------------------------
// Inheritance hierarchy
// ---------------------------------------------------------------------------

TEST(ContextBoundObjectTests, IsA_MarshalByRefObject) {
    ConcreteContext obj;
    EXPECT_NE(dynamic_cast<System::MarshalByRefObject*>(&obj), nullptr);
}

TEST(ContextBoundObjectTests, IsA_ContextBoundObject) {
    ConcreteContext obj;
    EXPECT_NE(dynamic_cast<System::ContextBoundObject*>(&obj), nullptr);
}

TEST(ContextBoundObjectTests, PolyDelete_ViaMarshalByRefObject) {
    std::unique_ptr<System::MarshalByRefObject> p = std::make_unique<ConcreteContext>();
    EXPECT_NE(p, nullptr);
}

TEST(ContextBoundObjectTests, PolyDelete_ViaContextBoundObject) {
    std::unique_ptr<System::ContextBoundObject> p = std::make_unique<ConcreteContext>();
    EXPECT_NE(p, nullptr);
}
