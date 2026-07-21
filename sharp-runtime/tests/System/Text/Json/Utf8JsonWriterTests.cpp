// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <limits>
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Text/Json/JsonDocument.hpp"
#include "System/Text/Json/Utf8JsonWriter.hpp"

using System::Text::Json::JsonDocument;
using System::Text::Json::JsonWriterOptions;
using System::Text::Json::Utf8JsonWriter;

TEST(Utf8JsonWriterTests, WriteEmptyObject) {
    Utf8JsonWriter w;
    w.WriteStartObject();
    w.WriteEndObject();
    EXPECT_EQ(w.GetString(), "{}");
}

TEST(Utf8JsonWriterTests, WriteEmptyArray) {
    Utf8JsonWriter w;
    w.WriteStartArray();
    w.WriteEndArray();
    EXPECT_EQ(w.GetString(), "[]");
}

TEST(Utf8JsonWriterTests, WriteObjectWithProperties) {
    Utf8JsonWriter w;
    w.WriteStartObject();
    w.WriteString("name", "Robert");
    w.WriteNumber("age", 42);
    w.WriteBoolean("active", true);
    w.WriteNull("nothing");
    w.WriteEndObject();
    EXPECT_EQ(w.GetString(), R"({"name":"Robert","age":42,"active":true,"nothing":null})");
}

// Regression tests for a wave-3 audit finding: WriteNumberValue(double) silently serialized
// NaN/+-Infinity as the JSON literal "null" (nlohmann::json's documented behavior for
// non-finite doubles) instead of throwing. Verified against JsonWriterHelper.cs's
// ValidateDouble, which throws ArgumentException for any non-finite double, since JSON has no
// representation for them.
TEST(Utf8JsonWriterTests, WriteNumberValueDouble_NaN_ThrowsArgumentException) {
    Utf8JsonWriter w;
    EXPECT_THROW(w.WriteNumberValue(std::numeric_limits<double>::quiet_NaN()), System::ArgumentException);
}

TEST(Utf8JsonWriterTests, WriteNumberValueDouble_PositiveInfinity_ThrowsArgumentException) {
    Utf8JsonWriter w;
    EXPECT_THROW(w.WriteNumberValue(std::numeric_limits<double>::infinity()), System::ArgumentException);
}

TEST(Utf8JsonWriterTests, WriteNumberValueDouble_NegativeInfinity_ThrowsArgumentException) {
    Utf8JsonWriter w;
    EXPECT_THROW(w.WriteNumberValue(-std::numeric_limits<double>::infinity()), System::ArgumentException);
}

TEST(Utf8JsonWriterTests, WriteNumberDouble_NaN_ThrowsBeforeWritingPropertyName) {
    Utf8JsonWriter w;
    w.WriteStartObject();
    EXPECT_THROW(w.WriteNumber("x", std::numeric_limits<double>::quiet_NaN()), System::ArgumentException);
    w.WriteEndObject();
    // No dangling "x" property name should have been written before the exception.
    EXPECT_EQ(w.GetString(), "{}");
}

TEST(Utf8JsonWriterTests, WriteArrayOfNumbers) {
    Utf8JsonWriter w;
    w.WriteStartArray();
    w.WriteNumberValue(1);
    w.WriteNumberValue(2);
    w.WriteNumberValue(3);
    w.WriteEndArray();
    EXPECT_EQ(w.GetString(), "[1,2,3]");
}

TEST(Utf8JsonWriterTests, WriteNestedObjectAndArray) {
    Utf8JsonWriter w;
    w.WriteStartObject();
    w.WriteStartArray("tags");
    w.WriteStringValue("a");
    w.WriteStringValue("b");
    w.WriteEndArray();
    w.WriteStartObject("nested");
    w.WriteNumber("x", 1);
    w.WriteEndObject();
    w.WriteEndObject();
    EXPECT_EQ(w.GetString(), R"({"tags":["a","b"],"nested":{"x":1}})");
}

TEST(Utf8JsonWriterTests, WriteRootScalar) {
    Utf8JsonWriter w;
    w.WriteNumberValue(42);
    EXPECT_EQ(w.GetString(), "42");
}

TEST(Utf8JsonWriterTests, WriteStringEscaping) {
    Utf8JsonWriter w;
    w.WriteStringValue("line1\nline2\t\"quoted\"\\backslash");
    EXPECT_EQ(w.GetString(), R"("line1\nline2\t\"quoted\"\\backslash")");
}

TEST(Utf8JsonWriterTests, WriteHtmlSensitiveCharsEscaped) {
    Utf8JsonWriter w;
    w.WriteStringValue("<a>&'b'</a>");
    EXPECT_EQ(w.GetString(), "\"\\u003ca\\u003e\\u0026\\u0027b\\u0027\\u003c/a\\u003e\"");
}

TEST(Utf8JsonWriterTests, Indented_ProducesPrettyPrintedJson) {
    JsonWriterOptions options;
    options.Indented = true;
    Utf8JsonWriter w(options);
    w.WriteStartObject();
    w.WriteNumber("x", 1);
    w.WriteEndObject();
    EXPECT_EQ(w.GetString(), "{\n  \"x\": 1\n}");
}

