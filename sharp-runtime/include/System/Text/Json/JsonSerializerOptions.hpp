// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/Json/JsonCommentHandling.hpp"
#include "System/Text/Json/JsonNamingPolicy.hpp"
#include "System/Text/Json/JsonSerializerDefaults.hpp"
#include "System/Text/Json/Serialization/JsonNumberHandling.hpp"
#include "System/Text/Json/Serialization/JsonUnknownTypeHandling.hpp"
#include "System/Text/Json/Serialization/JsonUnmappedMemberHandling.hpp"

namespace System::Text::Json {

    using SharpRuntime::intcs;

    /**
     * @brief Provides options to be used with JsonSerializer / JsonDocument / Utf8JsonWriter.
     *
     * C++ counterpart of .NET System.Text.Json.JsonSerializerOptions.
     *
     * @note No `Converters`, `TypeInfoResolver`(Chain), or `ReferenceHandler` — these all depend
     * on either reflection (out of scope, see CLAUDE.md's parity philosophy) or a source-generated
     * `JsonTypeInfo` metadata model this runtime doesn't build. No `Encoder` (see
     * JsonWriterOptions's doc comment). `RespectNullableAnnotations`/
     * `RespectRequiredConstructorParameters` are C#-nullable-annotation/required-member language
     * features with no C++ equivalent.
     */
    class JsonSerializerOptions {
        bool allowTrailingCommas_ = false;
        bool allowDuplicateProperties_ = true;
        intcs defaultBufferSize_ = 16 * 1024;
        std::shared_ptr<JsonNamingPolicy> dictionaryKeyPolicy_;
        bool ignoreReadOnlyProperties_ = false;
        bool ignoreReadOnlyFields_ = false;
        bool includeFields_ = false;
        intcs maxDepth_ = 0;
        Serialization::JsonNumberHandling numberHandling_ = Serialization::JsonNumberHandling::Strict;
        std::shared_ptr<JsonNamingPolicy> propertyNamingPolicy_;
        bool propertyNameCaseInsensitive_ = false;
        JsonCommentHandling readCommentHandling_ = JsonCommentHandling::Disallow;
        Serialization::JsonUnknownTypeHandling unknownTypeHandling_ = Serialization::JsonUnknownTypeHandling::JsonElement;
        Serialization::JsonUnmappedMemberHandling unmappedMemberHandling_ = Serialization::JsonUnmappedMemberHandling::Skip;
        bool writeIndented_ = false;
        intcs indentSize_ = 2;
        char indentCharacter_ = ' ';
        std::string newLine_ = "\n";

    public:
        /** @brief Constructs default options (no indentation, strict comments, case-sensitive). */
        JsonSerializerOptions() = default;

        /**
         * @brief Constructs options initialized from a set of well-known defaults.
         *
         * @note @c Strict's real-.NET effect also includes @c RespectNullableAnnotations and
         * @c RespectRequiredConstructorParameters (C#-nullable-annotation/required-member
         * language features with no C++ equivalent in this port, per the class doc-comment) --
         * only the two applicable properties (@c UnmappedMemberHandling,
         * @c AllowDuplicateProperties) are set here.
         */
        explicit JsonSerializerOptions(JsonSerializerDefaults defaults) {
            if (defaults == JsonSerializerDefaults::Web) {
                propertyNameCaseInsensitive_ = true;
                propertyNamingPolicy_ = JsonNamingPolicy::CamelCase();
                numberHandling_ = Serialization::JsonNumberHandling::AllowReadingFromString;
            } else if (defaults == JsonSerializerDefaults::Strict) {
                unmappedMemberHandling_ = Serialization::JsonUnmappedMemberHandling::Disallow;
                allowDuplicateProperties_ = false;
            }
        }

        [[nodiscard]] bool getAllowTrailingCommasProperty() const { return allowTrailingCommas_; }
        void setAllowTrailingCommasProperty(bool v) { allowTrailingCommas_ = v; }

