// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Net/Http/Headers/HttpHeaders.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Provides a view on top of an HttpHeaders collection that avoids forcing validation
     * or parsing on its contents.
     *
     * C++ counterpart of .NET System.Net.Http.Headers.HttpHeadersNonValidated.
     *
     * @note In .NET, this view surfaces raw, not-yet-parsed header strings, distinct from
     * HttpHeaders' lazily-parsed-and-cached representation. This runtime's HttpHeaders has no
     * such parsed-value cache (see its class doc note) — every access is already "non-validated"
     * in that sense — so this class is a thin, functionally-equivalent wrapper provided purely
     * for API-surface parity, not a behaviorally distinct view.
     */
    class HttpHeadersNonValidated {
        const HttpHeaders* headers_;

    public:
        /** @brief Constructs a view over @p headers. The referenced object must outlive this view. */
        explicit HttpHeadersNonValidated(const HttpHeaders& headers) : headers_(&headers) {}

        /** @return The number of distinct header names. */
        [[nodiscard]] SharpRuntime::intcs getCountProperty() const { return headers_->getCountProperty(); }

        /** @return true if @p headerName is present. */
        [[nodiscard]] bool Contains(const std::string& headerName) const { return headers_->Contains(headerName); }

        /** @brief Attempts to retrieve the values for @p headerName. */
        [[nodiscard]] bool TryGetValues(const std::string& headerName, std::vector<std::string>& values) const {
            return headers_->TryGetValues(headerName, values);
        }
    };

} // namespace System::Net::Http::Headers
