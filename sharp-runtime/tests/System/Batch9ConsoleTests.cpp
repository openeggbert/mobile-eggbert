// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ConsoleColor.hpp"
#include "System/ConsoleKey.hpp"
#include "System/ConsoleKeyInfo.hpp"
#include "System/ConsoleModifiers.hpp"
#include "System/ConsoleSpecialKey.hpp"
#include "System/ConsoleCancelEventArgs.hpp"
#include "System/Console.hpp"

using System::ConsoleColor;
using System::ConsoleKey;
using System::ConsoleKeyInfo;
using System::ConsoleModifiers;
using System::ConsoleSpecialKey;
using System::ConsoleCancelEventArgs;
using System::ConsoleCancelEventHandler;
using System::Console;

// ===========================================================================
// ConsoleColor
// ===========================================================================

TEST(ConsoleColorTests, Black_IsZero)   { EXPECT_EQ(static_cast<int>(ConsoleColor::Black), 0); }
TEST(ConsoleColorTests, White_Is15)     { EXPECT_EQ(static_cast<int>(ConsoleColor::White), 15); }
TEST(ConsoleColorTests, Red_Is12)       { EXPECT_EQ(static_cast<int>(ConsoleColor::Red), 12); }
TEST(ConsoleColorTests, DarkGray_Is8)   { EXPECT_EQ(static_cast<int>(ConsoleColor::DarkGray), 8); }
TEST(ConsoleColorTests, Blue_Is9)       { EXPECT_EQ(static_cast<int>(ConsoleColor::Blue), 9); }
TEST(ConsoleColorTests, Green_Is10)     { EXPECT_EQ(static_cast<int>(ConsoleColor::Green), 10); }
TEST(ConsoleColorTests, Yellow_Is14)    { EXPECT_EQ(static_cast<int>(ConsoleColor::Yellow), 14); }
TEST(ConsoleColorTests, Distinct_Colors_Are_Different) {
    EXPECT_NE(ConsoleColor::Black, ConsoleColor::White);
    EXPECT_NE(ConsoleColor::Red,   ConsoleColor::Blue);
}

// ===========================================================================
// ConsoleKey
// ===========================================================================

TEST(ConsoleKeyTests, None_IsZero)         { EXPECT_EQ(static_cast<int>(ConsoleKey::None), 0x00); }
TEST(ConsoleKeyTests, Backspace_Is8)       { EXPECT_EQ(static_cast<int>(ConsoleKey::Backspace), 0x08); }
TEST(ConsoleKeyTests, Enter_Is13)          { EXPECT_EQ(static_cast<int>(ConsoleKey::Enter), 0x0D); }
TEST(ConsoleKeyTests, Escape_Is27)         { EXPECT_EQ(static_cast<int>(ConsoleKey::Escape), 0x1B); }
TEST(ConsoleKeyTests, A_Is0x41)            { EXPECT_EQ(static_cast<int>(ConsoleKey::A), 0x41); }
TEST(ConsoleKeyTests, Z_Is0x5A)            { EXPECT_EQ(static_cast<int>(ConsoleKey::Z), 0x5A); }
TEST(ConsoleKeyTests, D0_Is0x30)           { EXPECT_EQ(static_cast<int>(ConsoleKey::D0), 0x30); }
TEST(ConsoleKeyTests, F1_Is0x70)           { EXPECT_EQ(static_cast<int>(ConsoleKey::F1), 0x70); }
TEST(ConsoleKeyTests, F12_Is0x7B)          { EXPECT_EQ(static_cast<int>(ConsoleKey::F12), 0x7B); }
TEST(ConsoleKeyTests, LeftArrow_Is0x25)    { EXPECT_EQ(static_cast<int>(ConsoleKey::LeftArrow), 0x25); }
TEST(ConsoleKeyTests, Delete_Is0x2E)       { EXPECT_EQ(static_cast<int>(ConsoleKey::Delete), 0x2E); }
TEST(ConsoleKeyTests, NumPad0_Is0x60)      { EXPECT_EQ(static_cast<int>(ConsoleKey::NumPad0), 0x60); }

// ===========================================================================
// ConsoleModifiers
// ===========================================================================

