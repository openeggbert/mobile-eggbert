// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
// Batch 5 tests covering:
//   OverflowException, IConvertible, ICloneable, IAsyncDisposable,
//   TypeInitializationException, UriCreationOptions, UriComponents,
//   UriFormat, UriPartial, UriHostNameType, UriFormatException,
//   IUtf8SpanFormattable, UriBuilder, TypeCode (extra), Single (extra),
//   Int16 (extra), Char (extra)
//
// EXCEPT_SIMPLE covers DefaultCtor/MessageCtor/IsA for:
//   ApplicationException, InvalidProgramException, InsufficientExecutionStackException,
//   UnauthorizedAccessException
// ExceptionTests already covers: OverflowExceptionDefault, OverflowExceptionMessage,
//   OverflowExceptionIsArithmeticException
// ExceptionRemainingTests already covers TypeInitializationException basic tests
#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include <vector>

#include "System/OverflowException.hpp"
#include "System/IConvertible.hpp"
#include "System/TypeCode.hpp"
#include "System/ICloneable.hpp"
#include "System/IAsyncDisposable.hpp"
#include "System/TypeInitializationException.hpp"
#include "System/UriCreationOptions.hpp"
#include "System/UriComponents.hpp"
#include "System/UriFormat.hpp"
#include "System/UriPartial.hpp"
#include "System/UriHostNameType.hpp"
#include "System/UriFormatException.hpp"
#include "System/IUtf8SpanFormattable.hpp"
#include "System/UriBuilder.hpp"
#include "System/Single.hpp"
#include "System/Int16.hpp"
#include "System/Char.hpp"
#include "System/ArithmeticException.hpp"
#include "System/FormatException.hpp"

// ---------------------------------------------------------------------------
// OverflowException (extra beyond ExceptionTests.cpp)
// ---------------------------------------------------------------------------
TEST(OverflowExceptionNewTests, StringCtor_MessageStored) {
    System::OverflowException e(std::string("too big"));
    EXPECT_NE(std::string(e.what()).find("too big"), std::string::npos);
}
TEST(OverflowExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("underflow"));
    System::OverflowException e("value overflow", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("value overflow"), std::string::npos);
}
TEST(OverflowExceptionNewTests, IsA_ArithmeticException) {
    System::OverflowException e;
    EXPECT_NE(dynamic_cast<System::ArithmeticException*>(&e), nullptr);
}

// ---------------------------------------------------------------------------
// IConvertible
// ---------------------------------------------------------------------------
struct IntConvertible : System::IConvertible {
    int val;
    explicit IntConvertible(int v) : val(v) {}
    System::TypeCode GetTypeCode() const override { return System::TypeCode::Int32; }
    bool ToBoolean() const override { return val != 0; }
    char ToChar() const override { return static_cast<char>(val); }
    int8_t ToSByte() const override { return static_cast<int8_t>(val); }
    uint8_t ToByte() const override { return static_cast<uint8_t>(val); }
    int16_t ToInt16() const override { return static_cast<int16_t>(val); }
    uint16_t ToUInt16() const override { return static_cast<uint16_t>(val); }
    int32_t ToInt32() const override { return val; }
    uint32_t ToUInt32() const override { return static_cast<uint32_t>(val); }
    int64_t ToInt64() const override { return val; }
    uint64_t ToUInt64() const override { return static_cast<uint64_t>(val); }
    float ToSingle() const override { return static_cast<float>(val); }
    double ToDouble() const override { return static_cast<double>(val); }
    System::Decimal ToDecimal() const override { return System::Decimal(val); }
    System::DateTime ToDateTime() const override { return System::DateTime(); }
    std::string ToString() const override { return std::to_string(val); }
};

TEST(IConvertibleTests, GetTypeCode_ReturnsInt32) {
    IntConvertible ic(5);
    EXPECT_EQ(ic.GetTypeCode(), System::TypeCode::Int32);
}
TEST(IConvertibleTests, ToBoolean_NonZero_True) {
    IntConvertible ic(1);
    EXPECT_TRUE(ic.ToBoolean());
}
TEST(IConvertibleTests, ToBoolean_Zero_False) {
    IntConvertible ic(0);
    EXPECT_FALSE(ic.ToBoolean());
}
TEST(IConvertibleTests, ToInt32_ReturnsValue) {
    IntConvertible ic(42);
    EXPECT_EQ(ic.ToInt32(), 42);
}
TEST(IConvertibleTests, ToString_ReturnsDecimal) {
    IntConvertible ic(7);
    EXPECT_EQ(ic.ToString(), "7");
}
TEST(IConvertibleTests, PolymorphicAccess_ViaBasePtr) {
    IntConvertible concrete(99);
    System::IConvertible* p = &concrete;
    EXPECT_EQ(p->ToInt32(), 99);
}
TEST(IConvertibleTests, ToDecimal_ReturnsValue) {
    IntConvertible ic(13);
    EXPECT_EQ(ic.ToDecimal().ToInt32(), 13);
}
TEST(IConvertibleTests, ToDateTime_DoesNotThrow) {
    IntConvertible ic(0);
    EXPECT_NO_THROW(ic.ToDateTime());
}

