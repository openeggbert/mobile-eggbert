// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Text/Json/JsonCommentHandling.hpp"

namespace System::Text::Json {

    using SharpRuntime::intcs;

    /**
     * @brief Provides the ability to define custom behavior when parsing JSON to create a JsonDocument.
     *
     * C++ counterpart of .NET System.Text.Json.JsonDocumentOptions.
     */
    struct JsonDocumentOptions {
        /** @brief The default maximum nesting depth used when MaxDepth is left at 0. */
        static constexpr intcs DefaultMaxDepth = 64;

        /** @brief How comments are handled (JsonDocument only supports Disallow/Skip, not Allow). Default Disallow. */
        JsonCommentHandling CommentHandling = JsonCommentHandling::Disallow;
        /** @brief The maximum allowed nesting depth, or 0 for the default of 64. */
        intcs MaxDepth = 0;
        /** @brief Whether a trailing comma in an object/array is allowed (and ignored). Default false. */
        bool AllowTrailingCommas = false;
        /** @brief Whether duplicate property names are allowed when parsing JSON objects. Default true. */
        bool AllowDuplicateProperties = true;

        /** @brief Validates CommentHandling/MaxDepth; throws System::ArgumentOutOfRangeException if invalid. */
        void Validate() const {
            if (static_cast<SharpRuntime::bytecs>(CommentHandling) > static_cast<SharpRuntime::bytecs>(JsonCommentHandling::Skip))
                throw System::ArgumentOutOfRangeException("CommentHandling");
            if (MaxDepth < 0)
                throw System::ArgumentOutOfRangeException("MaxDepth");
        }
    };

} // namespace System::Text::Json
