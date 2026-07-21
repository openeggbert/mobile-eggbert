// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/GC.hpp"

using System::GC;
using System::GCCollectionMode;
using System::GCNotificationStatus;
using System::GCKind;
using System::GCMemoryInfo;
using System::TimeSpan;

// ---------------------------------------------------------------------------
// GCCollectionMode enum
// ---------------------------------------------------------------------------

TEST(GCTests, GCCollectionMode_DefaultIsZero) {
    EXPECT_EQ(static_cast<int>(GCCollectionMode::Default), 0);
}

TEST(GCTests, GCCollectionMode_ForcedIsOne) {
    EXPECT_EQ(static_cast<int>(GCCollectionMode::Forced), 1);
}

TEST(GCTests, GCCollectionMode_OptimizedIsTwo) {
    EXPECT_EQ(static_cast<int>(GCCollectionMode::Optimized), 2);
}

TEST(GCTests, GCCollectionMode_AggressiveIsThree) {
    EXPECT_EQ(static_cast<int>(GCCollectionMode::Aggressive), 3);
}

// ---------------------------------------------------------------------------
// GCNotificationStatus enum
// ---------------------------------------------------------------------------

TEST(GCTests, GCNotificationStatus_SucceededIsZero) {
    EXPECT_EQ(static_cast<int>(GCNotificationStatus::Succeeded), 0);
}

TEST(GCTests, GCNotificationStatus_FailedIsOne) {
    EXPECT_EQ(static_cast<int>(GCNotificationStatus::Failed), 1);
}

TEST(GCTests, GCNotificationStatus_CanceledIsTwo) {
    EXPECT_EQ(static_cast<int>(GCNotificationStatus::Canceled), 2);
}

TEST(GCTests, GCNotificationStatus_TimeoutIsThree) {
    EXPECT_EQ(static_cast<int>(GCNotificationStatus::Timeout), 3);
}

TEST(GCTests, GCNotificationStatus_NotApplicableIsFour) {
    EXPECT_EQ(static_cast<int>(GCNotificationStatus::NotApplicable), 4);
}

// ---------------------------------------------------------------------------
// GCKind enum
// ---------------------------------------------------------------------------

TEST(GCTests, GCKind_AnyIsZero)          { EXPECT_EQ(static_cast<int>(GCKind::Any), 0); }
TEST(GCTests, GCKind_EphemeralIsOne)     { EXPECT_EQ(static_cast<int>(GCKind::Ephemeral), 1); }
TEST(GCTests, GCKind_FullBlockingIsTwo)  { EXPECT_EQ(static_cast<int>(GCKind::FullBlocking), 2); }
TEST(GCTests, GCKind_BackgroundIsThree) { EXPECT_EQ(static_cast<int>(GCKind::Background), 3); }

// ---------------------------------------------------------------------------
// Collect overloads — all no-op, must not throw
// ---------------------------------------------------------------------------

TEST(GCTests, Collect_NoArgs_DoesNotThrow_New) {
    EXPECT_NO_THROW(GC::Collect());
}

TEST(GCTests, Collect_WithGeneration_DoesNotThrow) {
    EXPECT_NO_THROW(GC::Collect(1));
}

TEST(GCTests, Collect_WithMode_DoesNotThrow) {
    EXPECT_NO_THROW(GC::Collect(2, GCCollectionMode::Forced));
}

TEST(GCTests, Collect_WithModeAndBlocking_DoesNotThrow) {
    EXPECT_NO_THROW(GC::Collect(2, GCCollectionMode::Optimized, true));
}

TEST(GCTests, Collect_WithModeBlockingCompacting_DoesNotThrow) {
    EXPECT_NO_THROW(GC::Collect(2, GCCollectionMode::Aggressive, true, false));
}

// ---------------------------------------------------------------------------
// Memory info
// ---------------------------------------------------------------------------

TEST(GCTests, GetTotalMemory_ReturnsZero_New) {
    EXPECT_EQ(GC::GetTotalMemory(false), 0LL);
}

TEST(GCTests, GetTotalMemory_ForceCollection_ReturnsZero) {
    EXPECT_EQ(GC::GetTotalMemory(true), 0LL);
}

TEST(GCTests, GetTotalAllocatedBytes_ReturnsZero) {
    EXPECT_EQ(GC::GetTotalAllocatedBytes(), 0LL);
}

TEST(GCTests, GetTotalAllocatedBytes_Precise_ReturnsZero) {
    EXPECT_EQ(GC::GetTotalAllocatedBytes(true), 0LL);
}

