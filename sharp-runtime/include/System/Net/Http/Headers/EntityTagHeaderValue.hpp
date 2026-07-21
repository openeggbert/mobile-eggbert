// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Represents an entity-tag header value (e.g. for ETag, If-Match, If-None-Match).
     *
     * C++ counterpart of .NET System.Net.Http.Headers.EntityTagHeaderValue.
     *
     * @note .NET's internal GetEntityTagLength (backing GenericHeaderParser-based header-string
     * parsing) is not ported; only the public surface is. ICloneable is not ported (no
     * reflection-based virtual-clone story, per NEXT.md).
     */
    class EntityTagHeaderValue {
        std::string tag_; ///< Includes the surrounding quotes, e.g. "\"abc\"".
        bool isWeak_ = false;

        EntityTagHeaderValue(const std::string& tag, bool isWeak, bool /*skipValidation*/) : tag_(tag), isWeak_(isWeak) {}

    public:
        /**
         * @brief Constructs a strong entity-tag.
         * @param tag The tag, given as a quoted-string including its surrounding quotes (e.g. "\"abc\"").
         * @throws System::FormatException if @p tag is not a valid quoted-string.
         */
        explicit EntityTagHeaderValue(const std::string& tag);

        /**
         * @brief Constructs an entity-tag with the given weakness.
         * @throws System::FormatException if @p tag is not a valid quoted-string.
         */
        EntityTagHeaderValue(const std::string& tag, bool isWeak);

        /** @return The tag, including its surrounding quotes. */
        [[nodiscard]] const std::string& getTagProperty() const { return tag_; }
        /** @return true if this is a weak entity-tag ("W/" prefix). */
        [[nodiscard]] bool getIsWeakProperty() const { return isWeak_; }

        /** @return "W/{tag}" if weak, otherwise just "{tag}". */
        [[nodiscard]] std::string ToString() const { return isWeak_ ? "W/" + tag_ : tag_; }

        /** Equality: IsWeak must match, and Tag is compared case-sensitively (it's a quoted-string). */
        [[nodiscard]] bool Equals(const EntityTagHeaderValue& other) const {
            return isWeak_ == other.isWeak_ && tag_ == other.tag_;
        }
        bool operator==(const EntityTagHeaderValue& o) const { return Equals(o); }
        bool operator!=(const EntityTagHeaderValue& o) const { return !Equals(o); }

        /** @return A hash code combining Tag and IsWeak. */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /**
         * @brief Parses a strong or weak entity-tag string (e.g. "\"abc\"" or "W/\"abc\"", or "*").
         * @throws System::FormatException if @p input is not a valid entity-tag.
         */
        [[nodiscard]] static EntityTagHeaderValue Parse(const std::string& input);

        /** Attempts to parse @p input as an EntityTagHeaderValue; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& input, EntityTagHeaderValue& parsedValue);

        /** @return The special "any" entity-tag ("*"), matching any entity-tag. */
        static const EntityTagHeaderValue& Any();
    };

} // namespace System::Net::Http::Headers
