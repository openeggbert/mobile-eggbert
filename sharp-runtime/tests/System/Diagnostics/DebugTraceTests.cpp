// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentNullException.hpp"
#include "System/Diagnostics/Debug.hpp"
#include "System/Diagnostics/DebugProvider.hpp"
#include "System/Diagnostics/Debugger.hpp"
#include "System/Diagnostics/Trace.hpp"
#include "System/Exception.hpp"
#include <memory>

using System::Diagnostics::Debug;
using System::Diagnostics::DebugProvider;
using System::Diagnostics::Debugger;
using System::Diagnostics::Trace;

// ---------------------------------------------------------------------------
// Debug tests
// Note: Debug methods compile to no-ops in NDEBUG builds — both branches must
// be safe. Assert(false) / Fail() intentionally omitted (they call assert()).
// ---------------------------------------------------------------------------

TEST(DebugTraceTests, Debug_Write_CString_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::Write("hello"));
}

TEST(DebugTraceTests, Debug_Write_String_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::Write(std::string("hello")));
}

TEST(DebugTraceTests, Debug_Write_EmptyString_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::Write(""));
}

TEST(DebugTraceTests, Debug_WriteLine_NoArg_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::WriteLine());
}

TEST(DebugTraceTests, Debug_WriteLine_CString_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::WriteLine("line"));
}

TEST(DebugTraceTests, Debug_WriteLine_String_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::WriteLine(std::string("line")));
}

TEST(DebugTraceTests, Debug_Assert_True_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::Assert(true));
}

TEST(DebugTraceTests, Debug_Assert_True_WithCStringMessage_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::Assert(true, "ok"));
}

TEST(DebugTraceTests, Debug_Assert_True_WithStringMessage_DoesNotThrow) {
    EXPECT_NO_THROW(Debug::Assert(true, std::string("ok")));
}

// ---------------------------------------------------------------------------
// DebugProvider / Debug::GetProvider/SetProvider/Indent tests
// ---------------------------------------------------------------------------

namespace {
class RecordingDebugProvider : public DebugProvider {
public:
    std::string written;
    int lastIndentLevel = -1;
    int lastIndentSize = -1;

    void Write(const std::string& message) override { written += message; }
    void OnIndentLevelChanged(int indentLevel) override { lastIndentLevel = indentLevel; }
    void OnIndentSizeChanged(int indentSize) override { lastIndentSize = indentSize; }
};
}

TEST(DebugTraceTests, SetProvider_RoutesWriteThroughCustomProvider) {
    // Debug::Write/WriteLine compile to no-ops in NDEBUG (Release) builds, so the
    // pass-through to the provider can only be observed in Debug builds.
    auto previous = Debug::GetProvider();
    auto recorder = std::make_shared<RecordingDebugProvider>();
    Debug::SetProvider(recorder);

    Debug::Write("hello");
    Debug::WriteLine("world");

#ifndef NDEBUG
    EXPECT_EQ(recorder->written, "helloworld\n");
#endif
    Debug::SetProvider(previous);
}

TEST(DebugTraceTests, SetProvider_ReturnsPreviousProvider) {
    auto previous = Debug::GetProvider();
    auto recorder = std::make_shared<RecordingDebugProvider>();
    auto returned = Debug::SetProvider(recorder);

    EXPECT_EQ(returned, previous);
    EXPECT_EQ(Debug::GetProvider(), recorder);
    Debug::SetProvider(previous);
}

TEST(DebugTraceTests, SetProvider_Null_ThrowsArgumentNullException) {
    EXPECT_THROW(Debug::SetProvider(nullptr), System::ArgumentNullException);
}

TEST(DebugTraceTests, IndentLevel_Indent_Unindent_ClampsAtZero) {
    auto previous = Debug::GetProvider();
    auto recorder = std::make_shared<RecordingDebugProvider>();
    Debug::SetProvider(recorder);

    Debug::setIndentLevelProperty(0);
    Debug::Unindent();
    EXPECT_EQ(Debug::getIndentLevelProperty(), 0);
    EXPECT_EQ(recorder->lastIndentLevel, 0);

    Debug::Indent();
    Debug::Indent();
    EXPECT_EQ(Debug::getIndentLevelProperty(), 2);
    EXPECT_EQ(recorder->lastIndentLevel, 2);

    Debug::setIndentLevelProperty(0);
    Debug::SetProvider(previous);
}

