// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/Net/Http/Headers/MediaTypeHeaderValue.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief A MediaTypeHeaderValue with a "q" quality-factor parameter, as used in Accept headers
     * (e.g. "text/html; q=0.8").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.MediaTypeWithQualityHeaderValue.
     */
    class MediaTypeWithQualityHeaderValue : public MediaTypeHeaderValue {
    public:
        /** Constructs a value with the given media type and no quality factor. */
        explicit MediaTypeWithQualityHeaderValue(const std::string& mediaType);
        /**
         * @brief Constructs a value with the given media type and quality factor.
         * @throws System::ArgumentOutOfRangeException if @p quality is outside [0.0, 1.0].
         */
        MediaTypeWithQualityHeaderValue(const std::string& mediaType, double quality);

        /** @return The "q" quality factor in [0.0, 1.0], or empty if not set. */
        [[nodiscard]] std::optional<double> getQualityProperty() const;
        /**
         * Sets (or removes, if empty) the "q" quality factor.
         * @throws System::ArgumentOutOfRangeException if @p value is outside [0.0, 1.0].
         */
        void setQualityProperty(std::optional<double> value);

        /**
         * @brief Parses "type/subtype[; param1=value1...][; q=quality]".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static MediaTypeWithQualityHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, MediaTypeWithQualityHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
