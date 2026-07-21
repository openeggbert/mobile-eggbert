// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Mime/MediaTypeNames.hpp"

using System::Net::Mime::MediaTypeNames;

TEST(MediaTypeNamesTests, Application_Json) {
    EXPECT_STREQ(MediaTypeNames::Application::Json, "application/json");
}

TEST(MediaTypeNamesTests, Application_Octet) {
    EXPECT_STREQ(MediaTypeNames::Application::Octet, "application/octet-stream");
}

TEST(MediaTypeNamesTests, Text_Plain) {
    EXPECT_STREQ(MediaTypeNames::Text::Plain, "text/plain");
}

TEST(MediaTypeNamesTests, Text_Html) {
    EXPECT_STREQ(MediaTypeNames::Text::Html, "text/html");
}

TEST(MediaTypeNamesTests, Image_Png) {
    EXPECT_STREQ(MediaTypeNames::Image::Png, "image/png");
}

TEST(MediaTypeNamesTests, Multipart_FormData) {
    EXPECT_STREQ(MediaTypeNames::Multipart::FormData, "multipart/form-data");
}

TEST(MediaTypeNamesTests, Font_Woff2) {
    EXPECT_STREQ(MediaTypeNames::Font::Woff2, "font/woff2");
}

TEST(MediaTypeNamesTests, Video_Mp4) {
    EXPECT_STREQ(MediaTypeNames::Video::Mp4, "video/mp4");
}
