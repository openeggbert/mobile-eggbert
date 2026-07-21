// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IFormatProvider.hpp"
#include "System/IndexOutOfRangeException.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a composite format string along with the arguments to be formatted.
     *
     * C++ counterpart of .NET System.FormattableString.
     *
     * In C#, instances are produced by the compiler from interpolated string literals
     * (e.g. @c $"Hello, {name}"). In C++, there is no such compiler support; construct
     * instances directly or use FormattableStringFactory::Create().
     *
     * Arguments are stored as std::string (rather than object?) because C++ has no
     * universal base type for arbitrary values.
     */
    class FormattableString {
    protected:
        std::string              format_;
        std::vector<std::string> args_;

    public:
        /**
         * @brief Initializes a new instance with the specified composite format string and arguments.
         * @param format A composite format string (e.g. "{0} and {1}").
         * @param args   Zero or more string arguments substituted for the placeholders.
         */
        explicit FormattableString(std::string format, std::vector<std::string> args = {})
            : format_(std::move(format)), args_(std::move(args)) {}

        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~FormattableString() = default;

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the composite format string.
         *
         * C++ counterpart of .NET FormattableString.Format.
         */
        [[nodiscard]] virtual const std::string& getFormatProperty() const { return format_; }

        /**
         * @brief Gets the number of arguments to be formatted.
         *
         * C++ counterpart of .NET FormattableString.ArgumentCount.
         */
        [[nodiscard]] virtual intcs getArgumentCountProperty() const {
            return static_cast<intcs>(args_.size());
        }

        // -----------------------------------------------------------------------
        // Argument access
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the argument at the specified index.
         *
         * C++ counterpart of .NET FormattableString.GetArgument(int).
         * @param index The zero-based index of the argument.
         * @return The string argument at @p index.
         * @throws System::IndexOutOfRangeException if @p index is out of bounds.
         */
        [[nodiscard]] virtual const std::string& GetArgument(intcs index) const {
            if (index < 0 || static_cast<std::size_t>(index) >= args_.size())
                throw System::IndexOutOfRangeException();
            return args_[static_cast<std::size_t>(index)];
        }

        /**
         * @brief Returns an array containing all format arguments.
         *
         * C++ counterpart of .NET FormattableString.GetArguments().
         * @return A copy of the internal argument vector.
         */
        [[nodiscard]] virtual std::vector<std::string> GetArguments() const { return args_; }

        // -----------------------------------------------------------------------
        // Formatting
        // -----------------------------------------------------------------------

        /**
         * @brief Formats the composite string with arguments substituted.
         *
         * C++ counterpart of .NET FormattableString.ToString().
         * Replaces @c {0}, @c {1}, … with the corresponding stored argument strings.
         * @return The formatted result string.
         */
        [[nodiscard]] virtual std::string ToString() const {
            std::string result = format_;
            for (int i = 0; i < static_cast<int>(args_.size()); ++i) {
                std::string placeholder = "{" + std::to_string(i) + "}";
                std::size_t pos = 0;
                while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                    result.replace(pos, placeholder.size(), args_[i]);
                    pos += args_[i].size();
                }
            }
            return result;
        }

        /**
         * @brief Formats the composite string using the specified format provider.
         *
         * C++ counterpart of .NET FormattableString.ToString(IFormatProvider).
         * The provider is accepted for API compatibility but ignored in this port
         * (no locale-aware formatting is implemented).
         * @param formatProvider An IFormatProvider (may be nullptr; ignored).
         * @return The formatted result string.
         */
        [[nodiscard]] virtual std::string ToString(const IFormatProvider* /*formatProvider*/) const {
            return ToString();
        }

        // -----------------------------------------------------------------------
        // Static helpers
        // -----------------------------------------------------------------------

        /**
         * @brief Formats the formattable string using the invariant culture.
         *
         * C++ counterpart of .NET FormattableString.Invariant(FormattableString).
         * In this port, culture has no effect, so the result equals ToString().
         * @param formattable The FormattableString to format.
         * @return The formatted string.
         */
        static std::string Invariant(const FormattableString& formattable) {
            return formattable.ToString();
        }

        /**
         * @brief Formats the formattable string using the current culture.
         *
         * C++ counterpart of .NET FormattableString.CurrentCulture(FormattableString).
         * In this port, culture has no effect, so the result equals ToString().
         * @param formattable The FormattableString to format.
         * @return The formatted string.
         */
        static std::string CurrentCulture(const FormattableString& formattable) {
            return formattable.ToString();
        }
    };

} // namespace System
