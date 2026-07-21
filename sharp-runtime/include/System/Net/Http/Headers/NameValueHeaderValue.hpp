// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents a name/value pair used in various HTTP header values (e.g. media-type
     * parameters), where the value part is optional (a name-only token is also valid).
     *
     * C++ counterpart of .NET System.Net.Http.Headers.NameValueHeaderValue.
     *
     * @note .NET's internal parsing helpers (GetNameValueLength, GetNameValueListLength, Find,
     * etc.) that back GenericHeaderParser-based header-string parsing are not ported — this
     * runtime has no equivalent generic per-header-field parser framework. Only the public surface
     * (construction, Name/Value, Equals/GetHashCode, ToString, a practical Parse/TryParse) is
     * ported. ICloneable is not ported (see NEXT.md: no reflection-based virtual-clone story).
     */
    class NameValueHeaderValue {
        std::string name_;
        std::string value_;

        static void checkValidToken(const std::string& value, const std::string& paramName);
        static void checkValueFormat(const std::string& value);
        static bool isValidQuotedString(const std::string& value);

    protected:
        /** Default-constructs with empty name/value, bypassing validation (protected: used internally by TryParse and by derived types like NameValueWithParametersHeaderValue). */
        NameValueHeaderValue() = default;

    public:
        /** Copy-constructs from another instance. Public (unlike .NET's protected internal copy ctor) since this type is stored by value in standard containers (e.g. std::vector) from outside the class hierarchy. */
        NameValueHeaderValue(const NameValueHeaderValue&) = default;

        /**
         * @brief Constructs a name-only value.
         * @throws System::ArgumentException if @p name is empty.
         * @throws System::FormatException if @p name is not a valid HTTP token.
         */
        explicit NameValueHeaderValue(const std::string& name);

        /**
         * @brief Constructs a name/value pair.
         * @throws System::ArgumentException if @p name is empty.
         * @throws System::FormatException if @p name is not a valid HTTP token, or @p value is
         * not empty/a valid token/a valid quoted-string.
         */
        NameValueHeaderValue(const std::string& name, const std::string& value);

        virtual ~NameValueHeaderValue() = default;

        /** @return The parameter name. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /** @return The parameter value, or "" if this is a name-only value. */
        [[nodiscard]] const std::string& getValueProperty() const { return value_; }
        /**
         * Sets the parameter value.
         * @throws System::FormatException if @p value is non-empty and not a valid token or quoted-string.
         */
        void setValueProperty(const std::string& value);

        /**
         * @brief Equality: name is always case-insensitive; value is case-sensitive if quoted,
         * case-insensitive otherwise (matching .NET's RFC 2616 §14.20-derived rule).
         */
        [[nodiscard]] bool Equals(const NameValueHeaderValue& other) const;
        bool operator==(const NameValueHeaderValue& o) const { return Equals(o); }
        bool operator!=(const NameValueHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /** @return "name=value", or just "name" if there is no value. */
        [[nodiscard]] std::string ToString() const;

        /**
         * @brief Parses a "name" or "name=value" string.
         * @throws System::FormatException if @p input is not a valid name or name/value pair.
         */
        [[nodiscard]] static NameValueHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input as a NameValueHeaderValue; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, NameValueHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
