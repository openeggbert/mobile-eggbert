// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ArgumentException.hpp"

using System::ArgumentOutOfRangeException;
using System::ArgumentException;

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, DefaultCtor_MessageNotEmpty) {
    ArgumentOutOfRangeException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(ArgumentOutOfRangeExceptionTests, DefaultCtor_ActualValue_Empty) {
    ArgumentOutOfRangeException ex;
    EXPECT_TRUE(ex.getActualValueProperty().empty());
}

TEST(ArgumentOutOfRangeExceptionTests, HResult_MatchesCorEArgumentOutOfRange) {
    // Regression: previously inherited ArgumentException's HResult instead of .NET's
    // distinct HResults.COR_E_ARGUMENTOUTOFRANGE (ArgumentOutOfRangeException.cs).
    ArgumentOutOfRangeException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131502));
}

TEST(ArgumentOutOfRangeExceptionTests, DefaultCtor_ParamName_Empty) {
    ArgumentOutOfRangeException ex;
    EXPECT_TRUE(ex.getParamNameProperty().empty());
}

TEST(ArgumentOutOfRangeExceptionTests, CharPtrCtor_StoresParamName) {
    ArgumentOutOfRangeException ex("tooLarge");
    EXPECT_EQ(ex.getParamNameProperty(), "tooLarge");
    EXPECT_NE(std::string(ex.what()).find("tooLarge"), std::string::npos);
}

TEST(ArgumentOutOfRangeExceptionTests, StringCtor_StoresParamName) {
    ArgumentOutOfRangeException ex(std::string("outOfBounds"));
    EXPECT_EQ(ex.getParamNameProperty(), "outOfBounds");
    EXPECT_NE(std::string(ex.what()).find("outOfBounds"), std::string::npos);
}

TEST(ArgumentOutOfRangeExceptionTests, ParamNameAndMessageCtor_BothStored) {
    ArgumentOutOfRangeException ex("index", "must be non-negative");
    EXPECT_EQ(ex.getParamNameProperty(), "index");
    EXPECT_NE(std::string(ex.what()).find("must be non-negative"), std::string::npos);
}

TEST(ArgumentOutOfRangeExceptionTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    ArgumentOutOfRangeException ex("outer msg", inner);
    EXPECT_NE(std::string(ex.what()).find("outer msg"), std::string::npos);
}

TEST(ArgumentOutOfRangeExceptionTests, InnerExceptionCtor_ActualValue_Empty) {
    auto inner = std::make_exception_ptr(std::runtime_error("x"));
    ArgumentOutOfRangeException ex("msg", inner);
    EXPECT_TRUE(ex.getActualValueProperty().empty());
}

TEST(ArgumentOutOfRangeExceptionTests, ThreeArgCtor_StoresParamName) {
    ArgumentOutOfRangeException ex("index", "42", "Index was out of range.");
    EXPECT_EQ(ex.getParamNameProperty(), "index");
}

TEST(ArgumentOutOfRangeExceptionTests, ThreeArgCtor_StoresActualValue) {
    ArgumentOutOfRangeException ex("index", "42", "Index was out of range.");
    EXPECT_EQ(ex.getActualValueProperty(), "42");
}

TEST(ArgumentOutOfRangeExceptionTests, ThreeArgCtor_StoresMessage) {
    ArgumentOutOfRangeException ex("index", "42", "Index was out of range.");
    EXPECT_NE(std::string(ex.what()).find("Index was out of range."), std::string::npos);
}

// Regression tests for ticket 353: real .NET's ArgumentOutOfRangeException.Message override
// always appends "Actual value was {actualValue}." AFTER the "(Parameter 'x')" marker whenever
// an actual value is supplied via the 3-arg constructor -- this is a defining, always-present
// feature of this exception type, not an optional extra. The previous implementation stored
// actualValue_ (so getActualValueProperty() worked) but never wove it into the message text
// itself, so what()/the message was missing this suffix entirely for every 3-arg constructor
// call and every ThrowIfXxx static helper (which all route through the 3-arg constructor
// internally).
TEST(ArgumentOutOfRangeExceptionTests, ThreeArgCtor_MessageIncludesActualValueSuffix) {
    ArgumentOutOfRangeException ex("index", "42", "Index was out of range.");
    std::string msg(ex.what());
    EXPECT_NE(msg.find("Actual value was 42."), std::string::npos);
    // Real .NET orders the parameter-name marker before the actual-value suffix.
    EXPECT_LT(msg.find("index"), msg.find("Actual value was 42."));
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegative_MessageIncludesActualValueSuffix) {
    try {
        ArgumentOutOfRangeException::ThrowIfNegative(-5, "count");
        FAIL() << "expected ArgumentOutOfRangeException";
    } catch (const ArgumentOutOfRangeException& ex) {
        std::string msg(ex.what());
        EXPECT_NE(msg.find("Actual value was -5."), std::string::npos);
    }
}

