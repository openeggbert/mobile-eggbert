// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include "System/Func.hpp"

TEST(FuncTests, Func_NoArg_ReturnsInt) {
    System::Func<int> f = [] { return 42; };
    EXPECT_EQ(f(), 42);
}

TEST(FuncTests, FuncT_OneArg_ReturnsDouble) {
    System::FuncT<int, double> f = [](int x) { return x * 2.5; };
    EXPECT_DOUBLE_EQ(f(4), 10.0);
}

TEST(FuncTests, FuncT2_TwoArgs_ReturnsSum) {
    System::FuncT2<int, int, int> f = [](int a, int b) { return a + b; };
    EXPECT_EQ(f(3, 4), 7);
}

TEST(FuncTests, FuncT3_ThreeArgs_ReturnsConcatenated) {
    System::FuncT3<std::string, std::string, std::string, std::string> f =
        [](std::string a, std::string b, std::string c) { return a + b + c; };
    EXPECT_EQ(f("x", "y", "z"), "xyz");
}

TEST(FuncTests, FuncT4_FourArgs_ReturnsSum) {
    System::FuncT4<int, int, int, int, int> f = [](int a, int b, int c, int d) { return a + b + c + d; };
    EXPECT_EQ(f(1, 2, 3, 4), 10);
}

TEST(FuncTests, FuncT8_EightArgs_ReturnsSum) {
    System::FuncT8<int, int, int, int, int, int, int, int, int> f =
        [](int a, int b, int c, int d, int e, int g, int h, int i) {
            return a + b + c + d + e + g + h + i;
        };
    EXPECT_EQ(f(1, 2, 3, 4, 5, 6, 7, 8), 36);
}

TEST(FuncTests, FuncT16_SixteenArgs_ReturnsSum) {
    System::FuncT16<int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int> f =
        [](int a, int b, int c, int d, int e, int g, int h, int i, int j, int k, int l, int m,
           int n, int o, int p, int q) {
            return a + b + c + d + e + g + h + i + j + k + l + m + n + o + p + q;
        };
    EXPECT_EQ(f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), 136);
}
