// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cctype>
#include <type_traits>
#include "System/InvalidOperationException.hpp"
#include "System/NotImplementedException.hpp"
#include "System/Text/Json/JsonDocument.hpp"
#include "System/Text/Json/JsonElement.hpp"
#include "System/Text/Json/JsonException.hpp"
#include "System/Text/Json/JsonSerializerDefaults.hpp"
#include "System/Text/Json/Serialization/JsonConverter.hpp"
#include "System/Text/Json/Serialization/JsonConverterFactory.hpp"
#include "System/Text/Json/Serialization/JsonKnownNamingPolicy.hpp"
#include "System/Text/Json/Serialization/JsonObjectCreationHandling.hpp"
#include "System/Text/Json/Serialization/JsonOnSerializationInterfaces.hpp"
#include "System/Text/Json/Serialization/JsonSerializationAttributes.hpp"
#include "System/Text/Json/Serialization/JsonStringEnumConverter.hpp"
#include "System/Text/Json/Serialization/JsonUnknownDerivedTypeHandling.hpp"
#include "System/Text/Json/Serialization/ReferenceHandler.hpp"
#include "System/Text/Json/Utf8JsonWriter.hpp"

using System::Text::Json::JsonDocument;
using System::Text::Json::JsonElement;
using System::Text::Json::JsonSerializerOptions;
using System::Text::Json::Utf8JsonWriter;
using namespace System::Text::Json::Serialization;

// ===========================================================================
// Simple enums
// ===========================================================================

TEST(JsonKnownNamingPolicyTests, Values) {
    EXPECT_EQ(static_cast<int>(JsonKnownNamingPolicy::Unspecified), 0);
    EXPECT_EQ(static_cast<int>(JsonKnownNamingPolicy::CamelCase), 1);
    EXPECT_EQ(static_cast<int>(JsonKnownNamingPolicy::PascalCase), 6);
}

TEST(JsonObjectCreationHandlingTests, Values) {
    EXPECT_EQ(static_cast<int>(JsonObjectCreationHandling::Replace), 0);
    EXPECT_EQ(static_cast<int>(JsonObjectCreationHandling::Populate), 1);
}

TEST(JsonUnknownDerivedTypeHandlingTests, Values) {
    EXPECT_EQ(static_cast<int>(JsonUnknownDerivedTypeHandling::FailSerialization), 0);
    EXPECT_EQ(static_cast<int>(JsonUnknownDerivedTypeHandling::FallBackToBaseType), 1);
    EXPECT_EQ(static_cast<int>(JsonUnknownDerivedTypeHandling::FallBackToNearestAncestor), 2);
}

TEST(JsonIgnoreConditionTests, HasWhenWritingAndWhenReading) {
    EXPECT_EQ(static_cast<int>(JsonIgnoreCondition::WhenWriting), 4);
    EXPECT_EQ(static_cast<int>(JsonIgnoreCondition::WhenReading), 5);
}

// ===========================================================================
// Attributes
// ===========================================================================

TEST(JsonAttributeTests, JsonPropertyNameAttribute_IsAJsonAttribute) {
    static_assert(std::is_base_of_v<JsonAttribute, JsonPropertyNameAttribute>);
}

TEST(JsonConstructorAttributeTests, ConstructibleAndIsJsonAttribute) {
    JsonConstructorAttribute a;
    (void)a;
    static_assert(std::is_base_of_v<JsonAttribute, JsonConstructorAttribute>);
}

TEST(JsonNumberHandlingAttributeTests, StoresTypedEnumValue) {
    JsonNumberHandlingAttribute a(JsonNumberHandling::AllowReadingFromString);
    EXPECT_EQ(a.getHandlingProperty(), JsonNumberHandling::AllowReadingFromString);
}

TEST(JsonObjectCreationHandlingAttributeTests, StoresValue) {
    JsonObjectCreationHandlingAttribute a(JsonObjectCreationHandling::Populate);
    EXPECT_EQ(a.getHandlingProperty(), JsonObjectCreationHandling::Populate);
}

TEST(JsonObjectCreationHandlingAttributeTests, InvalidValue_Throws) {
    EXPECT_THROW(JsonObjectCreationHandlingAttribute(static_cast<JsonObjectCreationHandling>(99)),
                 System::ArgumentOutOfRangeException);
}

TEST(JsonUnmappedMemberHandlingAttributeTests, StoresValue) {
    JsonUnmappedMemberHandlingAttribute a(JsonUnmappedMemberHandling::Disallow);
    EXPECT_EQ(a.getUnmappedMemberHandlingProperty(), JsonUnmappedMemberHandling::Disallow);
}

TEST(JsonStringEnumMemberNameAttributeTests, StoresName) {
    JsonStringEnumMemberNameAttribute a("custom_name");
    EXPECT_EQ(a.getNameProperty(), "custom_name");
}

TEST(JsonDerivedTypeAttributeTests, StoresTypeName) {
    JsonDerivedTypeAttribute a("Dog");
    EXPECT_EQ(a.getDerivedTypeNameProperty(), "Dog");
}

