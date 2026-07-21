// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "System/Net/Http/Headers/NameValueHeaderValue.hpp"
#include "System/DateTimeOffset.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents the value of the Content-Disposition header.
     *
     * C++ counterpart of .NET System.Net.Http.Headers.ContentDispositionHeaderValue.
     *
     * @note Name/FileName getters here only strip surrounding quotes; .NET additionally attempts
     * MIME encoded-word decoding (`=?utf-8?B?...?=`) for these — this runtime has no MIME decoder
     * elsewhere either, so that's not reproduced. FileName/FileNameStar setters use RFC 5987
     * percent-encoding (`UTF-8''...`), which IS implemented here since it's self-contained and
     * matters for real multipart file uploads.
     * @note CreationDate/ModificationDate/ReadDate use System::DateTimeOffset::ToString("r") to
     * format (RFC 1123, matching the HTTP spec), but parse via a small RFC 1123 parser local to
     * this .cpp file rather than System::DateTimeOffset::TryParse — that method only accepts
     * ISO-8601-style strings, not the "Www, dd Mon yyyy HH:mm:ss GMT" format ToString("r") itself
     * produces (a pre-existing asymmetry in DateTimeOffset, out of scope to fix here). .NET's
     * internal HttpDateParser's extra leniency for malformed dates isn't reproduced.
     * @note .NET's internal GetDispositionTypeLength parsing helper is not ported. ICloneable is
     * not ported (no reflection-based virtual-clone story, per NEXT.md).
     */
    class ContentDispositionHeaderValue {
        std::string dispositionType_;
        std::vector<NameValueHeaderValue> parameters_;

        [[nodiscard]] const NameValueHeaderValue* findParameter(const std::string& name) const;
        void removeParameter(const std::string& name);
        void setOrAddParameter(const std::string& name, const std::string& value);

    public:
        ContentDispositionHeaderValue() = default;

        /**
         * @brief Constructs a value with the given disposition type (e.g. "attachment", "inline", "form-data").
         * @throws System::ArgumentException if @p dispositionType is empty.
         * @throws System::FormatException if @p dispositionType is not a valid HTTP token.
         */
        explicit ContentDispositionHeaderValue(const std::string& dispositionType);

        /** @return The disposition type (e.g. "attachment"). */
        [[nodiscard]] const std::string& getDispositionTypeProperty() const { return dispositionType_; }
        /**
         * Sets the disposition type.
         * @throws System::FormatException if @p value is not a valid HTTP token.
         */
        void setDispositionTypeProperty(const std::string& value);

        /** @return The parameter list, mutable. */
        [[nodiscard]] std::vector<NameValueHeaderValue>& getParametersProperty() { return parameters_; }
        [[nodiscard]] const std::vector<NameValueHeaderValue>& getParametersProperty() const { return parameters_; }

        /** @return The "name" parameter (quotes stripped), or empty if not present. */
        [[nodiscard]] std::string getNameProperty() const;
        /** Sets (or removes, if empty) the "name" parameter. */
        void setNameProperty(const std::string& value);

        /** @return The "filename" parameter (quotes stripped), or empty if not present. */
        [[nodiscard]] std::string getFileNameProperty() const;
        /** Sets (or removes, if empty) the "filename" parameter, quoted. */
        void setFileNameProperty(const std::string& value);

        /** @return The RFC 5987-decoded "filename*" parameter, or empty if not present/undecodable. */
        [[nodiscard]] std::string getFileNameStarProperty() const;
        /** Sets (or removes, if empty) the "filename*" parameter, RFC 5987-encoded as UTF-8. */
        void setFileNameStarProperty(const std::string& value);

        /** @return The "creation-date" parameter, or empty if not present/unparsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getCreationDateProperty() const;
        /** Sets (or removes, if empty) the "creation-date" parameter. */
        void setCreationDateProperty(std::optional<System::DateTimeOffset> value);

        /** @return The "modification-date" parameter, or empty if not present/unparsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getModificationDateProperty() const;
        /** Sets (or removes, if empty) the "modification-date" parameter. */
        void setModificationDateProperty(std::optional<System::DateTimeOffset> value);

        /** @return The "read-date" parameter, or empty if not present/unparsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getReadDateProperty() const;
        /** Sets (or removes, if empty) the "read-date" parameter. */
        void setReadDateProperty(std::optional<System::DateTimeOffset> value);

        /** @return The "size" parameter, or empty if not present/unparsable. */
        [[nodiscard]] std::optional<SharpRuntime::longcs> getSizeProperty() const;
        /**
         * Sets (or removes, if empty) the "size" parameter.
         * @throws std::out_of_range if @p value is negative.
         */
        void setSizeProperty(std::optional<SharpRuntime::longcs> value);

        /** @return "dispositiontype; param1=value1; param2=value2...". */
        [[nodiscard]] std::string ToString() const;

        /** Equality: DispositionType is case-insensitive; Parameters compared order-independently. */
        [[nodiscard]] bool Equals(const ContentDispositionHeaderValue& other) const;
        bool operator==(const ContentDispositionHeaderValue& o) const { return Equals(o); }
        bool operator!=(const ContentDispositionHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses "dispositiontype[; param1=value1; param2=value2...]".
         * @throws System::FormatException if @p input is not valid.
         */
        [[nodiscard]] static ContentDispositionHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, ContentDispositionHeaderValue& parsedValue);
    };

} // namespace System::Net::Http::Headers
