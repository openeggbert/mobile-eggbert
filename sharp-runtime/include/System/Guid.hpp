// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <array>
#include <string>
#include <cstdint>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Span.hpp"

namespace System {

    using SharpRuntime::intcs;
    using SharpRuntime::uintcs;
    using SharpRuntime::shortcs;
    using SharpRuntime::ushortcs;
    using SharpRuntime::bytecs;

    class IFormatProvider;
    class DateTimeOffset;

    /**
     * @brief Represents a globally unique identifier (GUID).
     *
     * C++ counterpart of .NET System.Guid.
     *
     * @note Status: Partial. Deviations from .NET:
     *   - Not a polymorphic IEquatable&lt;Guid&gt;/IComparable&lt;Guid&gt;/ISpanFormattable/
     *     IParsable&lt;Guid&gt; implementer (matching this codebase's DateTime/Half
     *     convention over TimeSpan's): equivalent named methods are provided instead so
     *     Guid stays a trivially-copyable 16-byte value, which also matters for
     *     ToByteArray()/TryWriteBytes() bit-for-bit correctness.
     *   - The legacy "compat" parsing quirks .NET's D/N/X parsers accept (embedded
     *     "0x"/"+" prefixes inside a fixed-width component, e.g. "0xdddddd-0xdd-...")
     *     are not implemented; .NET's own source documents these as "incredibly rare".
     *   - Only ASCII whitespace is trimmed/eaten during parsing (.NET trims/eats any
     *     Unicode whitespace character).
     *   - Numeric-component overflow in the "X" format always throws
     *     System::OverflowException. .NET's actual behavior is inconsistent here
     *     depending on entry point (the Guid(string) constructor throws
     *     OverflowException, but Guid.Parse/ParseExact throw a generic FormatException
     *     for the same input) — an accidental quirk of its throw-style plumbing that is
     *     not worth reproducing.
     *   - Equals(object)/CompareTo(object) are omitted; there is no boxing/object
     *     equivalent in this runtime for a bare value type.
     */
    class Guid {
    private:
        // Canonical field-order bytes: big-endian representation of the Guid's _a/_b/_c
        // integer fields followed directly by the 8 byte fields _d.._k. This is exactly
        // the byte sequence shown (as hex) in the "D"/"N"/"B"/"P" string forms, and
        // equals .NET's ToByteArray(bigEndian: true). Kept distinct from .NET's own
        // internal (native/little-endian) field layout so that string parsing/formatting
        // and CompareTo (plain lexicographic byte compare) fall out naturally.
        std::array<bytecs, 16> bytes_;

        [[nodiscard]] static Guid FromCanonicalBytes(const std::array<bytecs, 16>& canonicalBytes) {
            Guid g;
            g.bytes_ = canonicalBytes;
            return g;
        }

        [[nodiscard]] uintcs getAProperty() const noexcept {
            return (static_cast<uintcs>(bytes_[0]) << 24) | (static_cast<uintcs>(bytes_[1]) << 16) |
                   (static_cast<uintcs>(bytes_[2]) << 8)  |  static_cast<uintcs>(bytes_[3]);
        }
        [[nodiscard]] ushortcs getBProperty() const noexcept {
            return static_cast<ushortcs>((bytes_[4] << 8) | bytes_[5]);
        }
        [[nodiscard]] ushortcs getCProperty() const noexcept {
            return static_cast<ushortcs>((bytes_[6] << 8) | bytes_[7]);
        }

        [[nodiscard]] std::string ToStringCore(const std::string& format) const;

    public:
        /** @brief Initializes a new Guid with all bytes set to zero. */
        Guid();

        /**
         * @brief Creates a new Guid from a 16-byte array.
         *
         * C++ counterpart of .NET Guid(byte[]) / Guid(ReadOnlySpan&lt;byte&gt;). The
         * bytes are interpreted the same way .NET's default ToByteArray() produces them
         * (native little-endian for the first three components).
         */
        explicit Guid(const std::array<bytecs, 16>& b);

        /**
         * @brief Creates a new Guid from a 16-byte array in the specified byte order.
         *
         * C++ counterpart of .NET Guid(ReadOnlySpan&lt;byte&gt;, bool bigEndian).
         */
        Guid(const std::array<bytecs, 16>& b, bool bigEndian);

        /**
         * @brief Creates a new Guid from a 16-byte span.
         *
         * C++ counterpart of .NET Guid(ReadOnlySpan&lt;byte&gt;).
         * @throws System::ArgumentException if @p b does not have length 16.
         */
        explicit Guid(const ReadOnlySpan<bytecs>& b);

