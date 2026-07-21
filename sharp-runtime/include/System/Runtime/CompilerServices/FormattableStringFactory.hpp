// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "System/FormattableString.hpp"

namespace System::Runtime::CompilerServices {

    /**
     * @brief Provides a static method to create a FormattableString object from a
     * composite format string and its arguments.
     *
     * C++ counterpart of .NET System.Runtime.CompilerServices.FormattableStringFactory.
     * In C#, the compiler calls this factory when it lowers an interpolated string
     * literal to a FormattableString. In C++, callers must invoke Create() directly.
     */
    class FormattableStringFactory {
    public:
        FormattableStringFactory() = delete;

        /**
         * @brief Creates a FormattableString from a composite format string and
         * zero or more string arguments.
         *
         * C++ counterpart of .NET FormattableStringFactory.Create(string, params object[]).
         * @param format    A composite format string (e.g. "{0} and {1}").
         * @param arguments Zero or more string arguments substituted for the placeholders.
         * @return A FormattableString wrapping @p format and @p arguments.
         * @throws std::invalid_argument if @p format is empty (mirrors ArgumentNullException).
         */
        static System::FormattableString Create(std::string format,
                                                std::vector<std::string> arguments = {}) {
            return System::FormattableString(std::move(format), std::move(arguments));
        }
    };

} // namespace System::Runtime::CompilerServices
