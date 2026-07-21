// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cctype>
#include <cwctype>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Globalization {

using SharpRuntime::charcs;
using SharpRuntime::intcs;

/**
 * @brief Defines text properties and behaviors, such as casing, that are specific to a writing system.
 *
 * C++ counterpart of .NET System.Globalization.TextInfo.
 * This implementation performs locale-insensitive ASCII casing; full Unicode locale-aware
 * casing is not supported.
 */
class TextInfo {
public:
    /**
     * @brief Constructs a TextInfo for the given culture name.
     *
     * C++ counterpart of .NET CultureInfo.TextInfo.
     * @param cultureName The culture name (e.g. "en-US"); defaults to "en-US".
     */
    explicit TextInfo(const std::string& cultureName = "en-US")
        : cultureName_(cultureName) {}

    /**
     * @brief Gets the culture name associated with this TextInfo.
     *
     * C++ counterpart of .NET TextInfo.CultureName.
     * @return The culture name string.
     */
    [[nodiscard]] const std::string& getCultureNameProperty() const { return cultureName_; }

    /**
     * @brief Gets a value indicating whether this TextInfo is read-only.
     *
     * C++ counterpart of .NET TextInfo.IsReadOnly.
     * @return true if this instance is read-only; otherwise false.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const { return isReadOnly_; }

    /**
     * @brief Gets a value indicating whether the writing system is right-to-left.
     *
     * C++ counterpart of .NET TextInfo.IsRightToLeft.
     * Stub — always returns false.
     * @return Always false.
     */
    [[nodiscard]] bool getIsRightToLeftProperty() const { return false; }

    /**
     * @brief Gets the ANSI code page for this writing system.
     *
     * C++ counterpart of .NET TextInfo.ANSICodePage.
     * Stub — always returns 0.
     * @return Always 0.
     */
    [[nodiscard]] intcs getANSICodePageProperty() const { return 0; }

    /**
     * @brief Gets the EBCDIC code page for this writing system.
     *
     * C++ counterpart of .NET TextInfo.EBCDICCodePage.
     * Stub — always returns 0.
     * @return Always 0.
     */
    [[nodiscard]] intcs getEBCDICCodePageProperty() const { return 0; }

    /**
     * @brief Gets the culture locale identifier.
     *
     * C++ counterpart of .NET TextInfo.LCID.
     * Stub — always returns 0.
     * @return Always 0.
     */
    [[nodiscard]] intcs getLCIDProperty() const { return 0; }

    /**
     * @brief Gets the Macintosh code page for this writing system.
     *
     * C++ counterpart of .NET TextInfo.MacCodePage.
     * Stub — always returns 0.
     * @return Always 0.
     */
    [[nodiscard]] intcs getMacCodePageProperty() const { return 0; }

    /**
     * @brief Gets the OEM code page for this writing system.
     *
     * C++ counterpart of .NET TextInfo.OEMCodePage.
     * Stub — always returns 0.
     * @return Always 0.
     */
    [[nodiscard]] intcs getOEMCodePageProperty() const { return 0; }

    /**
     * @brief Gets the list separator for this writing system.
     *
     * C++ counterpart of .NET TextInfo.ListSeparator.
     * @return The list separator string (default ",").
     */
    [[nodiscard]] std::string getListSeparatorProperty() const { return listSeparator_; }

    /**
     * @brief Sets the list separator for this writing system.
     *
     * C++ counterpart of .NET TextInfo.ListSeparator setter.
     * @param value The new list separator string.
     * @throws System::InvalidOperationException if this instance is read-only.
     */
    void setListSeparatorProperty(const std::string& value) {
        VerifyWritable();
        listSeparator_ = value;
    }

    /**
     * @brief Converts a UTF-16 character to its lowercase equivalent.
     *
     * C++ counterpart of .NET TextInfo.ToLower(char).
     * @param c The character to convert.
     * @return The lowercase equivalent.
     */
    [[nodiscard]] charcs ToLower(charcs c) const {
        if (c < 128) return static_cast<charcs>(std::tolower(static_cast<int>(c)));
        return static_cast<charcs>(std::towlower(static_cast<wint_t>(c)));
    }

    /**
     * @brief Converts a UTF-16 string to lowercase.
     *
     * C++ counterpart of .NET TextInfo.ToLower(string) (UTF-16 variant).
     * @param str The string to convert.
     * @return The lowercase string.
     */
    [[nodiscard]] std::u16string ToLower(const std::u16string& str) const {
        std::u16string result = str;
        for (auto& c : result) c = ToLower(c);
        return result;
    }

    /**
     * @brief Converts a UTF-8 string to lowercase.
     *
     * C++ counterpart of .NET TextInfo.ToLower(string).
     * @param str The string to convert.
     * @return The lowercase string.
     */
    [[nodiscard]] std::string ToLower(const std::string& str) const {
        std::string result = str;
        for (auto& c : result)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return result;
    }

    /**
     * @brief Converts a UTF-16 character to its uppercase equivalent.
     *
     * C++ counterpart of .NET TextInfo.ToUpper(char).
     * @param c The character to convert.
     * @return The uppercase equivalent.
     */
    [[nodiscard]] charcs ToUpper(charcs c) const {
        if (c < 128) return static_cast<charcs>(std::toupper(static_cast<int>(c)));
        return static_cast<charcs>(std::towupper(static_cast<wint_t>(c)));
    }

