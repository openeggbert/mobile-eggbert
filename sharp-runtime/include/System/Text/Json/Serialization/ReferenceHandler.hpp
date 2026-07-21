// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <map>
#include <memory>
#include <set>
#include <string>
#include "System/InvalidOperationException.hpp"
#include "System/Text/Json/JsonException.hpp"
#include "System/Text/Json/Serialization/ReferenceResolver.hpp"

namespace System::Text::Json::Serialization {

    /**
     * @brief Defines how a JsonSerializer deals with references on serialization and deserialization.
     *
     * C++ counterpart of .NET System.Text.Json.Serialization.ReferenceHandler.
     * @note See ReferenceResolver's doc comment — these resolvers are real but not automatically
     * wired into JsonSerializer::Serialize/Deserialize.
     */
    class ReferenceHandler {
    public:
        virtual ~ReferenceHandler() = default;

        /** @return The resolver to use for a serialization (@p writing true) or deserialization (false) call. */
        [[nodiscard]] virtual std::shared_ptr<ReferenceResolver> CreateResolver(bool writing) const = 0;

        /** @return A handler that preserves duplicate references and cycles via `$id`/`$ref`/`$values` metadata. */
        static const std::shared_ptr<ReferenceHandler>& Preserve();
        /** @return A handler that silently ignores an object when a reference cycle is detected during serialization. */
        static const std::shared_ptr<ReferenceHandler>& IgnoreCycles();
    };

    namespace detail {

        // Mirrors .NET's internal PreserveReferenceResolver: a bidirectional id<->object map used
        // to detect and round-trip duplicate references/cycles via $id/$ref metadata.
        class PreserveReferenceResolver : public ReferenceResolver {
            unsigned referenceCount_ = 0;
            std::map<std::string, const void*> idToObject_;
            std::map<const void*, std::string> objectToId_;

        public:
            void AddReference(const std::string& referenceId, const void* value) override {
                if (!idToObject_.emplace(referenceId, value).second)
                    throw JsonException("A duplicate '$id' value was found while preserving references: '" + referenceId + "'.");
            }
            [[nodiscard]] std::string GetReference(const void* value, bool& alreadyExists) override {
                auto it = objectToId_.find(value);
                if (it != objectToId_.end()) {
                    alreadyExists = true;
                    return it->second;
                }
                alreadyExists = false;
                std::string id = std::to_string(++referenceCount_);
                objectToId_.emplace(value, id);
                return id;
            }
            [[nodiscard]] const void* ResolveReference(const std::string& referenceId) override {
                auto it = idToObject_.find(referenceId);
                if (it == idToObject_.end())
                    throw JsonException("The reference value '" + referenceId + "' was not found.");
                return it->second;
            }
        };

        // Mirrors .NET's internal IgnoreReferenceResolver: tracks only the ancestor chain currently
        // being serialized (no $id/$ref metadata is ever emitted), so cycles can be detected and
        // skipped without round-trip support.
        class IgnoreReferenceResolver : public ReferenceResolver {
            std::set<const void*> seen_;

        public:
            void AddReference(const std::string&, const void*) override {
                throw System::InvalidOperationException();
            }
            [[nodiscard]] std::string GetReference(const void* value, bool& alreadyExists) override {
                alreadyExists = !seen_.insert(value).second;
                return "";
            }
            [[nodiscard]] const void* ResolveReference(const std::string&) override {
                throw System::InvalidOperationException();
            }
        };

        class PreserveReferenceHandler final : public ReferenceHandler {
        public:
            [[nodiscard]] std::shared_ptr<ReferenceResolver> CreateResolver(bool /*writing*/) const override {
                return std::make_shared<PreserveReferenceResolver>();
            }
        };

        class IgnoreReferenceHandler final : public ReferenceHandler {
        public:
            [[nodiscard]] std::shared_ptr<ReferenceResolver> CreateResolver(bool /*writing*/) const override {
                return std::make_shared<IgnoreReferenceResolver>();
            }
        };

    } // namespace detail

    inline const std::shared_ptr<ReferenceHandler>& ReferenceHandler::Preserve() {
        static const std::shared_ptr<ReferenceHandler> instance = std::make_shared<detail::PreserveReferenceHandler>();
        return instance;
    }
    inline const std::shared_ptr<ReferenceHandler>& ReferenceHandler::IgnoreCycles() {
        static const std::shared_ptr<ReferenceHandler> instance = std::make_shared<detail::IgnoreReferenceHandler>();
        return instance;
    }

} // namespace System::Text::Json::Serialization
