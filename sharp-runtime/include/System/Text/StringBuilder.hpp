// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text
{
    using SharpRuntime::intcs;

    /**
     * <summary>
     * Provides a mutable string buffer for efficient string construction.
     *
     * Lightweight C++ emulation of .NET System.Text.StringBuilder, intended
     * primarily for source-porting convenience in the SharpRuntime layer.
     * Only a practical subset of the original .NET API is provided.
     * </summary>
     */
    class StringBuilder
    {
    private:
        std::string buffer; ///< Internal text buffer.

    public:
        /** Initializes a new empty instance of the StringBuilder class. */
        StringBuilder();

        /**
         * Initializes a new instance of the StringBuilder class with the specified initial text.
         * @param value Initial text.
         */
        explicit StringBuilder(const std::string& value);

        /** Removes all characters from the current instance. */
        void Clear();

        /**
         * Appends the specified string to this instance.
         * @param value String to append.
         * @return Reference to this instance.
         */
        StringBuilder& Append(const std::string& value);

        /**
         * Appends the specified null-terminated C string to this instance.
         * If @p value is nullptr, nothing is appended.
         * @param value C string to append.
         * @return Reference to this instance.
         */
        StringBuilder& Append(const char* value);

        /**
         * Appends the string representation of the specified integer value.
         * @param value Integer value to append.
         * @return Reference to this instance.
         */
        StringBuilder& Append(intcs value);

        /**
         * Returns the current contents of this instance as a string.
         * @return The accumulated string.
         */
        [[nodiscard]] std::string ToString() const;
    };
}
