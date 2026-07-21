// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/Net/Http/Headers/ProductHeaderValue.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents either a product token or an RFC 2616 comment, as used in the User-Agent,
     * Server, and Via header values (e.g. "MyProduct/1.0" or "(comment)").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.ProductInfoHeaderValue.
     *
     * @note Exactly one of Product/Comment is ever populated (mirroring .NET's mutually exclusive
     * design), never both. .NET's internal GetProductInfoLength parsing helper is not ported.
     * ICloneable is not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class ProductInfoHeaderValue {
        std::optional<ProductHeaderValue> product_;
        std::string comment_;

        ProductInfoHeaderValue() = default;

    public:
        /**
         * @brief Constructs a product-token value with the given name and version.
         * @throws System::ArgumentException if @p productName is empty.
         * @throws System::FormatException if @p productName or a non-empty @p productVersion is
         * not a valid HTTP token.
         */
        ProductInfoHeaderValue(const std::string& productName, const std::string& productVersion);

        /** @brief Constructs a product-token value from an existing ProductHeaderValue. */
        explicit ProductInfoHeaderValue(const ProductHeaderValue& product);

        /**
         * @brief Constructs a comment value (e.g. "(compatible; MSIE 6.0)").
         * @throws System::FormatException if @p comment is not a valid RFC 2616 comment (must be
         * enclosed in balanced parentheses, up to 5 levels of nesting).
         */
        explicit ProductInfoHeaderValue(const std::string& comment);

        /** @return The product token, or empty if this is a comment value. */
        [[nodiscard]] const std::optional<ProductHeaderValue>& getProductProperty() const { return product_; }
        /** @return The comment text (including its parentheses), or "" if this is a product value. */
        [[nodiscard]] const std::string& getCommentProperty() const { return comment_; }

        /** @return The product's ToString(), or the comment text if this is a comment value. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: Comments compare case-sensitively; products delegate to ProductHeaderValue::Equals. */
        [[nodiscard]] bool Equals(const ProductInfoHeaderValue& other) const;
        bool operator==(const ProductInfoHeaderValue& o) const { return Equals(o); }
        bool operator!=(const ProductInfoHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses a product token ("name" or "name/version") or a comment ("(...)").
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static ProductInfoHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, ProductInfoHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
