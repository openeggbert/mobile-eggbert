// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Collections/Specialized/NameValueCollection.hpp"
#include "System/Net/HttpRequestHeader.hpp"
#include "System/Net/HttpResponseHeader.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::intcs;
    using SharpRuntime::bytecs;

    /**
     * @brief Contains protocol headers associated with a request or response.
     *
     * C++ counterpart of .NET System.Net.WebHeaderCollection. Composes (rather than
     * inherits) System::Collections::Specialized::NameValueCollection, since that type
     * has no virtual override points to hook into.
     *
     * @note GetValues() returns the raw stored values rather than re-splitting comma-separated
     * multi-value headers per .NET's internal per-header HeaderInfoTable parser table
     * (e.g. Set-Cookie's special quote-aware comma splitting) — that table is a large,
     * request/response-plumbing-specific internal data structure not reproduced here.
     * IsRestricted() reproduces the table's IsRequestRestricted/IsResponseRestricted flags,
     * which are the practically useful part for a standalone header bag.
     */
    class WebHeaderCollection {
        enum class CollectionType { Unknown, WebRequestKind, WebResponseKind };

        System::Collections::Specialized::NameValueCollection inner_;
        mutable CollectionType type_ = CollectionType::Unknown;

        [[nodiscard]] bool allowHttpRequestHeader() const;
        [[nodiscard]] bool allowHttpResponseHeader() const;

    public:
        WebHeaderCollection() = default;

        /** Adds a header from name and value, validating both. */
        void Add(const std::string& name, const std::string& value);
        /** Adds a header from a combined "Name: Value" string. */
        void Add(const std::string& combinedHeader);
        /** Adds a well-known request header. @throws System::InvalidOperationException if this collection is locked to response headers. */
        void Add(HttpRequestHeader header, const std::string& value);
        /** Adds a well-known response header. @throws System::InvalidOperationException if this collection is locked to request headers. */
        void Add(HttpResponseHeader header, const std::string& value);

        /** Sets a header to a single value, replacing any existing values, validating both. */
        void Set(const std::string& name, const std::string& value);
        /** Sets a well-known request header. */
        void Set(HttpRequestHeader header, const std::string& value);
        /** Sets a well-known response header. */
        void Set(HttpResponseHeader header, const std::string& value);

        /** Removes the named header. */
        void Remove(const std::string& name);
        /** Removes a well-known request header. */
        void Remove(HttpRequestHeader header);
        /** Removes a well-known response header. */
        void Remove(HttpResponseHeader header);

        /** @return All values for @p name as a comma-joined string, or "" if not found. */
        [[nodiscard]] std::string Get(const std::string& name) const { return inner_.Get(name); }
        /** @return All values for the key at @p index as a comma-joined string. */
        [[nodiscard]] std::string Get(intcs index) const { return inner_.Get(index); }
        /** @return The raw stored values for @p name (see class-level note on multi-value splitting). */
        [[nodiscard]] std::vector<std::string> GetValues(const std::string& name) const { return inner_.GetValues(name); }
        /** @return The raw stored values for the key at @p index. */
        [[nodiscard]] std::vector<std::string> GetValues(intcs index) const { return inner_.GetValues(index); }
        /** @return The key at the given zero-based index. */
        [[nodiscard]] const std::string& GetKey(intcs index) const { return inner_.GetKey(index); }

        /** @return The value for the given well-known request header. @throws System::InvalidOperationException if locked to response headers. */
        [[nodiscard]] std::string Get(HttpRequestHeader header) const;
        /** @return The value for the given well-known response header. @throws System::InvalidOperationException if locked to request headers. */
        [[nodiscard]] std::string Get(HttpResponseHeader header) const;

        /** @return The number of unique header names. */
        [[nodiscard]] intcs getCountProperty() const { return inner_.getCountProperty(); }
        /** @return All header names in insertion order. */
        [[nodiscard]] const std::vector<std::string>& getAllKeysProperty() const { return inner_.AllKeys(); }

        /** Removes all headers. */
        void Clear() { inner_.Clear(); }

        /** @return "Name: Value\r\n" for each header, followed by a final "\r\n". */
        [[nodiscard]] std::string ToString() const;
        /** @return ToString(), encoded as ASCII bytes. */
        [[nodiscard]] std::vector<bytecs> ToByteArray() const;

        /** @return true if @p headerName is a request-restricted (or, if @p response, response-restricted) header. */
        [[nodiscard]] static bool IsRestricted(const std::string& headerName, bool response = false);

        /** Throws System::ArgumentException if @p name contains characters not permitted in an HTTP header name. */
        static std::string CheckBadHeaderNameChars(const std::string& name);
        /** Throws System::ArgumentException if @p value contains characters not permitted in an HTTP header value. */
        static std::string CheckBadHeaderValueChars(const std::string& value);
    };

} // namespace System::Net