TEST(ConsoleModifiersTests, None_IsZero)       { EXPECT_EQ(static_cast<int>(ConsoleModifiers::None), 0); }
TEST(ConsoleModifiersTests, Alt_IsOne)         { EXPECT_EQ(static_cast<int>(ConsoleModifiers::Alt), 1); }
TEST(ConsoleModifiersTests, Shift_IsTwo)       { EXPECT_EQ(static_cast<int>(ConsoleModifiers::Shift), 2); }
TEST(ConsoleModifiersTests, Control_IsFour)    { EXPECT_EQ(static_cast<int>(ConsoleModifiers::Control), 4); }

TEST(ConsoleModifiersTests, OrOperator_CombinesValues) {
    auto combined = ConsoleModifiers::Shift | ConsoleModifiers::Control;
    EXPECT_EQ(static_cast<int>(combined), 6);
}

TEST(ConsoleModifiersTests, AndOperator_MasksValue) {
    auto combined = ConsoleModifiers::Shift | ConsoleModifiers::Alt;
    auto masked   = combined & ConsoleModifiers::Shift;
    EXPECT_EQ(masked, ConsoleModifiers::Shift);
}

// ===========================================================================
// ConsoleSpecialKey
// ===========================================================================

TEST(ConsoleSpecialKeyTests, ControlC_IsZero)     { EXPECT_EQ(static_cast<int>(ConsoleSpecialKey::ControlC), 0); }
TEST(ConsoleSpecialKeyTests, ControlBreak_IsOne)  { EXPECT_EQ(static_cast<int>(ConsoleSpecialKey::ControlBreak), 1); }
TEST(ConsoleSpecialKeyTests, ControlC_NotBreak)   { EXPECT_NE(ConsoleSpecialKey::ControlC, ConsoleSpecialKey::ControlBreak); }

// ===========================================================================
// ConsoleKeyInfo
// ===========================================================================

TEST(ConsoleKeyInfoTests, DefaultConstructor_NullKey) {
    ConsoleKeyInfo ki;
    EXPECT_EQ(ki.getKeyCharProperty(), '\0');
    EXPECT_EQ(ki.getKeyProperty(), ConsoleKey::None);
    EXPECT_EQ(ki.getModifiersProperty(), ConsoleModifiers::None);
}

TEST(ConsoleKeyInfoTests, Constructor_StoresKeyChar) {
    ConsoleKeyInfo ki('A', ConsoleKey::A, false, false, false);
    EXPECT_EQ(ki.getKeyCharProperty(), 'A');
}

TEST(ConsoleKeyInfoTests, Constructor_StoresKey) {
    ConsoleKeyInfo ki('a', ConsoleKey::A, false, false, false);
    EXPECT_EQ(ki.getKeyProperty(), ConsoleKey::A);
}

TEST(ConsoleKeyInfoTests, Constructor_ShiftModifier) {
    ConsoleKeyInfo ki('A', ConsoleKey::A, true, false, false);
    EXPECT_NE(static_cast<int>(ki.getModifiersProperty() & ConsoleModifiers::Shift), 0);
}

TEST(ConsoleKeyInfoTests, Constructor_AltModifier) {
    ConsoleKeyInfo ki('a', ConsoleKey::A, false, true, false);
    EXPECT_NE(static_cast<int>(ki.getModifiersProperty() & ConsoleModifiers::Alt), 0);
}

TEST(ConsoleKeyInfoTests, Constructor_ControlModifier) {
    ConsoleKeyInfo ki('a', ConsoleKey::A, false, false, true);
    EXPECT_NE(static_cast<int>(ki.getModifiersProperty() & ConsoleModifiers::Control), 0);
}

TEST(ConsoleKeyInfoTests, Equality_SameValues) {
    ConsoleKeyInfo a('x', ConsoleKey::X, false, false, false);
    ConsoleKeyInfo b('x', ConsoleKey::X, false, false, false);
    EXPECT_TRUE(a == b);
}

TEST(ConsoleKeyInfoTests, Inequality_DifferentChar) {
    ConsoleKeyInfo a('a', ConsoleKey::A, false, false, false);
    ConsoleKeyInfo b('b', ConsoleKey::B, false, false, false);
    EXPECT_TRUE(a != b);
}

