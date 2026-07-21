// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Nodes {

    /**
     * @brief Provides options to be used with JsonNode / derived types (JsonObject/JsonArray/JsonValue).
     *
     * C++ counterpart of .NET System.Text.Json.Nodes.JsonNodeOptions.
     */
    struct JsonNodeOptions {
        /** @brief Whether JsonObject property lookups are case-insensitive. Default false. */
        bool PropertyNameCaseInsensitive = false;
    };

} // namespace System::Text::Json::Nodes
