// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Serialization {

    /**
     * @brief Defines how deserializing a type declared as an object is handled during deserialization.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonUnknownTypeHandling.
     */
    enum class JsonUnknownTypeHandling {
        /** @brief Deserialized as a JsonElement. */
        JsonElement = 0,
        /** @brief Deserialized as a JsonNode. */
        JsonNode = 1,
    };

} // namespace System::Text::Json::Serialization
