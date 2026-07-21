// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/Json/JsonValueKind.hpp"
#include "nlohmann/json.hpp"

namespace System::Text::Json {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    class JsonProperty;

    /**
     * @brief Represents a single JSON value (object, array, string, number, boolean, or null).
     *
     * C++ counterpart of .NET System.Text.Json.JsonElement.
     *
     * @note Backed directly by a node in the owning JsonDocument's parsed `nlohmann::ordered_json` tree
     * (shared ownership via aliasing `shared_ptr`, so a JsonElement keeps the whole document tree
     * alive) rather than reproducing .NET's compact binary token database over a raw UTF-8 buffer
     * — same observable API, simpler implementation.
     */
    class JsonElement {
        std::shared_ptr<const nlohmann::ordered_json> node_;
        std::string propertyName_; // set only for elements obtained via EnumerateObject(); see JsonProperty

        friend class JsonProperty;
        friend class JsonDocument;

        [[nodiscard]] const nlohmann::ordered_json& require(JsonValueKind expected, const char* what) const;

    public:
        /** @brief Constructs an undefined JsonElement. */
        JsonElement() = default;
        /** @brief Wraps a node from a JsonDocument's parsed tree (internal; use JsonDocument::getRootElementProperty()/GetProperty()/etc. instead). */
        explicit JsonElement(std::shared_ptr<const nlohmann::ordered_json> node) : node_(std::move(node)) {}

        /** @return The kind of this JSON value. */
        [[nodiscard]] JsonValueKind getValueKindProperty() const {
            if (!node_) return JsonValueKind::Undefined;
            if (node_->is_object()) return JsonValueKind::Object;
            if (node_->is_array()) return JsonValueKind::Array;
            if (node_->is_string()) return JsonValueKind::String;
            if (node_->is_number()) return JsonValueKind::Number;
            if (node_->is_boolean()) return node_->get<bool>() ? JsonValueKind::True : JsonValueKind::False;
            if (node_->is_null()) return JsonValueKind::Null;
            return JsonValueKind::Undefined;
        }

        /**
         * @return The value as a string, or "" if this element's ValueKind is Null.
         * @throws System::InvalidOperationException if this element is any other non-string kind.
         * @note .NET's GetString() returns `string?` and special-cases Null to return null
         * before the type check (JsonDocument.cs's GetString); this port's GetString() keeps
         * the non-nullable std::string signature every other caller already relies on, so Null
         * maps to "" instead. Check getValueKindProperty() == JsonValueKind::Null first if the
         * null/empty-string distinction matters.
         */
        [[nodiscard]] std::string GetString() const {
            if (node_ && node_->is_null()) return {};
            return require(JsonValueKind::String, "String").get<std::string>();
        }

        /** @return The value as a 32-bit integer. @throws System::InvalidOperationException/System::FormatException. */
        [[nodiscard]] intcs GetInt32() const;
        /** @return The value as a 64-bit integer. @throws System::InvalidOperationException/System::FormatException. */
        [[nodiscard]] longcs GetInt64() const;
        /** @return The value as a double. @throws System::InvalidOperationException if this element is not a JSON number. */
        [[nodiscard]] double GetDouble() const { return require(JsonValueKind::Number, "Number").get<double>(); }
        /** @return The value as a boolean (true if ValueKind is True, false if False). @throws System::InvalidOperationException otherwise. */
        [[nodiscard]] bool GetBoolean() const;

        /**
         * @brief Tries to get this element's value as a 32-bit integer.
         * @return false (without throwing) if this is a Number whose value doesn't fit an
         * intcs, or isn't an integer literal (e.g. has a decimal point/exponent).
         * @throws System::InvalidOperationException if this element's ValueKind isn't Number.
         */
        [[nodiscard]] bool TryGetInt32(intcs& value) const;
        /**
         * @brief Tries to get this element's value as a 64-bit integer.
         * @return false (without throwing) if this is a Number whose value doesn't fit a
         * longcs, or isn't an integer literal (e.g. has a decimal point/exponent).
         * @throws System::InvalidOperationException if this element's ValueKind isn't Number.
         */
        [[nodiscard]] bool TryGetInt64(longcs& value) const;
        /** @brief Tries to get this element's value as a double without throwing on failure. */
        [[nodiscard]] bool TryGetDouble(double& value) const {
            if (!node_ || !node_->is_number()) { value = 0; return false; }
            value = node_->get<double>();
            return true;
        }

        /** @return The raw JSON text of this element. */
        [[nodiscard]] std::string GetRawText() const { return node_ ? node_->dump() : std::string(); }

        /**
         * @brief Tries to get a named object property.
         * @return false (without throwing) if this is an Object with no such property.
         * @throws System::InvalidOperationException if this element's ValueKind isn't Object
         * (verified against JsonDocument.TryGetProperty.cs's TryGetNamedPropertyValue, which
         * checks the token type unconditionally before searching for the property, even for
         * the Try-prefixed overload).
         */
        [[nodiscard]] bool TryGetProperty(const std::string& name, JsonElement& out) const {
            const auto& n = require(JsonValueKind::Object, "Object");
            auto it = n.find(name);
            if (it == n.end()) return false;
            out = JsonElement(std::shared_ptr<const nlohmann::ordered_json>(node_, &(*it)));
            return true;
        }

        /**
         * @return A named object property.
         * @throws System::InvalidOperationException if this element's ValueKind isn't Object.
         * @throws System::Collections::Generic::KeyNotFoundException if no such property exists.
         */
        [[nodiscard]] JsonElement GetProperty(const std::string& name) const;

        /**
         * @return true if this element is an object containing a property named @p name.
         * @throws System::InvalidOperationException if this element's ValueKind isn't Object.
         */
        [[nodiscard]] bool HasProperty(const std::string& name) const {
            JsonElement ignored;
            return TryGetProperty(name, ignored);
        }

        /** @return The number of elements in this JSON array. @throws System::InvalidOperationException if not an array. */
        [[nodiscard]] intcs GetArrayLength() const {
            return static_cast<intcs>(require(JsonValueKind::Array, "Array").size());
        }

        /** @return The array element at @p index. @throws System::InvalidOperationException/System::IndexOutOfRangeException. */
        [[nodiscard]] JsonElement operator[](intcs index) const;

        /** @return The items of this JSON array. @throws System::InvalidOperationException if not an array. */
        [[nodiscard]] std::vector<JsonElement> EnumerateArray() const {
            const auto& arr = require(JsonValueKind::Array, "Array");
            std::vector<JsonElement> result;
            result.reserve(arr.size());
            for (const auto& item : arr) result.emplace_back(std::shared_ptr<const nlohmann::ordered_json>(node_, &item));
            return result;
        }

        /** @return The properties of this JSON object, in document order. @throws System::InvalidOperationException if not an object. */
        [[nodiscard]] std::vector<JsonProperty> EnumerateObject() const;

        /** @return A deep copy of this element that owns its own storage. */
        [[nodiscard]] JsonElement Clone() const {
            if (!node_) return JsonElement();
            return JsonElement(std::make_shared<const nlohmann::ordered_json>(*node_));
        }

        /** @return The raw JSON text of this element (same as GetRawText()). */
        [[nodiscard]] std::string ToString() const { return GetRawText(); }
    };

} // namespace System::Text::Json
