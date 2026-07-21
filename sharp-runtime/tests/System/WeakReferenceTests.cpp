// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/WeakReference.hpp"

using System::WeakReference;
using System::WeakReferenceT;

TEST(WeakReferenceTest, DefaultCtorIsNotAlive) {
    WeakReference wr;
    EXPECT_FALSE(wr.getIsAliveProperty());
}

TEST(WeakReferenceTest, AliveWhenTargetExists) {
    auto obj = std::make_shared<int>(42);
    WeakReference wr(obj);
    EXPECT_TRUE(wr.getIsAliveProperty());
}

TEST(WeakReferenceTest, DeadAfterReset) {
    WeakReference wr;
    {
        auto obj = std::make_shared<int>(42);
        wr = WeakReference(obj);
        EXPECT_TRUE(wr.getIsAliveProperty());
    }
    EXPECT_FALSE(wr.getIsAliveProperty());
}

TEST(WeakReferenceTest, GetTarget) {
    auto obj = std::make_shared<int>(99);
    WeakReference wr(obj);
    auto locked = wr.getTargetProperty();
    EXPECT_NE(locked, nullptr);
}

TEST(WeakReferenceTest, SetTarget) {
    WeakReference wr;
    auto obj = std::make_shared<int>(1);
    wr.setTargetProperty(obj);
    EXPECT_TRUE(wr.getIsAliveProperty());
}

TEST(WeakReferenceTest, TrackResurrection) {
    auto obj = std::make_shared<int>(0);
    WeakReference wr(obj, true);
    EXPECT_TRUE(wr.getTrackResurrectionProperty());
}

TEST(WeakReferenceTest, TryGetTargetSuccess) {
    auto obj = std::make_shared<int>(5);
    WeakReference wr(obj);
    std::shared_ptr<void> out;
    EXPECT_TRUE(wr.TryGetTarget(out));
    EXPECT_NE(out, nullptr);
}

TEST(WeakReferenceTest, TryGetTargetFail) {
    WeakReference wr;
    std::shared_ptr<void> out;
    EXPECT_FALSE(wr.TryGetTarget(out));
}

TEST(WeakReferenceTTest, TypedAlive) {
    auto obj = std::make_shared<std::string>("hello");
    WeakReferenceT<std::string> wr(obj);
    EXPECT_TRUE(wr.getIsAliveProperty());
}

TEST(WeakReferenceTTest, TryGetTarget) {
    auto obj = std::make_shared<int>(7);
    WeakReferenceT<int> wr(obj);
    std::shared_ptr<int> out;
    EXPECT_TRUE(wr.TryGetTarget(out));
    EXPECT_EQ(*out, 7);
}