TEST(GCTests, GetAllocatedBytesForCurrentThread_ReturnsZero) {
    EXPECT_EQ(GC::GetAllocatedBytesForCurrentThread(), 0LL);
}

TEST(GCTests, GetTotalPauseDuration_ReturnsZero) {
    EXPECT_EQ(GC::GetTotalPauseDuration(), TimeSpan::Zero);
}

// ---------------------------------------------------------------------------
// GCMemoryInfo
// ---------------------------------------------------------------------------

TEST(GCTests, GetGCMemoryInfo_NoArgs_DoesNotThrow) {
    EXPECT_NO_THROW(GC::GetGCMemoryInfo());
}

TEST(GCTests, GetGCMemoryInfo_WithKind_DoesNotThrow) {
    EXPECT_NO_THROW(GC::GetGCMemoryInfo(GCKind::FullBlocking));
}

TEST(GCTests, GCMemoryInfo_TotalAvailableMemoryBytes_IsZero) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_EQ(info.getTotalAvailableMemoryBytesProperty(), 0LL);
}

TEST(GCTests, GCMemoryInfo_HeapSizeBytes_IsZero) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_EQ(info.getHeapSizeBytesProperty(), 0LL);
}

TEST(GCTests, GCMemoryInfo_PauseDuration_IsZero) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_EQ(info.getPauseDurationProperty(), TimeSpan::Zero);
}

TEST(GCTests, GCMemoryInfo_PinnedObjectsCount_IsZero) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_EQ(info.getPinnedObjectsCountProperty(), 0LL);
}

TEST(GCTests, GCMemoryInfo_FinalizationPendingCount_IsZero) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_EQ(info.getFinalizationPendingCountProperty(), 0LL);
}

TEST(GCTests, GCMemoryInfo_PromotedBytes_IsZero) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_EQ(info.getPromotedBytesProperty(), 0LL);
}

TEST(GCTests, GCMemoryInfo_PauseTimePercentage_IsZero) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_DOUBLE_EQ(info.getPauseTimePercentageProperty(), 0.0);
}

TEST(GCTests, GCMemoryInfo_GenerationInfo_IsEmpty) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_TRUE(info.getGenerationInfoProperty().empty());
}

TEST(GCTests, GCMemoryInfo_PauseDurations_IsEmpty) {
    GCMemoryInfo info = GC::GetGCMemoryInfo();
    EXPECT_TRUE(info.getPauseDurationsProperty().empty());
}

// ---------------------------------------------------------------------------
// Generation
// ---------------------------------------------------------------------------

TEST(GCTests, MaxGeneration_ReturnsTwo_New) {
    EXPECT_EQ(GC::MaxGeneration(), 2);
}

TEST(GCTests, GetMaxGenerationProperty_ReturnsTwo) {
    EXPECT_EQ(GC::getMaxGenerationProperty(), 2);
}

TEST(GCTests, GetGeneration_ReturnsZero_New) {
    int x = 42;
    EXPECT_EQ(GC::GetGeneration(&x), 0);
}

TEST(GCTests, CollectionCount_ReturnsZero) {
    EXPECT_EQ(GC::CollectionCount(0), 0);
    EXPECT_EQ(GC::CollectionCount(2), 0);
}

// ---------------------------------------------------------------------------
// Finalizer control
// ---------------------------------------------------------------------------

TEST(GCTests, SuppressFinalize_DoesNotThrow) {
    int x = 0;
    EXPECT_NO_THROW(GC::SuppressFinalize(&x));
}

TEST(GCTests, ReRegisterForFinalize_DoesNotThrow) {
    int x = 0;
    EXPECT_NO_THROW(GC::ReRegisterForFinalize(&x));
}

TEST(GCTests, WaitForPendingFinalizers_DoesNotThrow_New) {
    EXPECT_NO_THROW(GC::WaitForPendingFinalizers());
}

TEST(GCTests, KeepAlive_DoesNotThrow_New) {
    int v = 99;
    EXPECT_NO_THROW(GC::KeepAlive(v));
}

// ---------------------------------------------------------------------------
// Memory pressure
// ---------------------------------------------------------------------------

TEST(GCTests, AddMemoryPressure_DoesNotThrow) {
    EXPECT_NO_THROW(GC::AddMemoryPressure(1024LL));
}

TEST(GCTests, RemoveMemoryPressure_DoesNotThrow) {
    EXPECT_NO_THROW(GC::RemoveMemoryPressure(1024LL));
}

TEST(GCTests, RefreshMemoryLimit_DoesNotThrow) {
    EXPECT_NO_THROW(GC::RefreshMemoryLimit());
}

