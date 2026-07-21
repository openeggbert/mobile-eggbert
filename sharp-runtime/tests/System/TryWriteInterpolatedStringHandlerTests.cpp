// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TryWriteInterpolatedStringHandler.hpp"

using System::TryWriteInterpolatedStringHandler;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(TryWriteInterpolatedStringHandlerTests, Ctor_SufficientBuffer_ShouldAppendTrue) {
    char buf[32];
    bool shouldAppend = false;
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf), 5, shouldAppend);
    EXPECT_TRUE(shouldAppend);
    EXPECT_TRUE(h.getSuccessProperty());
}

TEST(TryWriteInterpolatedStringHandlerTests, Ctor_InsufficientBuffer_ShouldAppendFalse) {
    char buf[3];
    bool shouldAppend = true;
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf), 10, shouldAppend);
    EXPECT_FALSE(shouldAppend);
    EXPECT_FALSE(h.getSuccessProperty());
}

// ---------------------------------------------------------------------------
// AppendLiteral
// ---------------------------------------------------------------------------

TEST(TryWriteInterpolatedStringHandlerTests, AppendLiteral_String_Succeeds) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    EXPECT_TRUE(h.AppendLiteral(std::string("hello")));
    EXPECT_EQ(h.getCharsWrittenProperty(), 5u);
    EXPECT_EQ(h.getString(), "hello");
}

TEST(TryWriteInterpolatedStringHandlerTests, AppendLiteral_Cstring_Succeeds) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    EXPECT_TRUE(h.AppendLiteral("world"));
    EXPECT_EQ(h.getString(), "world");
}

TEST(TryWriteInterpolatedStringHandlerTests, AppendLiteral_BufferFull_Fails) {
    char buf[3];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    EXPECT_FALSE(h.AppendLiteral("toolong"));
    EXPECT_FALSE(h.getSuccessProperty());
}

TEST(TryWriteInterpolatedStringHandlerTests, AppendLiteral_Multiple_Concatenates) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendLiteral("foo");
    h.AppendLiteral("bar");
    EXPECT_EQ(h.getString(), "foobar");
}

// ---------------------------------------------------------------------------
// AppendFormatted
// ---------------------------------------------------------------------------

TEST(TryWriteInterpolatedStringHandlerTests, AppendFormatted_Int) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendLiteral("x=");
    h.AppendFormatted(42);
    EXPECT_EQ(h.getString(), "x=42");
}

TEST(TryWriteInterpolatedStringHandlerTests, AppendFormatted_Double) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendFormatted(3.14);
    EXPECT_FALSE(h.getString().empty());
}

TEST(TryWriteInterpolatedStringHandlerTests, AppendFormatted_String) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendLiteral("name=");
    h.AppendFormatted(std::string("Alice"));
    EXPECT_EQ(h.getString(), "name=Alice");
}

TEST(TryWriteInterpolatedStringHandlerTests, AppendFormatted_Bool) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendFormatted(true);
    EXPECT_EQ(h.getString(), "1");
}

TEST(TryWriteInterpolatedStringHandlerTests, AppendFormatted_WithFormat_Ignored) {
    char buf[32];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendFormatted(255, std::string("X2"));
    EXPECT_FALSE(h.getString().empty());
}

// ---------------------------------------------------------------------------
// Failure propagation
// ---------------------------------------------------------------------------

TEST(TryWriteInterpolatedStringHandlerTests, AfterFailure_SubsequentAppends_False) {
    char buf[5];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendLiteral("hello");   // exactly fits
    bool ok = h.AppendLiteral("!"); // one too many
    EXPECT_FALSE(ok);
    EXPECT_FALSE(h.getSuccessProperty());
}

TEST(TryWriteInterpolatedStringHandlerTests, GetString_OnFailure_Empty) {
    char buf[3];
    TryWriteInterpolatedStringHandler h(buf, sizeof(buf));
    h.AppendLiteral("toolong");
    EXPECT_EQ(h.getString(), "");
}