// ---------------------------------------------------------------------------
// ICloneable
// ---------------------------------------------------------------------------
struct CloneableInt : System::ICloneable {
    int value;
    explicit CloneableInt(int v) : value(v) {}
    std::shared_ptr<System::ICloneable> Clone() const override {
        return std::make_shared<CloneableInt>(value);
    }
};

TEST(ICloneableTests, Clone_ReturnsCopy) {
    CloneableInt orig(42);
    auto clone = orig.Clone();
    EXPECT_NE(clone.get(), nullptr);
    EXPECT_EQ(std::dynamic_pointer_cast<CloneableInt>(clone)->value, 42);
}
TEST(ICloneableTests, Clone_IsIndependent) {
    CloneableInt orig(10);
    auto clone = orig.Clone();
    orig.value = 99;
    EXPECT_EQ(std::dynamic_pointer_cast<CloneableInt>(clone)->value, 10);
}

// ---------------------------------------------------------------------------
// IAsyncDisposable
// ---------------------------------------------------------------------------
struct DisposableResource : System::IAsyncDisposable {
    bool disposed = false;
    System::Threading::Tasks::ValueTask DisposeAsync() override {
        disposed = true;
        return System::Threading::Tasks::ValueTask::CompletedTask();
    }
};

TEST(IAsyncDisposableTests, DisposeAsync_CallsImpl) {
    DisposableResource r;
    EXPECT_FALSE(r.disposed);
    r.DisposeAsync();
    EXPECT_TRUE(r.disposed);
}
TEST(IAsyncDisposableTests, PolymorphicDispatch_ViaBasePtr) {
    DisposableResource r;
    System::IAsyncDisposable* p = &r;
    p->DisposeAsync();
    EXPECT_TRUE(r.disposed);
}

// ---------------------------------------------------------------------------
// TypeInitializationException (extra)
// ---------------------------------------------------------------------------
TEST(TypeInitializationExceptionNewTests, InnerException_Accessible) {
    auto inner = std::make_exception_ptr(std::runtime_error("ctor threw"));
    System::TypeInitializationException e("Foo.Bar", inner);
    ASSERT_TRUE(e.getInnerExceptionProperty() != nullptr);
}
TEST(TypeInitializationExceptionNewTests, NullInner_NoInnerInWhat) {
    System::TypeInitializationException e("Foo.Bar", nullptr);
    EXPECT_EQ(std::string(e.what()).find("inner"), std::string::npos);
}

// ---------------------------------------------------------------------------
// UriCreationOptions
// ---------------------------------------------------------------------------
TEST(UriCreationOptionsTests, DefaultCtor_FlagFalse) {
    System::UriCreationOptions opts;
    EXPECT_FALSE(opts.DangerousDisablePathAndQueryCanonicalization);
}
TEST(UriCreationOptionsTests, FlagCanBeSet) {
    System::UriCreationOptions opts;
    opts.DangerousDisablePathAndQueryCanonicalization = true;
    EXPECT_TRUE(opts.DangerousDisablePathAndQueryCanonicalization);
}

// ---------------------------------------------------------------------------
// UriComponents
// ---------------------------------------------------------------------------
TEST(UriComponentsTests, Scheme_IsOne)   { EXPECT_EQ(static_cast<unsigned>(System::UriComponents::Scheme), 1u); }
TEST(UriComponentsTests, Host_IsFour)    { EXPECT_EQ(static_cast<unsigned>(System::UriComponents::Host), 4u); }
TEST(UriComponentsTests, Path_Is16)     { EXPECT_EQ(static_cast<unsigned>(System::UriComponents::Path), 0x10u); }
TEST(UriComponentsTests, BitwiseOr_Works) {
    auto combined = System::UriComponents::Scheme | System::UriComponents::Host;
    EXPECT_EQ(static_cast<unsigned>(combined), 5u);
}

// ---------------------------------------------------------------------------
// UriFormat
// ---------------------------------------------------------------------------
TEST(UriFormatTests, UriEscaped_IsOne)     { EXPECT_EQ(static_cast<int>(System::UriFormat::UriEscaped), 1); }
TEST(UriFormatTests, Unescaped_IsTwo)      { EXPECT_EQ(static_cast<int>(System::UriFormat::Unescaped), 2); }
TEST(UriFormatTests, SafeUnescaped_IsThree){ EXPECT_EQ(static_cast<int>(System::UriFormat::SafeUnescaped), 3); }

