// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text::Unicode {

    using SharpRuntime::intcs;

    /**
     * @brief Provides static methods that validate UTF-16 strings.
     *
     * C++ counterpart of .NET System.Text.Unicode.Utf16.
     */
    class Utf16 {
    public:
        Utf16() = delete;

        /**
         * @brief Finds the index of the first invalid UTF-16 subsequence.
         * @param value The UTF-16 input text to examine.
         * @return The index of the first invalid UTF-16 subsequence (an unpaired surrogate),
         * or -1 if the entire input is valid.
         */
        static intcs IndexOfInvalidSubsequence(const std::u16string& value) {
            for (size_t i = 0; i < value.size(); ++i) {
                char16_t c = value[i];
                if (c >= 0xD800 && c <= 0xDBFF) { // high surrogate
                    if (i + 1 >= value.size() || value[i + 1] < 0xDC00 || value[i + 1] > 0xDFFF) {
                        return static_cast<intcs>(i);
                    }
                    ++i; // consumed the low surrogate too
                } else if (c >= 0xDC00 && c <= 0xDFFF) { // unpaired low surrogate
                    return static_cast<intcs>(i);
                }
            }
            return -1;
        }

        /**
         * @brief Validates that the value is well-formed UTF-16.
         * @param value The UTF-16 input text to validate.
         * @return true if @p value is well-formed UTF-16, false otherwise.
         */
        static bool IsValid(const std::u16string& value) { return IndexOfInvalidSubsequence(value) < 0; }
    };

} // namespace System::Text::Unicode