TEST(JsonPolymorphicAttributeTests, DefaultFields) {
    JsonPolymorphicAttribute a;
    EXPECT_EQ(a.UnknownDerivedTypeHandling, JsonUnknownDerivedTypeHandling::FailSerialization);
    EXPECT_FALSE(a.IgnoreUnrecognizedTypeDiscriminators);
    EXPECT_TRUE(a.TypeDiscriminatorPropertyName.empty());
}

TEST(JsonPolymorphicAttributeTests, UnknownDerivedTypeHandling_CanBeSetToEachValue) {
    JsonPolymorphicAttribute a;
    a.UnknownDerivedTypeHandling = JsonUnknownDerivedTypeHandling::FallBackToNearestAncestor;
    EXPECT_EQ(a.UnknownDerivedTypeHandling, JsonUnknownDerivedTypeHandling::FallBackToNearestAncestor);
}

// ===========================================================================
// IJsonOnSerializing / IJsonOnSerialized / IJsonOnDeserializing / IJsonOnDeserialized
// ===========================================================================

namespace {
class LifecycleTrackedObject : public IJsonOnSerializing,
                                public IJsonOnSerialized,
                                public IJsonOnDeserializing,
                                public IJsonOnDeserialized {
public:
    bool serializing = false, serialized = false, deserializing = false, deserialized = false;
    void OnSerializing() override { serializing = true; }
    void OnSerialized() override { serialized = true; }
    void OnDeserializing() override { deserializing = true; }
    void OnDeserialized() override { deserialized = true; }
};
} // namespace

TEST(JsonOnSerializationInterfacesTests, CanBeImplementedAndInvoked) {
    LifecycleTrackedObject obj;
    obj.OnSerializing();
    obj.OnSerialized();
    obj.OnDeserializing();
    obj.OnDeserialized();
    EXPECT_TRUE(obj.serializing);
    EXPECT_TRUE(obj.serialized);
    EXPECT_TRUE(obj.deserializing);
    EXPECT_TRUE(obj.deserialized);
}

// ===========================================================================
// JsonConverter<T> / JsonConverterFactory
// ===========================================================================

