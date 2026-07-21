// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json {

    /**
     * @brief Signifies what default options are used by JsonSerializerOptions.
     *
     * C++ counterpart of .NET System.Text.Json.JsonSerializerDefaults.
     */
    enum class JsonSerializerDefaults {
        /**
         * @brief General-purpose values: property names are case-sensitive, "PascalCase" name
         * formatting is used. Applied if a JsonSerializerDefaults isn't specified.
         */
        General = 0,
        /** @brief Web-based scenarios: property names are case-insensitive, "camelCase" name formatting is used. */
        Web = 1,
        /** @brief Stricter policies applied when deserializing (case-sensitive, no unmapped/duplicate properties). */
        Strict = 2,
    };

} // namespace System::Text::Json
