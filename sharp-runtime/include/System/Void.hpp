// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>

namespace System {

    /**
     * @brief Indicates a method that does not return a value; that is, the
     * method has the void return type.
     *
     * C++ counterpart of .NET System.Void struct.
     * This type exists primarily to allow representation of @c void as a
     * generic type argument (e.g. @c Nullable<Void>, @c Task<Void>) when
     * porting C# code that uses reflection or generic programming patterns
     * that require an actual type where @c void would normally appear.
     *
     * @note You cannot use System::Void in C++ code directly as a variable
     * type in the same way as @c void, but you can use it as a template
     * argument where a concrete type is required.
     */
    struct Void {
        /**
         * @brief Returns an empty string.
         *
         * C++ convenience method; System.Void has no documented ToString in .NET.
         */
        [[nodiscard]] std::string ToString() const { return ""; }

        /** @brief Two Void values are always equal. */
        bool operator==(const Void&) const noexcept { return true; }
        /** @brief Two Void values are never unequal. */
        bool operator!=(const Void&) const noexcept { return false; }
    };

} // namespace System
