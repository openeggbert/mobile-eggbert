// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include "System/Text/Json/JsonDocument.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include <memory>

namespace System::Net::Http::Json {

    /**
     * @brief Extension-like static methods for reading JSON from HttpContent.
     *
     * C++ counterpart of .NET System.Net.Http.Json.HttpContentJsonExtensions.
     *
     * @note .NET's `ReadFromJsonAsync<T>`/`ReadFromJsonAsync(Type)` deserialize into an arbitrary
     * reflectable type via `JsonSerializer`; this runtime has no reflection and
     * `System::Text::Json::JsonSerializer::Serialize<T>()`/typed-Deserialize are intentional stubs
     * (see their class doc notes). Only the always-available, non-generic form is ported: it
     * parses the content into a `System::Text::Json::JsonDocument` tree (via
     * `JsonSerializer::Deserialize`, which already works), which the caller then navigates via
     * `JsonElement`. A synchronous overload is also provided (.NET does not have one; content
     * reading here is already synchronous, see HttpContent's class doc note), in addition to the
     * async form matching .NET's naming.
     */
    struct HttpContentJsonExtensions {
        HttpContentJsonExtensions() = delete;

        /** @brief Reads @p content and parses it as JSON, synchronously. */
        static std::shared_ptr<System::Text::Json::JsonDocument> ReadFromJson(const std::shared_ptr<HttpContent>& content) {
            return System::Text::Json::JsonDocument::Parse(content->ReadAsString());
        }

        /** @brief Reads @p content and parses it as JSON, asynchronously. */
        static System::Threading::Tasks::TaskT<std::shared_ptr<System::Text::Json::JsonDocument>>
        ReadFromJsonAsync(const std::shared_ptr<HttpContent>& content) {
            return System::Threading::Tasks::TaskT<std::shared_ptr<System::Text::Json::JsonDocument>>::Run(
                [content]() { return ReadFromJson(content); });
        }
    };

} // namespace System::Net::Http::Json
