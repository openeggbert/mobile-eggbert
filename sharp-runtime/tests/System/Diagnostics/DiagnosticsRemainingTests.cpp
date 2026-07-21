// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for remaining Diagnostics types: DebuggerDisplayAttribute,
// DebuggerBrowsableAttribute, StackFrame, StackTrace, UnreachableException,
// the marker-only debugger attributes, ConditionalAttribute,
// DebuggableAttribute, DebuggerTypeProxyAttribute, DebuggerVisualizerAttribute.
#include <gtest/gtest.h>
#include <string>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Diagnostics/DebuggerDisplayAttribute.hpp"
#include "System/Diagnostics/DebuggerBrowsableAttribute.hpp"
#include "System/Diagnostics/StackFrame.hpp"
#include "System/Diagnostics/StackFrameExtensions.hpp"
#include "System/Diagnostics/StackTrace.hpp"
#include "System/Diagnostics/UnreachableException.hpp"
#include "System/Diagnostics/DebuggerHiddenAttribute.hpp"
#include "System/Diagnostics/DebuggerStepThroughAttribute.hpp"
#include "System/Diagnostics/DebuggerNonUserCodeAttribute.hpp"
#include "System/Diagnostics/DebuggerStepperBoundaryAttribute.hpp"
#include "System/Diagnostics/ConditionalAttribute.hpp"
#include "System/Diagnostics/DebuggableAttribute.hpp"
#include "System/Diagnostics/DebuggerTypeProxyAttribute.hpp"
#include "System/Diagnostics/DebuggerVisualizerAttribute.hpp"
#include "System/Diagnostics/StackTraceHiddenAttribute.hpp"
#include "System/Diagnostics/DebuggerDisableUserUnhandledExceptionsAttribute.hpp"
#include "System/Diagnostics/DebuggerGuidedStepThroughAttribute.hpp"

using System::Diagnostics::DebuggerDisplayAttribute;
using System::Diagnostics::DebuggerBrowsableAttribute;
using System::Diagnostics::DebuggerBrowsableState;
using System::Diagnostics::StackFrame;
using System::Diagnostics::StackFrameExtensions;
using System::Diagnostics::StackTrace;
using System::Diagnostics::UnreachableException;
using System::Diagnostics::DebuggerHiddenAttribute;
using System::Diagnostics::DebuggerStepThroughAttribute;
using System::Diagnostics::DebuggerNonUserCodeAttribute;

// ===========================================================================
// DebuggerBrowsableState
// ===========================================================================

TEST(DebuggerBrowsableStateTests, Never_IsZero) {
    EXPECT_EQ(static_cast<int>(DebuggerBrowsableState::Never), 0);
}

TEST(DebuggerBrowsableStateTests, Collapsed_IsTwo) {
    EXPECT_EQ(static_cast<int>(DebuggerBrowsableState::Collapsed), 2);
}

TEST(DebuggerBrowsableStateTests, RootHidden_IsThree) {
    EXPECT_EQ(static_cast<int>(DebuggerBrowsableState::RootHidden), 3);
}

// ===========================================================================
// DebuggerBrowsableAttribute
// ===========================================================================

TEST(DebuggerBrowsableAttributeTests, Constructor_StoresState) {
    DebuggerBrowsableAttribute attr(DebuggerBrowsableState::Never);
    EXPECT_EQ(attr.getStateProperty(), DebuggerBrowsableState::Never);
}

TEST(DebuggerBrowsableAttributeTests, Collapsed_State) {
    DebuggerBrowsableAttribute attr(DebuggerBrowsableState::Collapsed);
    EXPECT_EQ(attr.getStateProperty(), DebuggerBrowsableState::Collapsed);
}

TEST(DebuggerBrowsableAttributeTests, RootHidden_State) {
    DebuggerBrowsableAttribute attr(DebuggerBrowsableState::RootHidden);
    EXPECT_EQ(attr.getStateProperty(), DebuggerBrowsableState::RootHidden);
}

TEST(DebuggerBrowsableAttributeTests, OutOfRangeState_Throws) {
    // Verified against DebuggerBrowsableAttribute.cs: the constructor validates
    // state < Never || state > RootHidden -- a pure min/max range check. Note this does
    // NOT specially catch the removed Expanded=1 value (1 falls within [Never=0,
    // RootHidden=3], so real .NET's check silently accepts it too; only genuinely
    // out-of-range values throw).
    EXPECT_THROW(DebuggerBrowsableAttribute(static_cast<DebuggerBrowsableState>(4)), System::ArgumentOutOfRangeException);
    EXPECT_THROW(DebuggerBrowsableAttribute(static_cast<DebuggerBrowsableState>(-1)), System::ArgumentOutOfRangeException);
    EXPECT_NO_THROW(DebuggerBrowsableAttribute(static_cast<DebuggerBrowsableState>(1)));
}