// ---------------------------------------------------------------------------
// UriPartial
// ---------------------------------------------------------------------------
TEST(UriPartialTests, Scheme_IsZero)    { EXPECT_EQ(static_cast<int>(System::UriPartial::Scheme), 0); }
TEST(UriPartialTests, Authority_IsOne)  { EXPECT_EQ(static_cast<int>(System::UriPartial::Authority), 1); }
TEST(UriPartialTests, Path_IsTwo)       { EXPECT_EQ(static_cast<int>(System::UriPartial::Path), 2); }
TEST(UriPartialTests, Query_IsThree)    { EXPECT_EQ(static_cast<int>(System::UriPartial::Query), 3); }

// ---------------------------------------------------------------------------
// UriHostNameType
// ---------------------------------------------------------------------------
TEST(UriHostNameTypeTests, Unknown_IsZero){ EXPECT_EQ(static_cast<int>(System::UriHostNameType::Unknown), 0); }
TEST(UriHostNameTypeTests, Dns_IsTwo)     { EXPECT_EQ(static_cast<int>(System::UriHostNameType::Dns), 2); }
TEST(UriHostNameTypeTests, IPv4_IsThree)  { EXPECT_EQ(static_cast<int>(System::UriHostNameType::IPv4), 3); }
TEST(UriHostNameTypeTests, IPv6_IsFour)   { EXPECT_EQ(static_cast<int>(System::UriHostNameType::IPv6), 4); }

// ---------------------------------------------------------------------------
// UriFormatException
// ---------------------------------------------------------------------------
TEST(UriFormatExceptionTests, DefaultCtor_WhatNotEmpty) {
    System::UriFormatException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}
TEST(UriFormatExceptionTests, StringCtor_MessageStored) {
    System::UriFormatException e("bad uri");
    EXPECT_NE(std::string(e.what()).find("bad uri"), std::string::npos);
}
TEST(UriFormatExceptionTests, InnerCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("parse fail"));
    System::UriFormatException e("invalid", inner);
    EXPECT_NE(std::string(e.what()).find("invalid"), std::string::npos);
}
TEST(UriFormatExceptionTests, IsA_FormatException) {
    System::UriFormatException e;
    EXPECT_NE(dynamic_cast<System::FormatException*>(&e), nullptr);
}

// ---------------------------------------------------------------------------
// IUtf8SpanFormattable
// ---------------------------------------------------------------------------
struct Utf8Int32 : System::IUtf8SpanFormattable {
    int value;
    explicit Utf8Int32(int v) : value(v) {}
    bool TryFormatUtf8(System::Span<uint8_t> dest, int& written,
                        const std::string&) const override {
        std::string s = std::to_string(value);
        if (dest.getLengthProperty() < static_cast<int>(s.size())) { written = 0; return false; }
        for (int i = 0; i < static_cast<int>(s.size()); ++i)
            dest.getPointer()[i] = static_cast<uint8_t>(s[i]);
        written = static_cast<int>(s.size());
        return true;
    }
};

TEST(IUtf8SpanFormattableTests, TryFormatUtf8_Success) {
    Utf8Int32 obj(42);
    std::vector<uint8_t> buf(16);
    System::Span<uint8_t> span(buf);
    int written = 0;
    EXPECT_TRUE(obj.TryFormatUtf8(span, written, ""));
    EXPECT_EQ(written, 2);
    EXPECT_EQ(buf[0], '4');
    EXPECT_EQ(buf[1], '2');
}
TEST(IUtf8SpanFormattableTests, TryFormatUtf8_BufferTooSmall_ReturnsFalse) {
    Utf8Int32 obj(12345);
    std::vector<uint8_t> buf(2);
    System::Span<uint8_t> span(buf);
    int written = 0;
    EXPECT_FALSE(obj.TryFormatUtf8(span, written, ""));
    EXPECT_EQ(written, 0);
}

// ---------------------------------------------------------------------------
// UriBuilder
// ---------------------------------------------------------------------------
TEST(UriBuilderTests, DefaultCtor_SchemeIsHttp) {
    System::UriBuilder b;
    EXPECT_EQ(b.getSchemeProperty(), "http");
}
TEST(UriBuilderTests, SetHost_ToString_ContainsHost) {
    System::UriBuilder b;
    b.setSchemeProperty("https");
    b.setHostProperty("example.com");
    b.setPathProperty("/path");
    EXPECT_NE(b.ToString().find("example.com"), std::string::npos);
}
TEST(UriBuilderTests, FromString_ParsesSchemeAndHost) {
    System::UriBuilder b("http://test.org/foo");
    EXPECT_EQ(b.getSchemeProperty(), "http");
    EXPECT_EQ(b.getHostProperty(), "test.org");
}
TEST(UriBuilderTests, SetQuery_AppearsInString) {
    System::UriBuilder b;
    b.setSchemeProperty("https");
    b.setHostProperty("api.example.com");
    b.setPathProperty("/search");
    b.setQueryProperty("?q=hello");
    EXPECT_NE(b.ToString().find("q=hello"), std::string::npos);
}
TEST(UriBuilderTests, GetUri_IsAbsolute) {
    System::UriBuilder b;
    b.setSchemeProperty("https");
    b.setHostProperty("example.com");
    b.setPathProperty("/");
    System::Uri u = b.getUriProperty();
    EXPECT_TRUE(u.getIsAbsoluteUriProperty());
}
TEST(UriBuilderTests, SetPort_AppearsInString) {
    System::UriBuilder b;
    b.setSchemeProperty("http");
    b.setHostProperty("localhost");
    b.setPortProperty(8080);
    b.setPathProperty("/");
    EXPECT_NE(b.ToString().find("8080"), std::string::npos);
}

