// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/Json/Nodes/JsonNode.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Text/Json/JsonException.hpp"
#include "System/Text/Json/Nodes/JsonArray.hpp"
#include "System/Text/Json/Nodes/JsonObject.hpp"
#include "System/Text/Json/Nodes/JsonValue.hpp"

namespace System::Text::Json::Nodes {

    void JsonNode::AssignParent(JsonNode* parent) {
        // Verified against JsonNode.cs's internal AssignParent: real .NET throws
        // InvalidOperationException both when this node already has a parent (attaching it
        // elsewhere without detaching first would leave the original container holding a
        // dangling reference to a node that now silently reports a different parent) and when
        // walking up from `parent` reaches `this` (attaching would close a cycle: `this` is
        // already an ancestor of, or equal to, `parent`).
        if (getParentProperty())
            throw System::InvalidOperationException("The node already has a parent.");
        for (JsonNode* p = parent; p; p = p->getParentProperty()) {
            if (p == this)
                throw System::InvalidOperationException("A node cycle was detected.");
        }
        parent_ = parent;
    }

    JsonArray& JsonNode::AsArray() {
        auto* p = dynamic_cast<JsonArray*>(this);
        if (!p) throw System::InvalidOperationException("The node is not a JsonArray.");
        return *p;
    }

    JsonObject& JsonNode::AsObject() {
        auto* p = dynamic_cast<JsonObject*>(this);
        if (!p) throw System::InvalidOperationException("The node is not a JsonObject.");
        return *p;
    }

    JsonValue& JsonNode::AsValue() {
        auto* p = dynamic_cast<JsonValue*>(this);
        if (!p) throw System::InvalidOperationException("The node is not a JsonValue.");
        return *p;
    }

    std::string JsonNode::ToJsonString(const JsonSerializerOptions& options) const {
        auto j = toNlohmann();
        return options.getWriteIndentedProperty()
                   ? j.dump(static_cast<int>(options.getIndentSizeProperty()), options.getIndentCharacterProperty())
                   : j.dump();
    }

    bool JsonNode::DeepEquals(const JsonNode* node1, const JsonNode* node2) {
        if (node1 == node2) return true;
        if (node1 == nullptr || node2 == nullptr) return false;
        return node1->toNlohmann() == node2->toNlohmann();
    }

    namespace {
        std::shared_ptr<JsonNode> fromNlohmann(const nlohmann::ordered_json& j, JsonNodeOptions options) {
            if (j.is_null()) return nullptr;
            if (j.is_object()) {
                auto obj = std::make_shared<JsonObject>(options);
                for (const auto& [key, val] : j.items()) obj->Add(key, fromNlohmann(val, options));
                return obj;
            }
            if (j.is_array()) {
                auto arr = std::make_shared<JsonArray>(options);
                for (const auto& item : j) arr->Add(fromNlohmann(item, options));
                return arr;
            }
            return JsonValue::FromNlohmann(j, options);
        }
    } // namespace

    std::shared_ptr<JsonNode> JsonNode::Parse(const std::string& json, JsonNodeOptions options) {
        try {
            auto j = nlohmann::ordered_json::parse(json);
            return fromNlohmann(j, options);
        } catch (const nlohmann::ordered_json::parse_error& e) {
            throw JsonException(std::string("'") + e.what() + "'.");
        }
    }

} // namespace System::Text::Json::Nodes
