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
     * @brief Represents a media type used in a Content-Type header, as defined in RFC 2616
     * (e.g. "text/plain; charset=iso-8859-5").
     *
     * C++ counterpart of .NET System.Net.Http.Headers.MediaTypeHeaderValue.
     *
     * @note .NET's internal GetMediaTypeLength parsing helper is not ported; Parse/TryParse here
     * implement the same "type/subtype[; param1=value1; param2=value2...]" algorithm directly.
     * ICloneable is not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class MediaTypeHeaderValue {
        std::string mediaType_;
        std::vector<NameValueHeaderValue> parameters_;

        MediaTypeHeaderValue() = default;

    public:
        /**
         * @brief Constructs a value with the given media type and no parameters.
         * @throws System::ArgumentException if @p mediaType is empty.
         * @throws System::FormatException if @p mediaType is not a valid "type/subtype" (no
         * internal/leading/trailing whitespace, both type and subtype valid HTTP tokens).
         */
        explicit MediaTypeHeaderValue(const std::string& mediaType);

        /** @brief Constructs a value with the given media type and charset parameter. */
        MediaTypeHeaderValue(const std::string& mediaType, const std::string& charSet);

        /** @return The media type (e.g. "text/plain"). */
        [[nodiscard]] const std::string& getMediaTypeProperty() const { return mediaType_; }
        /**
         * Sets the media type.
         * @throws System::FormatException if @p value is not a valid "type/subtype".
         */
        void setMediaTypeProperty(const std::string& value);

        /** @return The "charset" parameter value, or "" if not present. */
        [[nodiscard]] std::string getCharSetProperty() const;
        /** Sets (or removes, if empty) the "charset" parameter. */
        void setCharSetProperty(const std::string& value);

        /** @return The parameter list, mutable. */
        [[nodiscard]] std::vector<NameValueHeaderValue>& getParametersProperty() { return parameters_; }
        [[nodiscard]] const std::vector<NameValueHeaderValue>& getParametersProperty() const { return parameters_; }

        /** @return "mediatype; param1=value1; param2=value2...", or just "mediatype" if there are no parameters. */
        [[nodiscard]] std::string ToString() const;

        /** Equality: MediaType is case-insensitive; Parameters compared order-independently. */
        [[nodiscard]] bool Equals(const MediaTypeHeaderValue& other) const;
        bool operator==(const MediaTypeHeaderValue& o) const { return Equals(o); }
        bool operator!=(const MediaTypeHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "type/subtype[; param1=value1; param2=value2...]".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static MediaTypeHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, MediaTypeHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
