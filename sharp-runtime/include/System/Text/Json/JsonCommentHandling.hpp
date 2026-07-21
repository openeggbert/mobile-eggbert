// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text::Json {

    /**
     * @brief Defines the various ways a JSON reader can deal with comments.
     *
     * C++ counterpart of .NET System.Text.Json.JsonCommentHandling.
     */
    enum class JsonCommentHandling : SharpRuntime::bytecs {
        /** @brief Comments are treated as invalid JSON if found and a JsonException is thrown. */
        Disallow = 0,
        /** @brief Comments are allowed and ignored. */
        Skip = 1,
        /** @brief Comments are allowed and treated as valid tokens the caller can access. */
        Allow = 2,
    };

} // namespace System::Text::Json