TEST(ConsoleKeyInfoTests, AllModifiers_Combined) {
    ConsoleKeyInfo ki('X', ConsoleKey::X, true, true, true);
    auto mods = ki.getModifiersProperty();
    EXPECT_NE(static_cast<int>(mods & ConsoleModifiers::Shift),   0);
    EXPECT_NE(static_cast<int>(mods & ConsoleModifiers::Alt),     0);
    EXPECT_NE(static_cast<int>(mods & ConsoleModifiers::Control), 0);
}

TEST(ConsoleKeyInfoTests, Constructor_ThrowsOnKeyOutOfRange) {
    EXPECT_THROW(
        ConsoleKeyInfo('\0', static_cast<ConsoleKey>(256), false, false, false),
        System::ArgumentOutOfRangeException
    );
}

// ===========================================================================
// ConsoleCancelEventArgs
// ===========================================================================

TEST(ConsoleCancelEventArgsTests, Constructor_StoresSpecialKey) {
    ConsoleCancelEventArgs args(ConsoleSpecialKey::ControlC);
    EXPECT_EQ(args.getSpecialKeyProperty(), ConsoleSpecialKey::ControlC);
}

TEST(ConsoleCancelEventArgsTests, Cancel_DefaultIsFalse) {
    ConsoleCancelEventArgs args(ConsoleSpecialKey::ControlBreak);
    EXPECT_FALSE(args.getCancelProperty());
}

TEST(ConsoleCancelEventArgsTests, Cancel_CanBeSetToTrue) {
    ConsoleCancelEventArgs args(ConsoleSpecialKey::ControlC);
    args.setCancelProperty(true);
    EXPECT_TRUE(args.getCancelProperty());
}

TEST(ConsoleCancelEventArgsTests, IsA_EventArgs) {
    ConsoleCancelEventArgs args(ConsoleSpecialKey::ControlC);
    EXPECT_NE(dynamic_cast<System::EventArgs*>(&args), nullptr);
}

// ===========================================================================
// ConsoleCancelEventHandler
// ===========================================================================

TEST(ConsoleCancelEventHandlerTests, Callable) {
    bool called = false;
    ConsoleCancelEventHandler h = [&](void*, ConsoleCancelEventArgs& e) {
        called = true;
        e.setCancelProperty(true);
    };
    ConsoleCancelEventArgs args(ConsoleSpecialKey::ControlC);
    h(nullptr, args);
    EXPECT_TRUE(called);
    EXPECT_TRUE(args.getCancelProperty());
}

// ===========================================================================
// Console new API
// ===========================================================================

TEST(ConsoleTests, Title_RoundTrip) {
    Console::setTitleProperty("TestTitle");
    EXPECT_EQ(Console::getTitleProperty(), "TestTitle");
    Console::setTitleProperty("");
}

TEST(ConsoleTests, KeyAvailable_ReturnsFalse) {
    EXPECT_FALSE(Console::getKeyAvailableProperty());
}

TEST(ConsoleTests, CursorVisible_DefaultTrue) {
    EXPECT_TRUE(Console::getCursorVisibleProperty());
}

TEST(ConsoleTests, CursorVisible_CanBeSetFalse) {
    Console::setCursorVisibleProperty(false);
    EXPECT_FALSE(Console::getCursorVisibleProperty());
    Console::setCursorVisibleProperty(true);
}

TEST(ConsoleTests, TreatControlCAsInput_DefaultFalse) {
    EXPECT_FALSE(Console::getTreatControlCAsInputProperty());
}

TEST(ConsoleTests, TreatControlCAsInput_RoundTrip) {
    Console::setTreatControlCAsInputProperty(true);
    EXPECT_TRUE(Console::getTreatControlCAsInputProperty());
    Console::setTreatControlCAsInputProperty(false);
}

TEST(ConsoleTests, AddCancelKeyPressHandler_DoesNotThrow) {
    EXPECT_NO_THROW(
        Console::addCancelKeyPressHandler([](void*, ConsoleCancelEventArgs&) {})
    );
}
