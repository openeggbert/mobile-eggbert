// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::Json::Serialization {

    /**
     * @brief Determines how deserialization will handle object creation for fields or properties.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonObjectCreationHandling.
     */
    enum class JsonObjectCreationHandling {
        /** @brief A new instance will always be created when deserializing a field or property. */
        Replace = 0,
        /** @brief Attempt to populate any instance already found on a deserialized field or property. */
        Populate = 1,
    };

} // namespace System::Text::Json::Serialization
