// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include "System/FormatException.hpp"

namespace System {

    /**
     * @brief Represents the Boolean (true/false) value type and its string equivalents.
     *
     * C++ counterpart of .NET System.Boolean.
     * All members are static; the class cannot be instantiated.
     */
    class Boolean {
    public:
        Boolean() = delete;

        /** @brief The string representation of the Boolean true value ("True"). */
        static constexpr const char* TrueString  = "True";

        /** @brief The string representation of the Boolean false value ("False"). */
        static constexpr const char* FalseString = "False";

        /**
         * @brief Parses a string as a Boolean value.
         *
         * C++ counterpart of .NET Boolean.Parse(string).
         * Case-insensitive; leading/trailing whitespace is ignored.
         * @throws System::FormatException if the string is not recognised.
         */
        [[nodiscard]] static bool Parse(const std::string& s) {
            bool result;
            if (!TryParse(s, result))
                throw System::FormatException("String was not recognized as a valid Boolean.");
            return result;
        }

        /**
         * @brief Attempts to parse a string as a Boolean value.
         *
         * C++ counterpart of .NET Boolean.TryParse(string, out bool).
         * Case-insensitive; leading/trailing whitespace is ignored.
         * @return true if @p s was successfully parsed; false otherwise.
         */
        static bool TryParse(const std::string& s, bool& result) {
            auto notws = [](unsigned char c) { return !std::isspace(c); };
            auto b = std::find_if(s.begin(), s.end(), notws);
            auto e = std::find_if(s.rbegin(), s.rend(), notws).base();
            std::string t(b, e < b ? b : e);
            std::transform(t.begin(), t.end(), t.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (t == "true")  { result = true;  return true; }
            if (t == "false") { result = false; return true; }
            result = false; return false;
        }

        /**
         * @brief Converts a Boolean value to its string representation.
         *
         * C++ counterpart of .NET Boolean.ToString().
         * @return "True" if @p value is true; "False" otherwise.
         */
        [[nodiscard]] static std::string ToString(bool value) {
            return value ? "True" : "False";
        }

        /**
         * @brief Compares two Boolean values.
         *
         * C++ counterpart of .NET Boolean.CompareTo(bool).
         * @return Negative if a < b, zero if equal, positive if a > b.
         */
        [[nodiscard]] static int CompareTo(bool a, bool b) noexcept {
            return static_cast<int>(a) - static_cast<int>(b);
        }

        /**
         * @brief Returns true if the two Boolean values are equal.
         *
         * C++ counterpart of .NET Boolean.Equals(bool).
         */
        [[nodiscard]] static bool Equals(bool a, bool b) noexcept { return a == b; }

        /**
         * @brief Returns a hash code for the specified Boolean value.
         *
         * C++ counterpart of .NET Boolean.GetHashCode().
         */
        [[nodiscard]] static int GetHashCode(bool value) noexcept {
            return value ? 1 : 0;
        }
    };

} // namespace System