        /**
         * @brief Creates a new Guid from a 16-byte span in the specified byte order.
         *
         * C++ counterpart of .NET Guid(ReadOnlySpan&lt;byte&gt;, bool bigEndian).
         * @throws System::ArgumentException if @p b does not have length 16.
         */
        Guid(const ReadOnlySpan<bytecs>& b, bool bigEndian);

        /**
         * @brief Creates a new Guid initialized to the value represented by the arguments.
         *
         * C++ counterpart of .NET Guid(uint, ushort, ushort, byte, byte, byte, byte, byte, byte, byte, byte).
         */
        Guid(uintcs a, ushortcs b, ushortcs c,
             bytecs d, bytecs e, bytecs f, bytecs g, bytecs h, bytecs i, bytecs j, bytecs k);

        /**
         * @brief Creates a new Guid initialized to the value represented by the arguments.
         *
         * C++ counterpart of .NET Guid(int, short, short, byte[]). The fixed-size
         * std::array gives the length-8 requirement compile-time enforcement instead
         * of a runtime ArgumentException.
         */
        Guid(intcs a, shortcs b, shortcs c, const std::array<bytecs, 8>& d);

        /**
         * @brief Creates a new Guid initialized to the value represented by the arguments.
         *
         * C++ counterpart of .NET Guid(int, short, short, byte, byte, byte, byte, byte, byte, byte, byte).
         * The bytes are specified individually to avoid endianness issues.
         */
        Guid(intcs a, shortcs b, shortcs c,
             bytecs d, bytecs e, bytecs f, bytecs g, bytecs h, bytecs i, bytecs j, bytecs k);

        /**
         * @brief Creates a new Guid from its string representation.
         *
         * C++ counterpart of .NET Guid(string). Accepts "N", "D", "B", "P" and "X"
         * formats, auto-detected as .NET does.
         * @throws System::FormatException if the string is not a recognized Guid format.
         * @throws System::OverflowException if a numeric component of the "X" format overflows.
         */
        explicit Guid(const std::string& guidString);

        /** @brief Represents a Guid whose value is all zeros. */
        static const Guid Empty;

        /**
         * @brief Gets a Guid where all bits are set: FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF.
         *
         * C++ counterpart of .NET Guid.AllBitsSet.
         */
        [[nodiscard]] static Guid getAllBitsSetProperty();

        /**
         * @brief Creates a new Guid with a random value (RFC 4122 version 4).
         */
        [[nodiscard]] static Guid NewGuid();

        /**
         * @brief Creates a new Guid according to RFC 9562, following the Version 7 format,
         * using the current UTC time.
         *
         * C++ counterpart of .NET Guid.CreateVersion7().
         */
        [[nodiscard]] static Guid CreateVersion7();

        /**
         * @brief Creates a new Guid according to RFC 9562, following the Version 7 format,
         * using the given timestamp.
         *
         * C++ counterpart of .NET Guid.CreateVersion7(DateTimeOffset).
         * @throws System::ArgumentOutOfRangeException if @p timestamp is prior to the Unix epoch.
         */
        [[nodiscard]] static Guid CreateVersion7(const DateTimeOffset& timestamp);

        /**
         * @brief Gets the value of the variant field for this Guid.
         *
         * C++ counterpart of .NET Guid.Variant.
         */
        [[nodiscard]] intcs getVariantProperty() const noexcept { return bytes_[8] >> 4; }

        /**
         * @brief Gets the value of the version field for this Guid.
         *
         * C++ counterpart of .NET Guid.Version.
         */
        [[nodiscard]] intcs getVersionProperty() const noexcept { return bytes_[6] >> 4; }

        /** @brief Parses a Guid from its string representation. Throws FormatException on invalid input. */
        [[nodiscard]] static Guid Parse(const std::string& s);
        /** @brief Parses a Guid from a character span. Throws FormatException on invalid input. */
        [[nodiscard]] static Guid Parse(const ReadOnlySpan<char>& s);
        /** @brief Parses a Guid from a UTF-8 byte span. Throws FormatException on invalid input. */
        [[nodiscard]] static Guid Parse(const ReadOnlySpan<bytecs>& utf8Text);
        /** @brief C++ counterpart of .NET Guid.Parse(string, IFormatProvider). The provider is ignored. */
        [[nodiscard]] static Guid Parse(const std::string& s, const IFormatProvider* provider);

        /** @brief Tries to parse a Guid; returns false without throwing if the format is invalid. */
        static bool TryParse(const std::string& s, Guid& result);
        /** @brief Tries to parse a Guid from a character span. */
        static bool TryParse(const ReadOnlySpan<char>& s, Guid& result);
        /** @brief Tries to parse a Guid from a UTF-8 byte span. */
        static bool TryParse(const ReadOnlySpan<bytecs>& utf8Text, Guid& result);
        /** @brief C++ counterpart of .NET Guid.TryParse(string, IFormatProvider, out Guid). The provider is ignored. */
        static bool TryParse(const std::string& s, const IFormatProvider* provider, Guid& result);

