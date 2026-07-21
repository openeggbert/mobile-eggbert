// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
// Tests for: ISpanParsable, IUtf8SpanFormattable, IUtf8SpanParsable
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <string>
#include "System/ISpanParsable.hpp"
#include "System/IUtf8SpanFormattable.hpp"
#include "System/IUtf8SpanParsable.hpp"

// ---------------------------------------------------------------------------
// ISpanParsable
// ---------------------------------------------------------------------------
struct SpanParsableInt : public System::ISpanParsable<SpanParsableInt> {
    int value = 0;
    // From IParsable<SpanParsableInt>
    SpanParsableInt Parse(const std::string& s, const System::IFormatProvider*) override {
        SpanParsableInt r; r.value = std::stoi(s); return r;
    }
    bool TryParse(const std::string& s, const System::IFormatProvider*, SpanParsableInt& res) override {
        try { res = Parse(s, nullptr); return true; } catch (...) { return false; }
    }
    // From ISpanParsable<SpanParsableInt>
    SpanParsableInt Parse(const System::ReadOnlySpan<char>& s, const System::IFormatProvider*) override {
        std::string str(s.begin(), s.end());
        SpanParsableInt r; r.value = std::stoi(str); return r;
    }
    bool TryParse(const System::ReadOnlySpan<char>& s, const System::IFormatProvider*, SpanParsableInt& res) override {
        try {
            std::string str(s.begin(), s.end());
            res.value = std::stoi(str); return true;
        } catch (...) { return false; }
    }
};

TEST(ISpanParsableTests2, ParseFromString) {
    SpanParsableInt p;
    auto r = p.Parse(std::string("42"), nullptr);
    EXPECT_EQ(r.value, 42);
}

TEST(ISpanParsableTests2, ParseFromSpan) {
    std::string s = "77";
    System::ReadOnlySpan<char> span(s.data(), s.size());
    SpanParsableInt p;
    auto r = p.Parse(span, nullptr);
    EXPECT_EQ(r.value, 77);
}

// ---------------------------------------------------------------------------
// IUtf8SpanFormattable
// ---------------------------------------------------------------------------
struct Utf8FormattableInt : public System::IUtf8SpanFormattable {
    int value;
    explicit Utf8FormattableInt(int v) : value(v) {}
    bool TryFormatUtf8(System::Span<uint8_t> dest, int& bytesWritten,
                       const std::string& /*fmt*/) const override {
        std::string s = std::to_string(value);
        if (static_cast<int>(s.size()) > dest.getLengthProperty()) { bytesWritten = 0; return false; }
        std::memcpy(dest.getPointer(), s.data(), s.size());
        bytesWritten = static_cast<int>(s.size());
        return true;
    }
};

TEST(IUtf8SpanFormattableTests2, TryFormatUtf8_WritesBytes) {
    Utf8FormattableInt fi(99);
    uint8_t buf[32];
    System::Span<uint8_t> span(buf, sizeof(buf));
    int written = 0;
    EXPECT_TRUE(fi.TryFormatUtf8(span, written, ""));
    EXPECT_EQ(written, 2);
    EXPECT_EQ(buf[0], '9');
    EXPECT_EQ(buf[1], '9');
}

TEST(IUtf8SpanFormattableTests2, TryFormatUtf8_WithProvider_IgnoresProviderAndDelegates) {
    // Default 4-arg TryFormatUtf8(..., provider) forwards to the 3-arg overload without
    // requiring implementers to override it. Invoked through the base interface, since the
    // derived class's 3-arg override otherwise hides the base's 4-arg overload from
    // unqualified name lookup.
    Utf8FormattableInt fi(7);
    const System::IUtf8SpanFormattable& base = fi;
    uint8_t buf[32];
    System::Span<uint8_t> span(buf, sizeof(buf));
    int written = 0;
    EXPECT_TRUE(base.TryFormatUtf8(span, written, "", nullptr));
    EXPECT_EQ(written, 1);
    EXPECT_EQ(buf[0], '7');
}

// ---------------------------------------------------------------------------
// IUtf8SpanParsable
// ---------------------------------------------------------------------------
struct Utf8ParsableInt : public System::IUtf8SpanParsable<Utf8ParsableInt> {
    int value = 0;
    Utf8ParsableInt ParseUtf8(const System::ReadOnlySpan<uint8_t>& s) const override {
        std::string str(reinterpret_cast<const char*>(s.getPointer()), s.getLengthProperty());
        Utf8ParsableInt r; r.value = std::stoi(str); return r;
    }
    bool TryParseUtf8(const System::ReadOnlySpan<uint8_t>& s, Utf8ParsableInt& result) const noexcept override {
        try {
            std::string str(reinterpret_cast<const char*>(s.getPointer()), s.getLengthProperty());
            result.value = std::stoi(str); return true;
        } catch (...) { return false; }
    }
};

TEST(IUtf8SpanParsableTests2, ParseUtf8_ReturnsValue) {
    uint8_t bytes[] = {'1', '2', '3'};
    System::Span<uint8_t> span(bytes, 3);
    Utf8ParsableInt p;
    auto r = p.ParseUtf8(span);
    EXPECT_EQ(r.value, 123);
}

TEST(IUtf8SpanParsableTests2, TryParseUtf8_InvalidInput_ReturnsFalse) {
    uint8_t bytes[] = {'x', 'y', 'z'};
    System::Span<uint8_t> span(bytes, 3);
    Utf8ParsableInt p;
    Utf8ParsableInt r;
    EXPECT_FALSE(p.TryParseUtf8(span, r));
}