// ---------------------------------------------------------------------------
// No-GC region
// ---------------------------------------------------------------------------

TEST(GCTests, TryStartNoGCRegion_ReturnsFalse) {
    EXPECT_FALSE(GC::TryStartNoGCRegion(1024LL * 1024));
}

TEST(GCTests, TryStartNoGCRegion_WithDisallowFullBlockingGC_ReturnsFalse) {
    EXPECT_FALSE(GC::TryStartNoGCRegion(1024LL * 1024, true));
}

TEST(GCTests, TryStartNoGCRegion_WithLohSize_ReturnsFalse) {
    // Explicit SharpRuntime::longcs cast on the second argument: longcs is int64_t, which on
    // LP64 platforms (Linux/macOS x86_64) is `long`, a DISTINCT type from the `long long` of a
    // bare LL literal even though both are 64 bits -- passing two bare `long long` values here
    // would make this call ambiguous between the (longcs,bool) and (longcs,longcs) overloads,
    // since long long->longcs and long long->bool are equally-ranked standard conversions.
    EXPECT_FALSE(GC::TryStartNoGCRegion(1024LL * 1024, static_cast<SharpRuntime::longcs>(512LL * 1024)));
}

TEST(GCTests, TryStartNoGCRegion_WithLohSizeAndDisallowFullBlockingGC_ReturnsFalse) {
    EXPECT_FALSE(GC::TryStartNoGCRegion(1024LL * 1024, 512LL * 1024, true));
}

TEST(GCTests, EndNoGCRegion_DoesNotThrow) {
    EXPECT_NO_THROW(GC::EndNoGCRegion());
}

TEST(GCTests, RegisterNoGCRegionCallback_DoesNotThrow) {
    bool called = false;
    EXPECT_NO_THROW(GC::RegisterNoGCRegionCallback(4096LL, [&]{ called = true; }));
    EXPECT_FALSE(called);
}

// ---------------------------------------------------------------------------
// Full GC notifications
// ---------------------------------------------------------------------------

TEST(GCTests, RegisterForFullGCNotification_DoesNotThrow) {
    EXPECT_NO_THROW(GC::RegisterForFullGCNotification(90, 90));
}

TEST(GCTests, CancelFullGCNotification_DoesNotThrow) {
    EXPECT_NO_THROW(GC::CancelFullGCNotification());
}

// Regression tests for a misleading-result gap found while auditing GC (ticket 70):
// WaitForFullGCApproach/WaitForFullGCComplete previously returned GCNotificationStatus::
// Succeeded unconditionally, which asserts "a full GC approach/completion was genuinely
// detected" -- a false claim, since RegisterForFullGCNotification() is a no-op and nothing is
// ever actually monitored. NotApplicable is real .NET's own value for "this API doesn't apply
// to the current GC configuration", which honestly describes this permanently-no-GC port.
TEST(GCTests, WaitForFullGCApproach_ReturnsNotApplicable) {
    EXPECT_EQ(GC::WaitForFullGCApproach(0), GCNotificationStatus::NotApplicable);
}

TEST(GCTests, WaitForFullGCApproach_DefaultTimeout_ReturnsNotApplicable) {
    EXPECT_EQ(GC::WaitForFullGCApproach(), GCNotificationStatus::NotApplicable);
}

TEST(GCTests, WaitForFullGCApproach_TimeSpanTimeout_ReturnsNotApplicable) {
    EXPECT_EQ(GC::WaitForFullGCApproach(TimeSpan::FromSeconds(1)), GCNotificationStatus::NotApplicable);
}

TEST(GCTests, WaitForFullGCComplete_ReturnsNotApplicable) {
    EXPECT_EQ(GC::WaitForFullGCComplete(0), GCNotificationStatus::NotApplicable);
}

TEST(GCTests, WaitForFullGCComplete_DefaultTimeout_ReturnsNotApplicable) {
    EXPECT_EQ(GC::WaitForFullGCComplete(), GCNotificationStatus::NotApplicable);
}

TEST(GCTests, WaitForFullGCComplete_TimeSpanTimeout_ReturnsNotApplicable) {
    EXPECT_EQ(GC::WaitForFullGCComplete(TimeSpan::FromSeconds(1)), GCNotificationStatus::NotApplicable);
}

// ---------------------------------------------------------------------------
// Legacy helper
// ---------------------------------------------------------------------------

TEST(GCTests, GetGCMemoryInfo_TotalAvailableMemoryBytes_IsZero) {
    EXPECT_EQ(GC::GetGCMemoryInfo_TotalAvailableMemoryBytes(), 0LL);
}
