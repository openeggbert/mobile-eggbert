// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <utility>
#include "System/Text/Json/JsonElement.hpp"

namespace System::Text::Json {

    /**
     * @brief Represents a single property (name/value pair) for a JSON object.
     *
     * C++ counterpart of .NET System.Text.Json.JsonProperty.
     */
    class JsonProperty {
        std::string name_;
        JsonElement value_;

    public:
        JsonProperty() = default;
        JsonProperty(std::string name, JsonElement value) : name_(std::move(name)), value_(std::move(value)) {}

        /** @return The name of this property. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** @return The value of this property. */
        [[nodiscard]] const JsonElement& getValueProperty() const { return value_; }

        /** @return true if @p text matches the name of this property. */
        [[nodiscard]] bool NameEquals(const std::string& text) const { return name_ == text; }

        /** @return The raw JSON text of this property's value. */
        [[nodiscard]] std::string ToString() const { return value_.ToString(); }
    };

} // namespace System::Text::Json
