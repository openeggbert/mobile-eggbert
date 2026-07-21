// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/Json/JsonSerializerOptions.hpp"
#include "System/Text/Json/JsonValueKind.hpp"
#include "System/Text/Json/Nodes/JsonNodeOptions.hpp"
#include "nlohmann/json.hpp"

namespace System::Text::Json::Nodes {

    using SharpRuntime::intcs;

    class JsonArray;
    class JsonObject;
    class JsonValue;

    /**
     * @brief The abstract base class representing a mutable single node within a JSON tree.
     *
     * C++ counterpart of .NET System.Text.Json.Nodes.JsonNode.
     *
     * @note JSON `null` is represented the same way as in .NET: a `nullptr` `shared_ptr<JsonNode>`
     * slot in a parent JsonObject/JsonArray, not a distinct node instance.
     */
    class JsonNode {
        JsonNode* parent_ = nullptr; // non-owning; children are owned by their parent container
        JsonNodeOptions options_;

    protected:
        explicit JsonNode(JsonNodeOptions options = {}) : options_(options) {}

    public:
        virtual ~JsonNode() = default;

        /** @return The options this node was constructed with. */
        [[nodiscard]] JsonNodeOptions getOptionsProperty() const { return options_; }

        /** @return The parent node, or nullptr if this is a root node. */
        [[nodiscard]] JsonNode* getParentProperty() const { return parent_; }

        /** @return The root node of the tree this node belongs to. */
        [[nodiscard]] JsonNode* getRootProperty() {
            JsonNode* n = this;
            while (n->parent_) n = n->parent_;
            return n;
        }

        /** @return The kind of JSON value this node represents. */
        [[nodiscard]] virtual JsonValueKind GetValueKind() const = 0;

        /** @brief Internal: converts this node's subtree to an nlohmann::ordered_json value. Not part of .NET's public surface. */
        [[nodiscard]] virtual nlohmann::ordered_json toNlohmann() const = 0;

        /**
         * @brief Internal: attaches this node to a parent container. Not part of .NET's public
         * surface (mirrors JsonNode.cs's internal AssignParent).
         * @throws System::InvalidOperationException if this node already has a parent (would
         * silently detach it from whichever container actually still holds it), or if @p parent
         * is this node itself or one of this node's own descendants (would create a cycle).
         */
        void AssignParent(JsonNode* parent);

        /** @brief Internal: clears the parent container pointer. Not part of .NET's public surface (mirrors JsonNode.cs's internal DetachParent). */
        void DetachParent() { parent_ = nullptr; }

        /** @return This node cast to JsonArray. @throws System::InvalidOperationException if this isn't a JsonArray. */
        [[nodiscard]] JsonArray& AsArray();
        /** @return This node cast to JsonObject. @throws System::InvalidOperationException if this isn't a JsonObject. */
        [[nodiscard]] JsonObject& AsObject();
        /** @return This node cast to JsonValue. @throws System::InvalidOperationException if this isn't a JsonValue. */
        [[nodiscard]] JsonValue& AsValue();

        /** @return A deep copy of this node (and, if a container, its whole subtree) with no parent. */
        [[nodiscard]] virtual std::shared_ptr<JsonNode> DeepClone() const = 0;

        /** @brief Serializes this node to a JSON string, optionally formatted per @p options. */
        [[nodiscard]] std::string ToJsonString(const JsonSerializerOptions& options = JsonSerializerOptions::Default()) const;

        /** @return Same as ToJsonString() with default options. */
        [[nodiscard]] std::string ToString() const { return ToJsonString(); }

        /** @return true if @p node1 and @p node2 are both nullptr, or both non-null and deeply equal. */
        [[nodiscard]] static bool DeepEquals(const JsonNode* node1, const JsonNode* node2);

        /**
         * @brief Parses JSON text into a JsonNode tree.
         * @return The root node, or nullptr if @p json is the literal "null".
         */
        [[nodiscard]] static std::shared_ptr<JsonNode> Parse(const std::string& json, JsonNodeOptions options = {});
    };

} // namespace System::Text::Json::Nodes
