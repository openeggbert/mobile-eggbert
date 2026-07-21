// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// Created by robertvokac on 5/30/25.
//

#pragma once

#include <cstdint>
#include <limits>
#include <string>
#define CONTAINS(STRING, SUBSTR) ((STRING).find(SUBSTR) != std::string::npos)
#define INTERNAL

namespace SharpRuntime
{
    /**
     * @brief 8-bit signed integer type compatible with C# @c sbyte.
     * Range: -128 to 127 (System.SByte)
     */
    using sbytecs = int8_t;

    /**
     * @brief 8-bit unsigned integer type compatible with C# @c byte.
     * Range: 0 to 255 (System.Byte)
     */
    using bytecs = uint8_t;

    /**
     * @brief 8-bit unsigned byte type compatible with C# @c byte.
     */
    using ubytecs = bytecs;

    /**
     * @brief 16-bit signed integer type compatible with C# @c short.
     * Range: -32,768 to 32,767 (System.Int16)
     */
    using shortcs = int16_t;

    /**
     * @brief 16-bit unsigned integer type compatible with C# @c ushort.
     * Range: 0 to 65,535 (System.UInt16)
     */
    using ushortcs = uint16_t;

    /**
     * @brief 32-bit signed integer type compatible with C# @c int.
     * Range: -2,147,483,648 to 2,147,483,647 (System.Int32)
     */
    using intcs = int32_t;

    /**
     * @brief 32-bit unsigned integer type compatible with C# @c uint.
     * Range: 0 to 4,294,967,295 (System.UInt32)
     */
    using uintcs = uint32_t;

    /**
     * @brief 64-bit signed integer type compatible with C# @c long.
     * Range: -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 (System.Int64)
     */
    using longcs = int64_t;

    /**
     * @brief 64-bit unsigned integer type compatible with C# @c ulong.
     * Range: 0 to 18,446,744,073,709,551,615 (System.UInt64)
     */
    using ulongcs = uint64_t;

    /**
     * @brief 16-bit Unicode character type compatible with C# @c char.
     */
    using charcs = char16_t;

    /// Native pointer-sized unsigned integer, mapped to std::uintptr_t.
    using IntPtr = std::uintptr_t;

    /**
     * @brief Maximum value of @c SharpRuntime::sbytecs.
     */
    inline constexpr sbytecs SBYTECS_MAX = std::numeric_limits<sbytecs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::sbytecs.
     */
    inline constexpr sbytecs SBYTECS_MIN = std::numeric_limits<sbytecs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::bytecs.
     */
    inline constexpr bytecs BYTECS_MAX = std::numeric_limits<bytecs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::bytecs.
     */
    inline constexpr bytecs BYTECS_MIN = std::numeric_limits<bytecs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::shortcs.
     */
    inline constexpr shortcs SHORTCS_MAX = std::numeric_limits<shortcs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::shortcs.
     */
    inline constexpr shortcs SHORTCS_MIN = std::numeric_limits<shortcs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::ushortcs.
     */
    inline constexpr ushortcs USHORTCS_MAX = std::numeric_limits<ushortcs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::ushortcs.
     */
    inline constexpr ushortcs USHORTCS_MIN = std::numeric_limits<ushortcs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::intcs.
     */
    inline constexpr intcs INTCS_MAX = std::numeric_limits<intcs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::intcs.
     */
    inline constexpr intcs INTCS_MIN = std::numeric_limits<intcs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::uintcs.
     */
    inline constexpr uintcs UINTCS_MAX = std::numeric_limits<uintcs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::uintcs.
     */
    inline constexpr uintcs UINTCS_MIN = std::numeric_limits<uintcs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::longcs.
     */
    inline constexpr longcs LONGCS_MAX = std::numeric_limits<longcs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::longcs.
     */
    inline constexpr longcs LONGCS_MIN = std::numeric_limits<longcs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::ulongcs.
     */
    inline constexpr ulongcs ULONGCS_MAX = std::numeric_limits<ulongcs>::max();

    /**
     * @brief Minimum value of @c SharpRuntime::ulongcs.
     */
    inline constexpr ulongcs ULONGCS_MIN = std::numeric_limits<ulongcs>::min();

    /**
     * @brief Maximum value of @c SharpRuntime::byte.
     */
    inline constexpr bytecs BYTE_MAX = BYTECS_MAX;

    /**
     * @brief Minimum value of @c SharpRuntime::byte.
     */
    inline constexpr bytecs BYTE_MIN = BYTECS_MIN;

    // ---------------------------------------------------------------------------
    // C#-name aliases — mirror the .NET primitive type names used in FNA source
    // ---------------------------------------------------------------------------

    /** C# @c SByte – 8-bit signed integer. */
    using SByte = sbytecs;
    /** C# @c Byte – 8-bit unsigned integer. */
    using Byte = bytecs;
    /** C# @c Int16 – 16-bit signed integer. */
    using Int16 = shortcs;
    /** C# @c UInt16 – 16-bit unsigned integer. */
    using UInt16 = ushortcs;
    /** C# @c Int32 – 32-bit signed integer. */
    using Int32 = intcs;
    /** C# @c UInt32 – 32-bit unsigned integer. */
    using UInt32 = uintcs;
    /** C# @c Int64 – 64-bit signed integer. */
    using Int64 = longcs;
    /** C# @c UInt64 – 64-bit unsigned integer. */
    using UInt64 = ulongcs;
    /** C# @c Single – 32-bit IEEE 754 floating-point. */
    using Single = float;
    /** C# @c String – mapped to std::string. */
    using String = std::string;
} // namespace SharpRuntime
