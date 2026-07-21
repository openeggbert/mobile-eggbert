// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Serialization {

    /**
     * @brief The JsonNamingPolicy to be used at run time.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonKnownNamingPolicy.
     */
    enum class JsonKnownNamingPolicy {
        /** @brief JSON property names are not converted. */
        Unspecified = 0,
        /** @brief Use the built-in JsonNamingPolicy::CamelCase(). */
        CamelCase = 1,
        /** @brief Use the built-in JsonNamingPolicy::SnakeCaseLower(). */
        SnakeCaseLower = 2,
        /** @brief Use the built-in JsonNamingPolicy::SnakeCaseUpper(). */
        SnakeCaseUpper = 3,
        /** @brief Use the built-in JsonNamingPolicy::KebabCaseLower(). */
        KebabCaseLower = 4,
        /** @brief Use the built-in JsonNamingPolicy::KebabCaseUpper(). */
        KebabCaseUpper = 5,
        /** @brief Use the built-in JsonNamingPolicy::PascalCase(). */
        PascalCase = 6,
    };

} // namespace System::Text::Json::Serialization
