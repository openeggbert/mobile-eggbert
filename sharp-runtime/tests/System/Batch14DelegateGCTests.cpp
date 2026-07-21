// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Delegate.hpp"
#include "System/MulticastDelegate.hpp"
#include "System/MulticastNotSupportedException.hpp"
#include "System/GCCollectionMode.hpp"
#include "System/GCNotificationStatus.hpp"
#include "System/AppContext.hpp"

// ===========================================================================
// Delegate::getTargetProperty
// ===========================================================================

TEST(DelegateTargetTests, Target_AlwaysNullptr) {
    auto d = std::make_shared<System::Delegate>([](){});
    EXPECT_EQ(d->getTargetProperty(), nullptr);
}

TEST(DelegateTargetTests, Target_EmptyDelegate_Nullptr) {
    System::Delegate d;
    EXPECT_EQ(d.getTargetProperty(), nullptr);
}

TEST(DelegateTargetTests, Target_MulticastDelegate_Nullptr) {
    auto d = std::make_shared<System::MulticastDelegate>([](){});
    EXPECT_EQ(d->getTargetProperty(), nullptr);
}

// ===========================================================================
// GCCollectionMode — standalone header
// ===========================================================================

TEST(GCCollectionModeStandaloneTests, Default_IsZero) {
    EXPECT_EQ(static_cast<int>(System::GCCollectionMode::Default), 0);
}

TEST(GCCollectionModeStandaloneTests, Forced_IsOne) {
    EXPECT_EQ(static_cast<int>(System::GCCollectionMode::Forced), 1);
}

TEST(GCCollectionModeStandaloneTests, Optimized_IsTwo) {
    EXPECT_EQ(static_cast<int>(System::GCCollectionMode::Optimized), 2);
}

TEST(GCCollectionModeStandaloneTests, Aggressive_IsThree) {
    EXPECT_EQ(static_cast<int>(System::GCCollectionMode::Aggressive), 3);
}

TEST(GCCollectionModeStandaloneTests, ValuesAreDistinct) {
    EXPECT_NE(System::GCCollectionMode::Default, System::GCCollectionMode::Forced);
    EXPECT_NE(System::GCCollectionMode::Optimized, System::GCCollectionMode::Aggressive);
}

// ===========================================================================
// GCNotificationStatus — standalone header
// ===========================================================================

TEST(GCNotificationStatusStandaloneTests, Succeeded_IsZero) {
    EXPECT_EQ(static_cast<int>(System::GCNotificationStatus::Succeeded), 0);
}

TEST(GCNotificationStatusStandaloneTests, Failed_IsOne) {
    EXPECT_EQ(static_cast<int>(System::GCNotificationStatus::Failed), 1);
}

TEST(GCNotificationStatusStandaloneTests, Canceled_IsTwo) {
    EXPECT_EQ(static_cast<int>(System::GCNotificationStatus::Canceled), 2);
}

TEST(GCNotificationStatusStandaloneTests, Timeout_IsThree) {
    EXPECT_EQ(static_cast<int>(System::GCNotificationStatus::Timeout), 3);
}

TEST(GCNotificationStatusStandaloneTests, NotApplicable_IsFour) {
    EXPECT_EQ(static_cast<int>(System::GCNotificationStatus::NotApplicable), 4);
}

TEST(GCNotificationStatusStandaloneTests, ValuesAreDistinct) {
    EXPECT_NE(System::GCNotificationStatus::Succeeded, System::GCNotificationStatus::Failed);
    EXPECT_NE(System::GCNotificationStatus::Canceled, System::GCNotificationStatus::Timeout);
}

// ===========================================================================
// MulticastNotSupportedException
// ===========================================================================

TEST(MulticastNotSupportedExceptionTests, IsA_SystemException_New) {
    System::MulticastNotSupportedException ex;
    EXPECT_NE(dynamic_cast<System::SystemException*>(&ex), nullptr);
}

TEST(MulticastNotSupportedExceptionTests, InnerExceptionCtor_WhatContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("inner error"));
    System::MulticastNotSupportedException ex("outer", inner);
    std::string w = ex.what();
    EXPECT_NE(w.find("outer"), std::string::npos);
}

// ===========================================================================
// AppContext — additional edge-case coverage
// ===========================================================================

TEST(AppContextExtraTests, SetSwitch_EmptyName_Throws) {
    EXPECT_THROW(System::AppContext::SetSwitch("", true), System::ArgumentException);
}

TEST(AppContextExtraTests, TryGetSwitch_EmptyName_Throws) {
    bool enabled = false;
    EXPECT_THROW(System::AppContext::TryGetSwitch("", enabled), System::ArgumentException);
}

TEST(AppContextExtraTests, TryGetSwitch_UnknownSwitch_ReturnsFalse) {
    bool enabled = true;
    bool found = System::AppContext::TryGetSwitch("sharp-runtime.nonexistent.switch", enabled);
    EXPECT_FALSE(found);
    EXPECT_FALSE(enabled);
}

TEST(AppContextExtraTests, SetSwitch_ThenGet_RoundTrip) {
    System::AppContext::SetSwitch("sharp-runtime.test.switch", true);
    bool enabled = false;
    EXPECT_TRUE(System::AppContext::TryGetSwitch("sharp-runtime.test.switch", enabled));
    EXPECT_TRUE(enabled);
}

TEST(AppContextExtraTests, SetData_GetData_RoundTrip) {
    int value = 42;
    System::AppContext::SetData("test.data.key", &value);
    void* result = System::AppContext::GetData("test.data.key");
    EXPECT_EQ(result, &value);
}

TEST(AppContextExtraTests, GetData_UnknownKey_ReturnsNullptr) {
    EXPECT_EQ(System::AppContext::GetData("sharp-runtime.unknown.data.xyz"), nullptr);
}
