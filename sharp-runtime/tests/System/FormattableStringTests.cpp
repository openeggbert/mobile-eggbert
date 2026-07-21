// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/FormattableString.hpp"

using System::FormattableString;

TEST(FormattableStringTests2, FormatProperty_ReturnsFormat) {
    FormattableString fs("Hello {0}!", {"World"});
    EXPECT_EQ(fs.getFormatProperty(), "Hello {0}!");
}

TEST(FormattableStringTests2, ArgumentCount_One) {
    FormattableString fs("{0}", {"x"});
    EXPECT_EQ(fs.getArgumentCountProperty(), 1);
}

TEST(FormattableStringTests2, ArgumentCount_Zero) {
    FormattableString fs("no args");
    EXPECT_EQ(fs.getArgumentCountProperty(), 0);
}

TEST(FormattableStringTests2, GetArgument_ReturnsCorrect) {
    FormattableString fs("{0} {1}", {"hello", "world"});
    EXPECT_EQ(fs.GetArgument(0), "hello");
    EXPECT_EQ(fs.GetArgument(1), "world");
}

TEST(FormattableStringTests2, GetArguments_ReturnsAll) {
    FormattableString fs("{0}", {"a"});
    auto args = fs.GetArguments();
    ASSERT_EQ(args.size(), 1u);
    EXPECT_EQ(args[0], "a");
}

TEST(FormattableStringTests2, ToString_SubstitutesPlaceholders) {
    FormattableString fs("Hello {0}!", {"World"});
    EXPECT_EQ(fs.ToString(), "Hello World!");
}

TEST(FormattableStringTests2, ToString_TwoPlaceholders) {
    FormattableString fs("{0} + {1}", {"2", "3"});
    EXPECT_EQ(fs.ToString(), "2 + 3");
}

TEST(FormattableStringTests2, ToString_NoPlaceholders) {
    FormattableString fs("static text");
    EXPECT_EQ(fs.ToString(), "static text");
}

TEST(FormattableStringTests2, Invariant_SameAsToString) {
    FormattableString fs("value={0}", {"42"});
    EXPECT_EQ(FormattableString::Invariant(fs), fs.ToString());
}

TEST(FormattableStringTests2, CurrentCulture_SameAsToString) {
    FormattableString fs("x={0}", {"1"});
    EXPECT_EQ(FormattableString::CurrentCulture(fs), fs.ToString());
}

TEST(FormattableStringTests2, GetArgument_OutOfRange_Throws) {
    FormattableString fs("{0}", {"a"});
    EXPECT_THROW(fs.GetArgument(5), System::IndexOutOfRangeException);
}
