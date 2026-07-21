// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/String.hpp"
#include "System/StringComparison.hpp"
#include "System/Text/Json/Nodes/JsonNode.hpp"

namespace System::Text::Json::Nodes {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a mutable JSON object (an ordered collection of name/value pairs).
     *
     * C++ counterpart of .NET System.Text.Json.Nodes.JsonObject.
     *
     * @note Backed by a `std::vector<std::pair<...>>` (linear lookup) rather than .NET's hybrid
     * dictionary — preserves insertion order (which .NET's JsonObject also guarantees) without a
     * separate name-to-index hash map; fine for the modestly-sized config/content JSON this
     * runtime's game code realistically works with.
     */
    class JsonObject : public JsonNode {
        std::vector<std::pair<std::string, std::shared_ptr<JsonNode>>> properties_;

        // Verified against JsonObject.IDictionary.cs's CreateDictionary(): real .NET's backing
        // dictionary uses StringComparer.OrdinalIgnoreCase (when PropertyNameCaseInsensitive is
        // set) as the comparer for every operation -- lookup, ContainsKey, the duplicate-key
        // check in Add(), Remove(), the indexer -- not just reads. Since findIndex() is this
        // port's single choke point for all of those, fixing it here covers every operation
        // uniformly, matching .NET's design.
        [[nodiscard]] intcs findIndex(const std::string& propertyName) const {
            bool caseInsensitive = getOptionsProperty().PropertyNameCaseInsensitive;
            for (size_t i = 0; i < properties_.size(); ++i) {
                bool match = caseInsensitive
                    ? System::String::Equals(properties_[i].first, propertyName, System::StringComparison::OrdinalIgnoreCase)
                    : properties_[i].first == propertyName;
                if (match) return static_cast<intcs>(i);
            }
            return -1;
        }

    public:
        explicit JsonObject(JsonNodeOptions options = {}) : JsonNode(options) {}

        /** @return The number of properties in the object. */
        [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(properties_.size()); }

        /** @return true if a property named @p propertyName exists. */
        [[nodiscard]] bool ContainsKey(const std::string& propertyName) const { return findIndex(propertyName) >= 0; }

        /** @brief Tries to get the value of @p propertyName; returns false if not present. */
        [[nodiscard]] bool TryGetPropertyValue(const std::string& propertyName, std::shared_ptr<JsonNode>& value) const {
            intcs idx = findIndex(propertyName);
            if (idx < 0) return false;
            value = properties_[static_cast<size_t>(idx)].second;
            return true;
        }

        /** @brief Adds a new property. @throws System::ArgumentException if @p propertyName already exists. */
        void Add(const std::string& propertyName, std::shared_ptr<JsonNode> value) {
            if (ContainsKey(propertyName))
                throw System::ArgumentException("An item with the same key has already been added.", "propertyName");
            if (value) value->AssignParent(this);
            properties_.emplace_back(propertyName, std::move(value));
        }

        /** @brief Removes the property named @p propertyName. @return true if it was present. */
        bool Remove(const std::string& propertyName) {
            intcs idx = findIndex(propertyName);
            if (idx < 0) return false;
            if (auto& value = properties_[static_cast<size_t>(idx)].second) value->DetachParent();
            properties_.erase(properties_.begin() + idx);
            return true;
        }

        /** @return The value of @p propertyName, or nullptr if not present (verified against JsonNode.cs's string indexer). */
        [[nodiscard]] std::shared_ptr<JsonNode> operator[](const std::string& propertyName) const {
            intcs idx = findIndex(propertyName);
            if (idx < 0) return nullptr;
            return properties_[static_cast<size_t>(idx)].second;
        }

        /** @brief Sets the value of @p propertyName, adding it if not already present. */
        void SetItem(const std::string& propertyName, std::shared_ptr<JsonNode> value) {
            intcs idx = findIndex(propertyName);
            if (idx >= 0) {
                auto& slot = properties_[static_cast<size_t>(idx)].second;
                if (slot == value) return;
                if (slot) slot->DetachParent();
                if (value) value->AssignParent(this);
                slot = std::move(value);
            } else {
                if (value) value->AssignParent(this);
                properties_.emplace_back(propertyName, std::move(value));
            }
        }

        /** @brief Removes all properties from the object. */
        void Clear() {
            for (auto& [name, value] : properties_) if (value) value->DetachParent();
            properties_.clear();
        }

        [[nodiscard]] auto begin() const { return properties_.begin(); }
        [[nodiscard]] auto end() const { return properties_.end(); }

        [[nodiscard]] JsonValueKind GetValueKind() const override { return JsonValueKind::Object; }

        [[nodiscard]] nlohmann::ordered_json toNlohmann() const override {
            nlohmann::ordered_json obj = nlohmann::ordered_json::object();
            for (const auto& [name, value] : properties_) obj[name] = value ? value->toNlohmann() : nlohmann::ordered_json(nullptr);
            return obj;
        }

        [[nodiscard]] std::shared_ptr<JsonNode> DeepClone() const override {
            auto clone = std::make_shared<JsonObject>(getOptionsProperty());
            for (const auto& [name, value] : properties_) clone->Add(name, value ? value->DeepClone() : nullptr);
            return clone;
        }
    };

} // namespace System::Text::Json::Nodes
