// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Text::Json {

    using SharpRuntime::intcs;

    /**
     * @brief Provides the ability to define custom behavior when writing JSON using Utf8JsonWriter.
     *
     * C++ counterpart of .NET System.Text.Json.JsonWriterOptions.
     *
     * @note No `Encoder` property — `System.Text.Encodings.Web`'s `JavaScriptEncoder` (HTML/JS
     * string-escaping customization) is out of scope in this runtime; the writer always uses its
     * built-in default escaping.
     */
    struct JsonWriterOptions {
        /** @brief The default maximum nesting depth used when MaxDepth is left at 0. */
        static constexpr intcs DefaultMaxDepth = 1000;

        /** @brief Whether the writer should pretty-print with indentation and new lines. Default false. */
        bool Indented = false;
        /** @brief The indentation character (space or tab only) used when Indented is true. Default space. */
        char IndentCharacter = ' ';
        /** @brief The indentation size (0-127) used when Indented is true. Default 2. */
        intcs IndentSize = 2;
        /** @brief The maximum allowed nesting depth, or 0 for the default of 1000. */
        intcs MaxDepth = 0;
        /** @brief If true, skip structural validation, allowing the caller to write invalid JSON. Default false. */
        bool SkipValidation = false;
        /** @brief The new-line string used when Indented is true: "\n" or "\r\n". Default "\n". */
        std::string NewLine = "\n";

        /** @brief Validates IndentCharacter/IndentSize/MaxDepth/NewLine; throws System::ArgumentOutOfRangeException if invalid. */
        void Validate() const {
            if (IndentCharacter != ' ' && IndentCharacter != '\t')
                throw System::ArgumentOutOfRangeException("IndentCharacter");
            if (IndentSize < 0 || IndentSize > 127)
                throw System::ArgumentOutOfRangeException("IndentSize");
            if (MaxDepth < 0)
                throw System::ArgumentOutOfRangeException("MaxDepth");
            if (NewLine != "\n" && NewLine != "\r\n")
                throw System::ArgumentOutOfRangeException("NewLine");
        }
    };

} // namespace System::Text::Json
