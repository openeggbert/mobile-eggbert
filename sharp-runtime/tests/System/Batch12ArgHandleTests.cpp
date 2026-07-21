// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/RuntimeArgumentHandle.hpp"
#include "System/ArgIterator.hpp"

// ===========================================================================
// RuntimeArgumentHandle
// ===========================================================================

TEST(RuntimeArgumentHandleTests, DefaultConstruct_DoesNotThrow) {
    EXPECT_NO_THROW(System::RuntimeArgumentHandle{});
}

TEST(RuntimeArgumentHandleTests, IsDefaultConstructible) {
    System::RuntimeArgumentHandle h;
    (void)h;
    SUCCEED();
}

TEST(RuntimeArgumentHandleTests, IsCopyConstructible) {
    System::RuntimeArgumentHandle h1;
    System::RuntimeArgumentHandle h2 = h1;
    (void)h2;
    SUCCEED();
}

// ===========================================================================
// ArgIterator
// ===========================================================================

TEST(ArgIteratorTests, Constructor_Throws) {
    System::RuntimeArgumentHandle h;
    EXPECT_THROW(System::ArgIterator it(h), System::NotSupportedException);
}

TEST(ArgIteratorTests, Constructor_WithPtr_Throws) {
    System::RuntimeArgumentHandle h;
    EXPECT_THROW(System::ArgIterator it(h, nullptr), System::NotSupportedException);
}

TEST(ArgIteratorTests, End_DoesNotThrow) {
    // End() is a no-op — we cannot construct a valid ArgIterator,
    // so we test via a default-initialised one using placement tricks.
    // Instead just verify End is callable via a default struct.
    alignas(System::ArgIterator) char buf[sizeof(System::ArgIterator)] = {};
    auto* it = reinterpret_cast<System::ArgIterator*>(buf);
    EXPECT_NO_THROW(it->End());
}

TEST(ArgIteratorTests, GetHashCode_ReturnsZero) {
    alignas(System::ArgIterator) char buf[sizeof(System::ArgIterator)] = {};
    auto* it = reinterpret_cast<System::ArgIterator*>(buf);
    EXPECT_EQ(it->GetHashCode(), 0);
}

TEST(ArgIteratorTests, Equals_ReturnsFalse) {
    alignas(System::ArgIterator) char buf1[sizeof(System::ArgIterator)] = {};
    alignas(System::ArgIterator) char buf2[sizeof(System::ArgIterator)] = {};
    auto* a = reinterpret_cast<System::ArgIterator*>(buf1);
    auto* b = reinterpret_cast<System::ArgIterator*>(buf2);
    EXPECT_FALSE(a->Equals(*b));
}

TEST(ArgIteratorTests, GetNextArg_Throws) {
    alignas(System::ArgIterator) char buf[sizeof(System::ArgIterator)] = {};
    auto* it = reinterpret_cast<System::ArgIterator*>(buf);
    EXPECT_THROW(it->GetNextArg(), System::NotSupportedException);
}

TEST(ArgIteratorTests, GetNextArgType_Throws) {
    alignas(System::ArgIterator) char buf[sizeof(System::ArgIterator)] = {};
    auto* it = reinterpret_cast<System::ArgIterator*>(buf);
    EXPECT_THROW(it->GetNextArgType(), System::NotSupportedException);
}

TEST(ArgIteratorTests, GetRemainingCount_Throws) {
    alignas(System::ArgIterator) char buf[sizeof(System::ArgIterator)] = {};
    auto* it = reinterpret_cast<System::ArgIterator*>(buf);
    EXPECT_THROW(it->GetRemainingCount(), System::NotSupportedException);
}
