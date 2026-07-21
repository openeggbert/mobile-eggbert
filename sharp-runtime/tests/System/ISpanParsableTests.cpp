// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "System/ISpanParsable.hpp"
#include "System/IParsable.hpp"
#include "System/FormatException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

using SharpRuntime::intcs;

namespace {
    /**
     * Minimal concrete type that implements ISpanParsable<IntParser>.
     * Parses decimal integer strings.
     */
    struct IntParser : public System::ISpanParsable<IntParser> {
        int value = 0;

        IntParser() = default;
        explicit IntParser(int v) : value(v) {}

        // --- IParsable<IntParser> ---

        IntParser Parse(const std::string& s,
                        const System::IFormatProvider* /*provider*/) override {
            try { return IntParser(std::stoi(s)); }
            catch (...) { throw System::FormatException("Invalid integer: " + s); }
        }

        bool TryParse(const std::string& s,
                      const System::IFormatProvider* /*provider*/,
                      IntParser& result) override {
            try { result = IntParser(std::stoi(s)); return true; }
            catch (...) { return false; }
        }

        // --- ISpanParsable<IntParser> ---

        IntParser Parse(const System::ReadOnlySpan<char>& s,
                        const System::IFormatProvider* provider) override {
            std::string str(s.getPointer(), static_cast<std::size_t>(s.getLengthProperty()));
            return Parse(str, provider);
        }

        bool TryParse(const System::ReadOnlySpan<char>& s,
                      const System::IFormatProvider* provider,
                      IntParser& result) override {
            std::string str(s.getPointer(), static_cast<std::size_t>(s.getLengthProperty()));
            return TryParse(str, provider, result);
        }
    };
} // anonymous namespace

// ---------------------------------------------------------------------------
// Type hierarchy
// ---------------------------------------------------------------------------

TEST(ISpanParsableTests, IsA_IParsable) {
    IntParser p;
    EXPECT_NE(dynamic_cast<System::IParsable<IntParser>*>(&p), nullptr);
}

TEST(ISpanParsableTests, IsA_ISpanParsable) {
    IntParser p;
    EXPECT_NE(dynamic_cast<System::ISpanParsable<IntParser>*>(&p), nullptr);
}

// ---------------------------------------------------------------------------
// IParsable (std::string) overloads
// ---------------------------------------------------------------------------

TEST(ISpanParsableTests, Parse_String_ValidInt) {
    IntParser p;
    IntParser r = p.Parse("42", nullptr);
    EXPECT_EQ(r.value, 42);
}

TEST(ISpanParsableTests, Parse_String_NegativeInt) {
    IntParser p;
    EXPECT_EQ(p.Parse("-7", nullptr).value, -7);
}

TEST(ISpanParsableTests, Parse_String_Invalid_Throws) {
    IntParser p;
    EXPECT_THROW(p.Parse("abc", nullptr), System::FormatException);
}

TEST(ISpanParsableTests, TryParse_String_Valid_ReturnsTrue) {
    IntParser p;
    IntParser result;
    EXPECT_TRUE(p.TryParse("123", nullptr, result));
    EXPECT_EQ(result.value, 123);
}

TEST(ISpanParsableTests, TryParse_String_Invalid_ReturnsFalse) {
    IntParser p;
    IntParser result;
    EXPECT_FALSE(p.TryParse("not-a-number", nullptr, result));
}

TEST(ISpanParsableTests, TryParse_String_Zero) {
    IntParser p;
    IntParser result;
    EXPECT_TRUE(p.TryParse("0", nullptr, result));
    EXPECT_EQ(result.value, 0);
}

// ---------------------------------------------------------------------------
// ISpanParsable (ReadOnlySpan<char>) overloads
// ---------------------------------------------------------------------------

TEST(ISpanParsableTests, Parse_Span_ValidInt) {
    IntParser p;
    std::string s = "99";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_EQ(p.Parse(span, nullptr).value, 99);
}

TEST(ISpanParsableTests, Parse_Span_NegativeInt) {
    IntParser p;
    std::string s = "-5";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_EQ(p.Parse(span, nullptr).value, -5);
}

TEST(ISpanParsableTests, Parse_Span_Invalid_Throws) {
    IntParser p;
    std::string s = "bad";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_THROW(p.Parse(span, nullptr), System::FormatException);
}

TEST(ISpanParsableTests, TryParse_Span_Valid_ReturnsTrue) {
    IntParser p;
    IntParser result;
    std::string s = "77";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_TRUE(p.TryParse(span, nullptr, result));
    EXPECT_EQ(result.value, 77);
}

TEST(ISpanParsableTests, TryParse_Span_Invalid_ReturnsFalse) {
    IntParser p;
    IntParser result;
    std::string s = "xyz";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_FALSE(p.TryParse(span, nullptr, result));
}

TEST(ISpanParsableTests, TryParse_Span_RoundTrip) {
    IntParser p;
    IntParser result;
    std::string s = "1000";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_TRUE(p.TryParse(span, nullptr, result));
    EXPECT_EQ(result.value, 1000);
}

// ---------------------------------------------------------------------------
// Polymorphic dispatch via base pointer
// ---------------------------------------------------------------------------

TEST(ISpanParsableTests, PolyDispatch_Parse_Span_ViaBasePtr) {
    IntParser concrete;
    System::ISpanParsable<IntParser>* iface = &concrete;
    std::string s = "55";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_EQ(iface->Parse(span, nullptr).value, 55);
}

TEST(ISpanParsableTests, PolyDispatch_TryParse_Span_ViaBasePtr) {
    IntParser concrete;
    System::ISpanParsable<IntParser>* iface = &concrete;
    IntParser result;
    std::string s = "33";
    System::ReadOnlySpan<char> span(s.data(), static_cast<intcs>(s.size()));
    EXPECT_TRUE(iface->TryParse(span, nullptr, result));
    EXPECT_EQ(result.value, 33);
}
