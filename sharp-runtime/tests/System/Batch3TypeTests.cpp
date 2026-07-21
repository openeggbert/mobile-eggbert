// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
// Batch 3 non-exception type tests:
//   MidpointRounding, UInt128, MarshalByRefObject, EventHandler,
//   ReadOnlyMemory, Activator, ThreadStaticAttribute, FlagsAttribute
#include <gtest/gtest.h>
#include <limits>
#include "System/MidpointRounding.hpp"
#include "System/UInt128.hpp"
#include "System/MarshalByRefObject.hpp"
#include "System/EventHandler.hpp"
#include "System/EventArgs.hpp"
#include "System/ReadOnlyMemory.hpp"
#include "System/Activator.hpp"
#include "System/ThreadStaticAttribute.hpp"
#include "System/FlagsAttribute.hpp"
#include "System/Attribute.hpp"

// ---------------------------------------------------------------------------
// MidpointRounding (extra tests; basic ToEven/AwayFromZero/ToPositiveInfinity
//   are in SystemTypesRemainingTests.cpp)
// ---------------------------------------------------------------------------
TEST(MidpointRoundingNewTests, ToZero_IsTwo) {
    EXPECT_EQ(static_cast<int>(System::MidpointRounding::ToZero), 2);
}
TEST(MidpointRoundingNewTests, ToNegativeInfinity_IsThree) {
    EXPECT_EQ(static_cast<int>(System::MidpointRounding::ToNegativeInfinity), 3);
}
TEST(MidpointRoundingNewTests, ToEven_NeToZero) {
    EXPECT_NE(System::MidpointRounding::ToEven, System::MidpointRounding::ToZero);
}

// ---------------------------------------------------------------------------
// UInt128 (extra tests; basic arithmetic/ToString/comparison in Task40Tests.cpp)
// ---------------------------------------------------------------------------
TEST(UInt128NewTests, BitwiseAnd) {
    System::UInt128 a(0xFF), b(0x0F);
    EXPECT_EQ(static_cast<unsigned long long>(a & b), 0x0FULL);
}
TEST(UInt128NewTests, BitwiseOr) {
    System::UInt128 a(0xF0), b(0x0F);
    EXPECT_EQ(static_cast<unsigned long long>(a | b), 0xFFULL);
}
TEST(UInt128NewTests, BitwiseXor) {
    System::UInt128 a(0xFF), b(0x0F);
    EXPECT_EQ(static_cast<unsigned long long>(a ^ b), 0xF0ULL);
}
TEST(UInt128NewTests, ShiftLeft) {
    System::UInt128 a(1);
    EXPECT_EQ(static_cast<unsigned long long>(a << 4), 16ULL);
}
TEST(UInt128NewTests, ShiftRight) {
    System::UInt128 a(256);
    EXPECT_EQ(static_cast<unsigned long long>(a >> 4), 16ULL);
}
TEST(UInt128NewTests, GetUpperLower_FromParts) {
    System::UInt128 v(0xABCDEF01ULL, 0x12345678ULL);
    EXPECT_EQ(v.getUpperProperty(), 0xABCDEF01ULL);
    EXPECT_EQ(v.getLowerProperty(), 0x12345678ULL);
}
TEST(UInt128NewTests, MinValue_IsZero) {
    EXPECT_EQ(static_cast<unsigned long long>(System::UInt128::MinValue()), 0ULL);
}
TEST(UInt128NewTests, MaxValue_UpperIsMax) {
    EXPECT_EQ(System::UInt128::MaxValue().getUpperProperty(), 0xFFFFFFFFFFFFFFFFULL);
}
TEST(UInt128NewTests, Modulo) {
    System::UInt128 a(10), b(3);
    EXPECT_EQ(static_cast<unsigned long long>(a % b), 1ULL);
}
TEST(UInt128NewTests, Subtraction) {
    System::UInt128 a(10), b(3);
    EXPECT_EQ(static_cast<unsigned long long>(a - b), 7ULL);
}

