// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/MultipartContent.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Guid.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>

namespace System::Net::Http {

    namespace {
        bool isAllowedBoundaryChar(unsigned char c) {
            static constexpr const char* allowed =
                " '()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
            return c != 0 && std::strchr(allowed, static_cast<char>(c)) != nullptr;
        }
    }

    std::string MultipartContent::generateBoundary() {
        return System::Guid::NewGuid().ToString();
    }

    void MultipartContent::validateBoundary(const std::string& boundary) {
        System::ArgumentException::ThrowIfNullOrWhiteSpace(boundary, "boundary");

        if (boundary.length() > 70) {
            throw System::ArgumentOutOfRangeException("boundary", "Boundary must be 70 characters or fewer.");
        }
        if (boundary.back() == ' ') {
            throw System::ArgumentException("Boundary cannot end with a space.", "boundary");
        }
        if (!std::all_of(boundary.begin(), boundary.end(), [](char c) { return isAllowedBoundaryChar(static_cast<unsigned char>(c)); })) {
            throw System::ArgumentException("Boundary contains characters not allowed by RFC 2046.", "boundary");
        }
    }

    std::string MultipartContent::buildMediaType(const std::string& subtype, const std::string& boundary) {
        std::string quotedBoundary = boundary;
        if (quotedBoundary.empty() || quotedBoundary.front() != '"') {
            quotedBoundary = "\"" + quotedBoundary + "\"";
        }
        return "multipart/" + subtype + "; boundary=" + quotedBoundary;
    }

    MultipartContent::MultipartContent() : MultipartContent("mixed", generateBoundary()) {}

    MultipartContent::MultipartContent(const std::string& subtype) : MultipartContent(subtype, generateBoundary()) {}

    MultipartContent::MultipartContent(const std::string& subtype, const std::string& boundary) {
        System::ArgumentException::ThrowIfNullOrWhiteSpace(subtype, "subtype");
        validateBoundary(boundary);

        boundary_ = boundary;
        mediaType_ = buildMediaType(subtype, boundary);
    }

    void MultipartContent::Add(const std::shared_ptr<HttpContent>& content) {
        AddWithHeaderLine(content, "");
    }

    void MultipartContent::AddWithHeaderLine(const std::shared_ptr<HttpContent>& content, const std::string& extraHeaderLine) {
        System::ArgumentNullException::ThrowIfNull(content.get(), "content");
        parts_.push_back(Part{content, extraHeaderLine});
    }

    std::vector<std::shared_ptr<HttpContent>> MultipartContent::getContentsProperty() const {
        std::vector<std::shared_ptr<HttpContent>> result;
        result.reserve(parts_.size());
        for (const auto& part : parts_) result.push_back(part.content);
        return result;
    }

    std::vector<SharpRuntime::bytecs> MultipartContent::ReadAsByteArray() const {
        std::string s = ReadAsString();
        return std::vector<SharpRuntime::bytecs>(s.begin(), s.end());
    }

    std::string MultipartContent::ReadAsString() const {
        std::string result;
        result += "--" + boundary_ + "\r\n";

        for (size_t i = 0; i < parts_.size(); ++i) {
            if (i != 0) {
                result += "\r\n--" + boundary_ + "\r\n";
            }

            const Part& part = parts_[i];
            std::string contentType = part.content->getContentTypeProperty();
            if (!contentType.empty()) {
                result += "Content-Type: " + contentType;
                std::string charset = part.content->getCharSetProperty();
                if (!charset.empty()) {
                    result += "; charset=" + charset;
                }
                result += "\r\n";
            }
            result += part.extraHeaderLines;
            result += "\r\n";
            result += part.content->ReadAsString();
        }

        result += "\r\n--" + boundary_ + "--\r\n";
        return result;
    }

} // namespace System::Net::Http
