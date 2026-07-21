// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Console.hpp"

using System::Console;

// ---------------------------------------------------------------------------
// NewLine constant
// ---------------------------------------------------------------------------

TEST(ConsoleTests, NewLine_IsNonEmpty) {
    EXPECT_FALSE(Console::NewLine.empty());
}

// ---------------------------------------------------------------------------
// Write overloads — must not throw (output to stdout is tested implicitly)
// ---------------------------------------------------------------------------

TEST(ConsoleTests, Write_String_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write(std::string("hello")));
}

TEST(ConsoleTests, Write_CString_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write("world"));
}

TEST(ConsoleTests, Write_Char_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write('A'));
}

TEST(ConsoleTests, Write_Int_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write(static_cast<SharpRuntime::intcs>(42)));
}

TEST(ConsoleTests, Write_Long_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write(static_cast<SharpRuntime::longcs>(1234567890LL)));
}

TEST(ConsoleTests, Write_Double_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write(3.14));
}

TEST(ConsoleTests, Write_Float_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write(2.71f));
}

TEST(ConsoleTests, Write_BoolTrue_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write(true));
}

TEST(ConsoleTests, Write_BoolFalse_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write(false));
}

// ---------------------------------------------------------------------------
// WriteLine overloads
// ---------------------------------------------------------------------------

TEST(ConsoleTests, WriteLine_NoArg_DoesNotThrow) {
    EXPECT_NO_THROW(Console::WriteLine());
}

TEST(ConsoleTests, WriteLine_String_DoesNotThrow) {
    EXPECT_NO_THROW(Console::WriteLine(std::string("line")));
}

TEST(ConsoleTests, WriteLine_CString_DoesNotThrow) {
    EXPECT_NO_THROW(Console::WriteLine("cstr line"));
}

TEST(ConsoleTests, WriteLine_Int_DoesNotThrow) {
    EXPECT_NO_THROW(Console::WriteLine(static_cast<SharpRuntime::intcs>(0)));
}

TEST(ConsoleTests, WriteLine_Bool_DoesNotThrow) {
    EXPECT_NO_THROW(Console::WriteLine(true));
}

// ---------------------------------------------------------------------------
// Error stream
// ---------------------------------------------------------------------------

TEST(ConsoleTests, Error_Write_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Error_Write("err"));
}

TEST(ConsoleTests, Error_WriteLine_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Error_WriteLine("err line"));
}

// ---------------------------------------------------------------------------
// Format-string Write / WriteLine
// ---------------------------------------------------------------------------

TEST(ConsoleTests, Write_Format_IntArg_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write("val={0}", static_cast<SharpRuntime::intcs>(42)));
}

TEST(ConsoleTests, Write_Format_DoubleArg_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write("val={0}", 3.14));
}

TEST(ConsoleTests, Write_Format_StringArg_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write("hello {0}", std::string("world")));
}

TEST(ConsoleTests, Write_Format_TwoIntArgs_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Write("{0}+{1}", static_cast<SharpRuntime::intcs>(1), static_cast<SharpRuntime::intcs>(2)));
}

TEST(ConsoleTests, WriteLine_Format_IntArg_DoesNotThrow) {
    EXPECT_NO_THROW(Console::WriteLine("n={0}", static_cast<SharpRuntime::intcs>(7)));
}

TEST(ConsoleTests, WriteLine_Format_StringArg_DoesNotThrow) {
    EXPECT_NO_THROW(Console::WriteLine("{0}!", std::string("hi")));
}

// ---------------------------------------------------------------------------
// Colors and cursor (new API)
// ---------------------------------------------------------------------------

TEST(ConsoleTests, SetForegroundColor_DoesNotThrow) {
    EXPECT_NO_THROW(Console::setForegroundColorProperty(System::ConsoleColor::Red));
    Console::ResetColor();
}

TEST(ConsoleTests, SetBackgroundColor_DoesNotThrow) {
    EXPECT_NO_THROW(Console::setBackgroundColorProperty(System::ConsoleColor::Blue));
    Console::ResetColor();
}

TEST(ConsoleTests, GetForegroundColor_AfterSet) {
    Console::setForegroundColorProperty(System::ConsoleColor::Green);
    EXPECT_EQ(Console::getForegroundColorProperty(), System::ConsoleColor::Green);
    Console::ResetColor();
}

TEST(ConsoleTests, ResetColor_DoesNotThrow) {
    EXPECT_NO_THROW(Console::ResetColor());
}

TEST(ConsoleTests, Clear_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Clear());
}

TEST(ConsoleTests, WindowWidth_Positive) {
    EXPECT_GT(Console::getWindowWidthProperty(), 0);
}

TEST(ConsoleTests, WindowHeight_Positive) {
    EXPECT_GT(Console::getWindowHeightProperty(), 0);
}

TEST(ConsoleTests, Beep_DoesNotThrow) {
    EXPECT_NO_THROW(Console::Beep());
}

TEST(ConsoleTests, SetCursorPosition_DoesNotThrow) {
    EXPECT_NO_THROW(Console::SetCursorPosition(0, 0));
}
