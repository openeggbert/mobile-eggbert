// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Text/Json/JsonElement.hpp"
#include "System/Text/Json/JsonSerializerOptions.hpp"
#include "System/Text/Json/Utf8JsonWriter.hpp"

namespace System::Text::Json::Serialization {

    /**
     * @brief Converts an object or value to or from JSON.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonConverter<T>.
     *
     * @note Real .NET reads from a `ref Utf8JsonReader` (a low-level, forward-only token stream);
     * this runtime has no `Utf8JsonReader` (see JsonReaderState's `plan.sqlite3` note — it isn't
     * tracked at all and JsonDocument covers the practical read path instead), so `Read()` here
     * takes a parsed `JsonElement` — the DOM-based equivalent input a converter would actually
     * need. `Type`/`typeToConvert` parameters are dropped (no reflection `Type` system).
     */
    template <typename T>
    class JsonConverter {
    public:
        virtual ~JsonConverter() = default;

        /** @brief Reads and converts JSON into a @p T value. */
        [[nodiscard]] virtual T Read(const JsonElement& element, const JsonSerializerOptions& options) const = 0;

        /** @brief Writes @p value as JSON. */
        virtual void Write(Utf8JsonWriter& writer, const T& value, const JsonSerializerOptions& options) const = 0;

        /** @return Whether this converter handles the JSON `null` literal itself (default: no — the caller treats null as a distinct case). */
        [[nodiscard]] virtual bool HandleNull() const { return false; }
    };

} // namespace System::Text::Json::Serialization
