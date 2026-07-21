// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include "System/Attribute.hpp"
#include "System/Runtime/CompilerServices/MethodImplOptions.hpp"
#include "System/Runtime/CompilerServices/MethodImplAttribute.hpp"
#include "System/Runtime/CompilerServices/CallerAttributes.hpp"
#include "System/Runtime/CompilerServices/CompilerGeneratedAttribute.hpp"
#include "System/Runtime/GCSettings.hpp"
#include "System/Runtime/AmbiguousImplementationException.hpp"
#include "System/Runtime/InteropServices/InteropAttributes.hpp"
#include "System/Runtime/InteropServices/ExternalException.hpp"
#include "System/Runtime/Versioning/VersioningAttributes.hpp"

using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Runtime::Versioning;
using System::Runtime::GCSettings;
using System::Runtime::GCLatencyMode;
using System::Runtime::GCLargeObjectHeapCompactionMode;
using System::Runtime::AmbiguousImplementationException;

// ===========================================================================
// MethodImplOptions
// ===========================================================================

TEST(MethodImplOptionsTests, NoInlining_Value) {
    EXPECT_EQ(static_cast<int>(MethodImplOptions::NoInlining), 0x0008);
}

TEST(MethodImplOptionsTests, AggressiveInlining_Value) {
    EXPECT_EQ(static_cast<int>(MethodImplOptions::AggressiveInlining), 0x0100);
}

TEST(MethodImplOptionsTests, Synchronized_Value) {
    EXPECT_EQ(static_cast<int>(MethodImplOptions::Synchronized), 0x0020);
}

TEST(MethodImplOptionsTests, OrOperator_CombinesFlags) {
    auto combined = MethodImplOptions::NoInlining | MethodImplOptions::NoOptimization;
    EXPECT_EQ(static_cast<int>(combined), 0x0008 | 0x0040);
}

// ===========================================================================
// MethodImplAttribute
// ===========================================================================

TEST(MethodImplAttributeTests, Constructor_EnumValue) {
    MethodImplAttribute attr(MethodImplOptions::AggressiveInlining);
    EXPECT_EQ(attr.getValueProperty(), MethodImplOptions::AggressiveInlining);
}

TEST(MethodImplAttributeTests, Constructor_IntValue) {
    MethodImplAttribute attr(static_cast<int16_t>(0x0008));
    EXPECT_EQ(attr.getValueProperty(), MethodImplOptions::NoInlining);
}

// ===========================================================================
// Caller Attributes
// ===========================================================================

