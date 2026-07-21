// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/Text/Json/JsonDocument.hpp"
#include "System/Text/Json/JsonProperty.hpp"

using System::Text::Json::JsonDocument;
using System::Text::Json::JsonDocumentOptions;
using System::Text::Json::JsonValueKind;

// ---------------------------------------------------------------------------
// Primitives
// ---------------------------------------------------------------------------

TEST(JsonTests, ParseNullLiteral) {
    auto doc = JsonDocument::Parse("null");
    EXPECT_EQ(doc->getRootElementProperty().getValueKindProperty(), JsonValueKind::Null);
}

TEST(JsonTests, ParseTrueLiteral) {
    auto doc = JsonDocument::Parse("true");
    EXPECT_EQ(doc->getRootElementProperty().getValueKindProperty(), JsonValueKind::True);
    EXPECT_TRUE(doc->getRootElementProperty().GetBoolean());
}

TEST(JsonTests, ParseFalseLiteral) {
    auto doc = JsonDocument::Parse("false");
    EXPECT_EQ(doc->getRootElementProperty().getValueKindProperty(), JsonValueKind::False);
    EXPECT_FALSE(doc->getRootElementProperty().GetBoolean());
}

TEST(JsonTests, ParseIntegerNumber) {
    auto doc = JsonDocument::Parse("42");
    EXPECT_EQ(doc->getRootElementProperty().getValueKindProperty(), JsonValueKind::Number);
    EXPECT_EQ(doc->getRootElementProperty().GetInt32(), 42);
}

TEST(JsonTests, ParseNegativeNumber) {
    auto doc = JsonDocument::Parse("-7");
    EXPECT_EQ(doc->getRootElementProperty().GetInt32(), -7);
}

TEST(JsonTests, ParseDouble) {
    auto doc = JsonDocument::Parse("3.14");
    EXPECT_NEAR(doc->getRootElementProperty().GetDouble(), 3.14, 1e-9);
}

TEST(JsonTests, ParseString) {
    auto doc = JsonDocument::Parse("\"hello\"");
    EXPECT_EQ(doc->getRootElementProperty().getValueKindProperty(), JsonValueKind::String);
    EXPECT_EQ(doc->getRootElementProperty().GetString(), "hello");
}

TEST(JsonTests, ParseStringWithEscape) {
    auto doc = JsonDocument::Parse("\"hello\\nworld\"");
    EXPECT_EQ(doc->getRootElementProperty().GetString(), "hello\nworld");
}

// ---------------------------------------------------------------------------
// Objects
// ---------------------------------------------------------------------------

TEST(JsonTests, ParseEmptyObject) {
    auto doc = JsonDocument::Parse("{}");
    EXPECT_EQ(doc->getRootElementProperty().getValueKindProperty(), JsonValueKind::Object);
}

TEST(JsonTests, ParseObjectStringProperty) {
    auto doc = JsonDocument::Parse(R"({"name":"Alice"})");
    auto name = doc->getRootElementProperty().GetProperty("name");
    EXPECT_EQ(name.getValueKindProperty(), JsonValueKind::String);
    EXPECT_EQ(name.GetString(), "Alice");
}

TEST(JsonTests, ParseObjectNumberProperty) {
    auto doc = JsonDocument::Parse(R"({"age":30})");
    EXPECT_EQ(doc->getRootElementProperty().GetProperty("age").GetInt32(), 30);
}

TEST(JsonTests, ParseObjectBoolProperty) {
    auto doc = JsonDocument::Parse(R"({"active":true})");
    EXPECT_TRUE(doc->getRootElementProperty().GetProperty("active").GetBoolean());
}

TEST(JsonTests, ParseObjectNullProperty) {
    auto doc = JsonDocument::Parse(R"({"data":null})");
    EXPECT_EQ(doc->getRootElementProperty().GetProperty("data").getValueKindProperty(),
              JsonValueKind::Null);
}

