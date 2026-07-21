// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http {

/**
 * @brief Abstract base class for HTTP request/response body content.
 *
 * C++ counterpart of .NET System.Net.Http.HttpContent, deliberately reduced in scope to match
 * this runtime's synchronous, no-Stream content model (see CLAUDE.md / NEXT.md: "no rewrite of
 * System.Net.Http's synchronous content model to a Stream/Task-based one", an established
 * prior-session decision, not an oversight). Real .NET's HttpContent centers on a `Headers`
 * property (HttpContentHeaders) and Stream-based SerializeToStream(Async)/CopyTo(Async)/
 * ReadAsStream(Async) methods; this port instead exposes only the two synchronous, fully-buffered
 * accessors below plus direct getContentTypeProperty()/getCharSetProperty() getters (populated at
 * construction time from constructor parameters rather than a mutable Headers collection).
 */
class HttpContent {
public:
    virtual ~HttpContent() = default;

    /** Returns the body as a UTF-8 string. */
    [[nodiscard]] virtual std::string ReadAsString() const = 0;

    /** Returns the body as raw bytes. */
    [[nodiscard]] virtual std::vector<SharpRuntime::bytecs> ReadAsByteArray() const = 0;

    /** Returns the MIME media type (e.g. "text/plain", "application/json"). */
    [[nodiscard]] virtual std::string getContentTypeProperty() const = 0;

    /** Returns the charset (e.g. "utf-8"), or empty if not applicable. */
    [[nodiscard]] virtual std::string getCharSetProperty() const { return ""; }
};

} // namespace System::Net::Http
