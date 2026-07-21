// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/AssemblyLoadEventArgs.hpp"

using System::AssemblyLoadEventArgs;
using System::AssemblyLoadEventHandler;

TEST(AssemblyLoadEventArgsTests, Ctor_StoresName) {
    AssemblyLoadEventArgs args("MyAssembly");
    EXPECT_EQ(args.getLoadedAssemblyProperty(), "MyAssembly");
}

TEST(AssemblyLoadEventArgsTests, Ctor_EmptyName) {
    AssemblyLoadEventArgs args("");
    EXPECT_TRUE(args.getLoadedAssemblyProperty().empty());
}

TEST(AssemblyLoadEventArgsTests, DerivesFromEventArgs) {
    AssemblyLoadEventArgs args("Test");
    System::EventArgs& base = args;
    (void)base;
    SUCCEED();
}

TEST(AssemblyLoadEventArgsTests, Handler_InvokedWithCorrectArgs) {
    std::string captured;
    AssemblyLoadEventHandler handler = [&](void*, AssemblyLoadEventArgs& a) {
        captured = a.getLoadedAssemblyProperty();
    };
    AssemblyLoadEventArgs args("SomeAssembly");
    handler(nullptr, args);
    EXPECT_EQ(captured, "SomeAssembly");
}
