// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/ProductInfoHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"

namespace System::Net::Http::Headers {

    namespace {
        std::string trim(const std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t");
            return s.substr(start, end - start + 1);
        }

        bool isValidComment(const std::string& s) {
            if (s.empty() || s.front() != '(') return false;
            int depth = 0;
            size_t i = 0;
            while (i < s.size()) {
                char c = s[i];
                if (c == '\\') {
                    if (i + 1 >= s.size()) return false;
                    unsigned char next = static_cast<unsigned char>(s[i + 1]);
                    if (next > 127 || next == '\r' || next == '\n' || next == 0) return false;
                    i += 2;
                    continue;
                }
                if (c == '(') {
                    ++depth;
                    if (depth > 5) return false;
                    ++i;
                    continue;
                }
                if (c == ')') {
                    --depth;
                    ++i;
                    if (depth == 0) return i == s.size();
                    if (depth < 0) return false;
                    continue;
                }
                if (static_cast<unsigned char>(c) < 0x20 && c != '\t') return false;
                ++i;
            }
            return false;
        }
    }

    ProductInfoHeaderValue::ProductInfoHeaderValue(const std::string& productName, const std::string& productVersion)
        : product_(ProductHeaderValue(productName, productVersion)) {}

    ProductInfoHeaderValue::ProductInfoHeaderValue(const ProductHeaderValue& product) : product_(product) {}

    ProductInfoHeaderValue::ProductInfoHeaderValue(const std::string& comment) : comment_(comment) {
        if (!isValidComment(comment)) {
            throw System::FormatException("The comment is not a valid RFC 2616 comment: " + comment);
        }
    }

    std::string ProductInfoHeaderValue::ToString() const {
        return product_.has_value() ? product_->ToString() : comment_;
    }

    bool ProductInfoHeaderValue::Equals(const ProductInfoHeaderValue& other) const {
        if (!product_.has_value()) {
            return !other.product_.has_value() && comment_ == other.comment_;
        }
        return other.product_.has_value() && *product_ == *other.product_;
    }

    SharpRuntime::intcs ProductInfoHeaderValue::GetHashCode() const {
        if (!product_.has_value()) {
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(comment_));
        }
        return product_->GetHashCode();
    }

    bool ProductInfoHeaderValue::TryParse(const std::string& input, ProductInfoHeaderValue& parsedValue) {
        std::string trimmed = trim(input);
        if (trimmed.empty()) return false;

        if (trimmed.front() == '(') {
            if (!isValidComment(trimmed)) return false;
            parsedValue = ProductInfoHeaderValue(trimmed);
            return true;
        }

        ProductHeaderValue product("x");
        if (!ProductHeaderValue::TryParse(trimmed, product)) return false;
        parsedValue = ProductInfoHeaderValue(product);
        return true;
    }

    ProductInfoHeaderValue ProductInfoHeaderValue::Parse(const std::string& input) {
        ProductInfoHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The product-info header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
