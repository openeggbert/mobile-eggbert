// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Attribute.hpp"
#include "System/Text/Json/Serialization/JsonNumberHandling.hpp"
#include "System/Text/Json/Serialization/JsonObjectCreationHandling.hpp"
#include "System/Text/Json/Serialization/JsonUnknownDerivedTypeHandling.hpp"
#include "System/Text/Json/Serialization/JsonUnmappedMemberHandling.hpp"

namespace System::Text::Json::Serialization {

    using SharpRuntime::intcs;

    /** @brief The base class of serialization attributes. C++ counterpart of .NET System.Text.Json.Serialization.JsonAttribute. */
    class JsonAttribute : public System::Attribute {};

    /** @brief Specifies the JSON property name for a member. */
    class JsonPropertyNameAttribute : public JsonAttribute {
        std::string name_;
    public:
        /** @brief Constructs the attribute with the given JSON property name. */
        explicit JsonPropertyNameAttribute(const std::string& name) : name_(name) {}
        /** @return The JSON property name. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
    };

    /** @brief Controls when a property is ignored during JSON serialization. */
    enum class JsonIgnoreCondition {
        /** @brief Property is never ignored during serialization or deserialization. */
        Never = 0,
        /** @brief Property is always ignored during serialization and deserialization. */
        Always = 1,
        /** @brief Property is ignored during serialization if its value is the type's default. */
        WhenWritingDefault = 2,
        /** @brief Property is ignored during serialization if its value is null (reference types only). */
        WhenWritingNull = 3,
        /** @brief Property is ignored during serialization. */
        WhenWriting = 4,
        /** @brief Property is ignored during deserialization. */
        WhenReading = 5,
    };

    /** @brief Marks a property as ignored during JSON serialization/deserialization. */
    class JsonIgnoreAttribute : public JsonAttribute {
        JsonIgnoreCondition condition_ = JsonIgnoreCondition::Always;
    public:
        /** @brief Constructs the attribute using the Always condition. */
        JsonIgnoreAttribute() = default;
        /** @brief Constructs the attribute with the specified ignore condition. */
        explicit JsonIgnoreAttribute(JsonIgnoreCondition cond) : condition_(cond) {}
        /** @return The condition under which the property is ignored. */
        [[nodiscard]] JsonIgnoreCondition getConditionProperty() const { return condition_; }
        /** @brief Sets the condition under which the property is ignored. */
        void setConditionProperty(JsonIgnoreCondition c) { condition_ = c; }
    };

    /** @brief Associates a custom JsonConverter with a property or type (by type name — see JsonConverterAttribute's doc comment). */
    class JsonConverterAttribute : public JsonAttribute {
        std::string converterTypeName_;
    public:
        /** @brief Constructs the attribute with no explicit converter type. */
        JsonConverterAttribute() = default;
        /**
         * @brief Constructs the attribute naming the converter type to use.
         * @note Real .NET stores a `System.Type`; this port has no reflection/`Type` system (see
         * CLAUDE.md's parity philosophy), so the converter is identified by name only — resolving
         * that name to an actual `JsonConverter` instance is left to caller-side lookup code.
         */
        explicit JsonConverterAttribute(const std::string& typeName) : converterTypeName_(typeName) {}
        /** @return The converter type name. */
        [[nodiscard]] const std::string& getConverterTypeNameProperty() const { return converterTypeName_; }
    };

    /** @brief Specifies the serialization order of a JSON property. */
    class JsonPropertyOrderAttribute : public JsonAttribute {
        intcs order_;
    public:
        /** @brief Constructs the attribute with the given order value. */
        explicit JsonPropertyOrderAttribute(intcs order) : order_(order) {}
        /** @return The serialization order. */
        [[nodiscard]] intcs getOrderProperty() const { return order_; }
    };

    /** @brief Marks a non-public property for inclusion in JSON serialization. */
    class JsonIncludeAttribute : public JsonAttribute {};

    /** @brief Marks a property as required in JSON serialization. */
    class JsonRequiredAttribute : public JsonAttribute {};

    /** @brief Marks a property for receiving extension data (extra JSON properties). */
    class JsonExtensionDataAttribute : public JsonAttribute {};

