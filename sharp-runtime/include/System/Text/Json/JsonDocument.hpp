// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "System/IDisposable.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Text/Json/JsonDocumentOptions.hpp"
#include "System/Text/Json/JsonElement.hpp"
#include "System/Text/Json/JsonException.hpp"
#include "nlohmann/json.hpp"

namespace System::Text::Json {

    /**
     * @brief Represents the in-memory tree of a parsed JSON document, providing read-only access
     * via a root JsonElement.
     *
     * C++ counterpart of .NET System.Text.Json.JsonDocument.
     */
    class JsonDocument : public System::IDisposable {
        std::shared_ptr<const nlohmann::ordered_json> root_;
        bool disposed_ = false;

        explicit JsonDocument(std::shared_ptr<const nlohmann::ordered_json> root) : root_(std::move(root)) {}

        // Verified against Utf8JsonReader.cs (ReadFirstToken/ConsumeNextTokenOrRollback): real .NET
        // throws when about to open a container while already at the configured MaxDepth -- i.e. at
        // most MaxDepth levels of nested containers are allowed. currentDepth here is the nesting
        // level of the container being examined (1 for a root-level object/array).
        static void checkMaxDepth(const nlohmann::ordered_json& node, SharpRuntime::intcs currentDepth,
                                   SharpRuntime::intcs maxDepth) {
            if (!node.is_object() && !node.is_array()) return;
            if (currentDepth > maxDepth) {
                throw JsonException("The maximum configured depth of " + std::to_string(maxDepth) +
                    " has been exceeded. Cannot read next JSON " + (node.is_object() ? "object" : "array") + ".");
            }
            for (const auto& child : node)
                checkMaxDepth(child, currentDepth + 1, maxDepth);
        }

    public:
        ~JsonDocument() override = default;

        /** @brief Releases the root element and marks the document as disposed. */
        void Dispose() override {
            disposed_ = true;
            root_.reset();
        }

        /** @return The root JsonElement of this document. @throws System::ObjectDisposedException if disposed. */
        [[nodiscard]] JsonElement getRootElementProperty() const {
            if (disposed_) throw System::ObjectDisposedException("JsonDocument");
            return JsonElement(root_);
        }

        /**
         * @brief Parses UTF-8/ASCII JSON text and returns a JsonDocument.
         * @throws JsonException if the input is not valid JSON.
         */
        static std::shared_ptr<JsonDocument> Parse(const std::string& json, JsonDocumentOptions options = {}) {
            options.Validate();
            try {
                auto parsed = std::make_shared<const nlohmann::ordered_json>(
                    nlohmann::ordered_json::parse(json, /*callback=*/nullptr, /*allow_exceptions=*/true,
                                          /*ignore_comments=*/options.CommentHandling != JsonCommentHandling::Disallow));
                auto effectiveMaxDepth = options.MaxDepth == 0 ? JsonDocumentOptions::DefaultMaxDepth : options.MaxDepth;
                checkMaxDepth(*parsed, 1, effectiveMaxDepth);
                return std::shared_ptr<JsonDocument>(new JsonDocument(std::move(parsed)));
            } catch (const nlohmann::ordered_json::parse_error& e) {
                throw JsonException(std::string("'") + e.what() + "'.");
            }
        }

        /** @brief Parses a JSON value (alias for Parse). */
        static std::shared_ptr<JsonDocument> ParseValue(const std::string& json, JsonDocumentOptions options = {}) {
            return Parse(json, options);
        }
    };

} // namespace System::Text::Json