    /**
     * @brief Converts a UTF-16 string to uppercase.
     *
     * C++ counterpart of .NET TextInfo.ToUpper(string) (UTF-16 variant).
     * @param str The string to convert.
     * @return The uppercase string.
     */
    [[nodiscard]] std::u16string ToUpper(const std::u16string& str) const {
        std::u16string result = str;
        for (auto& c : result) c = ToUpper(c);
        return result;
    }

    /**
     * @brief Converts a UTF-8 string to uppercase.
     *
     * C++ counterpart of .NET TextInfo.ToUpper(string).
     * @param str The string to convert.
     * @return The uppercase string.
     */
    [[nodiscard]] std::string ToUpper(const std::string& str) const {
        std::string result = str;
        for (auto& c : result)
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return result;
    }

    /**
     * @brief Converts the specified string to title case.
     *
     * C++ counterpart of .NET TextInfo.ToTitleCase(string).
     * Each word starts with an uppercase letter; remaining letters are lowercased --
     * EXCEPT a word that is entirely uppercase (no lowercase letters at all, e.g. "USA",
     * "NASA") is left unchanged, matching .NET's real behavior of preserving acronyms
     * (TextInfo.cs's `hasLowerCase` flag, "in line with Word 2000 behavior of
     * titlecasing"). A word whose first letter is lowercase is always normally
     * title-cased even if the rest happens to be uppercase (e.g. "uSA" -> "Usa"), since
     * .NET's hasLowerCase check covers the first letter too.
     * @param str The string to convert.
     * @return The title-cased string.
     */
    [[nodiscard]] std::string ToTitleCase(const std::string& str) const {
        std::string result = str;
        std::size_t i = 0;
        // Matches real .NET's word-boundary detection (TextInfo.cs's c_wordSeparatorMask):
        // whitespace AND most punctuation categories (dash, open/close, quote, other
        // punctuation, symbols, etc.) are word separators -- NOT just whitespace. Letters and
        // digits are never separators. The apostrophe is a documented exception (real .NET
        // gives it bespoke mid-word handling this port doesn't replicate, but excluding it
        // from the separator set here reproduces the same observable result for the common
        // case, e.g. "o'brien" -> "O'brien"). Previously splitting only on whitespace meant a
        // hyphenated word like "mary-jane" was treated as one word and title-cased as
        // "Mary-jane" instead of "Mary-Jane" -- confirmed via a standalone repro before this
        // fix, matching real .NET's actual documented behavior for punctuation-separated words.
        auto isWordBoundary = [](unsigned char c) {
            return std::isspace(c) || (std::ispunct(c) && c != '\'');
        };
        while (i < result.size()) {
            if (isWordBoundary(static_cast<unsigned char>(result[i]))) { ++i; continue; }
            std::size_t wordStart = i;
            while (i < result.size() && !isWordBoundary(static_cast<unsigned char>(result[i]))) ++i;
            std::size_t wordEnd = i;
            bool hasLower = false;
            for (std::size_t j = wordStart; j < wordEnd; ++j) {
                if (std::islower(static_cast<unsigned char>(result[j]))) { hasLower = true; break; }
            }
            result[wordStart] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[wordStart])));
            if (hasLower) {
                for (std::size_t j = wordStart + 1; j < wordEnd; ++j)
                    result[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[j])));
            }
            // else: all-uppercase acronym -- leave characters after the first unchanged.
        }
        return result;
    }

    /**
     * @brief Returns a mutable copy of this TextInfo.
     *
     * C++ counterpart of .NET TextInfo.Clone().
     * @return A modifiable copy.
     */
    [[nodiscard]] TextInfo Clone() const {
        TextInfo copy = *this;
        copy.isReadOnly_ = false;
        return copy;
    }

    /**
     * @brief Returns a read-only copy of the given TextInfo.
     *
     * C++ counterpart of .NET TextInfo.ReadOnly(TextInfo).
     * @param textInfo The source instance.
     * @return A read-only copy.
     */
    static TextInfo ReadOnly(const TextInfo& textInfo) {
        TextInfo copy = textInfo;
        copy.isReadOnly_ = true;
        return copy;
    }

    /**
     * @brief Returns true if both TextInfo instances have the same culture name.
     *
     * C++ counterpart of .NET TextInfo.Equals(object).
     * @param other The TextInfo to compare.
     * @return true if the culture names are equal.
     */
    bool operator==(const TextInfo& other) const { return cultureName_ == other.cultureName_; }

    /**
     * @brief Returns a string representation of this TextInfo.
     *
     * C++ counterpart of .NET TextInfo.ToString().
     * @return A string in the form "TextInfo - <cultureName>".
     */
    [[nodiscard]] std::string ToString() const { return "TextInfo - " + cultureName_; }

private:
    std::string cultureName_;
    std::string listSeparator_{"," };
    bool isReadOnly_{false};

    void VerifyWritable() const {
        if (isReadOnly_) throw System::InvalidOperationException("Instance is read-only.");
    }
};

} // namespace System::Globalization
