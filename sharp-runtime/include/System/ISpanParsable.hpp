// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IParsable.hpp"
#include "System/Span.hpp"

namespace System {

    class IFormatProvider;

    /**
     * @brief Defines a mechanism for parsing a span of characters to a value.
     *
     * C++ counterpart of .NET System.ISpanParsable&lt;TSelf&gt; (.NET 7+).
     * Extends IParsable&lt;TSelf&gt; with overloads that accept a ReadOnlySpan&lt;char&gt;
     * instead of a string, allowing parsing from character spans without a heap
     * allocation.
     *
     * Because C++ has no static abstract interface members, both Parse and TryParse
     * are virtual (instance) methods here. Concrete types override all four methods:
     * the two inherited from IParsable&lt;TSelf&gt; (std::string overloads) and the two
     * added here (ReadOnlySpan&lt;char&gt; overloads).
     *
     * @tparam TSelf The type that implements this interface (CRTP pattern).
     */
    template<typename TSelf>
    class ISpanParsable : public IParsable<TSelf> {
    public:
        virtual ~ISpanParsable() = default;

        // Without these, declaring Parse/TryParse below would hide IParsable<TSelf>'s
        // std::string overloads from name lookup on this type (C++ name-hiding, not
        // overload resolution) — breaking the "concrete types override all four methods"
        // contract documented above, since callers couldn't reach the string overloads
        // through an ISpanParsable<TSelf>-typed reference/pointer.
        using IParsable<TSelf>::Parse;
        using IParsable<TSelf>::TryParse;

        /**
         * @brief Parses a span of characters into a value of type TSelf.
         *
         * C++ counterpart of .NET ISpanParsable&lt;TSelf&gt;.Parse(ReadOnlySpan&lt;char&gt;, IFormatProvider).
         * @param s        The character span to parse.
         * @param provider An optional format-provider; may be nullptr.
         * @return The parsed value.
         * @throws System::FormatException if @p s is not in the correct format.
         * @throws System::OverflowException if @p s is not representable by TSelf.
         */
        virtual TSelf Parse(const ReadOnlySpan<char>& s,
                            const IFormatProvider* provider) = 0;

        /**
         * @brief Tries to parse a span of characters into a value of type TSelf.
         *
         * C++ counterpart of .NET ISpanParsable&lt;TSelf&gt;.TryParse(ReadOnlySpan&lt;char&gt;, IFormatProvider, out TSelf).
         * @param s        The character span to parse.
         * @param provider An optional format-provider; may be nullptr.
         * @param result   On success, receives the parsed value.
         * @return true if parsing succeeded; false otherwise.
         */
        virtual bool TryParse(const ReadOnlySpan<char>& s,
                               const IFormatProvider* provider,
                               TSelf& result) = 0;
    };

} // namespace System
