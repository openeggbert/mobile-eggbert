// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents a product identifier and version, as used in the User-Agent and Server
     * header values.
     *
     * C++ counterpart of .NET System.Net.Http.Headers.ProductHeaderValue.
     *
     * @note Version is a plain string, not System::Version, since a product version can be
     * something like "x11" that isn't a valid Version (matches .NET's own reasoning).
     * @note .NET's internal GetProductLength parsing helper is not ported; only the public
     * surface is. ICloneable is not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class ProductHeaderValue {
        std::string name_;
        std::string version_;

    public:
        /**
         * @brief Constructs a product identifier with no version.
         * @throws System::ArgumentException if @p name is empty.
         * @throws System::FormatException if @p name is not a valid HTTP token.
         */
        explicit ProductHeaderValue(const std::string& name);

        /**
         * @brief Constructs a product identifier and version.
         * @throws System::ArgumentException if @p name is empty.
         * @throws System::FormatException if @p name or a non-empty @p version is not a valid HTTP token.
         */
        ProductHeaderValue(const std::string& name, const std::string& version);

        /** @return The product name. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** @return The product version, or "" if none. */
        [[nodiscard]] const std::string& getVersionProperty() const { return version_; }

        /** @return "name/version", or just "name" if there is no version. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: both Name and Version are compared case-insensitively. */
        [[nodiscard]] bool Equals(const ProductHeaderValue& other) const;
        bool operator==(const ProductHeaderValue& o) const { return Equals(o); }
        bool operator!=(const ProductHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses a "name" or "name/version" string.
         * @throws System::FormatException if @p input is not a valid product identifier.
         */
        [[nodiscard]] static ProductHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input as a ProductHeaderValue; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, ProductHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
