// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <memory>
#include "System/IFormatProvider.hpp"

namespace System {

    /**
     * @brief Defines a method that supports custom formatting of the value of an object.
     *
     * C++ counterpart of .NET System.ICustomFormatter.
     * Implement this interface to provide formatting logic beyond what the
     * standard format strings offer.
     */
    class ICustomFormatter {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~ICustomFormatter() = default;

        /**
         * @brief Converts the value of a specified object to an equivalent string
         * representation using specified format and culture-specific formatting
         * information.
         *
         * C++ counterpart of .NET ICustomFormatter.Format(string, object, IFormatProvider).
         * @param format         A format string containing formatting specifications.
         * @param arg            An object to format (as void* because C++ has no object root).
         * @param formatProvider Culture-specific format information, or nullptr for invariant.
         * @return The string representation of @p arg.
         */
        [[nodiscard]] virtual std::string Format(
            const std::string& format,
            const void* arg,
            const IFormatProvider* formatProvider) const = 0;
    };

} // namespace System
