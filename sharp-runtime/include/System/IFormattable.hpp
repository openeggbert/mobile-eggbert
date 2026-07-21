// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IFormatProvider.hpp"

namespace System {

    /**
     * @brief Provides a mechanism for formatting the value of an object into a
     * string representation using a format string.
     *
     * C++ counterpart of .NET System.IFormattable.
     */
    class IFormattable {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IFormattable() = default;

        /**
         * @brief Formats the value of the current instance using the specified format.
         *
         * C++ counterpart of .NET IFormattable.ToString(string, IFormatProvider), with the
         * formatProvider argument omitted. Kept as the single required override so existing
         * implementers are unaffected; see the two-argument overload below for API parity.
         * @param format A format string, or empty for the default format.
         * @return The value of the current instance in the specified format.
         */
        [[nodiscard]] virtual std::string ToString(const std::string& format) const = 0;

        /**
         * @brief Formats the value of the current instance using the specified format
         * and culture-specific format information.
         *
         * C++ counterpart of .NET IFormattable.ToString(string, IFormatProvider).
         * The default implementation ignores @p formatProvider and forwards to
         * ToString(format); override to honor culture-specific formatting.
         * @param format         A format string, or empty for the default format.
         * @param formatProvider Culture-specific format information, or nullptr for invariant.
         * @return The value of the current instance in the specified format.
         */
        [[nodiscard]] virtual std::string ToString(const std::string& format,
                                                     const IFormatProvider* formatProvider) const {
            (void)formatProvider;
            return ToString(format);
        }
    };

} // namespace System
