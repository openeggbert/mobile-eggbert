// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System
{
    /**
     * @brief Provides utility methods similar to selected members of .NET System.String.
     *
     * This class is not a wrapper around std::string. Instead, it offers
     * static helper methods useful for source-porting C# code to C++.
     * All methods mirror .NET String members with the same name and semantics.
     *
     * @note Status: Reduced to the subset actually exercised by this codebase
     *   (IsNullOrEmpty, StartsWith(string,string), Split(string,char),
     *   Format(string,string), and the width/fill ToString(int,int,char) helper).
     *   See git history for the previously fuller .NET-parity surface (Compare,
     *   Contains, IndexOf/LastIndexOf family, Trim family, Pad family, casing,
     *   Concat/Join, Substring/Remove/Insert/Replace, GetHashCode, interning) if a
     *   future consumer needs it restored.
     */
    class String
    {
    public:
        /** @brief Deleted constructor — all members are static. */
        String() = delete;
        /** @brief Deleted destructor — class is not instantiable. */
        ~String() = delete;

        /**
         * @brief Returns true if @p value is empty.
         *
         * C++ counterpart of .NET String.IsNullOrEmpty where null maps to empty.
         */
        static bool IsNullOrEmpty(const std::string& value);

        /**
         * @brief Returns true if @p value starts with @p prefix.
         */
        static bool StartsWith(const std::string& value, const std::string& prefix);

        /**
         * @brief Splits @p value into substrings separated by @p delimiter.
         */
        static std::vector<std::string> Split(const std::string& value, char delimiter);

        /**
         * @brief Replaces the {0} placeholder in @p format with @p arg0.
         */
        static std::string Format(const std::string& format, const std::string& arg0);

        /**
         * @brief Right-justifies @p value in a field of @p width, padding on the left with @p fill.
         */
        static std::string ToString(SharpRuntime::intcs value, SharpRuntime::intcs width, char fill = '0');
    };

} // namespace System