// ---------------------------------------------------------------------------
// Inheritance
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, IsA_ArgumentException) {
    EXPECT_THROW(throw ArgumentOutOfRangeException(), ArgumentException);
}

TEST(ArgumentOutOfRangeExceptionTests, IsA_StdException) {
    EXPECT_THROW(throw ArgumentOutOfRangeException(), std::exception);
}

// ---------------------------------------------------------------------------
// ThrowIfZero
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfZero_Zero_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfZero(0, "count"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfZero_NonZero_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfZero(1, "count"));
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfZero_Double_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfZero(0.0, "val"),
                 ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// ThrowIfNegative
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegative_Negative_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfNegative(-1, "x"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegative_Zero_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfNegative(0, "x"));
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegative_Positive_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfNegative(5, "x"));
}

// ---------------------------------------------------------------------------
// ThrowIfNegativeOrZero
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegativeOrZero_Negative_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfNegativeOrZero(-1, "n"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegativeOrZero_Zero_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfNegativeOrZero(0, "n"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegativeOrZero_Positive_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfNegativeOrZero(1, "n"));
}

// ---------------------------------------------------------------------------
// ThrowIfGreaterThan / ThrowIfGreaterThanOrEqual
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfGreaterThan_Greater_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfGreaterThan(10, 5, "val"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfGreaterThan_Equal_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfGreaterThan(5, 5, "val"));
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfGreaterThan_Less_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfGreaterThan(3, 5, "val"));
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfGreaterThanOrEqual_Equal_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfGreaterThanOrEqual(5, 5, "val"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfGreaterThanOrEqual_Less_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfGreaterThanOrEqual(4, 5, "val"));
}

// ---------------------------------------------------------------------------
// ThrowIfLessThan / ThrowIfLessThanOrEqual
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfLessThan_Less_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfLessThan(2, 5, "val"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfLessThan_Equal_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfLessThan(5, 5, "val"));
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfLessThanOrEqual_Equal_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfLessThanOrEqual(5, 5, "val"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfLessThanOrEqual_Greater_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfLessThanOrEqual(6, 5, "val"));
}

// ---------------------------------------------------------------------------
// ThrowIfEqual / ThrowIfNotEqual
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfEqual_Equal_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfEqual(7, 7, "val"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfEqual_NotEqual_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfEqual(7, 8, "val"));
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNotEqual_NotEqual_Throws) {
    EXPECT_THROW(ArgumentOutOfRangeException::ThrowIfNotEqual(7, 8, "val"),
                 ArgumentOutOfRangeException);
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNotEqual_Equal_NoThrow) {
    EXPECT_NO_THROW(ArgumentOutOfRangeException::ThrowIfNotEqual(7, 7, "val"));
}

// ---------------------------------------------------------------------------
// ActualValue and ParamName stored in thrown exception
// ---------------------------------------------------------------------------

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegative_ExceptionCarriesParamName) {
    try {
        ArgumentOutOfRangeException::ThrowIfNegative(-5, "myParam");
        FAIL() << "Expected ArgumentOutOfRangeException";
    } catch (const ArgumentOutOfRangeException& ex) {
        EXPECT_EQ(ex.getParamNameProperty(), "myParam");
    }
}

TEST(ArgumentOutOfRangeExceptionTests, ThrowIfNegative_ExceptionCarriesActualValue) {
    try {
        ArgumentOutOfRangeException::ThrowIfNegative(-5, "myParam");
        FAIL() << "Expected ArgumentOutOfRangeException";
    } catch (const ArgumentOutOfRangeException& ex) {
        EXPECT_EQ(ex.getActualValueProperty(), "-5");
    }
}
