// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Collections/Specialized/StringDictionary.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Mime {

    /**
     * @brief Represents a Content-Type header, as used by MIME/mail messages
     * (e.g. "text/plain; charset=utf-8").
     *
     * C++ counterpart of .NET System.Net.Mime.ContentType.
     *
     * @note .NET's ContentType is independent of System::Net::Http::Headers::MediaTypeHeaderValue
     * (a conceptually similar but separately-implemented type in a different namespace/library) —
     * this port follows the same independence, with its own practical RFC 2045 token/quoted-string
     * parser rather than reusing MediaTypeHeaderValue.
     * @note .NET's internal wire-persistence caching (`_isChanged`/`_isPersisted`, tied to
     * `HeaderCollection`/mail message serialization) is not reproduced — that machinery only
     * matters for `System.Net.Mail`'s message-writing pipeline, which is out of scope here.
     * ToString() always recomputes fresh from MediaType/Parameters instead of caching.
     * @note Name does not perform .NET's MIME encoded-word decoding (`=?utf-8?B?...?=`) — this
     * runtime has no MIME decoder elsewhere either (same deviation as
     * ContentDispositionHeaderValue::getFileNameProperty()).
     */
    class ContentType {
        std::string mediaType_;
        std::string subType_;
        System::Collections::Specialized::StringDictionary parameters_;

        void parseValue(const std::string& contentType);

    public:
        /** @brief Constructs a default ContentType of "application/octet-stream". */
        ContentType();

        /**
         * @brief Parses @p contentType.
         * @throws System::ArgumentException if @p contentType is empty.
         * @throws System::FormatException if @p contentType is not a valid Content-Type value.
         */
        explicit ContentType(const std::string& contentType);

        /** @return The "boundary" parameter, or "" if not present. */
        [[nodiscard]] std::string getBoundaryProperty() const { return parameters_.GetValue("boundary"); }
        /** @brief Sets (or removes, if empty) the "boundary" parameter. */
        void setBoundaryProperty(const std::string& value);

        /** @return The "charset" parameter, or "" if not present. */
        [[nodiscard]] std::string getCharSetProperty() const { return parameters_.GetValue("charset"); }
        /** @brief Sets (or removes, if empty) the "charset" parameter. */
        void setCharSetProperty(const std::string& value);

        /** @return "mediatype/subtype". */
        [[nodiscard]] std::string getMediaTypeProperty() const { return mediaType_ + "/" + subType_; }
        /**
         * Sets the media type.
         * @throws System::ArgumentException if @p value is empty.
         * @throws System::FormatException if @p value is not a valid "type/subtype" string.
         */
        void setMediaTypeProperty(const std::string& value);

        /** @return The "name" parameter, or "" if not present. */
        [[nodiscard]] std::string getNameProperty() const { return parameters_.GetValue("name"); }
        /** @brief Sets (or removes, if empty) the "name" parameter. */
        void setNameProperty(const std::string& value);

        /** @return The parameter dictionary, mutable. */
        [[nodiscard]] System::Collections::Specialized::StringDictionary& getParametersProperty() { return parameters_; }
        [[nodiscard]] const System::Collections::Specialized::StringDictionary& getParametersProperty() const { return parameters_; }

        /** @return "mediatype/subtype[; param1=value1; param2=value2...]". */
        [[nodiscard]] std::string ToString() const;

        /** Equality: compares ToString() case-insensitively, matching .NET. */
        [[nodiscard]] bool Equals(const ContentType& other) const;
        bool operator==(const ContentType& o) const { return Equals(o); }
        bool operator!=(const ContentType& o) const { return !Equals(o); }

        /** @return A hash code consistent with Equals(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;
    };

} // namespace System::Net::Mime