TEST(JsonTests, ParseObjectMultipleProperties) {
    auto doc = JsonDocument::Parse(R"({"x":1,"y":2,"z":3})");
    auto root = doc->getRootElementProperty();
    EXPECT_EQ(root.GetProperty("x").GetInt32(), 1);
    EXPECT_EQ(root.GetProperty("y").GetInt32(), 2);
    EXPECT_EQ(root.GetProperty("z").GetInt32(), 3);
}

TEST(JsonTests, EnumerateObject_PreservesDocumentOrder) {
    auto doc = JsonDocument::Parse(R"({"z":1,"a":2,"m":3})");
    std::vector<std::string> names;
    for (auto& p : doc->getRootElementProperty().EnumerateObject()) names.push_back(p.getNameProperty());
    ASSERT_EQ(names.size(), 3u);
    EXPECT_EQ(names[0], "z");
    EXPECT_EQ(names[1], "a");
    EXPECT_EQ(names[2], "m");
}

TEST(JsonTests, TryGetPropertyFound) {
    auto doc = JsonDocument::Parse(R"({"key":"value"})");
    System::Text::Json::JsonElement out;
    EXPECT_TRUE(doc->getRootElementProperty().TryGetProperty("key", out));
    EXPECT_EQ(out.GetString(), "value");
}

TEST(JsonTests, TryGetPropertyNotFound) {
    auto doc = JsonDocument::Parse(R"({"key":"value"})");
    System::Text::Json::JsonElement out;
    EXPECT_FALSE(doc->getRootElementProperty().TryGetProperty("missing", out));
}

TEST(JsonTests, GetPropertyMissingThrows) {
    auto doc = JsonDocument::Parse(R"({})");
    EXPECT_THROW(doc->getRootElementProperty().GetProperty("nope"), std::exception);
}

TEST(JsonTests, ParseNestedObject) {
    auto doc = JsonDocument::Parse(R"({"outer":{"inner":99}})");
    auto inner = doc->getRootElementProperty().GetProperty("outer").GetProperty("inner");
    EXPECT_EQ(inner.GetInt32(), 99);
}

// ---------------------------------------------------------------------------
// Arrays
// ---------------------------------------------------------------------------

TEST(JsonTests, ParseEmptyArray) {
    auto doc = JsonDocument::Parse("[]");
    EXPECT_EQ(doc->getRootElementProperty().getValueKindProperty(), JsonValueKind::Array);
    EXPECT_TRUE(doc->getRootElementProperty().EnumerateArray().empty());
}

TEST(JsonTests, ParseIntegerArray) {
    auto doc = JsonDocument::Parse("[1,2,3]");
    auto items = doc->getRootElementProperty().EnumerateArray();
    ASSERT_EQ(static_cast<int>(items.size()), 3);
    EXPECT_EQ(items[0].GetInt32(), 1);
    EXPECT_EQ(items[1].GetInt32(), 2);
    EXPECT_EQ(items[2].GetInt32(), 3);
}

TEST(JsonTests, ParseStringArray) {
    auto doc = JsonDocument::Parse(R"(["a","b","c"])");
    auto items = doc->getRootElementProperty().EnumerateArray();
    ASSERT_EQ(static_cast<int>(items.size()), 3);
    EXPECT_EQ(items[0].GetString(), "a");
    EXPECT_EQ(items[2].GetString(), "c");
}

TEST(JsonTests, ParseMixedArray) {
    auto doc = JsonDocument::Parse(R"([1,true,"x",null])");
    auto items = doc->getRootElementProperty().EnumerateArray();
    ASSERT_EQ(static_cast<int>(items.size()), 4);
    EXPECT_EQ(items[0].getValueKindProperty(), JsonValueKind::Number);
    EXPECT_EQ(items[1].getValueKindProperty(), JsonValueKind::True);
    EXPECT_EQ(items[2].getValueKindProperty(), JsonValueKind::String);
    EXPECT_EQ(items[3].getValueKindProperty(), JsonValueKind::Null);
}

