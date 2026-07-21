// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/IUtf8SpanParsable.hpp"

// Concrete implementation for testing
struct Utf8Int : System::IUtf8SpanParsable<Utf8Int> {
    int value = 0;
    explicit Utf8Int(int v = 0) : value(v) {}

    Utf8Int ParseUtf8(const System::ReadOnlySpan<uint8_t>& utf8Text) const override {
        std::string s(reinterpret_cast<const char*>(utf8Text.getPointer()),
                      static_cast<std::size_t>(utf8Text.getLengthProperty()));
        return Utf8Int(std::stoi(s));
    }

    bool TryParseUtf8(const System::ReadOnlySpan<uint8_t>& utf8Text, Utf8Int& result) const noexcept override {
        try {
            result = ParseUtf8(utf8Text);
            return true;
        } catch (...) {
            result = Utf8Int(0);
            return false;
        }
    }
};

TEST(IUtf8SpanParsableTests, ParseUtf8_ValidNumber_CorrectResult) {
    std::vector<uint8_t> bytes = {'4', '2'};
    System::Span<uint8_t> span(bytes);
    Utf8Int parser;
    EXPECT_EQ(parser.ParseUtf8(span).value, 42);
}

TEST(IUtf8SpanParsableTests, TryParseUtf8_ValidNumber_ReturnsTrueAndValue) {
    std::vector<uint8_t> bytes = {'7'};
    System::Span<uint8_t> span(bytes);
    Utf8Int parser, result;
    EXPECT_TRUE(parser.TryParseUtf8(span, result));
    EXPECT_EQ(result.value, 7);
}

TEST(IUtf8SpanParsableTests, TryParseUtf8_Invalid_ReturnsFalse) {
    std::vector<uint8_t> bytes = {'x', 'y', 'z'};
    System::Span<uint8_t> span(bytes);
    Utf8Int parser, result;
    EXPECT_FALSE(parser.TryParseUtf8(span, result));
    EXPECT_EQ(result.value, 0);
}

TEST(IUtf8SpanParsableTests, ParseUtf8_InvalidInput_Throws) {
    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    System::Span<uint8_t> span(bytes);
    Utf8Int parser;
    EXPECT_THROW(parser.ParseUtf8(span), std::exception);
}

TEST(IUtf8SpanParsableTests, TryParseUtf8_Zero_CorrectValue) {
    std::vector<uint8_t> bytes = {'0'};
    System::Span<uint8_t> span(bytes);
    Utf8Int parser, result;
    EXPECT_TRUE(parser.TryParseUtf8(span, result));
    EXPECT_EQ(result.value, 0);
}

TEST(IUtf8SpanParsableTests, ParseUtf8_WithProvider_IgnoresProviderAndDelegates) {
    // Default 2-arg ParseUtf8(..., provider) forwards to the 1-arg overload without
    // requiring implementers to override it. Invoked through the base interface, since
    // the derived class's 1-arg override otherwise hides the base's 2-arg overload.
    std::vector<uint8_t> bytes = {'5'};
    System::Span<uint8_t> span(bytes);
    Utf8Int parser;
    const System::IUtf8SpanParsable<Utf8Int>& base = parser;
    EXPECT_EQ(base.ParseUtf8(span, nullptr).value, 5);
}

TEST(IUtf8SpanParsableTests, TryParseUtf8_WithProvider_IgnoresProviderAndDelegates) {
    std::vector<uint8_t> bytes = {'8'};
    System::Span<uint8_t> span(bytes);
    Utf8Int parser, result;
    const System::IUtf8SpanParsable<Utf8Int>& base = parser;
    EXPECT_TRUE(base.TryParseUtf8(span, nullptr, result));
    EXPECT_EQ(result.value, 8);
}
