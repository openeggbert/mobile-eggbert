// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <string>
#include "System/IndexOutOfRangeException.hpp"

namespace System::Net {

    /**
     * @brief Identifies well-known HTTP request headers.
     *
     * C++ counterpart of .NET System.Net.HttpRequestHeader.
     */
    enum class HttpRequestHeader {
        CacheControl = 0,
        Connection = 1,
        Date = 2,
        KeepAlive = 3,
        Pragma = 4,
        Trailer = 5,
        TransferEncoding = 6,
        Upgrade = 7,
        Via = 8,
        Warning = 9,
        Allow = 10,
        ContentLength = 11,
        ContentType = 12,
        ContentEncoding = 13,
        ContentLanguage = 14,
        ContentLocation = 15,
        ContentMd5 = 16,
        ContentRange = 17,
        Expires = 18,
        LastModified = 19,

        Accept = 20,
        AcceptCharset = 21,
        AcceptEncoding = 22,
        AcceptLanguage = 23,
        Authorization = 24,
        Cookie = 25,
        Expect = 26,
        From = 27,
        Host = 28,
        IfMatch = 29,
        IfModifiedSince = 30,
        IfNoneMatch = 31,
        IfRange = 32,
        IfUnmodifiedSince = 33,
        MaxForwards = 34,
        ProxyAuthorization = 35,
        Referer = 36,
        Range = 37,
        Te = 38,
        Translate = 39,
        UserAgent = 40,
    };

    /**
     * @return The wire-format header name for @p header (e.g. "Cache-Control").
     *
     * C++ counterpart of .NET's internal HttpRequestHeader.GetName() extension method,
     * which indexes a raw array by `(int)header` with no bounds check.
     * @throws System::IndexOutOfRangeException if @p header is not a defined enumerator.
     */
    inline const std::string& HttpRequestHeaderGetName(HttpRequestHeader header) {
        static const std::array<std::string, 41> names = {
            "Cache-Control", "Connection", "Date", "Keep-Alive", "Pragma", "Trailer",
            "Transfer-Encoding", "Upgrade", "Via", "Warning", "Allow", "Content-Length",
            "Content-Type", "Content-Encoding", "Content-Language", "Content-Location",
            "Content-MD5", "Content-Range", "Expires", "Last-Modified", "Accept",
            "Accept-Charset", "Accept-Encoding", "Accept-Language", "Authorization",
            "Cookie", "Expect", "From", "Host", "If-Match", "If-Modified-Since",
            "If-None-Match", "If-Range", "If-Unmodified-Since", "Max-Forwards",
            "Proxy-Authorization", "Referer", "Range", "Te", "Translate", "User-Agent",
        };
        auto index = static_cast<size_t>(header);
        if (index >= names.size()) throw System::IndexOutOfRangeException();
        return names[index];
    }

} // namespace System::Net
