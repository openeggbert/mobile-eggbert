// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/TextElementEnumerator.hpp"

namespace System::Globalization {

using SharpRuntime::intcs;

/**
 * @brief Provides iteration over and retrieval of text elements in a string.
 *
 * C++ counterpart of .NET System.Globalization.StringInfo.
 * In this stub, text elements are treated as UTF-8 byte SEQUENCES (a single ASCII byte, or a
 * complete multi-byte UTF-8 character kept together) rather than full Unicode grapheme clusters
 * — combining-character grouping (e.g. a base letter followed by a combining accent forming one
 * user-perceived character) is not implemented. This matches the UTF-8-aware decoding already
 * used by TextElementEnumerator::MoveNext() — every entry point on this class (GetNextTextElement,
 * GetNextTextElementLength, ParseCombiningCharacters, LengthInTextElements, and
 * GetTextElementEnumerator) is consistent with that decoding, so none of them truncate or
 * mis-split a multi-byte character.
 */
class StringInfo {
    std::string string_;

    // Returns the byte length of the UTF-8 sequence starting at str[index] (index must be a
    // valid, in-bounds position). Matches TextElementEnumerator::MoveNext()'s decode logic
    // exactly, clamped so it never reads past the end of str.
    static size_t utf8ElementLength(const std::string& str, size_t index) {
        unsigned char c = static_cast<unsigned char>(str[index]);
        size_t len = 1;
        if      ((c & 0x80) == 0x00) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        return std::min(len, str.size() - index);
    }

public:
    /**
     * @brief Constructs an empty StringInfo.
     *
     * C++ counterpart of .NET StringInfo().
     */
    StringInfo() = default;

    /**
     * @brief Constructs a StringInfo wrapping the given string.
     *
     * C++ counterpart of .NET StringInfo(string).
     * @param str The string to wrap.
     */
    explicit StringInfo(const std::string& str) : string_(str) {}

    /**
     * @brief Gets the underlying string.
     *
     * C++ counterpart of .NET StringInfo.String.
     * @return A const reference to the wrapped string.
     */
    [[nodiscard]] const std::string& getStringProperty() const { return string_; }

    /**
     * @brief Sets the underlying string.
     *
     * C++ counterpart of .NET StringInfo.String setter.
     * @param v The new string value.
     */
    void setStringProperty(const std::string& v) { string_ = v; }

    /**
     * @brief Gets the number of text elements in the string.
     *
     * C++ counterpart of .NET StringInfo.LengthInTextElements.
     * Counts UTF-8 byte-sequence elements (see the class doc-comment) — a multi-byte character
     * counts as one element, not one per byte.
     * @return The number of text elements in the string.
     */
    [[nodiscard]] intcs getLengthInTextElementsProperty() const {
        intcs count = 0;
        for (size_t i = 0; i < string_.size(); i += utf8ElementLength(string_, i)) ++count;
        return count;
    }

    /**
     * @brief Returns a substring starting at the given text element index.
     *
     * C++ counterpart of .NET StringInfo.SubstringByTextElements(int).
     * @param startingTextElement The zero-based index of the first text element.
     * @return The substring from @p startingTextElement to the end.
     * @throws System::ArgumentOutOfRangeException if @p startingTextElement is out of range.
     */
    [[nodiscard]] std::string SubstringByTextElements(intcs startingTextElement) const {
        return SubstringByTextElements(startingTextElement,
                                       static_cast<intcs>(string_.size()) - startingTextElement);
    }

    /**
     * @brief Returns a substring of the given number of text elements.
     *
     * C++ counterpart of .NET StringInfo.SubstringByTextElements(int, int).
     * @param startingTextElement   The zero-based index of the first text element.
     * @param lengthInTextElements  The number of text elements to include.
     * @return The specified substring.
     * @throws System::ArgumentOutOfRangeException if @p startingTextElement or
     *         @p lengthInTextElements is negative or out of range for the string.
     */
    [[nodiscard]] std::string SubstringByTextElements(intcs startingTextElement,
                                                       intcs lengthInTextElements) const {
        size_t length = string_.size();
        if (static_cast<unsigned int>(startingTextElement) >= static_cast<unsigned int>(length)) {
            throw System::ArgumentOutOfRangeException("startingTextElement");
        }
        if (static_cast<unsigned int>(lengthInTextElements) >
            static_cast<unsigned int>(length - static_cast<size_t>(startingTextElement))) {
            throw System::ArgumentOutOfRangeException("lengthInTextElements");
        }
        return string_.substr(static_cast<size_t>(startingTextElement),
                              static_cast<size_t>(lengthInTextElements));
    }

