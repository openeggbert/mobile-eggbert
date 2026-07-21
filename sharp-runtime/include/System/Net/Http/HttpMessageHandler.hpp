// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "System/Net/Http/HttpRequestMessage.hpp"
#include "System/Net/Http/HttpResponseMessage.hpp"

namespace System::Net::Http {

    /**
     * @brief A base type for HTTP message handlers.
     *
     * C++ counterpart of .NET System.Net.Http.HttpMessageHandler. Real .NET exposes this
     * through SendAsync(...); this port's HttpClient is synchronous by design (a documented,
     * deliberate reduced-scope decision -- see CLAUDE.md), so this base exposes the
     * synchronous Send(request) that HttpClient::Send already delegates to. HttpClient can be
     * constructed with a custom HttpMessageHandler (typically a DelegatingHandler chain
     * terminating in an HttpClientHandler) to intercept/modify requests and responses --
     * matching real .NET's `new HttpClient(handler)` extensibility pattern.
     */
    class HttpMessageHandler {
    public:
        virtual ~HttpMessageHandler() = default;

        /** @brief Sends an HTTP request and returns the response. */
        virtual std::shared_ptr<HttpResponseMessage> Send(std::shared_ptr<HttpRequestMessage> request) = 0;
    };

} // namespace System::Net::Http
