// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Text/NormalizationForm.hpp"

namespace System {

    /**
     * @brief Provides extension-like static methods for Unicode string normalization.
     *
     * C++ counterpart of .NET System.StringNormalizationExtensions.
     *
     * @note Full Unicode normalization (NFC/NFD/NFKC/NFKD) requires external ICU or
     * similar tables and is outside the scope of sharp-runtime. These stubs preserve
     * the API surface: IsNormalized always returns true and Normalize returns the
     * input unchanged, which is correct for ASCII-only strings.
     */
    struct StringNormalizationExtensions {
        StringNormalizationExtensions() = delete;

        /**
         * @brief Determines whether the string is in Unicode NFC form.
         * @param str The string to check.
         * @return true (stub — ASCII strings are always NFC).
         */
        static bool IsNormalized(const std::string& str) {
            return IsNormalized(str, System::Text::NormalizationForm::FormC);
        }

        /**
         * @brief Determines whether the string is in the specified normalization form.
         * @param str The string to check.
         * @param form The normalization form.
         * @return true (stub — ASCII strings satisfy all normalization forms).
         */
        static bool IsNormalized(const std::string& /*str*/,
                                 System::Text::NormalizationForm /*form*/) {
            return true;
        }

        /**
         * @brief Returns the string normalized to NFC.
         * @param str The string to normalize.
         * @return The input string unchanged (stub — correct for ASCII input).
         */
        static std::string Normalize(const std::string& str) {
            return Normalize(str, System::Text::NormalizationForm::FormC);
        }

        /**
         * @brief Returns the string normalized to the specified form.
         * @param str  The string to normalize.
         * @param form The normalization form.
         * @return The input string unchanged (stub — correct for ASCII input).
         */
        static std::string Normalize(const std::string& str,
                                     System::Text::NormalizationForm /*form*/) {
            return str;
        }
    };

} // namespace System
