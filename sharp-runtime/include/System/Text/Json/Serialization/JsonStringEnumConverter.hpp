// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <utility>
#include <vector>
#include "System/Text/Json/JsonException.hpp"
#include "System/Text/Json/Serialization/JsonConverter.hpp"

namespace System::Text::Json::Serialization {

    /**
     * @brief Converts an enum to and from its string representation.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.JsonStringEnumConverter<TEnum>.
     *
     * @note Real .NET discovers enum member names via reflection (`Enum.GetNames`), optionally
     * remapped through a `JsonNamingPolicy` or `[JsonStringEnumMemberName]`. C++ enums have no
     * built-in name reflection, so this port instead takes the name table explicitly as a
     * constructor argument — still a real, working converter (not a stub), just with the mapping
     * supplied by the caller instead of discovered automatically.
     */
    template <typename TEnum>
    class JsonStringEnumConverter : public JsonConverter<TEnum> {
        std::vector<std::pair<TEnum, std::string>> names_;

    public:
        /** @brief Constructs the converter from an explicit list of (enum value, name) pairs. */
        explicit JsonStringEnumConverter(std::vector<std::pair<TEnum, std::string>> names) : names_(std::move(names)) {}

        [[nodiscard]] TEnum Read(const JsonElement& element, const JsonSerializerOptions& /*options*/) const override {
            std::string s = element.GetString();
            for (const auto& [value, name] : names_)
                if (name == s) return value;
            throw JsonException("The JSON value '" + s + "' could not be converted.");
        }

        void Write(Utf8JsonWriter& writer, const TEnum& value, const JsonSerializerOptions& /*options*/) const override {
            for (const auto& [v, name] : names_) {
                if (v == value) {
                    writer.WriteStringValue(name);
                    return;
                }
            }
            throw JsonException("The enum value could not be converted to a JSON string.");
        }
    };

} // namespace System::Text::Json::Serialization
