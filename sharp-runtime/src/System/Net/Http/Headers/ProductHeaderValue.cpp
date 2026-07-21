// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/ProductHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>

namespace System::Net::Http::Headers {

    namespace {
        bool isHttpTokenChar(unsigned char c) {
            static constexpr const char* extras = "!#$%&'*+-.^_`|~";
            return c != 0 && (std::isalnum(c) || std::strchr(extras, static_cast<char>(c)) != nullptr);
        }

        bool isToken(const std::string& s) {
            return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) { return isHttpTokenChar(static_cast<unsigned char>(c)); });
        }

        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            if (a.size() != b.size()) return false;
            return std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
                return std::tolower(x) == std::tolower(y);
            });
        }

        SharpRuntime::intcs hashIgnoreCase(const std::string& s) {
            std::string lower = s;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(lower));
        }

        void checkValidToken(const std::string& value, const std::string& paramName) {
            System::ArgumentException::ThrowIfNullOrEmpty(value, paramName);
            if (!isToken(value)) {
                throw System::FormatException("The value is not a valid HTTP token: " + value);
            }
        }
    }

    ProductHeaderValue::ProductHeaderValue(const std::string& name) : ProductHeaderValue(name, "") {}

    ProductHeaderValue::ProductHeaderValue(const std::string& name, const std::string& version) : name_(name) {
        checkValidToken(name, "name");
        if (!version.empty()) {
            checkValidToken(version, "version");
            version_ = version;
        }
    }

    std::string ProductHeaderValue::ToString() const {
        if (version_.empty()) return name_;
        return name_ + "/" + version_;
    }

    bool ProductHeaderValue::Equals(const ProductHeaderValue& other) const {
        return equalsIgnoreCase(name_, other.name_) && equalsIgnoreCase(version_, other.version_);
    }

    SharpRuntime::intcs ProductHeaderValue::GetHashCode() const {
        SharpRuntime::intcs result = hashIgnoreCase(name_);
        if (!version_.empty()) result ^= hashIgnoreCase(version_);
        return result;
    }

    bool ProductHeaderValue::TryParse(const std::string& input, ProductHeaderValue& parsedValue) {
        size_t start = input.find_first_not_of(" \t");
        if (start == std::string::npos) return false;
        size_t end = input.find_last_not_of(" \t");
        std::string trimmed = input.substr(start, end - start + 1);

        size_t slash = trimmed.find('/');
        std::string name = (slash == std::string::npos) ? trimmed : trimmed.substr(0, slash);
        std::string version = (slash == std::string::npos) ? "" : trimmed.substr(slash + 1);

        if (!isToken(name)) return false;
        if (slash != std::string::npos && !isToken(version)) return false;

        parsedValue = ProductHeaderValue(name, version);
        return true;
    }

    ProductHeaderValue ProductHeaderValue::Parse(const std::string& input) {
        ProductHeaderValue result("x");
        if (!TryParse(input, result)) {
            throw System::FormatException("The product header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
