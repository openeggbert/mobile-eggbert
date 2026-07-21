// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/MultipartFormDataContent.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"

namespace System::Net::Http {

    std::string MultipartFormDataContent::buildContentDisposition(const std::string& name, const std::string* fileName) {
        std::string line = "Content-Disposition: form-data; name=\"" + name + "\"";
        if (fileName != nullptr) {
            line += "; filename=\"" + *fileName + "\"";
        }
        line += "\r\n";
        return line;
    }

    MultipartFormDataContent::MultipartFormDataContent() : MultipartContent("form-data") {}

    MultipartFormDataContent::MultipartFormDataContent(const std::string& boundary)
        : MultipartContent("form-data", boundary) {}

    void MultipartFormDataContent::Add(const std::shared_ptr<HttpContent>& content) {
        System::ArgumentNullException::ThrowIfNull(content.get(), "content");
        AddWithHeaderLine(content, "Content-Disposition: form-data\r\n");
    }

    void MultipartFormDataContent::Add(const std::shared_ptr<HttpContent>& content, const std::string& name) {
        System::ArgumentNullException::ThrowIfNull(content.get(), "content");
        System::ArgumentException::ThrowIfNullOrWhiteSpace(name, "name");
        AddWithHeaderLine(content, buildContentDisposition(name, nullptr));
    }

    void MultipartFormDataContent::Add(const std::shared_ptr<HttpContent>& content, const std::string& name, const std::string& fileName) {
        System::ArgumentNullException::ThrowIfNull(content.get(), "content");
        System::ArgumentException::ThrowIfNullOrWhiteSpace(name, "name");
        System::ArgumentException::ThrowIfNullOrWhiteSpace(fileName, "fileName");
        AddWithHeaderLine(content, buildContentDisposition(name, &fileName));
    }

} // namespace System::Net::Http