namespace {
class UpperCaseStringConverter : public JsonConverter<std::string> {
public:
    [[nodiscard]] std::string Read(const JsonElement& element, const JsonSerializerOptions&) const override {
        return element.GetString();
    }
    void Write(Utf8JsonWriter& writer, const std::string& value, const JsonSerializerOptions&) const override {
        std::string upper = value;
        for (char& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        writer.WriteStringValue(upper);
    }
};

class StringConverterFactory : public JsonConverterFactory {
public:
    [[nodiscard]] bool CanConvert(const std::string& typeName) const override { return typeName == "string"; }
};
} // namespace

TEST(JsonConverterTests, Read_ExtractsValueFromJsonElement) {
    auto doc = JsonDocument::Parse("\"hello\"");
    UpperCaseStringConverter converter;
    EXPECT_EQ(converter.Read(doc->getRootElementProperty(), JsonSerializerOptions::Default()), "hello");
}

TEST(JsonConverterTests, Write_UsesCustomLogic) {
    UpperCaseStringConverter converter;
    Utf8JsonWriter writer;
    converter.Write(writer, "hello", JsonSerializerOptions::Default());
    EXPECT_EQ(writer.GetString(), "\"HELLO\"");
}

TEST(JsonConverterTests, HandleNull_DefaultsFalse) {
    UpperCaseStringConverter converter;
    EXPECT_FALSE(converter.HandleNull());
}

TEST(JsonConverterFactoryTests, CanConvert_MatchesByTypeName) {
    StringConverterFactory factory;
    EXPECT_TRUE(factory.CanConvert("string"));
    EXPECT_FALSE(factory.CanConvert("int"));
}

// ===========================================================================
// JsonStringEnumConverter<TEnum>
// ===========================================================================

namespace {
enum class Color { Red, Green, Blue };
}

TEST(JsonStringEnumConverterTests, Write_EmitsMappedName) {
    JsonStringEnumConverter<Color> converter({{Color::Red, "Red"}, {Color::Green, "Green"}, {Color::Blue, "Blue"}});
    Utf8JsonWriter writer;
    converter.Write(writer, Color::Green, JsonSerializerOptions::Default());
    EXPECT_EQ(writer.GetString(), "\"Green\"");
}

TEST(JsonStringEnumConverterTests, Read_ParsesMappedName) {
    JsonStringEnumConverter<Color> converter({{Color::Red, "Red"}, {Color::Green, "Green"}, {Color::Blue, "Blue"}});
    auto doc = JsonDocument::Parse("\"Blue\"");
    EXPECT_EQ(converter.Read(doc->getRootElementProperty(), JsonSerializerOptions::Default()), Color::Blue);
}

TEST(JsonStringEnumConverterTests, Read_UnknownName_Throws) {
    JsonStringEnumConverter<Color> converter({{Color::Red, "Red"}});
    auto doc = JsonDocument::Parse("\"Purple\"");
    EXPECT_THROW(converter.Read(doc->getRootElementProperty(), JsonSerializerOptions::Default()),
                 System::Text::Json::JsonException);
}

// ===========================================================================
// ReferenceHandler / ReferenceResolver
// ===========================================================================

TEST(ReferenceHandlerTests, Preserve_ReturnsSameInstance) {
    EXPECT_EQ(ReferenceHandler::Preserve(), ReferenceHandler::Preserve());
}

TEST(ReferenceHandlerTests, Preserve_And_IgnoreCycles_AreDistinct) {
    EXPECT_NE(ReferenceHandler::Preserve(), ReferenceHandler::IgnoreCycles());
}

TEST(ReferenceResolverTests, Preserve_GetReference_TracksIdentity) {
    auto resolver = ReferenceHandler::Preserve()->CreateResolver(/*writing=*/true);
    int a = 1, b = 2;
    bool alreadyExists = true;
    std::string idA = resolver->GetReference(&a, alreadyExists);
    EXPECT_FALSE(alreadyExists);
    std::string idASecondTime = resolver->GetReference(&a, alreadyExists);
    EXPECT_TRUE(alreadyExists);
    EXPECT_EQ(idA, idASecondTime);
    std::string idB = resolver->GetReference(&b, alreadyExists);
    EXPECT_FALSE(alreadyExists);
    EXPECT_NE(idA, idB);
}

TEST(ReferenceResolverTests, Preserve_AddThenResolve_RoundTrips) {
    auto resolver = ReferenceHandler::Preserve()->CreateResolver(/*writing=*/false);
    int value = 42;
    resolver->AddReference("1", &value);
    EXPECT_EQ(resolver->ResolveReference("1"), &value);
}

TEST(ReferenceResolverTests, Preserve_AddDuplicateId_Throws) {
    auto resolver = ReferenceHandler::Preserve()->CreateResolver(/*writing=*/false);
    int a = 1, b = 2;
    resolver->AddReference("1", &a);
    EXPECT_THROW(resolver->AddReference("1", &b), System::Text::Json::JsonException);
}

TEST(ReferenceResolverTests, Preserve_ResolveMissingId_Throws) {
    auto resolver = ReferenceHandler::Preserve()->CreateResolver(/*writing=*/false);
    EXPECT_THROW(resolver->ResolveReference("missing"), System::Text::Json::JsonException);
}

TEST(ReferenceResolverTests, IgnoreCycles_GetReference_DetectsRepeatedIdentity) {
    auto resolver = ReferenceHandler::IgnoreCycles()->CreateResolver(/*writing=*/true);
    int a = 1;
    bool alreadyExists = true;
    resolver->GetReference(&a, alreadyExists);
    EXPECT_FALSE(alreadyExists);
    resolver->GetReference(&a, alreadyExists);
    EXPECT_TRUE(alreadyExists);
}

TEST(ReferenceResolverTests, IgnoreCycles_AddReference_ThrowsInvalidOperation) {
    auto resolver = ReferenceHandler::IgnoreCycles()->CreateResolver(/*writing=*/false);
    EXPECT_THROW(resolver->AddReference("1", nullptr), System::InvalidOperationException);
}

// ===========================================================================
// JsonSerializerOptions defaults
// ===========================================================================

// Regression test for a wave-3 audit finding: AllowDuplicateProperties defaulted to false,
// where real .NET's backing field (_allowDuplicateProperties) defaults to true -- the sibling
// JsonDocumentOptions.AllowDuplicateProperties in this same codebase already correctly
// defaults true, confirming this was an oversight, not a deliberate divergence.
TEST(JsonSerializerOptionsTests, AllowDuplicateProperties_DefaultsTrue) {
    JsonSerializerOptions opts;
    EXPECT_TRUE(opts.getAllowDuplicatePropertiesProperty());
}

// Regression test for a wave-3 audit finding: JsonSerializerDefaults::Strict was a silent
// no-op (the constructor only handled ::Web), so requesting Strict defaults silently produced
// General behavior instead. Verified against JsonSerializerOptions.cs's
// JsonSerializerOptions(JsonSerializerDefaults) constructor.
TEST(JsonSerializerOptionsTests, StrictDefaults_DisallowsUnmappedAndDuplicateProperties) {
    JsonSerializerOptions opts(System::Text::Json::JsonSerializerDefaults::Strict);
    EXPECT_EQ(opts.getUnmappedMemberHandlingProperty(), JsonUnmappedMemberHandling::Disallow);
    EXPECT_FALSE(opts.getAllowDuplicatePropertiesProperty());
}

TEST(JsonSerializerOptionsTests, WebDefaults_Unaffected) {
    JsonSerializerOptions opts(System::Text::Json::JsonSerializerDefaults::Web);
    EXPECT_TRUE(opts.getPropertyNameCaseInsensitiveProperty());
    EXPECT_TRUE(opts.getAllowDuplicatePropertiesProperty());
}
