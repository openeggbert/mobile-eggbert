// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "System/Text/Json/JsonDocument.hpp"
#include "System/Text/Json/JsonException.hpp"
#include "System/Text/Json/JsonSerializerOptions.hpp"
#include "nlohmann/json.hpp"

namespace System::Text::Json {

    /**
     * @brief Provides static methods for serializing objects to JSON and deserializing JSON to
     * objects.
     *
     * C++ counterpart of .NET System.Text.Json.JsonSerializer.
     *
     * @note Real .NET `Serialize<T>`/`Deserialize<T>` walk an arbitrary type's members via
     * reflection (or a source-generated `JsonTypeInfo`) — both are out of scope here (see
     * CLAUDE.md's parity philosophy: reflection is a permanent deviation). The template overloads
     * below instead rely on `nlohmann::ordered_json`'s compile-time ADL customization points
     * (`to_json`/`from_json` free functions, or `NLOHMANN_DEFINE_TYPE_INTRUSIVE`), which already
     * gives real, working serialization for primitives, `std::string`, `std::vector<T>`,
     * `std::map<std::string, T>`, `std::optional<T>`, and any user type that defines those
     * functions — a compile-time customization point standing in for reflection, not a stub.
     */
    class JsonSerializer {
    public:
        JsonSerializer() = delete;

        /** @brief Serializes @p value to a JSON string. @throws JsonException on serialization failure. */
        template <typename T>
        static std::string Serialize(const T& value, const JsonSerializerOptions& opts = JsonSerializerOptions::Default()) {
            try {
                nlohmann::ordered_json j = value;
                return opts.getWriteIndentedProperty() ? j.dump(static_cast<int>(opts.getIndentSizeProperty()),
                                                                  opts.getIndentCharacterProperty())
                                                        : j.dump();
            } catch (const nlohmann::ordered_json::exception& e) {
                throw JsonException(e.what());
            }
        }

        /** @brief Serializes @p element to a JSON string (re-formatted per @p opts's indentation settings). */
        static std::string Serialize(const JsonElement& element, const JsonSerializerOptions& opts = JsonSerializerOptions::Default()) {
            if (!opts.getWriteIndentedProperty()) return element.GetRawText();
            try {
                auto j = nlohmann::ordered_json::parse(element.GetRawText());
                return j.dump(static_cast<int>(opts.getIndentSizeProperty()), opts.getIndentCharacterProperty());
            } catch (const nlohmann::ordered_json::exception& e) {
                throw JsonException(e.what());
            }
        }

        /** @brief Deserializes @p json into a value of type @p T. @throws JsonException on invalid input/shape mismatch. */
        template <typename T>
        static T Deserialize(const std::string& json, const JsonSerializerOptions& /*opts*/ = JsonSerializerOptions::Default()) {
            try {
                return nlohmann::ordered_json::parse(json).get<T>();
            } catch (const nlohmann::ordered_json::exception& e) {
                throw JsonException(e.what());
            }
        }

        /** @brief Deserializes @p json into a JsonDocument (untyped tree). */
        static std::shared_ptr<JsonDocument> Deserialize(const std::string& json,
                                                          const JsonSerializerOptions& /*opts*/ = JsonSerializerOptions::Default()) {
            return JsonDocument::Parse(json);
        }
    };

} // namespace System::Text::Json
