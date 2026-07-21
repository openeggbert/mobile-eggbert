// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/Net/Http/Headers/TransferCodingHeaderValue.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief A TransferCodingHeaderValue with a "q" quality-factor parameter, as used in the TE
     * header (e.g. "gzip; q=0.8").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.TransferCodingWithQualityHeaderValue.
     */
    class TransferCodingWithQualityHeaderValue : public TransferCodingHeaderValue {
    public:
        /** Constructs a value with the given transfer-coding token and no quality factor. */
        explicit TransferCodingWithQualityHeaderValue(const std::string& value);
        /**
         * @brief Constructs a value with the given transfer-coding token and quality factor.
         * @throws System::ArgumentOutOfRangeException if @p quality is outside [0.0, 1.0].
         */
        TransferCodingWithQualityHeaderValue(const std::string& value, double quality);

        /** @return The "q" quality factor in [0.0, 1.0], or empty if not set. */
        [[nodiscard]] std::optional<double> getQualityProperty() const;
        /**
         * Sets (or removes, if empty) the "q" quality factor.
         * @throws System::ArgumentOutOfRangeException if @p value is outside [0.0, 1.0].
         */
        void setQualityProperty(std::optional<double> value);

        /**
         * @brief Parses "value[; param1=value1...][; q=quality]".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static TransferCodingWithQualityHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, TransferCodingWithQualityHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
