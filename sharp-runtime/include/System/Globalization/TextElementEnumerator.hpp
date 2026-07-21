// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <stdexcept>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Globalization {

using SharpRuntime::intcs;

/**
 * @brief Enumerates the text elements of a string.
 *
 * C++ counterpart of .NET System.Globalization.TextElementEnumerator.
 * Each text element may span one or more chars (grapheme cluster). This implementation
 * advances by UTF-8 code units: ASCII characters produce single-byte elements, and
 * multi-byte sequences are kept together. Combining-character grouping is not performed.
 */
class TextElementEnumerator {
public:
    /**
     * @brief Constructs a TextElementEnumerator over the given string.
     *
     * C++ counterpart of .NET StringInfo.GetTextElementEnumerator(string).
     * @param str The string to enumerate.
     */
    explicit TextElementEnumerator(const std::string& str) : str_(str) { Reset(); }

    /**
     * @brief Advances the enumerator to the next text element.
     *
     * C++ counterpart of .NET TextElementEnumerator.MoveNext().
     * @return true if there is a next element; false if enumeration is complete.
     */
    bool MoveNext() {
        long long newOffset = offset_ + length_;
        offset_ = newOffset;
        length_ = 0;
        if (newOffset < 0 || static_cast<size_t>(newOffset) >= str_.size()) return false;

        size_t off = static_cast<size_t>(newOffset);
        unsigned char c = static_cast<unsigned char>(str_[off]);
        size_t len = 1;
        if      ((c & 0x80) == 0x00) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        length_ = static_cast<long long>(std::min(len, str_.size() - off));
        return true;
    }

    /**
     * @brief Returns the current text element as a string.
     *
     * C++ counterpart of .NET TextElementEnumerator.GetTextElement().
     * @return The current text element.
     * @throws System::InvalidOperationException if enumeration has either not started or has
     *         already finished (MoveNext() has not been called, or has returned false).
     */
    [[nodiscard]] std::string GetTextElement() const {
        VerifyStarted();
        return str_.substr(static_cast<size_t>(offset_), static_cast<size_t>(length_));
    }

    /**
     * @brief Gets the current text element.
     *
     * C++ counterpart of .NET TextElementEnumerator.Current.
     * @return The current text element string.
     */
    [[nodiscard]] std::string getCurrentProperty() const { return GetTextElement(); }

    /**
     * @brief Gets the index of the current text element in the original string.
     *
     * C++ counterpart of .NET TextElementEnumerator.ElementIndex.
     * @return The zero-based byte index of the current element.
     * @throws System::InvalidOperationException if enumeration has either not started or has
     *         already finished.
     */
    [[nodiscard]] intcs getElementIndexProperty() const {
        VerifyStarted();
        return static_cast<intcs>(offset_);
    }

    /**
     * @brief Resets the enumerator to the beginning of the string.
     *
     * C++ counterpart of .NET TextElementEnumerator.Reset().
     */
    void Reset() {
        // offset_ starts at str_.size() (out of range) and length_ is set so that
        // offset_ + length_ == 0, meaning the first MoveNext() begins at index 0.
        // This mirrors .NET's TextElementEnumerator.Reset(), which relies on signed-int
        // wraparound the same way. Also ensures GetTextElement()/ElementIndex throw until
        // MoveNext() runs.
        offset_ = static_cast<long long>(str_.size());
        length_ = -offset_;
    }

private:
    std::string str_;
    long long offset_ = 0;
    long long length_ = 0;

    void VerifyStarted() const {
        if (offset_ < 0 || static_cast<size_t>(offset_) >= str_.size()) {
            throw System::InvalidOperationException("Enumeration has either not started or has already finished.");
        }
    }
};

} // namespace System::Globalization
