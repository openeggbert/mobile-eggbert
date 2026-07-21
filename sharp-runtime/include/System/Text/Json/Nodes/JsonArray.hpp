// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Text/Json/Nodes/JsonNode.hpp"

namespace System::Text::Json::Nodes {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a mutable JSON array.
     *
     * C++ counterpart of .NET System.Text.Json.Nodes.JsonArray.
     */
    class JsonArray : public JsonNode {
        std::vector<std::shared_ptr<JsonNode>> items_;

    public:
        explicit JsonArray(JsonNodeOptions options = {}) : JsonNode(options) {}

        /** @return The number of items in the array. */
        [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(items_.size()); }

        /** @return The item at @p index. @throws System::ArgumentOutOfRangeException if out of range. */
        [[nodiscard]] const std::shared_ptr<JsonNode>& operator[](intcs index) const {
            if (index < 0 || index >= static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("index");
            return items_[static_cast<size_t>(index)];
        }

        /** @brief Replaces the item at @p index (adopts @p value as a child, clearing any prior parent link). */
        void SetItem(intcs index, std::shared_ptr<JsonNode> value) {
            if (index < 0 || index >= static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("index");
            if (value) value->AssignParent(this);
            if (auto& old = items_[static_cast<size_t>(index)]) old->DetachParent();
            items_[static_cast<size_t>(index)] = std::move(value);
        }

        /** @brief Appends @p item to the end of the array. */
        void Add(std::shared_ptr<JsonNode> item) {
            if (item) item->AssignParent(this);
            items_.push_back(std::move(item));
        }

        /** @brief Inserts @p item at @p index. @throws System::ArgumentOutOfRangeException if out of range. */
        void Insert(intcs index, std::shared_ptr<JsonNode> item) {
            if (index < 0 || index > static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("index");
            if (item) item->AssignParent(this);
            items_.insert(items_.begin() + index, std::move(item));
        }

        /** @brief Removes the item at @p index. @throws System::ArgumentOutOfRangeException if out of range. */
        void RemoveAt(intcs index) {
            if (index < 0 || index >= static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("index");
            if (auto& item = items_[static_cast<size_t>(index)]) item->DetachParent();
            items_.erase(items_.begin() + index);
        }

        /**
         * @brief Removes the first occurrence of @p item (by pointer identity) from the array.
         * C++ counterpart of .NET JsonArray's IList&lt;JsonNode?&gt;.Remove(JsonNode?).
         * @return true if @p item was found and removed; false otherwise.
         */
        bool Remove(const std::shared_ptr<JsonNode>& item) {
            intcs idx = IndexOf(item);
            if (idx < 0) return false;
            RemoveAt(idx);
            return true;
        }

        /** @brief Removes all items from the array. */
        void Clear() {
            for (auto& item : items_) if (item) item->DetachParent();
            items_.clear();
        }

        /** @return The index of @p item within the array, or -1 if not found (pointer identity). */
        [[nodiscard]] intcs IndexOf(const std::shared_ptr<JsonNode>& item) const {
            for (size_t i = 0; i < items_.size(); ++i)
                if (items_[i] == item) return static_cast<intcs>(i);
            return -1;
        }

        [[nodiscard]] auto begin() const { return items_.begin(); }
        [[nodiscard]] auto end() const { return items_.end(); }

        [[nodiscard]] JsonValueKind GetValueKind() const override { return JsonValueKind::Array; }

        [[nodiscard]] nlohmann::ordered_json toNlohmann() const override {
            nlohmann::ordered_json arr = nlohmann::ordered_json::array();
            for (const auto& item : items_) arr.push_back(item ? item->toNlohmann() : nlohmann::ordered_json(nullptr));
            return arr;
        }

        [[nodiscard]] std::shared_ptr<JsonNode> DeepClone() const override {
            auto clone = std::make_shared<JsonArray>(getOptionsProperty());
            for (const auto& item : items_) clone->Add(item ? item->DeepClone() : nullptr);
            return clone;
        }
    };

} // namespace System::Text::Json::Nodes