TEST(JsonTests, ParseArrayOfObjects) {
    auto doc = JsonDocument::Parse(R"([{"id":1},{"id":2}])");
    auto items = doc->getRootElementProperty().EnumerateArray();
    ASSERT_EQ(static_cast<int>(items.size()), 2);
    EXPECT_EQ(items[0].GetProperty("id").GetInt32(), 1);
    EXPECT_EQ(items[1].GetProperty("id").GetInt32(), 2);
}

// ---------------------------------------------------------------------------
// Complex document
// ---------------------------------------------------------------------------

TEST(JsonTests, ParseRealWorldDocument) {
    const std::string json = R"({
        "name": "Bob",
        "age": 25,
        "active": true,
        "score": 9.5,
        "tags": ["cpp", "dotnet"],
        "address": null
    })";
    auto doc = JsonDocument::Parse(json);
    auto root = doc->getRootElementProperty();
    EXPECT_EQ(root.GetProperty("name").GetString(), "Bob");
    EXPECT_EQ(root.GetProperty("age").GetInt32(), 25);
    EXPECT_TRUE(root.GetProperty("active").GetBoolean());
    EXPECT_NEAR(root.GetProperty("score").GetDouble(), 9.5, 1e-9);
    EXPECT_EQ(root.GetProperty("tags").EnumerateArray().size(), 2u);
    EXPECT_EQ(root.GetProperty("address").getValueKindProperty(), JsonValueKind::Null);
}

// ---------------------------------------------------------------------------
// Error handling
// ---------------------------------------------------------------------------

TEST(JsonTests, ParseInvalidJsonThrows) {
    EXPECT_THROW(JsonDocument::Parse("{bad json}"), std::exception);
}

TEST(JsonTests, ParseEmptyStringThrows) {
    EXPECT_THROW(JsonDocument::Parse(""), std::exception);
}

// ---------------------------------------------------------------------------
// Dispose
// ---------------------------------------------------------------------------

TEST(JsonTests, DisposeInvalidatesRoot) {
    auto doc = JsonDocument::Parse("{}");
    doc->Dispose();
    EXPECT_THROW(doc->getRootElementProperty(), std::exception);
}

TEST(JsonTests, ParseValueSameAsParse) {
    auto d1 = JsonDocument::Parse(R"({"k":1})");
    auto d2 = JsonDocument::ParseValue(R"({"k":1})");
    EXPECT_EQ(d1->getRootElementProperty().GetProperty("k").GetInt32(),
              d2->getRootElementProperty().GetProperty("k").GetInt32());
}

// ---------------------------------------------------------------------------
// MaxDepth enforcement
// ---------------------------------------------------------------------------

static std::string makeNestedArrayJson(int depth) {
    std::string json;
    for (int i = 0; i < depth; ++i) json += "[";
    json += "0";
    for (int i = 0; i < depth; ++i) json += "]";
    return json;
}

TEST(JsonTests, ParseWithinDefaultMaxDepth_Succeeds) {
    EXPECT_NO_THROW(JsonDocument::Parse(makeNestedArrayJson(64)));
}

TEST(JsonTests, ParseExceedingDefaultMaxDepth_Throws) {
    EXPECT_THROW(JsonDocument::Parse(makeNestedArrayJson(65)), std::exception);
}

TEST(JsonTests, ParseExceedingCustomMaxDepth_Throws) {
    JsonDocumentOptions options;
    options.MaxDepth = 3;
    EXPECT_THROW(JsonDocument::Parse(makeNestedArrayJson(4), options), std::exception);
}

TEST(JsonTests, ParseWithinCustomMaxDepth_Succeeds) {
    JsonDocumentOptions options;
    options.MaxDepth = 3;
    EXPECT_NO_THROW(JsonDocument::Parse(makeNestedArrayJson(3), options));
}
