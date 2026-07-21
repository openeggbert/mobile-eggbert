// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents a value of the Via header (e.g. "1.1 proxy.example.com (comment)").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.ViaHeaderValue.
     *
     * @note ReceivedBy validation here is a practical approximation of .NET's
     * HttpRuleParser.GetHostLength (full URI-RFC host-name grammar): it accepts either a plain
     * HTTP token, or a host-like string containing none of the characters that HTTP list-parsing
     * treats as delimiters (space, tab, CR, '/', ','). This covers ordinary hostnames,
     * "host:port", and bracketed IPv6 literals without reproducing the full URI host grammar.
     * @note .NET's internal GetViaLength parsing helper is not ported. ICloneable is not ported
     * (no reflection-based virtual-clone story, per NEXT.md).
     */
    class ViaHeaderValue {
        std::string protocolName_;
        std::string protocolVersion_;
        std::string receivedBy_;
        std::string comment_;

    public:
        /**
         * @brief Constructs a value with the given protocol version and received-by host.
         * @throws System::ArgumentException if @p receivedBy is empty.
         * @throws System::FormatException if @p protocolVersion is not a valid HTTP token, or
         * @p receivedBy is not a valid token or host-like string.
         */
        ViaHeaderValue(const std::string& protocolVersion, const std::string& receivedBy);

        /** @brief Constructs a value additionally specifying the protocol name (e.g. "HTTP"). */
        ViaHeaderValue(const std::string& protocolVersion, const std::string& receivedBy, const std::string& protocolName);

        /** @brief Constructs a value additionally specifying the protocol name and an RFC 2616 comment. */
        ViaHeaderValue(const std::string& protocolVersion, const std::string& receivedBy,
                       const std::string& protocolName, const std::string& comment);

        /** @return The protocol name (e.g. "HTTP"), or "" if not specified. */
        [[nodiscard]] const std::string& getProtocolNameProperty() const { return protocolName_; }
        /** @return The protocol version (e.g. "1.1"). */
        [[nodiscard]] const std::string& getProtocolVersionProperty() const { return protocolVersion_; }
        /** @return The host (and optionally port) that received the request. */
        [[nodiscard]] const std::string& getReceivedByProperty() const { return receivedBy_; }
        /** @return The trailing RFC 2616 comment, or "" if not specified. */
        [[nodiscard]] const std::string& getCommentProperty() const { return comment_; }

        /** @return "[protocolName/]protocolVersion receivedBy [comment]". */
        [[nodiscard]] std::string ToString() const;

        /** Equality: ProtocolVersion/ReceivedBy/ProtocolName are case-insensitive; Comment is case-sensitive. */
        [[nodiscard]] bool Equals(const ViaHeaderValue& other) const;
        bool operator==(const ViaHeaderValue& o) const { return Equals(o); }
        bool operator!=(const ViaHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "[protocolName/]protocolVersion receivedBy [comment]".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static ViaHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, ViaHeaderValue& parsedValue);

    private:
        ViaHeaderValue() = default;
    };

} // namespace System::Net::Http::Headers