    /**
     * @brief Returns the text element at the specified index in the given string.
     *
     * C++ counterpart of .NET StringInfo.GetNextTextElement(string, int).
     * Returns the full UTF-8 byte sequence starting at @p index (see the class doc-comment) —
     * consistent with GetTextElementEnumerator, so a multi-byte character is never truncated to
     * its leading byte.
     * @param str   The source string.
     * @param index The zero-based byte index (default 0).
     * @return The text element (1-4 bytes) starting at @p index, or an empty string if @p index
     *         is exactly str.size() (the end of the string).
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than
     *         str.size().
     */
    static std::string GetNextTextElement(const std::string& str, intcs index = 0) {
        if (index < 0 || index > static_cast<int>(str.size()))
            throw System::ArgumentOutOfRangeException("index");
        if (index == static_cast<int>(str.size())) return {};
        size_t i = static_cast<size_t>(index);
        return str.substr(i, utf8ElementLength(str, i));
    }

    /**
     * @brief Returns the length of the text element at the specified index.
     *
     * C++ counterpart of .NET StringInfo.GetNextTextElementLength(string, int).
     * Returns the byte length of the UTF-8 sequence starting at @p index (1-4), consistent with
     * GetNextTextElement/GetTextElementEnumerator.
     * @param str   The source string.
     * @param index The zero-based byte index (default 0).
     * @return The element's byte length (1-4) if @p index is a valid element position; 0 if
     *         @p index is exactly str.size() (the end of the string).
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than
     *         str.size().
     */
    static intcs GetNextTextElementLength(const std::string& str, intcs index = 0) {
        if (index < 0 || index > static_cast<int>(str.size()))
            throw System::ArgumentOutOfRangeException("index");
        if (index == static_cast<int>(str.size())) return 0;
        return static_cast<intcs>(utf8ElementLength(str, static_cast<size_t>(index)));
    }

    /**
     * @brief Returns the starting byte indices of each text element in the given string.
     *
     * C++ counterpart of .NET StringInfo.ParseCombiningCharacters(string).
     * Each entry is the starting byte index of one UTF-8 element (no combining-character
     * grouping) — consistent with GetNextTextElement/GetTextElementEnumerator, so a multi-byte
     * character contributes exactly one entry (its starting byte), not one per byte.
     * @param str The source string.
     * @return A vector of zero-based starting byte indices, one per text element in @p str.
     */
    static std::vector<intcs> ParseCombiningCharacters(const std::string& str) {
        std::vector<intcs> result;
        for (size_t i = 0; i < str.size(); i += utf8ElementLength(str, i))
            result.push_back(static_cast<intcs>(i));
        return result;
    }

    /**
     * @brief Returns a TextElementEnumerator for the entire string.
     *
     * C++ counterpart of .NET StringInfo.GetTextElementEnumerator(string).
     * @param str The string to enumerate.
     * @return A TextElementEnumerator positioned before the first element.
     */
    static TextElementEnumerator GetTextElementEnumerator(const std::string& str) {
        return TextElementEnumerator(str);
    }

    /**
     * @brief Returns a TextElementEnumerator starting at the given index.
     *
     * C++ counterpart of .NET StringInfo.GetTextElementEnumerator(string, int).
     * @param str   The string to enumerate.
     * @param index The zero-based byte index at which to start enumeration.
     * @return A TextElementEnumerator positioned before the first element at @p index.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than the string length.
     */
    static TextElementEnumerator GetTextElementEnumerator(const std::string& str, intcs index) {
        if (index < 0 || index > static_cast<int>(str.size()))
            throw System::ArgumentOutOfRangeException("index");
        return TextElementEnumerator(str.substr(index));
    }
};

} // namespace System::Globalization
