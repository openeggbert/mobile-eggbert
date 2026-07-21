// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Numerics {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * @brief Represents an arbitrarily large signed integer.
     *
     * Uses sign-magnitude representation with a base-10^9 digit vector
     * (least-significant limb first).
     *
     * Partial C++ counterpart of .NET System.Numerics.BigInteger.
     *
     * @note Status: Full arithmetic — add, subtract, multiply, divide, modulo,
     *   comparisons, ToString, Parse, TryParse all implemented.
     *   Bitwise operations are not implemented.
     */
    class BigInteger {
        bool                  negative_ = false;
        std::vector<uint32_t> mag_;   ///< Base-10^9 limbs, least-significant first.

        static constexpr uint32_t BASE = 1000000000u; ///< 10^9

        // ------------------------------------------------------------------
        // Private magnitude helpers (defined in BigInteger.cpp)
        // ------------------------------------------------------------------
        void trim();

        static int  cmpMag(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b);
        static std::vector<uint32_t> addMag(const std::vector<uint32_t>& a,
                                            const std::vector<uint32_t>& b);
        static std::vector<uint32_t> subMag(const std::vector<uint32_t>& a,
                                            const std::vector<uint32_t>& b);
        static std::vector<uint32_t> mulMag(const std::vector<uint32_t>& a,
                                            const std::vector<uint32_t>& b);
        static std::vector<uint32_t> divMagByLimb(const std::vector<uint32_t>& a,
                                                   uint32_t b, uint32_t& rem);
        /**
         * Knuth Algorithm D: divide magnitude a by b (b has ≥2 limbs).
         * Returns {quotient, remainder} magnitudes.
         */
        static std::pair<std::vector<uint32_t>, std::vector<uint32_t>>
               divmodMag(std::vector<uint32_t> a, std::vector<uint32_t> b);

        static std::vector<uint32_t> fromUInt64(uint64_t v);

    public:
        // ------------------------------------------------------------------
        // Constructors
        // ------------------------------------------------------------------

        /** @brief Initialises to zero. */
        BigInteger();

        /** @brief Constructs from a signed 32-bit integer. */
        BigInteger(intcs v);  // NOLINT(*-explicit-*)

        /** @brief Constructs from a signed 64-bit integer. */
        BigInteger(longcs v); // NOLINT(*-explicit-*)

        // ------------------------------------------------------------------
        // Properties
        // ------------------------------------------------------------------

        /** @brief Returns @c true if the value is zero. */
        [[nodiscard]] bool getIsZeroProperty()     const;

        /** @brief Returns @c true if the value is one. */
        [[nodiscard]] bool getIsOneProperty()      const;

        /** @brief Returns @c true if the value is negative. */
        [[nodiscard]] bool getIsNegativeProperty() const;

        /** @brief Returns -1, 0, or +1 for negative, zero, positive. */
        [[nodiscard]] int Sign() const;

        // ------------------------------------------------------------------
        // Parse / ToString
        // ------------------------------------------------------------------

        /**
         * @brief Parses a decimal integer string (optional leading '+'/'-').
         *
         * @throws System::FormatException on malformed input.
         */
        static BigInteger Parse(const std::string& s);

        /**
         * @brief Attempts to parse a decimal integer string.
         *
         * @param s       Input string.
         * @param result  Receives the parsed value on success.
         * @return @c true on success, @c false if the string is malformed.
         */
        static bool TryParse(const std::string& s, BigInteger& result);

        /** @brief Returns the decimal string representation. */
        [[nodiscard]] std::string ToString() const;

        // ------------------------------------------------------------------
        // Arithmetic operators
        // ------------------------------------------------------------------
        BigInteger operator-() const;
        /** @brief Returns the absolute value of this BigInteger. */
        BigInteger Abs()       const;

        BigInteger operator+(const BigInteger& o) const;
        BigInteger operator-(const BigInteger& o) const;
        BigInteger operator*(const BigInteger& o) const;

        /**
         * @brief Integer division (truncates toward zero, mirrors .NET).
         * @throws System::DivideByZeroException if @p o is zero.
         */
        BigInteger operator/(const BigInteger& o) const;

        /**
         * @brief Remainder after integer division (sign follows dividend, mirrors .NET).
         * @throws System::DivideByZeroException if @p o is zero.
         */
        BigInteger operator%(const BigInteger& o) const;

        BigInteger& operator+=(const BigInteger& o);
        BigInteger& operator-=(const BigInteger& o);
        BigInteger& operator*=(const BigInteger& o);
        BigInteger& operator/=(const BigInteger& o);
        BigInteger& operator%=(const BigInteger& o);

        // ------------------------------------------------------------------
        // Comparison operators
        // ------------------------------------------------------------------
        bool operator==(const BigInteger& o) const;
        bool operator!=(const BigInteger& o) const;
        bool operator< (const BigInteger& o) const;
        bool operator<=(const BigInteger& o) const;
        bool operator> (const BigInteger& o) const;
        bool operator>=(const BigInteger& o) const;

        // ------------------------------------------------------------------
        // Manifest constants
        // ------------------------------------------------------------------
        static const BigInteger Zero;     ///< 0
        static const BigInteger One;      ///< 1
        static const BigInteger MinusOne; ///< -1
    };

} // namespace System::Numerics
