// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Json/JsonContent.hpp"

using System::Net::Http::Json::JsonContent;

TEST(JsonContentTests, Constructor_FromRawJsonString) {
    JsonContent content("{\"a\":1}");
    EXPECT_EQ(content.ReadAsString(), "{\"a\":1}");
    EXPECT_EQ(content.getContentTypeProperty(), "application/json");
    EXPECT_EQ(content.getCharSetProperty(), "utf-8");
}

TEST(JsonContentTests, Create_FromNlohmannJson) {
    nlohmann::json j;
    j["name"] = "test";
    j["count"] = 42;
    JsonContent content = JsonContent::Create(j);
    auto parsed = nlohmann::json::parse(content.ReadAsString());
    EXPECT_EQ(parsed["name"], "test");
    EXPECT_EQ(parsed["count"], 42);
    EXPECT_EQ(content.getContentTypeProperty(), "application/json");
}

TEST(JsonContentTests, ReadAsByteArray_MatchesString) {
    JsonContent content("{\"x\":true}");
    auto bytes = content.ReadAsByteArray();
    std::string fromBytes(bytes.begin(), bytes.end());
    EXPECT_EQ(fromBytes, "{\"x\":true}");
}

TEST(JsonContentTests, Create_CustomMediaTypeAndCharset) {
    nlohmann::json j = {{"a", 1}};
    JsonContent content = JsonContent::Create(j, "application/problem+json", "ascii");
    EXPECT_EQ(content.getContentTypeProperty(), "application/problem+json");
    EXPECT_EQ(content.getCharSetProperty(), "ascii");
}
