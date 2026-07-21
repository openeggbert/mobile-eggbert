// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/DBNull.hpp"
#include "System/InvalidCastException.hpp"

using System::DBNull;
using System::InvalidCastException;

// Basic singleton/ToString/GetTypeCode/ToBoolean/ToInt32/ToDouble/ToChar/ToInt64/ToSingle
// coverage already exists in Task40Tests.cpp (suite DBNullTests). This file adds only the
// IConvertible conversions that were not yet covered there.

TEST(DBNullTests, ToSByteThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToSByte(), InvalidCastException);
}

TEST(DBNullTests, ToByteThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToByte(), InvalidCastException);
}

TEST(DBNullTests, ToInt16ThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToInt16(), InvalidCastException);
}

TEST(DBNullTests, ToUInt16ThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToUInt16(), InvalidCastException);
}

TEST(DBNullTests, ToUInt32ThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToUInt32(), InvalidCastException);
}

TEST(DBNullTests, ToUInt64ThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToUInt64(), InvalidCastException);
}

TEST(DBNullTests, ToDecimalThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToDecimal(), InvalidCastException);
}

TEST(DBNullTests, ToDateTimeThrowsInvalidCastException) {
    EXPECT_THROW(DBNull::Value().ToDateTime(), InvalidCastException);
}

TEST(DBNullTests, InvalidCastExceptionMessageMatchesDotNet) {
    try {
        DBNull::Value().ToInt32();
        FAIL() << "Expected InvalidCastException";
    } catch (const InvalidCastException& e) {
        EXPECT_STREQ(e.what(), "Object cannot be cast from DBNull to other types.");
    }
}