TEST(DebugTraceTests, IndentSize_SetAndGet_NotifiesProvider) {
    auto previous = Debug::GetProvider();
    auto recorder = std::make_shared<RecordingDebugProvider>();
    Debug::SetProvider(recorder);

    int original = Debug::getIndentSizeProperty();
    Debug::setIndentSizeProperty(8);
    EXPECT_EQ(Debug::getIndentSizeProperty(), 8);
    EXPECT_EQ(recorder->lastIndentSize, 8);

    Debug::setIndentSizeProperty(-5);
    EXPECT_EQ(Debug::getIndentSizeProperty(), 0);

    Debug::setIndentSizeProperty(original);
    Debug::SetProvider(previous);
}

// ---------------------------------------------------------------------------
// Trace tests
// Trace is always active (no NDEBUG guard). Assert(false) and Fail() only
// write to std::cerr — they do NOT abort — so they are safe to call here.
// ---------------------------------------------------------------------------

TEST(DebugTraceTests, Trace_Write_String_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::Write(std::string("hello")));
}

TEST(DebugTraceTests, Trace_Write_EmptyString_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::Write(std::string("")));
}

TEST(DebugTraceTests, Trace_WriteLine_String_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::WriteLine(std::string("line")));
}

TEST(DebugTraceTests, Trace_WriteLine_NoArg_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::WriteLine());
}

TEST(DebugTraceTests, Trace_TraceInformation_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::TraceInformation("info message"));
}

TEST(DebugTraceTests, Trace_TraceWarning_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::TraceWarning("warn message"));
}

TEST(DebugTraceTests, Trace_TraceError_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::TraceError("error message"));
}

TEST(DebugTraceTests, Trace_Assert_True_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::Assert(true));
}

TEST(DebugTraceTests, Trace_Assert_True_WithMessage_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::Assert(true, "all good"));
}

TEST(DebugTraceTests, Trace_Assert_False_WritesToCerrNoAbort) {
    // Trace::Assert(false) only emits to std::cerr — it does NOT call assert()
    EXPECT_NO_THROW(Trace::Assert(false, "intentional failure message"));
}

TEST(DebugTraceTests, Trace_Assert_False_NoMessage_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::Assert(false));
}

TEST(DebugTraceTests, Trace_Fail_DoesNotThrow) {
    // Trace::Fail() only writes to std::cerr — it does NOT abort
    EXPECT_NO_THROW(Trace::Fail("intentional fail"));
}

TEST(DebugTraceTests, Trace_Flush_DoesNotThrow) {
    EXPECT_NO_THROW(Trace::Flush());
}

TEST(DebugTraceTests, Trace_IndentLevel_DefaultsToZero) {
    EXPECT_EQ(Trace::getIndentLevelProperty(), 0);
}

TEST(DebugTraceTests, Trace_Indent_IncrementsLevel) {
    Trace::setIndentLevelProperty(0);
    Trace::Indent();
    EXPECT_EQ(Trace::getIndentLevelProperty(), 1);
    Trace::Unindent();
    EXPECT_EQ(Trace::getIndentLevelProperty(), 0);
}

TEST(DebugTraceTests, Trace_IndentLevel_ClampsNegativeToZero) {
    Trace::setIndentLevelProperty(-5);
    EXPECT_EQ(Trace::getIndentLevelProperty(), 0);
}

TEST(DebugTraceTests, Trace_IndentSize_DefaultsToFour) {
    EXPECT_EQ(Trace::getIndentSizeProperty(), 4);
}

TEST(DebugTraceTests, Trace_IndentSize_ClampsNegativeToZero) {
    Trace::setIndentSizeProperty(-1);
    EXPECT_EQ(Trace::getIndentSizeProperty(), 0);
    Trace::setIndentSizeProperty(4); // restore default for other tests
}

// ---------------------------------------------------------------------------
// Debugger tests
// ---------------------------------------------------------------------------

TEST(DebuggerTests, IsAttached_AlwaysFalse) {
    EXPECT_FALSE(Debugger::getIsAttachedProperty());
}

TEST(DebuggerTests, Launch_AlwaysFalse) {
    EXPECT_FALSE(Debugger::Launch());
}

TEST(DebuggerTests, IsLogging_AlwaysFalse) {
    EXPECT_FALSE(Debugger::IsLogging());
}

TEST(DebuggerTests, Log_DoesNotThrow_WithDefaults) {
    EXPECT_NO_THROW(Debugger::Log(0));
}

TEST(DebuggerTests, Log_DoesNotThrow_WithAllArgs) {
    EXPECT_NO_THROW(Debugger::Log(1, "category", "message"));
}

TEST(DebuggerTests, DefaultCategory_IsEmpty) {
    EXPECT_TRUE(Debugger::DefaultCategory.empty());
}

TEST(DebuggerTests, BreakForUserUnhandledException_DoesNotThrow) {
    System::Exception ex("test exception");
    EXPECT_NO_THROW(Debugger::BreakForUserUnhandledException(ex));
}
