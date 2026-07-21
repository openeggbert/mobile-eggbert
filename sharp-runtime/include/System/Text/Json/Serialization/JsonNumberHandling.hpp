// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Serialization {

    /**
     * @brief Determines how JsonSerializer handles numbers when serializing and deserializing. [Flags]
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonNumberHandling.
     *
     * @note Altering the default (Strict) is not defined by the JSON specification and can
     * produce JSON that other JSON implementations can't parse (same caveat as .NET's own docs).
     */
    enum class JsonNumberHandling {
        /** @brief Numbers are only read from/written as JSON numbers (no quotes). */
        Strict = 0x0,
        /** @brief Numbers can additionally be read from JSON string tokens. */
        AllowReadingFromString = 0x1,
        /** @brief Numbers are written as JSON strings (with quotes) instead of JSON numbers. */
        WriteAsString = 0x2,
        /** @brief "NaN"/"Infinity"/"-Infinity" string tokens are read/written as the corresponding floating-point constants. */
        AllowNamedFloatingPointLiterals = 0x4,
    };

    constexpr JsonNumberHandling operator|(JsonNumberHandling a, JsonNumberHandling b) {
        return static_cast<JsonNumberHandling>(static_cast<int>(a) | static_cast<int>(b));
    }
    constexpr JsonNumberHandling operator&(JsonNumberHandling a, JsonNumberHandling b) {
        return static_cast<JsonNumberHandling>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Text::Json::Serialization
