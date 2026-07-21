// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Json/HttpContentJsonExtensions.hpp"
#include "System/Net/Http/Json/JsonContent.hpp"

using namespace System::Net::Http;
using namespace System::Net::Http::Json;

TEST(HttpContentJsonExtensionsTests, ReadFromJson_ParsesContent) {
    auto content = std::make_shared<JsonContent>("{\"answer\":42}");
    auto doc = HttpContentJsonExtensions::ReadFromJson(content);
    ASSERT_TRUE(doc != nullptr);
    EXPECT_EQ(doc->getRootElementProperty().GetProperty("answer").GetInt32(), 42);
}

TEST(HttpContentJsonExtensionsTests, ReadFromJsonAsync_ParsesContent) {
    auto content = std::make_shared<JsonContent>("{\"ok\":true}");
    auto task = HttpContentJsonExtensions::ReadFromJsonAsync(content);
    auto doc = task.getResultProperty();
    ASSERT_TRUE(doc != nullptr);
    EXPECT_TRUE(doc->getRootElementProperty().GetProperty("ok").GetBoolean());
}