TEST(CallerAttributesTests, CallerMemberName_Instantiates) {
    CallerMemberNameAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(CallerAttributesTests, CallerFilePath_Instantiates) {
    CallerFilePathAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(CallerAttributesTests, CallerLineNumber_Instantiates) {
    CallerLineNumberAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(CallerAttributesTests, CallerArgumentExpression_StoresParameterName) {
    CallerArgumentExpressionAttribute attr("value");
    EXPECT_EQ(attr.getParameterNameProperty(), "value");
}

TEST(CallerAttributesTests, CallerArgumentExpression_EmptyName) {
    CallerArgumentExpressionAttribute attr("");
    EXPECT_EQ(attr.getParameterNameProperty(), "");
}

// ===========================================================================
// GCSettings
// ===========================================================================

TEST(GCSettingsTests, DefaultLatencyMode_IsInteractive) {
    // Reset to known state first (prior tests may have modified it)
    GCSettings::setLatencyModeProperty(GCLatencyMode::Interactive);
    EXPECT_EQ(GCSettings::getLatencyModeProperty(), GCLatencyMode::Interactive);
}

TEST(GCSettingsTests, SetLatencyMode_Batch) {
    GCSettings::setLatencyModeProperty(GCLatencyMode::Batch);
    EXPECT_EQ(GCSettings::getLatencyModeProperty(), GCLatencyMode::Batch);
    GCSettings::setLatencyModeProperty(GCLatencyMode::Interactive); // restore
}

TEST(GCSettingsTests, SetLatencyMode_LowLatency) {
    GCSettings::setLatencyModeProperty(GCLatencyMode::LowLatency);
    EXPECT_EQ(GCSettings::getLatencyModeProperty(), GCLatencyMode::LowLatency);
    GCSettings::setLatencyModeProperty(GCLatencyMode::Interactive); // restore
}

TEST(GCSettingsTests, DefaultCompactionMode_IsDefault) {
    GCSettings::setLargeObjectHeapCompactionModeProperty(GCLargeObjectHeapCompactionMode::Default);
    EXPECT_EQ(GCSettings::getLargeObjectHeapCompactionModeProperty(),
              GCLargeObjectHeapCompactionMode::Default);
}

TEST(GCSettingsTests, SetCompactionMode_CompactOnce) {
    GCSettings::setLargeObjectHeapCompactionModeProperty(GCLargeObjectHeapCompactionMode::CompactOnce);
    EXPECT_EQ(GCSettings::getLargeObjectHeapCompactionModeProperty(),
              GCLargeObjectHeapCompactionMode::CompactOnce);
    GCSettings::setLargeObjectHeapCompactionModeProperty(GCLargeObjectHeapCompactionMode::Default);
}

TEST(GCSettingsTests, IsServerGC_DefaultFalse) {
    EXPECT_FALSE(GCSettings::getIsServerGCProperty());
}

TEST(GCLatencyModeTests, Batch_IsZero) {
    EXPECT_EQ(static_cast<int>(GCLatencyMode::Batch), 0);
}

TEST(GCLatencyModeTests, Interactive_IsOne) {
    EXPECT_EQ(static_cast<int>(GCLatencyMode::Interactive), 1);
}

TEST(GCLatencyModeTests, NoGCRegion_IsFour) {
    EXPECT_EQ(static_cast<int>(GCLatencyMode::NoGCRegion), 4);
}

// ===========================================================================
// AmbiguousImplementationException
// ===========================================================================

TEST(AmbiguousImplementationExceptionTests, DefaultMessage) {
    AmbiguousImplementationException ex;
    EXPECT_STREQ(ex.what(), "Ambiguous implementation found.");
}

TEST(AmbiguousImplementationExceptionTests, CustomMessage) {
    AmbiguousImplementationException ex("Custom ambiguous message");
    EXPECT_STREQ(ex.what(), "Custom ambiguous message");
}

TEST(AmbiguousImplementationExceptionTests, IsThrowable) {
    EXPECT_THROW(throw AmbiguousImplementationException(), AmbiguousImplementationException);
}

// ===========================================================================
// InteropAttributes — enums
// ===========================================================================

TEST(LayoutKindTests, Sequential_IsZero) {
    EXPECT_EQ(static_cast<int>(LayoutKind::Sequential), 0);
}

TEST(LayoutKindTests, Explicit_IsTwo) {
    EXPECT_EQ(static_cast<int>(LayoutKind::Explicit), 2);
}

TEST(LayoutKindTests, Auto_IsThree) {
    EXPECT_EQ(static_cast<int>(LayoutKind::Auto), 3);
}

TEST(CharSetTests, Ansi_IsTwo) {
    EXPECT_EQ(static_cast<int>(CharSet::Ansi), 2);
}

TEST(CharSetTests, Unicode_IsThree) {
    EXPECT_EQ(static_cast<int>(CharSet::Unicode), 3);
}

TEST(CallingConventionTests, Cdecl_IsTwo) {
    EXPECT_EQ(static_cast<int>(CallingConvention::Cdecl), 2);
}

TEST(CallingConventionTests, StdCall_IsThree) {
    EXPECT_EQ(static_cast<int>(CallingConvention::StdCall), 3);
}

TEST(UnmanagedTypeTests, Bool_IsTwo) {
    EXPECT_EQ(static_cast<int>(UnmanagedType::Bool), 2);
}

TEST(UnmanagedTypeTests, LPStr_Is20) {
    EXPECT_EQ(static_cast<int>(UnmanagedType::LPStr), 20);
}

// ===========================================================================
// InteropAttributes — attribute classes
// ===========================================================================

TEST(StructLayoutAttributeTests, Constructor_StoresLayout) {
    StructLayoutAttribute attr(LayoutKind::Sequential);
    EXPECT_EQ(attr.Value, LayoutKind::Sequential);
}

TEST(StructLayoutAttributeTests, DefaultPack_IsEight) {
    StructLayoutAttribute attr(LayoutKind::Explicit);
    EXPECT_EQ(attr.Pack, 8);
}

TEST(StructLayoutAttributeTests, DefaultSize_IsZero) {
    StructLayoutAttribute attr(LayoutKind::Auto);
    EXPECT_EQ(attr.Size, 0);
}

TEST(FieldOffsetAttributeTests, Constructor_StoresOffset) {
    FieldOffsetAttribute attr(16);
    EXPECT_EQ(attr.Value, 16);
}

TEST(MarshalAsAttributeTests, Constructor_StoresType) {
    MarshalAsAttribute attr(UnmanagedType::LPStr);
    EXPECT_EQ(attr.Value, UnmanagedType::LPStr);
}

TEST(MarshalAsAttributeTests, DefaultSizeConst_IsZero) {
    MarshalAsAttribute attr(UnmanagedType::ByValArray);
    EXPECT_EQ(attr.SizeConst, 0);
}

TEST(DllImportAttributeTests, Constructor_StoresDllName) {
    DllImportAttribute attr("kernel32.dll");
    EXPECT_EQ(attr.Value, "kernel32.dll");
}

TEST(DllImportAttributeTests, DefaultSetLastError_False) {
    DllImportAttribute attr("mylib.dll");
    EXPECT_FALSE(attr.SetLastError);
}

TEST(ComVisibleAttributeTests, Constructor_True) {
    ComVisibleAttribute attr(true);
    EXPECT_TRUE(attr.Value);
}

TEST(ComVisibleAttributeTests, Constructor_False) {
    ComVisibleAttribute attr(false);
    EXPECT_FALSE(attr.Value);
}

TEST(GuidAttributeTests, Constructor_StoresGuid) {
    GuidAttribute attr("12345678-1234-1234-1234-123456789012");
    EXPECT_EQ(attr.Value, "12345678-1234-1234-1234-123456789012");
}

TEST(MarkerAttributeTests, InAttribute_Instantiates) {
    InAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(MarkerAttributeTests, OutAttribute_Instantiates) {
    OutAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(MarkerAttributeTests, OptionalAttribute_Instantiates) {
    OptionalAttribute attr;
    (void)attr;
    SUCCEED();
}

// ===========================================================================
// ExternalException
// ===========================================================================

TEST(ExternalExceptionTests, DefaultMessage) {
    ExternalException ex;
    EXPECT_STREQ(ex.what(), "External component has thrown an exception.");
}

TEST(ExternalExceptionTests, CustomMessage) {
    ExternalException ex("Native call failed");
    EXPECT_STREQ(ex.what(), "Native call failed");
}

TEST(ExternalExceptionTests, MessageWithInner) {
    auto inner = std::make_exception_ptr(std::runtime_error("access denied"));
    ExternalException ex("P/Invoke error", inner);
    std::string msg = ex.what();
    EXPECT_NE(msg.find("P/Invoke error"), std::string::npos);
}

TEST(ExternalExceptionTests, IsThrowable) {
    EXPECT_THROW(throw ExternalException(), ExternalException);
}

// ===========================================================================
// VersioningAttributes
// ===========================================================================

TEST(TargetFrameworkAttributeTests, Constructor_StoresFrameworkName) {
    TargetFrameworkAttribute attr(".NETCoreApp,Version=v8.0");
    EXPECT_EQ(attr.getFrameworkNameProperty(), ".NETCoreApp,Version=v8.0");
}

TEST(TargetFrameworkAttributeTests, DefaultDisplayName_IsEmpty) {
    TargetFrameworkAttribute attr(".NETCoreApp,Version=v8.0");
    EXPECT_EQ(attr.getFrameworkDisplayNameProperty(), "");
}

TEST(TargetFrameworkAttributeTests, SetDisplayName) {
    TargetFrameworkAttribute attr(".NETCoreApp,Version=v8.0");
    attr.setFrameworkDisplayNameProperty(".NET 8.0");
    EXPECT_EQ(attr.getFrameworkDisplayNameProperty(), ".NET 8.0");
}

TEST(SupportedOSPlatformAttributeTests, Constructor_StoresPlatform) {
    SupportedOSPlatformAttribute attr("windows");
    EXPECT_EQ(attr.getPlatformNameProperty(), "windows");
}

TEST(UnsupportedOSPlatformAttributeTests, Constructor_StoresPlatform) {
    UnsupportedOSPlatformAttribute attr("browser");
    EXPECT_EQ(attr.getPlatformNameProperty(), "browser");
    EXPECT_EQ(attr.getMessageProperty(), "");
}

TEST(UnsupportedOSPlatformAttributeTests, Constructor_WithMessage) {
    UnsupportedOSPlatformAttribute attr("browser", "Not supported in the browser sandbox.");
    EXPECT_EQ(attr.getPlatformNameProperty(), "browser");
    EXPECT_EQ(attr.getMessageProperty(), "Not supported in the browser sandbox.");
}

TEST(SupportedOSPlatformGuardAttributeTests, Constructor_StoresPlatform) {
    SupportedOSPlatformGuardAttribute attr("linux");
    EXPECT_EQ(attr.getPlatformNameProperty(), "linux");
}

TEST(UnsupportedOSPlatformGuardAttributeTests, Constructor_StoresPlatform) {
    UnsupportedOSPlatformGuardAttribute attr("windows");
    EXPECT_EQ(attr.getPlatformNameProperty(), "windows");
}

TEST(ObsoletedOSPlatformAttributeTests, Constructor_AllFields) {
    ObsoletedOSPlatformAttribute attr("ios", "Use X instead", "https://example.com");
    EXPECT_EQ(attr.getPlatformNameProperty(), "ios");
    EXPECT_EQ(attr.getMessageProperty(), "Use X instead");
    EXPECT_EQ(attr.getUrlProperty(), "https://example.com");
}

TEST(RequiresPreviewFeaturesAttributeTests, DefaultConstructor_EmptyMessage) {
    RequiresPreviewFeaturesAttribute attr;
    EXPECT_EQ(attr.getMessageProperty(), "");
}

TEST(RequiresPreviewFeaturesAttributeTests, Constructor_WithMessage) {
    RequiresPreviewFeaturesAttribute attr("Preview feature");
    EXPECT_EQ(attr.getMessageProperty(), "Preview feature");
}

// ===========================================================================
// CompilerGeneratedAttribute
// ===========================================================================

TEST(CompilerGeneratedAttributeTests, IsAttribute) {
    CompilerGeneratedAttribute attr;
    const System::Attribute& base = attr;
    (void)base;
    SUCCEED();
}