    /** @brief When placed on a constructor, indicates it should be used to create instances of the type on deserialization. */
    class JsonConstructorAttribute : public JsonAttribute {};

    /** @brief Specifies number-handling behavior for a property or type. */
    class JsonNumberHandlingAttribute : public JsonAttribute {
        JsonNumberHandling handling_;
    public:
        /** @brief Constructs the attribute with the given JsonNumberHandling value. */
        explicit JsonNumberHandlingAttribute(JsonNumberHandling handling) : handling_(handling) {}
        /** @return The number-handling value. */
        [[nodiscard]] JsonNumberHandling getHandlingProperty() const { return handling_; }
    };

    /**
     * @brief Determines how deserialization handles object creation for the annotated field,
     * property, or type.
     */
    class JsonObjectCreationHandlingAttribute : public JsonAttribute {
        JsonObjectCreationHandling handling_;
    public:
        /** @brief Constructs the attribute with the given handling value. @throws System::ArgumentOutOfRangeException if @p handling isn't a valid enumerator. */
        explicit JsonObjectCreationHandlingAttribute(JsonObjectCreationHandling handling) : handling_(handling) {
            if (handling != JsonObjectCreationHandling::Replace && handling != JsonObjectCreationHandling::Populate)
                throw System::ArgumentOutOfRangeException("handling");
        }
        /** @return The object-creation handling value. */
        [[nodiscard]] JsonObjectCreationHandling getHandlingProperty() const { return handling_; }
    };

    /** @brief Overrides JsonSerializerOptions::UnmappedMemberHandling for the annotated type. */
    class JsonUnmappedMemberHandlingAttribute : public JsonAttribute {
        JsonUnmappedMemberHandling handling_;
    public:
        /** @brief Constructs the attribute with the given handling value. */
        explicit JsonUnmappedMemberHandlingAttribute(JsonUnmappedMemberHandling handling) : handling_(handling) {}
        /** @return The unmapped-member handling value. */
        [[nodiscard]] JsonUnmappedMemberHandling getUnmappedMemberHandlingProperty() const { return handling_; }
    };

    /** @brief Determines the string value used when serializing an enum member. */
    class JsonStringEnumMemberNameAttribute : public System::Attribute {
        std::string name_;
    public:
        /** @brief Constructs the attribute with the given enum member name. */
        explicit JsonStringEnumMemberNameAttribute(const std::string& name) : name_(name) {}
        /** @return The name to apply to the enum member. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
    };

    /** @brief Enables polymorphic type serialization for a base class. */
    class JsonPolymorphicAttribute : public JsonAttribute {
    public:
        /**
         * @brief Controls how unknown derived types are handled.
         * @note Real .NET types this JsonUnknownDerivedTypeHandling, not bool -- an earlier
         * version of this port declared it as bool, which would silently discard everything but
         * FailSerialization/FallBackToBaseType and could never represent
         * FallBackToNearestAncestor.
         */
        JsonUnknownDerivedTypeHandling UnknownDerivedTypeHandling = JsonUnknownDerivedTypeHandling::FailSerialization;
        /** @brief Controls whether unrecognized type discriminators are ignored. */
        bool IgnoreUnrecognizedTypeDiscriminators = false;
        /** @brief The JSON property name used as a type discriminator. */
        std::string TypeDiscriminatorPropertyName;
    };

    /** @brief Associates a derived type with a base class for polymorphic deserialization. */
    class JsonDerivedTypeAttribute : public JsonAttribute {
        std::string derivedTypeName_;
    public:
        /** @brief Constructs the attribute for the given derived type name. */
        explicit JsonDerivedTypeAttribute(const std::string& typeName) : derivedTypeName_(typeName) {}
        /** @brief Constructs the attribute with a derived type name and a type discriminator value. */
        explicit JsonDerivedTypeAttribute(const std::string& typeName, const std::string& /*typeDiscriminator*/)
            : derivedTypeName_(typeName) {}
        /** @return The derived type name. */
        [[nodiscard]] const std::string& getDerivedTypeNameProperty() const { return derivedTypeName_; }
    };

} // namespace System::Text::Json::Serialization
