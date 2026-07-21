// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Text::Unicode {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a contiguous range of Unicode code points.
     *
     * C++ counterpart of .NET System.Text.Unicode.UnicodeRange.
     *
     * @note Currently only the Basic Multilingual Plane is supported (matches .NET).
     */
    class UnicodeRange {
        intcs firstCodePoint_;
        intcs length_;

    public:
        /**
         * @brief Creates a new UnicodeRange.
         * @param firstCodePoint The first code point in the range.
         * @param length The number of code points in the range.
         */
        UnicodeRange(intcs firstCodePoint, intcs length)
            : firstCodePoint_(firstCodePoint), length_(length) {
            // Parameter checking: the first code point and last code point must
            // lie within the BMP. See https://unicode.org/faq/blocks_ranges.html for more info.
            if (firstCodePoint < 0 || firstCodePoint > 0xFFFF)
                throw System::ArgumentOutOfRangeException("firstCodePoint");
            if (length < 0 || static_cast<long long>(firstCodePoint) + length > 0x10000)
                throw System::ArgumentOutOfRangeException("length");
        }

        /** @brief The first code point in this range. */
        [[nodiscard]] intcs getFirstCodePointProperty() const { return firstCodePoint_; }
        /** @brief The number of code points in this range. */
        [[nodiscard]] intcs getLengthProperty()         const { return length_; }

        /**
         * @brief Creates a new UnicodeRange from a span of characters.
         * @param firstCharacter The first character in the range.
         * @param lastCharacter The last character in the range.
         * @return The UnicodeRange representing this span.
         */
        static UnicodeRange Create(char16_t firstCharacter, char16_t lastCharacter) {
            if (lastCharacter < firstCharacter)
                throw System::ArgumentOutOfRangeException("lastCharacter");
            return UnicodeRange(firstCharacter, 1 + (lastCharacter - firstCharacter));
        }
    };

} // namespace System::Text::Unicode