// ===========================================================================
// DebuggerDisplayAttribute
// ===========================================================================

TEST(DebuggerDisplayAttributeTests, Constructor_StoresValue) {
    DebuggerDisplayAttribute attr("{Count}");
    EXPECT_EQ(attr.getValueProperty(), "{Count}");
}

TEST(DebuggerDisplayAttributeTests, DefaultName_Empty) {
    DebuggerDisplayAttribute attr("val");
    EXPECT_TRUE(attr.getNameProperty().empty());
}

TEST(DebuggerDisplayAttributeTests, SetName_GetName) {
    DebuggerDisplayAttribute attr("val");
    attr.setNameProperty("MyName");
    EXPECT_EQ(attr.getNameProperty(), "MyName");
}

TEST(DebuggerDisplayAttributeTests, SetType_GetType) {
    DebuggerDisplayAttribute attr("val");
    attr.setTypeProperty("MyType");
    EXPECT_EQ(attr.getTypeProperty(), "MyType");
}

// ===========================================================================
// StackFrame
// ===========================================================================

TEST(StackFrameTests, DefaultCtor_EmptyFileName) {
    StackFrame sf;
    EXPECT_TRUE(sf.getFileNameProperty().empty());
}

TEST(StackFrameTests, Constructor_StoresFileName) {
    StackFrame sf("main.cpp", 42);
    EXPECT_EQ(sf.getFileNameProperty(), "main.cpp");
}

TEST(StackFrameTests, Constructor_StoresLineNumber) {
    StackFrame sf("file.cpp", 10, 5);
    EXPECT_EQ(sf.getFileLineNumberProperty(), 10);
}

TEST(StackFrameTests, Constructor_StoresColumnNumber) {
    StackFrame sf("file.cpp", 10, 5);
    EXPECT_EQ(sf.getFileColumnNumberProperty(), 5);
}

TEST(StackFrameTests, DefaultILOffset_IsMinusOne) {
    StackFrame sf;
    EXPECT_EQ(sf.getILOffsetProperty(), StackFrame::OFFSET_UNKNOWN);
}

TEST(StackFrameTests, DefaultNativeOffset_IsOffsetUnknown) {
    // Verified against StackFrame.cs's InitMembers: both _ilOffset and _nativeOffset
    // default to OFFSET_UNKNOWN, not 0.
    StackFrame sf;
    EXPECT_EQ(sf.getNativeOffsetProperty(), StackFrame::OFFSET_UNKNOWN);
}

TEST(StackFrameTests, ToString_EmptyFile_ReturnsUnknown) {
    StackFrame sf;
    EXPECT_EQ(sf.ToString(), "<unknown>");
}

TEST(StackFrameTests, ToString_WithFile_ContainsFileName) {
    StackFrame sf("app.cpp", 100);
    std::string s = sf.ToString();
    EXPECT_NE(s.find("app.cpp"), std::string::npos);
}

TEST(StackFrameTests, ToString_ContainsLineNumber) {
    StackFrame sf("app.cpp", 100);
    std::string s = sf.ToString();
    EXPECT_NE(s.find("100"), std::string::npos);
}

// ===========================================================================
// StackFrameExtensions
// ===========================================================================

TEST(StackFrameExtensionsTests, HasSource_EmptyFileName_ReturnsFalse) {
    StackFrame sf;
    EXPECT_FALSE(StackFrameExtensions::HasSource(sf));
}

TEST(StackFrameExtensionsTests, HasSource_WithFileName_ReturnsTrue) {
    StackFrame sf("app.cpp", 10);
    EXPECT_TRUE(StackFrameExtensions::HasSource(sf));
}

TEST(StackFrameExtensionsTests, HasILOffset_Default_ReturnsFalse) {
    StackFrame sf;
    EXPECT_FALSE(StackFrameExtensions::HasILOffset(sf));
}

TEST(StackFrameExtensionsTests, HasMethod_EmptyMethodName_ReturnsFalse) {
    StackFrame sf;
    EXPECT_FALSE(StackFrameExtensions::HasMethod(sf));
}

TEST(StackFrameExtensionsTests, GetNativeIP_AlwaysZero) {
    StackFrame sf("app.cpp", 10);
    EXPECT_TRUE(StackFrameExtensions::GetNativeIP(sf).IsZero());
}

