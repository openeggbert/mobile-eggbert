// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>

namespace System::Net::Http::Json {

    /**
     * @brief HTTP content backed by pre-serialized JSON.
     *
     * C++ counterpart of .NET System.Net.Http.Json.JsonContent.
     *
     * @note .NET's `Create<T>(T, ...)`/`Create(object, Type, ...)` overloads serialize an
     * arbitrary reflectable type via a `JsonTypeInfo`; this runtime has no reflection and
     * `System::Text::Json::JsonSerializer::Serialize<T>()` is an intentional stub (see its class
     * doc note), so those overloads aren't reproduced. Instead, JsonContent is constructed either
     * directly from a pre-serialized JSON string, or via Create() from an `nlohmann::json` value
     * (this runtime's vendored JSON library) — the caller does the T-to-JSON conversion.
     * @note The constructor is public here (unlike .NET's private constructor + Create-only
     * factory pattern), since there's no `JsonTypeInfo` invariant to protect.
     */
    class JsonContent : public System::Net::Http::HttpContent {
        std::string json_;
        std::string mediaType_;
        std::string charset_;

    public:
        /**
         * @brief Constructs JsonContent from an already-serialized JSON string.
         * @param json      The serialized JSON text.
         * @param mediaType MIME type; defaults to "application/json".
         * @param charset   Character encoding; defaults to "utf-8".
         */
        explicit JsonContent(const std::string& json,
                              const std::string& mediaType = "application/json",
                              const std::string& charset = "utf-8")
            : json_(json), mediaType_(mediaType), charset_(charset) {}

        /**
         * @brief Creates JsonContent by serializing @p value.
         * @param value     The JSON value to serialize (`value.dump()`).
         * @param mediaType MIME type; defaults to "application/json".
         * @param charset   Character encoding; defaults to "utf-8".
         */
        static JsonContent Create(const nlohmann::json& value,
                                   const std::string& mediaType = "application/json",
                                   const std::string& charset = "utf-8") {
            return JsonContent(value.dump(), mediaType, charset);
        }

        /** Returns the serialized JSON content body as a string. */
        [[nodiscard]] std::string ReadAsString() const override { return json_; }

        /** Returns the serialized JSON content body as raw bytes (UTF-8 encoding assumed). */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> ReadAsByteArray() const override {
            return std::vector<SharpRuntime::bytecs>(json_.begin(), json_.end());
        }

        /** Returns the MIME type of the content (e.g. "application/json"). */
        [[nodiscard]] std::string getContentTypeProperty() const override { return mediaType_; }
        /** Returns the character set of the content (e.g. "utf-8"). */
        [[nodiscard]] std::string getCharSetProperty() const override { return charset_; }
    };

} // namespace System::Net::Http::Json
