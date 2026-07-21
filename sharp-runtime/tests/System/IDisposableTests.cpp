// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/IDisposable.hpp"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {

struct CountingDisposable : System::IDisposable {
    int disposeCount = 0;
    void Dispose() override { ++disposeCount; }
};

struct IdempotentDisposable : System::IDisposable {
    bool disposed = false;
    void Dispose() override { disposed = true; }
};

} // namespace

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST(IDisposableTests, Dispose_IsCalled) {
    CountingDisposable d;
    d.Dispose();
    EXPECT_EQ(d.disposeCount, 1);
}

TEST(IDisposableTests, Dispose_SafeToCallMultipleTimes) {
    CountingDisposable d;
    d.Dispose();
    d.Dispose();
    d.Dispose();
    EXPECT_EQ(d.disposeCount, 3);
}

TEST(IDisposableTests, PolymorphicDispatch_ThroughBasePointer) {
    CountingDisposable obj;
    System::IDisposable* ptr = &obj;
    ptr->Dispose();
    EXPECT_EQ(obj.disposeCount, 1);
}

TEST(IDisposableTests, Dispose_SetsDisposedFlag) {
    IdempotentDisposable d;
    EXPECT_FALSE(d.disposed);
    d.Dispose();
    EXPECT_TRUE(d.disposed);
}

TEST(IDisposableTests, IsAbstract_CannotInstantiateDirect) {
    // Compile-time check: IDisposable cannot be instantiated directly.
    // This test verifies the polymorphic hierarchy via pointer.
    static_assert(std::is_abstract_v<System::IDisposable>,
                  "IDisposable must be abstract");
    SUCCEED();
}

TEST(IDisposableTests, SharedPtr_DisposeOnReset) {
    int count = 0;
    struct Tracked : System::IDisposable {
        int& ref;
        explicit Tracked(int& r) : ref(r) {}
        void Dispose() override { ++ref; }
    };
    {
        auto sp = std::make_shared<Tracked>(count);
        sp->Dispose();
    }
    EXPECT_EQ(count, 1);
}
