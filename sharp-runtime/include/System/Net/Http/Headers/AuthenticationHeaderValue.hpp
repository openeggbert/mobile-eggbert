// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents authentication information in Authorization, ProxyAuthorization,
     * WWW-Authenticate, and Proxy-Authenticate header values (e.g. "Basic QWxhZGRpbjpvcGVu").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.AuthenticationHeaderValue.
     *
     * @note Like .NET, the Parameter is treated as one opaque, unparsed string — RFC 2617's
     * auth-param grammar is ambiguous enough (see .NET's own comment on this) that no attempt is
     * made to further structure it here.
     * @note .NET's internal GetAuthenticationLength parsing helper (which handles the
     * comma-separated multi-scheme case, e.g. "Digest a=b, c=d, NTLM") is not ported; Parse/
     * TryParse here handle the common single-scheme case only ("scheme" or "scheme parameter").
     * ICloneable is not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class AuthenticationHeaderValue {
        std::string scheme_;
        std::string parameter_;

    public:
        /**
         * @brief Constructs a scheme-only value (e.g. "NTLM").
         * @throws System::ArgumentException if @p scheme is empty.
         * @throws System::FormatException if @p scheme is not a valid HTTP token.
         */
        explicit AuthenticationHeaderValue(const std::string& scheme);

        /**
         * @brief Constructs a scheme + parameter value (e.g. "Basic", "QWxhZGRpbjpvcGVu").
         * @throws System::ArgumentException if @p scheme is empty.
         * @throws System::FormatException if @p scheme is not a valid HTTP token, or @p parameter
         * contains a bare CR, LF, or NUL character.
         */
        AuthenticationHeaderValue(const std::string& scheme, const std::string& parameter);

        /** @return The authentication scheme (e.g. "Basic", "Bearer"). */
        [[nodiscard]] const std::string& getSchemeProperty() const { return scheme_; }
        /** @return The scheme-specific parameter, or "" if none. */
        [[nodiscard]] const std::string& getParameterProperty() const { return parameter_; }

        /** @return "scheme parameter", or just "scheme" if there is no parameter. */
        [[nodiscard]] std::string ToString() const;

        /**
         * @brief Equality: Scheme is always case-insensitive. If both Parameters are empty, only
         * Scheme is compared; otherwise Parameter is compared case-sensitively (it can't be
         * reliably parsed, so no case-folding rule can be safely applied).
         */
        [[nodiscard]] bool Equals(const AuthenticationHeaderValue& other) const;
        bool operator==(const AuthenticationHeaderValue& o) const { return Equals(o); }
        bool operator!=(const AuthenticationHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "scheme" or "scheme parameter".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static AuthenticationHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, AuthenticationHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
