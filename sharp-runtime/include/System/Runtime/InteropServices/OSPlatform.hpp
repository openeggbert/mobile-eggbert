// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"

namespace System::Runtime::InteropServices {

    /**
     * @brief Represents an operating system platform.
     *
     * C++ counterpart of .NET System.Runtime.InteropServices.OSPlatform.
     */
    class OSPlatform {
        std::string name_;

        explicit OSPlatform(std::string name) : name_(std::move(name)) {}

        static std::string toUpperAscii(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            return s;
        }

    public:
        /** @brief Creates a new OSPlatform instance. Consider caching the result if called frequently. */
        [[nodiscard]] static OSPlatform Create(const std::string& osPlatform) {
            if (osPlatform.empty()) {
                throw System::ArgumentException("Value cannot be an empty string.", "osPlatform");
            }
            return OSPlatform(osPlatform);
        }

        [[nodiscard]] bool Equals(const OSPlatform& other) const {
            return toUpperAscii(name_) == toUpperAscii(other.name_);
        }
        bool operator==(const OSPlatform& o) const { return Equals(o); }
        bool operator!=(const OSPlatform& o) const { return !Equals(o); }

        /**
         * @brief Returns a case-insensitive hash of the platform name.
         * C++ counterpart of .NET OSPlatform.GetHashCode() (StringComparer.OrdinalIgnoreCase.GetHashCode(Name)).
         * Matches Equals()'s case-insensitive comparison, satisfying the Equals/GetHashCode contract.
         */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(toUpperAscii(name_)));
        }

        [[nodiscard]] std::string ToString() const { return name_; }

        static const OSPlatform FreeBSD;
        static const OSPlatform Linux;
        static const OSPlatform OSX;
        static const OSPlatform Windows;
    };

    inline const OSPlatform OSPlatform::FreeBSD{OSPlatform::Create("FREEBSD")};
    inline const OSPlatform OSPlatform::Linux{OSPlatform::Create("LINUX")};
    inline const OSPlatform OSPlatform::OSX{OSPlatform::Create("OSX")};
    inline const OSPlatform OSPlatform::Windows{OSPlatform::Create("WINDOWS")};

} // namespace System::Runtime::InteropServices
