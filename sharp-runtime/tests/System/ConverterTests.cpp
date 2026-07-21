// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include "System/Converter.hpp"

using System::Converter;

TEST(ConverterTests, IntToString_Lambda) {
    Converter<int, std::string> conv = [](int v) { return std::to_string(v); };
    EXPECT_EQ(conv(42), "42");
}

TEST(ConverterTests, StringToInt_Lambda) {
    Converter<std::string, int> conv = [](const std::string& s) { return std::stoi(s); };
    EXPECT_EQ(conv("123"), 123);
}

TEST(ConverterTests, DoubleToFloat_Lambda) {
    Converter<double, float> conv = [](double d) { return static_cast<float>(d); };
    EXPECT_FLOAT_EQ(conv(3.14), 3.14f);
}

TEST(ConverterTests, IsCallable_BoolCheck) {
    Converter<int, bool> conv = [](int v) { return v > 0; };
    EXPECT_TRUE(conv(1));
    EXPECT_FALSE(conv(-1));
}
