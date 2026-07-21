// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/MultipartContent.hpp"
#include <memory>
#include <string>

namespace System::Net::Http {

    /**
     * @brief Provides a container for content encoded using the multipart/form-data MIME type.
     *
     * C++ counterpart of .NET System.Net.Http.MultipartFormDataContent.
     *
     * @note Each part's Content-Disposition line here is
     * `form-data; name="{name}"[; filename="{fileName}"]` — .NET additionally emits an RFC 5987
     * `filename*=UTF-8''...` parameter for non-ASCII file names; that's not reproduced since
     * ContentDispositionHeaderValue (which would normally own this formatting) isn't ported.
     */
    class MultipartFormDataContent : public MultipartContent {
        static std::string buildContentDisposition(const std::string& name, const std::string* fileName);

    public:
        /** Constructs a multipart/form-data container with a randomly generated boundary. */
        MultipartFormDataContent();
        /** Constructs a multipart/form-data container with the given boundary. */
        explicit MultipartFormDataContent(const std::string& boundary);

        /** Appends @p content with a default "form-data" Content-Disposition (no name). */
        void Add(const std::shared_ptr<HttpContent>& content) override;

        /** Appends @p content with Content-Disposition: form-data; name="{name}". */
        void Add(const std::shared_ptr<HttpContent>& content, const std::string& name);

        /** Appends @p content with Content-Disposition: form-data; name="{name}"; filename="{fileName}". */
        void Add(const std::shared_ptr<HttpContent>& content, const std::string& name, const std::string& fileName);
    };

} // namespace System::Net::Http