// ---------------------------------------------------------------------------
// MarshalByRefObject (extra; Task42Tests has Instantiation_NoThrow)
// ---------------------------------------------------------------------------
TEST(MarshalByRefObjectNewTests, PolymorphicDeletion_NoLeak) {
    struct Derived : System::MarshalByRefObject {};
    std::unique_ptr<System::MarshalByRefObject> p = std::make_unique<Derived>();
    EXPECT_NE(p.get(), nullptr);
}
TEST(MarshalByRefObjectNewTests, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(System::MarshalByRefObject obj);
}

// ---------------------------------------------------------------------------
// EventHandler<TEventArgs>
// ---------------------------------------------------------------------------
TEST(EventHandlerTests, Add_And_Raise_CallsHandler) {
    System::EventHandler<System::EventArgs> eh;
    bool called = false;
    eh += [&](System::Object*, const System::EventArgs&) { called = true; };
    eh.Raise(nullptr, System::EventArgs{});
    EXPECT_TRUE(called);
}
TEST(EventHandlerTests, MultipleHandlers_AllCalled) {
    System::EventHandler<System::EventArgs> eh;
    int count = 0;
    eh += [&](System::Object*, const System::EventArgs&) { ++count; };
    eh += [&](System::Object*, const System::EventArgs&) { ++count; };
    eh.Raise(nullptr, System::EventArgs{});
    EXPECT_EQ(count, 2);
}
TEST(EventHandlerTests, Remove_HandlerNotCalledAfterRemove) {
    System::EventHandler<System::EventArgs> eh;
    bool called = false;
    auto token = eh.Add([&](System::Object*, const System::EventArgs&) { called = true; });
    eh.Remove(token);
    eh.Raise(nullptr, System::EventArgs{});
    EXPECT_FALSE(called);
}
TEST(EventHandlerTests, Clear_NoHandlersCalled) {
    System::EventHandler<System::EventArgs> eh;
    int count = 0;
    eh += [&](System::Object*, const System::EventArgs&) { ++count; };
    eh.Clear();
    eh.Raise(nullptr, System::EventArgs{});
    EXPECT_EQ(count, 0);
}
TEST(EventHandlerTests, Empty_TrueWhenNoHandlers) {
    System::EventHandler<System::EventArgs> eh;
    EXPECT_TRUE(eh.Empty());
}
TEST(EventHandlerTests, Size_ReflectsHandlerCount) {
    System::EventHandler<System::EventArgs> eh;
    eh += [](System::Object*, const System::EventArgs&) {};
    eh += [](System::Object*, const System::EventArgs&) {};
    EXPECT_EQ(eh.Size(), 2u);
}
TEST(EventHandlerTests, Invoke_SameAsRaise) {
    System::EventHandler<System::EventArgs> eh;
    int count = 0;
    eh += [&](System::Object*, const System::EventArgs&) { ++count; };
    eh.Invoke(nullptr, System::EventArgs{});
    EXPECT_EQ(count, 1);
}

