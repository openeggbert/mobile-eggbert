// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpClient.hpp"
#include "System/Net/Http/Json/JsonContent.hpp"
#include "System/Text/Json/JsonDocument.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "nlohmann/json.hpp"
#include <memory>
#include <string>

namespace System::Net::Http::Json {

    /**
     * @brief Extension-like static methods for sending/receiving JSON via HttpClient.
     *
     * C++ counterpart of .NET System.Net.Http.Json.HttpClientJsonExtensions.
     *
     * @note As with HttpContentJsonExtensions, .NET's generic `GetFromJsonAsync<T>`/
     * `PostAsJsonAsync<T>`/etc. overloads require reflection-based serialization this runtime
     * doesn't have. `GetFromJsonAsync`/`DeleteFromJsonAsync` return a parsed
     * `System::Text::Json::JsonDocument` instead of a `T`; the `*AsJsonAsync` senders take an
     * `nlohmann::json` value (serialized via JsonContent::Create) instead of an arbitrary `T`.
     * @note Unlike .NET (where a GC-tracked `HttpClient` reference captured by an async
     * continuation keeps the object alive automatically), every method here captures @p client
     * by reference into a `TaskT` that runs its lambda on a real background thread
     * (`std::async(std::launch::async, ...)`, see Task.hpp). The caller must keep @p client alive
     * until the returned task completes (e.g. via `getResultProperty()`/`Wait()`) — letting
     * @p client go out of scope while the task is still in flight is a dangling-reference bug
     * with no runtime safety net in C++. This mirrors real .NET's own "don't dispose HttpClient
     * while requests are in flight" contract, just enforced by convention instead of the CLR.
     */
    struct HttpClientJsonExtensions {
        HttpClientJsonExtensions() = delete;

        /** @brief Sends a GET request to @p requestUri and parses the response body as JSON. */
        static System::Threading::Tasks::TaskT<std::shared_ptr<System::Text::Json::JsonDocument>>
        GetFromJsonAsync(HttpClient& client, const std::string& requestUri) {
            return System::Threading::Tasks::TaskT<std::shared_ptr<System::Text::Json::JsonDocument>>(
                [&client, requestUri]() {
                    auto response = client.Get(requestUri);
                    response->EnsureSuccessStatusCode();
                    auto content = response->getContentProperty();
                    return System::Text::Json::JsonDocument::Parse(content ? content->ReadAsString() : "null");
                });
        }

        /** @brief Sends @p value as JSON in a POST request to @p requestUri. */
        static System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>
        PostAsJsonAsync(HttpClient& client, const std::string& requestUri, const nlohmann::json& value) {
            return System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>(
                [&client, requestUri, value]() {
                    auto content = std::make_shared<JsonContent>(JsonContent::Create(value));
                    return client.Post(requestUri, content);
                });
        }

        /** @brief Sends @p value as JSON in a PUT request to @p requestUri. */
        static System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>
        PutAsJsonAsync(HttpClient& client, const std::string& requestUri, const nlohmann::json& value) {
            return System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>(
                [&client, requestUri, value]() {
                    auto request = std::make_shared<HttpRequestMessage>(HttpMethod::Put(), requestUri);
                    request->setContentProperty(std::make_shared<JsonContent>(JsonContent::Create(value)));
                    return client.Send(request);
                });
        }

        /** @brief Sends @p value as JSON in a PATCH request to @p requestUri. */
        static System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>
        PatchAsJsonAsync(HttpClient& client, const std::string& requestUri, const nlohmann::json& value) {
            return System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>(
                [&client, requestUri, value]() {
                    auto request = std::make_shared<HttpRequestMessage>(HttpMethod::Patch(), requestUri);
                    request->setContentProperty(std::make_shared<JsonContent>(JsonContent::Create(value)));
                    return client.Send(request);
                });
        }

        /** @brief Sends a DELETE request to @p requestUri and parses the response body as JSON. */
        static System::Threading::Tasks::TaskT<std::shared_ptr<System::Text::Json::JsonDocument>>
        DeleteFromJsonAsync(HttpClient& client, const std::string& requestUri) {
            return System::Threading::Tasks::TaskT<std::shared_ptr<System::Text::Json::JsonDocument>>(
                [&client, requestUri]() {
                    auto request = std::make_shared<HttpRequestMessage>(HttpMethod::Delete_(), requestUri);
                    auto response = client.Send(request);
                    response->EnsureSuccessStatusCode();
                    auto content = response->getContentProperty();
                    return System::Text::Json::JsonDocument::Parse(content ? content->ReadAsString() : "null");
                });
        }
    };

} // namespace System::Net::Http::Json