TEST(Utf8JsonWriterTests, WriteValueWithoutPropertyNameInsideObject_Throws) {
    Utf8JsonWriter w;
    w.WriteStartObject();
    EXPECT_THROW(w.WriteStringValue("oops"), System::InvalidOperationException);
}

TEST(Utf8JsonWriterTests, WritePropertyNameInsideArray_Throws) {
    Utf8JsonWriter w;
    w.WriteStartArray();
    EXPECT_THROW(w.WritePropertyName("oops"), System::InvalidOperationException);
}

TEST(Utf8JsonWriterTests, WriteEndObjectOnArray_Throws) {
    Utf8JsonWriter w;
    w.WriteStartArray();
    EXPECT_THROW(w.WriteEndObject(), System::InvalidOperationException);
}

TEST(Utf8JsonWriterTests, SkipValidation_AllowsStructurallyInvalidWrites) {
    JsonWriterOptions options;
    options.SkipValidation = true;
    Utf8JsonWriter w(options);
    w.WriteStartObject();
    EXPECT_NO_THROW(w.WriteStringValue("oops"));
}

TEST(Utf8JsonWriterTests, Reset_ClearsBufferAndState) {
    Utf8JsonWriter w;
    w.WriteStartObject();
    w.WriteEndObject();
    w.Reset();
    EXPECT_EQ(w.GetString(), "");
    EXPECT_EQ(w.getCurrentDepthProperty(), 0);
    w.WriteNumberValue(1);
    EXPECT_EQ(w.GetString(), "1");
}

TEST(Utf8JsonWriterTests, WriteRawValue_InsertsVerbatimJson) {
    Utf8JsonWriter w;
    w.WriteStartArray();
    w.WriteRawValue("42");
    w.WriteRawValue(R"({"a":1})");
    w.WriteEndArray();
    EXPECT_EQ(w.GetString(), R"([42,{"a":1}])");
}

TEST(Utf8JsonWriterTests, WriteRawValue_InvalidJson_Throws) {
    Utf8JsonWriter w;
    EXPECT_THROW(w.WriteRawValue("not json"), std::exception);
}

TEST(Utf8JsonWriterTests, CurrentDepth_TracksNesting) {
    Utf8JsonWriter w;
    EXPECT_EQ(w.getCurrentDepthProperty(), 0);
    w.WriteStartObject();
    EXPECT_EQ(w.getCurrentDepthProperty(), 1);
    w.WriteStartArray("a");
    EXPECT_EQ(w.getCurrentDepthProperty(), 2);
    w.WriteEndArray();
    w.WriteEndObject();
    EXPECT_EQ(w.getCurrentDepthProperty(), 0);
}

TEST(Utf8JsonWriterTests, WriteNonAsciiString_EscapedAsUnicodeSequences) {
    Utf8JsonWriter w;
    w.WriteStringValue("caf\xC3\xA9");  // "café"
    EXPECT_EQ(w.GetString(), "\"caf\\u00e9\"");
}

TEST(Utf8JsonWriterTests, WriteAstralCharacter_EscapedAsSurrogatePair) {
    Utf8JsonWriter w;
    w.WriteStringValue("\xF0\x9F\x98\x80");  // U+1F600 GRINNING FACE
    EXPECT_EQ(w.GetString(), "\"\\ud83d\\ude00\"");
}

TEST(Utf8JsonWriterTests, WritePlusBacktickDel_Escaped) {
    Utf8JsonWriter w;
    w.WriteStringValue("a+b`c\x7F" "d");
    EXPECT_EQ(w.GetString(), "\"a\\u002bb\\u0060c\\u007fd\"");
}

TEST(Utf8JsonWriterTests, DefaultMaxDepth_Is1000) {
    Utf8JsonWriter w;
    EXPECT_EQ(w.getOptionsProperty().MaxDepth, 1000);
}

TEST(Utf8JsonWriterTests, ExceedingDefaultMaxDepth_Throws) {
    Utf8JsonWriter w;
    for (int i = 0; i < 1000; ++i)
        w.WriteStartArray();
    EXPECT_THROW(w.WriteStartArray(), System::InvalidOperationException);
}

TEST(Utf8JsonWriterTests, ExceedingCustomMaxDepth_Throws) {
    JsonWriterOptions options;
    options.MaxDepth = 3;
    Utf8JsonWriter w(options);
    w.WriteStartArray();
    w.WriteStartArray();
    w.WriteStartArray();
    EXPECT_THROW(w.WriteStartArray(), System::InvalidOperationException);
}

TEST(Utf8JsonWriterTests, WrittenJsonRoundTripsThroughJsonDocument) {
    Utf8JsonWriter w;
    w.WriteStartObject();
    w.WriteString("name", "Bob");
    w.WriteNumber("age", 25);
    w.WriteEndObject();
    auto doc = JsonDocument::Parse(w.GetString());
    auto root = doc->getRootElementProperty();
    EXPECT_EQ(root.GetProperty("name").GetString(), "Bob");
    EXPECT_EQ(root.GetProperty("age").GetInt32(), 25);
}