// ---------------------------------------------------------------------------
// ReadOnlyMemory<T>
// ---------------------------------------------------------------------------
TEST(ReadOnlyMemoryTests, DefaultCtor_IsEmpty) {
    System::ReadOnlyMemory<int> m;
    EXPECT_TRUE(m.getIsEmptyProperty());
    EXPECT_EQ(m.getLengthProperty(), 0);
}
TEST(ReadOnlyMemoryTests, ConstructFromVector_LengthCorrect) {
    std::vector<int> v = {1, 2, 3};
    System::ReadOnlyMemory<int> m(v);
    EXPECT_EQ(m.getLengthProperty(), 3);
}
TEST(ReadOnlyMemoryTests, ElementAccess_CorrectValue) {
    std::vector<int> v = {10, 20, 30};
    System::ReadOnlyMemory<int> m(v);
    EXPECT_EQ(m[0], 10);
    EXPECT_EQ(m[2], 30);
}
TEST(ReadOnlyMemoryTests, OutOfRange_Throws) {
    std::vector<int> v = {1};
    System::ReadOnlyMemory<int> m(v);
    EXPECT_THROW(m[5], System::ArgumentOutOfRangeException);
}
TEST(ReadOnlyMemoryTests, Slice_CorrectLength) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    System::ReadOnlyMemory<int> m(v);
    auto s = m.Slice(1, 3);
    EXPECT_EQ(s.getLengthProperty(), 3);
    EXPECT_EQ(s[0], 2);
}
TEST(ReadOnlyMemoryTests, ToArray_CopiesData) {
    std::vector<int> v = {7, 8, 9};
    System::ReadOnlyMemory<int> m(v);
    auto arr = m.ToArray();
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[1], 8);
}
TEST(ReadOnlyMemoryTests, Empty_IsEmpty) {
    auto e = System::ReadOnlyMemory<int>::getEmptyProperty();
    EXPECT_TRUE(e.getIsEmptyProperty());
}

// ---------------------------------------------------------------------------
// Activator
// ---------------------------------------------------------------------------
struct DefaultCtorStruct { int x = 42; };

TEST(ActivatorTests, CreateInstance_DefaultConstructible) {
    auto v = System::Activator::CreateInstance<DefaultCtorStruct>();
    EXPECT_EQ(v.x, 42);
}
TEST(ActivatorTests, CreateInstance_Int_IsZero) {
    auto v = System::Activator::CreateInstance<int>();
    EXPECT_EQ(v, 0);
}
TEST(ActivatorTests, CreateInstancePtr_NotNull) {
    auto p = System::Activator::CreateInstancePtr<DefaultCtorStruct>();
    EXPECT_NE(p.get(), nullptr);
    EXPECT_EQ(p->x, 42);
}
TEST(ActivatorTests, CreateInstancePtr_String_Empty) {
    auto p = System::Activator::CreateInstancePtr<std::string>();
    EXPECT_NE(p.get(), nullptr);
    EXPECT_TRUE(p->empty());
}
TEST(ActivatorTests, CreateInstance_WithArgs_SetsValue) {
    struct Point { int x, y; Point(int a, int b) : x(a), y(b) {} };
    auto p = System::Activator::CreateInstance<Point>(3, 7);
    EXPECT_EQ(p.x, 3);
    EXPECT_EQ(p.y, 7);
}
TEST(ActivatorTests, CreateInstancePtr_WithArgs_NotNull) {
    struct Point { int x, y; Point(int a, int b) : x(a), y(b) {} };
    auto p = System::Activator::CreateInstancePtr<Point>(5, 9);
    ASSERT_NE(p.get(), nullptr);
    EXPECT_EQ(p->x, 5);
    EXPECT_EQ(p->y, 9);
}
TEST(ActivatorTests, CreateInstance_String_WithArg) {
    auto s = System::Activator::CreateInstance<std::string>(std::string("hello"));
    EXPECT_EQ(s, "hello");
}

// ---------------------------------------------------------------------------
// ThreadStaticAttribute
// ---------------------------------------------------------------------------
TEST(ThreadStaticAttributeNewTests, IsA_Attribute) {
    System::ThreadStaticAttribute attr;
    EXPECT_NE(dynamic_cast<System::Attribute*>(&attr), nullptr);
}

// ---------------------------------------------------------------------------
// FlagsAttribute
// ---------------------------------------------------------------------------
TEST(FlagsAttributeNewTests, IsA_Attribute) {
    System::FlagsAttribute attr;
    EXPECT_NE(dynamic_cast<System::Attribute*>(&attr), nullptr);
}
TEST(FlagsAttributeNewTests, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(System::FlagsAttribute f);
}
