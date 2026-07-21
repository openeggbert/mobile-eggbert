// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Span.hpp"

namespace System {

    class IFormatProvider;

    /**
     * @brief Provides functionality to format the string representation of an object
     * into a span of UTF-8 bytes.
     *
     * C++ counterpart of .NET System.IUtf8SpanFormattable (introduced in .NET 8).
     * Unlike IParsable&lt;TSelf&gt;/ISpanParsable&lt;TSelf&gt;, .NET's interface is not
     * generic — TryFormat never returns TSelf, so no CRTP type parameter is needed here
     * either (a previous version of this header incorrectly added one).
     *
     * The method is named TryFormatUtf8, not TryFormat, so that a type implementing both
     * this interface and ISpanFormattable can do so unambiguously; .NET can tell the two
     * apart via `Span<byte>` vs `Span<char>` overload resolution plus explicit interface
     * implementation, a combination C++ virtual dispatch doesn't support as cleanly.
     */
    class IUtf8SpanFormattable {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IUtf8SpanFormattable() = default;

        /**
         * @brief Tries to format the value of the current instance as UTF-8 into
         * the provided span of bytes.
         *
         * C++ counterpart of .NET IUtf8SpanFormattable.TryFormat(Span&lt;byte&gt;, out int,
         * ReadOnlySpan&lt;char&gt;, IFormatProvider), with the provider argument omitted.
         * Kept as the single required override so existing implementers are unaffected;
         * see the four-argument overload below for API parity.
         * @param utf8Destination  Destination span to write UTF-8 bytes into.
         * @param bytesWritten     Receives the number of bytes written on success.
         * @param format           Optional format string (may be empty).
         * @return true if formatting succeeded and the output fits; false otherwise.
         */
        virtual bool TryFormatUtf8(Span<uint8_t> utf8Destination, SharpRuntime::intcs& bytesWritten,
                                    const std::string& format) const = 0;

        /**
         * @brief Tries to format the value of the current instance as UTF-8 into the
         * provided span of bytes, using culture-specific format information.
         *
         * C++ counterpart of .NET IUtf8SpanFormattable.TryFormat(Span&lt;byte&gt;, out int,
         * ReadOnlySpan&lt;char&gt;, IFormatProvider). The default implementation ignores
         * @p provider and forwards to the three-argument TryFormatUtf8(); override to
         * honor culture-specific formatting.
         * @param utf8Destination  Destination span to write UTF-8 bytes into.
         * @param bytesWritten     Receives the number of bytes written on success.
         * @param format           Optional format string (may be empty).
         * @param provider         Culture-specific format information, or nullptr for invariant.
         * @return true if formatting succeeded and the output fits; false otherwise.
         */
        virtual bool TryFormatUtf8(Span<uint8_t> utf8Destination, SharpRuntime::intcs& bytesWritten,
                                    const std::string& format,
                                    const IFormatProvider* provider) const {
            (void)provider;
            return TryFormatUtf8(utf8Destination, bytesWritten, format);
        }
    };

} // namespace System
