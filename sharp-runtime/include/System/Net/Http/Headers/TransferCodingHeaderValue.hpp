// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Net/Http/Headers/NameValueHeaderValue.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents a transfer-coding value, as used in the TE and Transfer-Encoding headers
     * (e.g. "gzip; q=0.5", "chunked").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.TransferCodingHeaderValue.
     *
     * @note .NET's internal GetTransferCodingLength parsing helper is not ported; Parse/TryParse
     * here implement the same "value[; param1=value1; param2=value2...]" algorithm directly.
     * ICloneable is not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class TransferCodingHeaderValue {
        std::string value_;
        std::vector<NameValueHeaderValue> parameters_;

    protected:
        TransferCodingHeaderValue() = default;

    public:
        /**
         * @brief Constructs a value with the given transfer-coding token and no parameters.
         * @throws System::ArgumentException if @p value is empty.
         * @throws System::FormatException if @p value is not a valid HTTP token.
         */
        explicit TransferCodingHeaderValue(const std::string& value);

        virtual ~TransferCodingHeaderValue() = default;

        /** @return The transfer-coding token (e.g. "gzip", "chunked"). */
        [[nodiscard]] const std::string& getValueProperty() const { return value_; }

        /** @return The parameter list, mutable. */
        [[nodiscard]] std::vector<NameValueHeaderValue>& getParametersProperty() { return parameters_; }
        [[nodiscard]] const std::vector<NameValueHeaderValue>& getParametersProperty() const { return parameters_; }

        /** @return "value; param1=value1; param2=value2...", or just "value" if there are no parameters. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: Value is case-insensitive; Parameters compared order-independently. */
        [[nodiscard]] bool Equals(const TransferCodingHeaderValue& other) const;
        bool operator==(const TransferCodingHeaderValue& o) const { return Equals(o); }
        bool operator!=(const TransferCodingHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "value[; param1=value1; param2=value2...]".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static TransferCodingHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, TransferCodingHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