TEST(StackFrameExtensionsTests, GetNativeImageBase_AlwaysZero) {
    StackFrame sf("app.cpp", 10);
    EXPECT_TRUE(StackFrameExtensions::GetNativeImageBase(sf).IsZero());
}

TEST(StackFrameExtensionsTests, HasNativeImage_AlwaysFalse) {
    StackFrame sf("app.cpp", 10);
    EXPECT_FALSE(StackFrameExtensions::HasNativeImage(sf));
}

// ===========================================================================
// StackTrace
// ===========================================================================

TEST(StackTraceTests, DefaultCtor_FrameCountZero) {
    StackTrace st;
    EXPECT_EQ(st.getFrameCountProperty(), 0);
}

TEST(StackTraceTests, Constructor_WithFrames_StoresCount) {
    std::vector<StackFrame> frames = {StackFrame("a.cpp", 1), StackFrame("b.cpp", 2)};
    StackTrace st(frames);
    EXPECT_EQ(st.getFrameCountProperty(), 2);
}

TEST(StackTraceTests, GetFrame_ValidIndex) {
    std::vector<StackFrame> frames = {StackFrame("x.cpp", 5)};
    StackTrace st(frames);
    const StackFrame* f = st.GetFrame(0);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->getFileNameProperty(), "x.cpp");
}

TEST(StackTraceTests, GetFrame_OutOfRange_ReturnsNull) {
    StackTrace st;
    EXPECT_EQ(st.GetFrame(0), nullptr);
}

TEST(StackTraceTests, GetFrames_ReturnsAllFrames) {
    std::vector<StackFrame> frames = {StackFrame("a.cpp", 1), StackFrame("b.cpp", 2)};
    StackTrace st(frames);
    EXPECT_EQ(st.GetFrames().size(), 2u);
}

TEST(StackTraceTests, ToString_ContainsAt) {
    std::vector<StackFrame> frames = {StackFrame("main.cpp", 42)};
    StackTrace st(frames);
    std::string s = st.ToString();
    EXPECT_NE(s.find("at"), std::string::npos);
}

// ===========================================================================
// UnreachableException
// ===========================================================================

TEST(UnreachableExceptionTests, DefaultCtor_WhatNotEmpty) {
    UnreachableException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(UnreachableExceptionTests, MessageCtor_WhatContainsMessage) {
    UnreachableException ex("should not reach here");
    EXPECT_NE(std::string(ex.what()).find("should not reach here"), std::string::npos);
}

TEST(UnreachableExceptionTests, IsA_Exception) {
    EXPECT_THROW(throw UnreachableException(), System::Exception);
}

// ===========================================================================
// Marker-only debugger attributes (just instantiation)
// ===========================================================================

TEST(DebuggerMarkerAttributesTests, DebuggerHiddenAttribute_DefaultCtor) {
    EXPECT_NO_THROW(DebuggerHiddenAttribute{});
}

TEST(DebuggerMarkerAttributesTests, DebuggerStepThroughAttribute_DefaultCtor) {
    EXPECT_NO_THROW(DebuggerStepThroughAttribute{});
}

TEST(DebuggerMarkerAttributesTests, DebuggerNonUserCodeAttribute_DefaultCtor) {
    EXPECT_NO_THROW(DebuggerNonUserCodeAttribute{});
}

TEST(DebuggerMarkerAttributesTests, StackTraceHiddenAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::Diagnostics::StackTraceHiddenAttribute{});
}

TEST(DebuggerMarkerAttributesTests, DebuggerGuidedStepThroughAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::Diagnostics::DebuggerGuidedStepThroughAttribute{});
}

TEST(DebuggerMarkerAttributesTests, DebuggerDisableUserUnhandledExceptionsAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::Diagnostics::DebuggerDisableUserUnhandledExceptionsAttribute{});
}

// ===========================================================================
// ConditionalAttribute
// ===========================================================================

TEST(ConditionalAttributeTests, ConditionString_Stored) {
    System::Diagnostics::ConditionalAttribute attr("DEBUG");
    EXPECT_EQ(attr.getConditionStringProperty(), "DEBUG");
}

TEST(ConditionalAttributeTests, ConditionString_EmptyAllowed) {
    System::Diagnostics::ConditionalAttribute attr("");
    EXPECT_TRUE(attr.getConditionStringProperty().empty());
}

// ===========================================================================
// DebuggableAttribute
// ===========================================================================

TEST(DebuggableAttributeTests, BoolCtor_TrackingEnabled) {
    System::Diagnostics::DebuggableAttribute attr(true, false);
    EXPECT_TRUE(attr.getIsJITTrackingEnabledProperty());
}