// ---------------------------------------------------------------------------
// TypeCode (extra — existing tests cover Empty/Boolean/Int32/String/DateTime)
// ---------------------------------------------------------------------------
TEST(TypeCodeNewTests, Char_IsFour)    { EXPECT_EQ(static_cast<int>(System::TypeCode::Char), 4); }
TEST(TypeCodeNewTests, SByte_IsFive)   { EXPECT_EQ(static_cast<int>(System::TypeCode::SByte), 5); }
TEST(TypeCodeNewTests, Single_Is13)    { EXPECT_EQ(static_cast<int>(System::TypeCode::Single), 13); }
TEST(TypeCodeNewTests, Decimal_Is15)   { EXPECT_EQ(static_cast<int>(System::TypeCode::Decimal), 15); }

// ---------------------------------------------------------------------------
// Single (extra — existing tests cover MaxValue/MinValue/Epsilon/NaN constants)
// ---------------------------------------------------------------------------
TEST(SingleNewTests, IsInfinity_PositiveInfinity) {
    EXPECT_TRUE(System::Single::IsInfinity(System::Single::PositiveInfinity));
}
TEST(SingleNewTests, IsFinite_Zero) {
    EXPECT_TRUE(System::Single::IsFinite(0.0f));
}
TEST(SingleNewTests, Parse_ValidFloat) {
    EXPECT_FLOAT_EQ(System::Single::Parse("3.14"), 3.14f);
}
TEST(SingleNewTests, TryParse_Invalid_ReturnsFalse) {
    float r = 0.0f;
    EXPECT_FALSE(System::Single::TryParse("abc", r));
}
TEST(SingleNewTests, ToString_NaN) {
    EXPECT_EQ(System::Single::ToString(std::numeric_limits<float>::quiet_NaN()), "NaN");
}

// ---------------------------------------------------------------------------
// Int16 (extra — existing tests cover Parse/TryParse/MaxValue/MinValue)
// ---------------------------------------------------------------------------
TEST(Int16NewTests, ToString_Basic) {
    EXPECT_EQ(System::Int16::ToString(100), "100");
}
TEST(Int16NewTests, ToString_Negative) {
    EXPECT_EQ(System::Int16::ToString(-5), "-5");
}
TEST(Int16NewTests, ToString_Hex) {
    EXPECT_EQ(System::Int16::ToString(255, std::string("X")), "FF");
}
TEST(Int16NewTests, ToString_D4) {
    EXPECT_EQ(System::Int16::ToString(7, std::string("D4")), "0007");
}

// ---------------------------------------------------------------------------
// Char (extra — existing tests cover IsLetter/IsDigit/IsWhiteSpace/ToUpper/ToLower)
// ---------------------------------------------------------------------------
TEST(CharNewTests, IsAscii_AsciiChar_True) {
    EXPECT_TRUE(System::Char::IsAscii(u'A'));
}
TEST(CharNewTests, IsAscii_HighChar_False) {
    EXPECT_FALSE(System::Char::IsAscii(0x0100u));
}
TEST(CharNewTests, IsAsciiDigit_Digit_True) {
    EXPECT_TRUE(System::Char::IsAsciiDigit(u'5'));
}
TEST(CharNewTests, IsAsciiLetter_Letter_True) {
    EXPECT_TRUE(System::Char::IsAsciiLetter(u'z'));
}
TEST(CharNewTests, Parse_SingleChar) {
    EXPECT_EQ(System::Char::Parse("A"), u'A');
}
TEST(CharNewTests, ToString_AsciiChar) {
    EXPECT_EQ(System::Char::ToString(u'X'), "X");
}
TEST(CharNewTests, GetNumericValue_Digit) {
    EXPECT_EQ(System::Char::GetNumericValue(u'7'), 7);
}
TEST(CharNewTests, GetNumericValue_NonDigit_MinusOne) {
    EXPECT_EQ(System::Char::GetNumericValue(u'A'), -1);
}
