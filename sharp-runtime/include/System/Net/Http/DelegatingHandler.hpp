// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Net/Http/HttpMessageHandler.hpp"

namespace System::Net::Http {

    /**
     * @brief A base type for HTTP handlers that delegate to another handler, forming a chain.
     *
     * C++ counterpart of .NET System.Net.Http.DelegatingHandler. A subclass overrides Send()
     * to inspect/modify the request, call getInnerHandlerProperty()->Send(request) to continue
     * the chain, then inspect/modify the response before returning it (the classic
     * request/response middleware-pipeline pattern -- auth-header injection, logging, retry
     * policies). This base class's own Send() forwards to the inner handler unchanged, so a
     * DelegatingHandler that doesn't override Send() is a transparent passthrough.
     */
    class DelegatingHandler : public HttpMessageHandler {
    public:
        DelegatingHandler() = default;
        explicit DelegatingHandler(std::shared_ptr<HttpMessageHandler> innerHandler)
            : innerHandler_(std::move(innerHandler)) {}

        /** @brief Gets or sets the inner handler this instance forwards requests to. */
        [[nodiscard]] const std::shared_ptr<HttpMessageHandler>& getInnerHandlerProperty() const {
            return innerHandler_;
        }
        void setInnerHandlerProperty(std::shared_ptr<HttpMessageHandler> v) { innerHandler_ = std::move(v); }

        /** @brief Forwards the request to the inner handler unchanged. Override to add behavior. */
        std::shared_ptr<HttpResponseMessage> Send(std::shared_ptr<HttpRequestMessage> request) override {
            System::ArgumentNullException::ThrowIfNull(request.get(), "request");
            if (!innerHandler_)
                throw System::InvalidOperationException(
                    "The inner handler has not been assigned.");
            return innerHandler_->Send(std::move(request));
        }

    private:
        std::shared_ptr<HttpMessageHandler> innerHandler_;
    };

} // namespace System::Net::Http
