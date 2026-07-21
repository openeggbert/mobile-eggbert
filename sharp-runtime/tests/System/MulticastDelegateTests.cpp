// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/MulticastDelegate.hpp"
#include "System/NotImplementedException.hpp"

using System::Delegate;
using System::MulticastDelegate;

// ---------------------------------------------------------------------------
// Inheritance
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, IsA_Delegate) {
    auto d = std::make_shared<MulticastDelegate>([]{});
    EXPECT_NE(dynamic_cast<Delegate*>(d.get()), nullptr);
}

// ---------------------------------------------------------------------------
// Construction and Invoke
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, DefaultCtor_Invoke_NoOp) {
    MulticastDelegate d;
    EXPECT_NO_THROW(d.Invoke());
}

TEST(MulticastDelegateTests, SingleTarget_Invoke_CallsLambda) {
    int count = 0;
    MulticastDelegate d([&]{ ++count; });
    d.Invoke();
    EXPECT_EQ(count, 1);
}

TEST(MulticastDelegateTests, OperatorCall_CallsLambda) {
    int count = 0;
    MulticastDelegate d([&]{ ++count; });
    d();
    EXPECT_EQ(count, 1);
}

// ---------------------------------------------------------------------------
// Combine — produces shared Delegate whose targets all fire
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, Combine_TwoTargets_BothInvoked) {
    int a = 0, b = 0;
    auto d1 = std::make_shared<MulticastDelegate>([&]{ ++a; });
    auto d2 = std::make_shared<MulticastDelegate>([&]{ ++b; });
    auto combined = Delegate::Combine(d1, d2);
    ASSERT_TRUE(combined);
    combined->Invoke();
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);
}

TEST(MulticastDelegateTests, Combine_InvocationListSize) {
    auto d1 = std::make_shared<MulticastDelegate>([]{});
    auto d2 = std::make_shared<MulticastDelegate>([]{});
    auto d3 = std::make_shared<MulticastDelegate>([]{});
    auto combined = Delegate::Combine(Delegate::Combine(d1, d2), d3);
    EXPECT_EQ(combined->GetInvocationList().size(), 3u);
}

// ---------------------------------------------------------------------------
// Equals — list-based comparison
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, Equals_SamePointer_True) {
    auto d = std::make_shared<MulticastDelegate>([]{});
    EXPECT_TRUE(d->Equals(*d));
}

TEST(MulticastDelegateTests, Equals_DifferentSingleTarget_False) {
    auto d1 = std::make_shared<MulticastDelegate>([]{});
    auto d2 = std::make_shared<MulticastDelegate>([]{});
    EXPECT_FALSE(d1->Equals(*d2));
}

TEST(MulticastDelegateTests, Equals_SameListPointers_True) {
    // Combine(t1,t2) twice: both results have the same two pointer entries
    auto t1 = std::make_shared<MulticastDelegate>([]{});
    auto t2 = std::make_shared<MulticastDelegate>([]{});
    auto c1 = Delegate::Combine(t1, t2);
    auto c2 = Delegate::Combine(t1, t2);
    ASSERT_TRUE(c1 && c2);
    // Delegate::Equals now compares invocation lists element-by-element
    EXPECT_TRUE(c1->Equals(*c2));
}

TEST(MulticastDelegateTests, Equals_DifferentListPointers_False) {
    auto t1 = std::make_shared<MulticastDelegate>([]{});
    auto t2 = std::make_shared<MulticastDelegate>([]{});
    auto t3 = std::make_shared<MulticastDelegate>([]{});
    auto c1 = Delegate::Combine(t1, t2);
    auto c2 = Delegate::Combine(t1, t3);
    ASSERT_TRUE(c1 && c2);
    EXPECT_FALSE(c1->Equals(*c2));
}

namespace {
    void FreeFunctionForDelegateEqualsTest() {}
    void OtherFreeFunctionForDelegateEqualsTest() {}
}

// Regression for ticket 345: real .NET's Delegate.Equals compares single-cast delegates by
// (Method, Target) value-equality, not by object identity -- two distinct delegate instances
// wrapping the same free function ARE Equals() in .NET. The port previously only ever returned
// true for the literal same object. A std::function generically can't be compared this way for
// arbitrary callables (no operator== for lambdas/closures), but the one mechanically comparable
// case -- both wrapping the identical plain function pointer -- is now handled.
TEST(MulticastDelegateTests, Equals_SamePlainFunctionPointer_DifferentInstances_True) {
    auto d1 = std::make_shared<MulticastDelegate>(FreeFunctionForDelegateEqualsTest);
    auto d2 = std::make_shared<MulticastDelegate>(FreeFunctionForDelegateEqualsTest);
    EXPECT_TRUE(d1->Equals(*d2));
    EXPECT_TRUE(*d1 == *d2);
}

TEST(MulticastDelegateTests, Equals_DifferentPlainFunctionPointers_False) {
    auto d1 = std::make_shared<MulticastDelegate>(FreeFunctionForDelegateEqualsTest);
    auto d2 = std::make_shared<MulticastDelegate>(OtherFreeFunctionForDelegateEqualsTest);
    EXPECT_FALSE(d1->Equals(*d2));
}

