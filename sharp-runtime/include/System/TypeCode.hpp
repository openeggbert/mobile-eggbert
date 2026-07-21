// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the type of an object.
     *
     * C++ counterpart of .NET System.TypeCode.
     * Used by IConvertible.GetTypeCode() to identify the underlying type.
     */
    enum class TypeCode {
        Empty    = 0,  /**< A null reference. */
        Object   = 1,  /**< A general type representing any reference or value type. */
        DBNull   = 2,  /**< A database null (column) value. */
        Boolean  = 3,  /**< A Boolean value. */
        Char     = 4,  /**< A Unicode character. */
        SByte    = 5,  /**< An 8-bit signed integer. */
        Byte     = 6,  /**< An 8-bit unsigned integer. */
        Int16    = 7,  /**< A 16-bit signed integer. */
        UInt16   = 8,  /**< A 16-bit unsigned integer. */
        Int32    = 9,  /**< A 32-bit signed integer. */
        UInt32   = 10, /**< A 32-bit unsigned integer. */
        Int64    = 11, /**< A 64-bit signed integer. */
        UInt64   = 12, /**< A 64-bit unsigned integer. */
        Single   = 13, /**< A single-precision floating-point number. */
        Double   = 14, /**< A double-precision floating-point number. */
        Decimal  = 15, /**< A decimal number. */
        DateTime = 16, /**< A date and time value. */
        String   = 18, /**< A Unicode character string. */
    };

} // namespace System
