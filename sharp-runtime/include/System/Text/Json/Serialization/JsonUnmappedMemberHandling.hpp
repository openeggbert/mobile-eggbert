// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Serialization {

    /**
     * @brief Determines how JsonSerializer handles JSON properties that cannot be mapped to a
     * specific member when deserializing object types.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonUnmappedMemberHandling.
     */
    enum class JsonUnmappedMemberHandling {
        /** @brief Silently skips any unmapped properties. Default behavior. */
        Skip = 0,
        /** @brief Throws an exception when an unmapped property is encountered. */
        Disallow = 1,
    };

} // namespace System::Text::Json::Serialization