TEST(DebuggableAttributeTests, BoolCtor_OptimizerDisabled) {
    System::Diagnostics::DebuggableAttribute attr(false, true);
    EXPECT_TRUE(attr.getIsJITOptimizerDisabledProperty());
}

TEST(DebuggableAttributeTests, BoolCtor_BothFalse_ModeIsNone) {
    System::Diagnostics::DebuggableAttribute attr(false, false);
    EXPECT_EQ(attr.getDebuggingFlagsProperty(),
              System::Diagnostics::DebuggableAttribute::DebuggingModes::None);
}

TEST(DebuggableAttributeTests, ModeCtor_StoresMode) {
    using DM = System::Diagnostics::DebuggableAttribute::DebuggingModes;
    System::Diagnostics::DebuggableAttribute attr(DM::DisableOptimizations);
    EXPECT_EQ(attr.getDebuggingFlagsProperty(), DM::DisableOptimizations);
}

TEST(DebuggableAttributeTests, ModeCtor_DerivedPropertiesReflectFlags) {
    // Verified against DebuggableAttribute.cs: IsJITTrackingEnabled/IsJITOptimizerDisabled
    // are computed properties that bit-test DebuggingFlags live, so they must be correct
    // regardless of which constructor built the instance -- not just the (bool,bool) one.
    using DM = System::Diagnostics::DebuggableAttribute::DebuggingModes;
    System::Diagnostics::DebuggableAttribute defaultAttr(DM::Default);
    EXPECT_TRUE(defaultAttr.getIsJITTrackingEnabledProperty());
    EXPECT_FALSE(defaultAttr.getIsJITOptimizerDisabledProperty());

    System::Diagnostics::DebuggableAttribute disabledOpt(DM::DisableOptimizations);
    EXPECT_FALSE(disabledOpt.getIsJITTrackingEnabledProperty());
    EXPECT_TRUE(disabledOpt.getIsJITOptimizerDisabledProperty());

    System::Diagnostics::DebuggableAttribute both(
        static_cast<DM>(static_cast<int>(DM::Default) | static_cast<int>(DM::DisableOptimizations)));
    EXPECT_TRUE(both.getIsJITTrackingEnabledProperty());
    EXPECT_TRUE(both.getIsJITOptimizerDisabledProperty());
}

// ===========================================================================
// DebuggerTypeProxyAttribute
// ===========================================================================

TEST(DebuggerTypeProxyAttributeTests, Constructor_StoresProxyTypeName) {
    System::Diagnostics::DebuggerTypeProxyAttribute attr("MyProxy");
    EXPECT_EQ(attr.getProxyTypeNameProperty(), "MyProxy");
}

TEST(DebuggerTypeProxyAttributeTests, Target_DefaultEmpty) {
    System::Diagnostics::DebuggerTypeProxyAttribute attr("P");
    EXPECT_TRUE(attr.getTargetProperty().empty());
}

TEST(DebuggerTypeProxyAttributeTests, SetTarget_Stored) {
    System::Diagnostics::DebuggerTypeProxyAttribute attr("P");
    attr.setTargetProperty("T");
    EXPECT_EQ(attr.getTargetProperty(), "T");
}

// ===========================================================================
// DebuggerVisualizerAttribute
// ===========================================================================

TEST(DebuggerVisualizerAttributeTests, SingleArgCtor_StoresVisualizerName) {
    System::Diagnostics::DebuggerVisualizerAttribute attr("MyViz");
    EXPECT_EQ(attr.getVisualizerTypeNameProperty(), "MyViz");
    EXPECT_TRUE(attr.getVisualizerObjectSourceTypeNameProperty().empty());
}

TEST(DebuggerVisualizerAttributeTests, TwoArgCtor_StoresSourceTypeName) {
    System::Diagnostics::DebuggerVisualizerAttribute attr("Viz", "Source");
    EXPECT_EQ(attr.getVisualizerObjectSourceTypeNameProperty(), "Source");
}

TEST(DebuggerVisualizerAttributeTests, SetDescription_Stored) {
    System::Diagnostics::DebuggerVisualizerAttribute attr("V");
    attr.setDescriptionProperty("My description");
    EXPECT_EQ(attr.getDescriptionProperty(), "My description");
}

TEST(DebuggerVisualizerAttributeTests, SetTarget_Stored) {
    System::Diagnostics::DebuggerVisualizerAttribute attr("V");
    attr.setTargetProperty("T");
    EXPECT_EQ(attr.getTargetProperty(), "T");
}