        /**
         * @brief Parses a Guid using the exact specified format.
         *
         * C++ counterpart of .NET Guid.ParseExact(string, string). @p format must be
         * one of "N", "D", "B", "P", "X" (case-insensitive).
         * @throws System::FormatException if @p s does not match @p format exactly.
         */
        [[nodiscard]] static Guid ParseExact(const std::string& s, const std::string& format);

        /** @brief Tries to parse a Guid using the exact specified format. */
        static bool TryParseExact(const std::string& s, const std::string& format, Guid& result);

        /**
         * @brief Returns the string representation in the form
         * "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
         */
        [[nodiscard]] std::string ToString() const;

        /**
         * @brief Returns the string representation in the specified format.
         *
         * Supported formats: "N" (no hyphens), "D" (default, with hyphens), "B"
         * ({braces}), "P" ((parentheses)), "X" (hex literal / comma-separated list).
         * @throws System::FormatException if @p format is not one of the above.
         */
        [[nodiscard]] std::string ToString(const std::string& format) const;

        /**
         * @brief Formats the value of this Guid using the specified format and
         * culture-specific format information.
         *
         * C++ counterpart of .NET Guid.ToString(string, IFormatProvider). The provider
         * is ignored (Guid formatting is always invariant).
         */
        [[nodiscard]] std::string ToString(const std::string& format, const IFormatProvider* provider) const;

        /**
         * @brief Tries to format this Guid into the provided character span.
         *
         * C++ counterpart of .NET Guid.TryFormat(Span&lt;char&gt;, out int, ReadOnlySpan&lt;char&gt;).
         * @throws System::FormatException if @p format is not a recognized format.
         */
        bool TryFormat(Span<char> destination, intcs& charsWritten, const std::string& format = "") const;

        /**
         * @brief Tries to format this Guid as UTF-8 into the provided byte span.
         *
         * C++ counterpart of .NET Guid.TryFormat(Span&lt;byte&gt;, out int, ReadOnlySpan&lt;char&gt;)
         * (the IUtf8SpanFormattable overload). Guid's formatted output is always ASCII,
         * so UTF-8 encoding is a direct byte copy.
         * @throws System::FormatException if @p format is not a recognized format.
         */
        bool TryFormatUtf8(Span<bytecs> utf8Destination, intcs& bytesWritten, const std::string& format = "") const;

        /**
         * @brief Returns a 16-element byte array containing this Guid, in native
         * (little-endian) component order — matching .NET's default ToByteArray().
         */
        [[nodiscard]] std::array<bytecs, 16> ToByteArray() const;

        /**
         * @brief Returns a 16-element byte array containing this Guid, in the
         * requested byte order.
         *
         * C++ counterpart of .NET Guid.ToByteArray(bool).
         */
        [[nodiscard]] std::array<bytecs, 16> ToByteArray(bool bigEndian) const;

        /** @brief Tries to write this Guid's bytes (native order) to @p destination. */
        bool TryWriteBytes(Span<bytecs> destination) const;

        /** @brief Tries to write this Guid's bytes, in the requested byte order, to @p destination. */
        bool TryWriteBytes(Span<bytecs> destination, bool bigEndian, intcs& bytesWritten) const;

        /**
         * @brief Returns a hash code for this Guid.
         *
         * C++ counterpart of .NET Guid.GetHashCode(): XORs the four 32-bit words of
         * the native (little-endian) byte representation together.
         */
        [[nodiscard]] intcs GetHashCode() const;

        /** @brief Returns true if this Guid is equal to the specified Guid. */
        [[nodiscard]] bool Equals(const Guid& other) const noexcept { return bytes_ == other.bytes_; }

        /**
         * @brief Compares this Guid to another.
         * @return negative if less, 0 if equal, positive if greater.
         */
        [[nodiscard]] intcs CompareTo(const Guid& other) const noexcept;

        bool operator==(const Guid& other) const noexcept { return bytes_ == other.bytes_; }
        bool operator!=(const Guid& other) const noexcept { return bytes_ != other.bytes_; }
        bool operator< (const Guid& other) const noexcept { return bytes_ <  other.bytes_; }
        bool operator<=(const Guid& other) const noexcept { return bytes_ <= other.bytes_; }
        bool operator> (const Guid& other) const noexcept { return bytes_ >  other.bytes_; }
        bool operator>=(const Guid& other) const noexcept { return bytes_ >= other.bytes_; }
    };

} // namespace System