// GetHashCode() previously hashed std::function::target<>()'s return value directly -- the
// internal storage slot's address, not the wrapped function pointer's value -- so it never
// actually produced equal hashes for two Delegate objects wrapping the same free function,
// despite Equals() (after the fix above) now considering them equal, which the hash contract
// requires.
TEST(MulticastDelegateTests, GetHashCode_SamePlainFunctionPointer_DifferentInstances_Equal) {
    auto d1 = std::make_shared<MulticastDelegate>(FreeFunctionForDelegateEqualsTest);
    auto d2 = std::make_shared<MulticastDelegate>(FreeFunctionForDelegateEqualsTest);
    EXPECT_EQ(d1->GetHashCode(), d2->GetHashCode());
}

TEST(MulticastDelegateTests, Equals_DifferentListLength_False) {
    auto t1 = std::make_shared<MulticastDelegate>([]{});
    auto t2 = std::make_shared<MulticastDelegate>([]{});
    auto single = t1;
    auto combined = Delegate::Combine(t1, t2);
    ASSERT_TRUE(combined);
    EXPECT_FALSE(single->Equals(*combined));
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, GetHashCode_SameObject_Consistent) {
    // Must be heap-allocated so GetInvocationList can call shared_from_this
    auto d = std::make_shared<MulticastDelegate>([]{});
    EXPECT_EQ(d->GetHashCode(), d->GetHashCode());
}

TEST(MulticastDelegateTests, GetHashCode_Empty_IsZero) {
    // Empty (no-target) delegate: hash is 0
    MulticastDelegate d;
    EXPECT_EQ(d.GetHashCode(), 0u);
}

TEST(MulticastDelegateTests, GetHashCode_Multicast_Consistent) {
    auto t1 = std::make_shared<MulticastDelegate>([]{});
    auto t2 = std::make_shared<MulticastDelegate>([]{});
    auto combined = Delegate::Combine(t1, t2);
    ASSERT_TRUE(combined);
    EXPECT_EQ(combined->GetHashCode(), combined->GetHashCode());
}

TEST(MulticastDelegateTests, GetHashCode_DifferentObjects_LikelyDifferent) {
    auto d1 = std::make_shared<MulticastDelegate>([]{});
    auto d2 = std::make_shared<MulticastDelegate>([]{});
    // Two distinct single-target delegates almost certainly have different hashes
    // (same pointer would be pathological — just check they're each consistent)
    EXPECT_EQ(d1->GetHashCode(), d1->GetHashCode());
    EXPECT_EQ(d2->GetHashCode(), d2->GetHashCode());
}

// ---------------------------------------------------------------------------
// Clone
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, Clone_IsMulticastDelegate) {
    auto d = std::make_shared<MulticastDelegate>([]{});
    auto cloned = d->Clone();
    ASSERT_TRUE(cloned);
    EXPECT_NE(dynamic_cast<MulticastDelegate*>(cloned.get()), nullptr);
}

TEST(MulticastDelegateTests, Clone_InvokesTarget) {
    int count = 0;
    auto d = std::make_shared<MulticastDelegate>([&]{ ++count; });
    auto cloned = d->Clone();
    cloned->Invoke();
    EXPECT_EQ(count, 1);
}

// ---------------------------------------------------------------------------
// HasSingleTarget
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, SingleTarget_HasSingleTarget_True) {
    auto d = std::make_shared<MulticastDelegate>([]{});
    EXPECT_TRUE(d->getHasSingleTargetProperty());
}

TEST(MulticastDelegateTests, MultiTarget_HasSingleTarget_False) {
    auto d1 = std::make_shared<MulticastDelegate>([]{});
    auto d2 = std::make_shared<MulticastDelegate>([]{});
    auto combined = Delegate::Combine(d1, d2);
    EXPECT_FALSE(combined->getHasSingleTargetProperty());
}

// ---------------------------------------------------------------------------
// Remove
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, Remove_OneTarget_OnlyOtherInvoked) {
    int a = 0, b = 0;
    auto d1 = std::make_shared<MulticastDelegate>([&]{ ++a; });
    auto d2 = std::make_shared<MulticastDelegate>([&]{ ++b; });
    auto combined = Delegate::Combine(d1, d2);
    auto result = Delegate::Remove(combined, d2);
    ASSERT_TRUE(result);
    result->Invoke();
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 0);
}

// ---------------------------------------------------------------------------
// DynamicInvoke — must throw
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, DynamicInvoke_Throws) {
    auto d = std::make_shared<MulticastDelegate>([]{});
    EXPECT_THROW(d->DynamicInvoke({}), System::NotImplementedException);
}

// ---------------------------------------------------------------------------
// EnumerateInvocationList
// ---------------------------------------------------------------------------

TEST(MulticastDelegateTests, EnumerateInvocationList_ThreeTargets) {
    auto d1 = std::make_shared<MulticastDelegate>([]{});
    auto d2 = std::make_shared<MulticastDelegate>([]{});
    auto d3 = std::make_shared<MulticastDelegate>([]{});
    auto combined = Delegate::Combine(Delegate::Combine(d1, d2), d3);
    int count = 0;
    auto e = Delegate::EnumerateInvocationList<Delegate>(combined);
    while (e.MoveNext()) ++count;
    EXPECT_EQ(count, 3);
}
