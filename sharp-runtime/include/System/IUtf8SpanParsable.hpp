// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <string>
#include "System/Span.hpp"

namespace System {

    class IFormatProvider;

    /**
     * @brief Defines a mechanism for parsing a span of UTF-8 characters to a value.
     *
     * C++ counterpart of .NET System.IUtf8SpanParsable<TSelf>.
     * In .NET this interface requires @c static abstract members, which C++ does
     * not support on base classes. The interface is represented as a CRTP-style
     * abstract base with virtual methods so that concrete types can be tested
     * polymorphically; an implementing type would still normally also expose real
     * `static` Parse/TryParse methods (as System::Guid does) for the ergonomic
     * call site.
     *
     * The methods are named ParseUtf8/TryParseUtf8, not Parse/TryParse: .NET's
     * IUtf8SpanParsable&lt;TSelf&gt; does not extend IParsable&lt;TSelf&gt; (they are
     * independent interfaces C# tells apart via parameter-type overload
     * resolution), and a C++ type implementing both would otherwise need
     * `using` declarations in every derived class to un-hide same-named
     * methods declared in two unrelated base classes.
     *
     * @tparam TSelf The concrete type that implements this interface.
     */
    template<typename TSelf>
    class IUtf8SpanParsable {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IUtf8SpanParsable() = default;

        /**
         * @brief Parses a span of UTF-8 bytes into a value.
         *
         * C++ counterpart of .NET IUtf8SpanParsable<TSelf>.Parse(ReadOnlySpan<byte>, IFormatProvider?),
         * with the provider argument omitted. Kept as the single required override so
         * existing implementers are unaffected; see the two-argument overload below for
         * API parity.
         * @param utf8Text A view of UTF-8 encoded characters to parse.
         * @return The parsed value.
         * @throws System::FormatException if @p utf8Text is not in the correct format.
         * @throws System::OverflowException if @p utf8Text is not representable by TSelf.
         */
        [[nodiscard]] virtual TSelf ParseUtf8(const ReadOnlySpan<uint8_t>& utf8Text) const = 0;

        /**
         * @brief Parses a span of UTF-8 bytes into a value, using culture-specific format
         * information.
         *
         * C++ counterpart of .NET IUtf8SpanParsable<TSelf>.Parse(ReadOnlySpan<byte>, IFormatProvider?).
         * The default implementation ignores @p provider and forwards to the
         * one-argument ParseUtf8(); override to honor culture-specific parsing.
         * @param utf8Text A view of UTF-8 encoded characters to parse.
         * @param provider Culture-specific format information, or nullptr for invariant.
         * @return The parsed value.
         */
        [[nodiscard]] virtual TSelf ParseUtf8(const ReadOnlySpan<uint8_t>& utf8Text,
                                               const IFormatProvider* provider) const {
            (void)provider;
            return ParseUtf8(utf8Text);
        }

        /**
         * @brief Tries to parse a span of UTF-8 bytes into a value.
         *
         * C++ counterpart of .NET IUtf8SpanParsable<TSelf>.TryParse(ReadOnlySpan<byte>, IFormatProvider?, out TSelf),
         * with the provider argument omitted. Kept as the single required override so
         * existing implementers are unaffected; see the three-argument overload below
         * for API parity.
         * @param utf8Text A view of UTF-8 encoded characters to parse.
         * @param result   On success, the parsed value; on failure, a default value.
         * @return true if parsing succeeded; false otherwise.
         */
        virtual bool TryParseUtf8(const ReadOnlySpan<uint8_t>& utf8Text, TSelf& result) const noexcept = 0;

        /**
         * @brief Tries to parse a span of UTF-8 bytes into a value, using culture-specific
         * format information.
         *
         * C++ counterpart of .NET IUtf8SpanParsable<TSelf>.TryParse(ReadOnlySpan<byte>, IFormatProvider?, out TSelf).
         * The default implementation ignores @p provider and forwards to the
         * two-argument TryParseUtf8(); override to honor culture-specific parsing.
         * @param utf8Text A view of UTF-8 encoded characters to parse.
         * @param provider Culture-specific format information, or nullptr for invariant.
         * @param result   On success, the parsed value; on failure, a default value.
         * @return true if parsing succeeded; false otherwise.
         */
        virtual bool TryParseUtf8(const ReadOnlySpan<uint8_t>& utf8Text, const IFormatProvider* provider,
                                   TSelf& result) const noexcept {
            (void)provider;
            return TryParseUtf8(utf8Text, result);
        }
    };

} // namespace System
