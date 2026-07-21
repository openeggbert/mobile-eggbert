// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
// Minimal tests for pure interfaces: IAsyncDisposable, IAsyncResult, ICloneable,
// IComparable, IConvertible, ICustomFormatter
#include <any>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "System/IAsyncDisposable.hpp"
#include "System/IAsyncResult.hpp"
#include "System/ICloneable.hpp"
#include "System/IComparable.hpp"
#include "System/ICustomFormatter.hpp"
#include "System/Threading/EventWaitHandle.hpp"

// ---------------------------------------------------------------------------
// IAsyncDisposable
// ---------------------------------------------------------------------------
struct MyDisposable : public System::IAsyncDisposable {
    bool disposed = false;
    System::Threading::Tasks::ValueTask DisposeAsync() override {
        disposed = true;
        return System::Threading::Tasks::ValueTask::CompletedTask();
    }
};

TEST(IAsyncDisposableTests2, DisposeAsync_IsCalled) {
    MyDisposable d;
    d.DisposeAsync();
    EXPECT_TRUE(d.disposed);
}

TEST(IAsyncDisposableTests2, IsPolymorphic) {
    MyDisposable d;
    System::IAsyncDisposable* p = &d;
    p->DisposeAsync();
    EXPECT_TRUE(d.disposed);
}

// ---------------------------------------------------------------------------
// IAsyncResult
// ---------------------------------------------------------------------------
struct SyncResult : public System::IAsyncResult {
    std::any state;
    mutable System::Threading::EventWaitHandle waitHandle{true, System::Threading::EventResetMode::ManualReset};
    bool getIsCompletedProperty() const override { return true; }
    bool getCompletedSynchronouslyProperty() const override { return true; }
    const std::any& getAsyncStateProperty() const override { return state; }
    System::Threading::WaitHandle& getAsyncWaitHandleProperty() const override { return waitHandle; }
};

TEST(IAsyncResultTests2, IsCompleted_ReturnsTrue) {
    SyncResult r;
    EXPECT_TRUE(r.getIsCompletedProperty());
}

TEST(IAsyncResultTests2, CompletedSynchronously_ReturnsTrue) {
    SyncResult r;
    EXPECT_TRUE(r.getCompletedSynchronouslyProperty());
}

TEST(IAsyncResultTests2, AsyncState_DefaultsToEmpty) {
    SyncResult r;
    EXPECT_FALSE(r.getAsyncStateProperty().has_value());
}

TEST(IAsyncResultTests2, AsyncState_HoldsAssignedValue) {
    SyncResult r;
    r.state = std::string("payload");
    EXPECT_EQ(std::any_cast<std::string>(r.getAsyncStateProperty()), "payload");
}

TEST(IAsyncResultTests2, AsyncWaitHandle_IsAccessible) {
    SyncResult r;
    EXPECT_NO_THROW(r.getAsyncWaitHandleProperty());
}

// ---------------------------------------------------------------------------
// ICloneable
// ---------------------------------------------------------------------------
struct CloneableInt : public System::ICloneable {
    int value;
    explicit CloneableInt(int v) : value(v) {}
    std::shared_ptr<ICloneable> Clone() const override {
        return std::make_shared<CloneableInt>(value);
    }
};

TEST(ICloneableTests2, Clone_ReturnsNewInstance) {
    CloneableInt c(42);
    auto cloned = c.Clone();
    auto* p = dynamic_cast<CloneableInt*>(cloned.get());
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->value, 42);
    EXPECT_NE(&c, p);
}

// ---------------------------------------------------------------------------
// IComparable
// ---------------------------------------------------------------------------
struct ComparableInt : public System::IComparable<ComparableInt> {
    int v;
    explicit ComparableInt(int x) : v(x) {}
    int CompareTo(const ComparableInt& o) const override { return v - o.v; }
};

TEST(IComparableTests2, CompareTo_Less_ReturnsNegative) {
    ComparableInt a(1), b(2);
    EXPECT_LT(a.CompareTo(b), 0);
}

TEST(IComparableTests2, CompareTo_Equal_ReturnsZero) {
    ComparableInt a(5), b(5);
    EXPECT_EQ(a.CompareTo(b), 0);
}

TEST(IComparableTests2, CompareTo_Greater_ReturnsPositive) {
    ComparableInt a(3), b(1);
    EXPECT_GT(a.CompareTo(b), 0);
}

// ---------------------------------------------------------------------------
// ICustomFormatter
// ---------------------------------------------------------------------------
struct UpperFormatter : public System::ICustomFormatter {
    std::string Format(const std::string& fmt, const void* /*arg*/,
                       const System::IFormatProvider* /*fp*/) const override {
        std::string r = fmt;
        for (auto& c : r) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return r;
    }
};

TEST(ICustomFormatterTests2, Format_UppercasesInput) {
    UpperFormatter f;
    EXPECT_EQ(f.Format("hello", nullptr, nullptr), "HELLO");
}