        [[nodiscard]] bool getAllowDuplicatePropertiesProperty() const { return allowDuplicateProperties_; }
        void setAllowDuplicatePropertiesProperty(bool v) { allowDuplicateProperties_ = v; }

        [[nodiscard]] intcs getDefaultBufferSizeProperty() const { return defaultBufferSize_; }
        void setDefaultBufferSizeProperty(intcs v) { defaultBufferSize_ = v; }

        [[nodiscard]] const std::shared_ptr<JsonNamingPolicy>& getDictionaryKeyPolicyProperty() const { return dictionaryKeyPolicy_; }
        void setDictionaryKeyPolicyProperty(std::shared_ptr<JsonNamingPolicy> v) { dictionaryKeyPolicy_ = std::move(v); }

        [[nodiscard]] bool getIgnoreReadOnlyPropertiesProperty() const { return ignoreReadOnlyProperties_; }
        void setIgnoreReadOnlyPropertiesProperty(bool v) { ignoreReadOnlyProperties_ = v; }

        [[nodiscard]] bool getIgnoreReadOnlyFieldsProperty() const { return ignoreReadOnlyFields_; }
        void setIgnoreReadOnlyFieldsProperty(bool v) { ignoreReadOnlyFields_ = v; }

        [[nodiscard]] bool getIncludeFieldsProperty() const { return includeFields_; }
        void setIncludeFieldsProperty(bool v) { includeFields_ = v; }

        [[nodiscard]] intcs getMaxDepthProperty() const { return maxDepth_; }
        void setMaxDepthProperty(intcs v) { maxDepth_ = v; }

        [[nodiscard]] Serialization::JsonNumberHandling getNumberHandlingProperty() const { return numberHandling_; }
        void setNumberHandlingProperty(Serialization::JsonNumberHandling v) { numberHandling_ = v; }

        [[nodiscard]] const std::shared_ptr<JsonNamingPolicy>& getPropertyNamingPolicyProperty() const { return propertyNamingPolicy_; }
        void setPropertyNamingPolicyProperty(std::shared_ptr<JsonNamingPolicy> v) { propertyNamingPolicy_ = std::move(v); }

        [[nodiscard]] bool getPropertyNameCaseInsensitiveProperty() const { return propertyNameCaseInsensitive_; }
        void setPropertyNameCaseInsensitiveProperty(bool v) { propertyNameCaseInsensitive_ = v; }

        [[nodiscard]] JsonCommentHandling getReadCommentHandlingProperty() const { return readCommentHandling_; }
        void setReadCommentHandlingProperty(JsonCommentHandling v) { readCommentHandling_ = v; }

        [[nodiscard]] Serialization::JsonUnknownTypeHandling getUnknownTypeHandlingProperty() const { return unknownTypeHandling_; }
        void setUnknownTypeHandlingProperty(Serialization::JsonUnknownTypeHandling v) { unknownTypeHandling_ = v; }

        [[nodiscard]] Serialization::JsonUnmappedMemberHandling getUnmappedMemberHandlingProperty() const { return unmappedMemberHandling_; }
        void setUnmappedMemberHandlingProperty(Serialization::JsonUnmappedMemberHandling v) { unmappedMemberHandling_ = v; }

        [[nodiscard]] bool getWriteIndentedProperty() const { return writeIndented_; }
        void setWriteIndentedProperty(bool v) { writeIndented_ = v; }

        [[nodiscard]] intcs getIndentSizeProperty() const { return indentSize_; }
        void setIndentSizeProperty(intcs v) { indentSize_ = v; }

        [[nodiscard]] char getIndentCharacterProperty() const { return indentCharacter_; }
        void setIndentCharacterProperty(char v) { indentCharacter_ = v; }

        [[nodiscard]] const std::string& getNewLineProperty() const { return newLine_; }
        void setNewLineProperty(const std::string& v) { newLine_ = v; }

        /** @return The default JsonSerializerOptions instance. */
        static const JsonSerializerOptions& Default() {
            static JsonSerializerOptions opts;
            return opts;
        }
    };

} // namespace System::Text::Json
