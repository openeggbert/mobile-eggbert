// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "System/Net/CookieContainer.hpp"
#include "System/Net/Http/HttpMessageHandler.hpp"

namespace System::Net::Http {

    /**
     * @brief The default, socket-level HttpMessageHandler used by HttpClient.
     *
     * C++ counterpart of .NET System.Net.Http.HttpClientHandler. This is the terminal handler
     * that actually performs the HTTP/1.1 request/response exchange over a TCP socket (the
     * logic previously lived directly inside HttpClient::Send; it now lives here so that
     * HttpClient can be pointed at a custom HttpMessageHandler/DelegatingHandler chain
     * instead). Supports HTTP only -- HTTPS/TLS is out of scope for this runtime (see
     * CLAUDE.md's documented deviations).
     */
    class HttpClientHandler : public HttpMessageHandler {
    public:
        HttpClientHandler();
        ~HttpClientHandler() override;

        HttpClientHandler(const HttpClientHandler&) = delete;
        HttpClientHandler& operator=(const HttpClientHandler&) = delete;

        std::shared_ptr<HttpResponseMessage> Send(std::shared_ptr<HttpRequestMessage> request) override;

        /**
         * @brief Gets or sets whether the handler sends a Cookie request header (built from
         * getCookieContainerProperty()) and captures Set-Cookie response headers into it.
         * Defaults to true, matching real .NET.
         */
        [[nodiscard]] bool getUseCookiesProperty() const { return useCookies_; }
        void setUseCookiesProperty(bool v) { useCookies_ = v; }

        /**
         * @brief Gets or sets the cookie container used to store server cookies and build the
         * Cookie request header. Defaults to a fresh, empty CookieContainer.
         */
        [[nodiscard]] const std::shared_ptr<System::Net::CookieContainer>& getCookieContainerProperty() const {
            return cookieContainer_;
        }
        void setCookieContainerProperty(std::shared_ptr<System::Net::CookieContainer> v) {
            cookieContainer_ = std::move(v);
        }

    private:
        bool useCookies_ = true;
        std::shared_ptr<System::Net::CookieContainer> cookieContainer_ =
            std::make_shared<System::Net::CookieContainer>();
    };

} // namespace System::Net::Http
