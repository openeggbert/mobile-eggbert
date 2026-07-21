// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <utility>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/Text/Unicode/Utf16.hpp"
#include "System/Text/Unicode/Utf8.hpp"

namespace System::Text::Json {

    /**
     * @brief Represents a pre-encoded (validated) UTF-8/JSON text value that can be written to a
     * Utf8JsonWriter without re-validating or re-escaping it every time.
     *
     * C++ counterpart of .NET System.Text.Json.JsonEncodedText.
     *
     * @note No `JavaScriptEncoder` parameter — `System.Text.Encodings.Web` is out of scope in this
     * runtime (see JsonWriterOptions's doc comment); `Encode()` validates the input is well-formed
     * but does not apply any custom escaping.
     */
    struct JsonEncodedText {
        std::string Value;

        JsonEncodedText() = default;

        /** @brief Encodes @p value; throws System::ArgumentException if it isn't well-formed UTF-16. */
        static JsonEncodedText Encode(const std::u16string& value) {
            if (!System::Text::Unicode::Utf16::IsValid(value))
                throw System::ArgumentException("The input string contains invalid UTF-16 text.", "value");
            std::vector<SharpRuntime::bytecs> utf8(value.size() * 4);
            SharpRuntime::intcs charsRead = 0, bytesWritten = 0;
            System::Text::Unicode::Utf8::FromUtf16(value, utf8, charsRead, bytesWritten,
                                                    /*replaceInvalidSequences=*/false);
            return Encode(std::string(utf8.begin(), utf8.begin() + bytesWritten));
        }

        /** @brief Wraps an already-narrow (UTF-8-ish) string as pre-encoded text. */
        static JsonEncodedText Encode(const std::string& value) {
            JsonEncodedText result;
            result.Value = value;
            return result;
        }

        [[nodiscard]] bool Equals(const JsonEncodedText& other) const { return Value == other.Value; }
        bool operator==(const JsonEncodedText& other) const { return Equals(other); }
        bool operator!=(const JsonEncodedText& other) const { return !Equals(other); }

        [[nodiscard]] std::string ToString() const { return Value; }
    };

} // namespace System::Text::Json
