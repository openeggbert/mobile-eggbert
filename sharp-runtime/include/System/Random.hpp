// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Span.hpp"

namespace System {

    using SharpRuntime::intcs;
    using SharpRuntime::bytecs;
    using SharpRuntime::longcs;

    /**
     * @brief Represents a pseudo-random number generator.
     *
     * C++ counterpart of .NET System.Random. Provides methods for generating
     * pseudo-random integers, floating-point values, byte sequences, and strings.
     *
     * @note Determinism: the seeded constructor (`Random(intcs seed)`) is a byte-for-byte
     *   port of .NET's own seeded algorithm (the Knuth-derived "subtractive" generator that
     *   backs .NET's `Random(int seed)` since .NET Framework 1.0, kept bit-stable across every
     *   .NET version specifically so seeded sequences are reproducible) -- verified against
     *   live Mono reference output for `Next()`, `Next(int)`, `Next(int,int)`, `NextDouble()`,
     *   and `NextBytes()` across several seeds including 0, a negative seed, and
     *   `Int32.MinValue`. The default (unseeded) constructor uses the same generator seeded
     *   from a hardware entropy source; real .NET makes no reproducibility promise for its own
     *   unseeded case either (it uses a different, faster algorithm there), so there is no
     *   parity gap to have here.
     */
    class Random {
    private:
        // Exact port of .NET's Random.CompatPrng (Random.CompatImpl.cs): a 56-element
        // Knuth "subtractive" lagged-Fibonacci-style generator. Index 0 of seedArray_ is
        // permanently unused -- this wastes one slot but matches Knuth's original scheme
        // (and .NET's own array layout) exactly, which is what makes the seeded sequence
        // reproducible against real .NET output.
        std::array<int32_t, 56> seedArray_{};
        int inext_ = 0;
        int inextp_ = 21;

        void initializeSeed(intcs seed);

        /** @brief Returns the next raw sample in [0, Int32.MaxValue), advancing the generator. */
        int32_t internalSample();

        /** @brief Returns the next raw sample as a double in [0.0, 1.0). */
        double prngSample() { return internalSample() * (1.0 / static_cast<double>(SharpRuntime::INTCS_MAX)); }

        /** @brief Higher-quality (but costlier) sample used for out-of-int32-range Next(min,max). */
        double getSampleForLargeRange();

        /** @brief Assembles a uniformly-distributed 64-bit value from three Next() calls. */
        uint64_t nextUInt64();

    public:
        /** @brief Initializes a new instance using a random seed from the hardware device. */
        Random();

        /**
         * @brief Initializes a new instance with the specified seed value.
         * @param seed A number used to calculate a starting value for the pseudo-random sequence.
         */
        explicit Random(intcs seed);

        /** @brief Random instances are not copyable. */
        Random(const Random&) = delete;
        /** @brief Random instances are not copyable. */
        Random& operator=(const Random&) = delete;

        virtual ~Random() = default;

        /** @brief Move constructor. */
        Random(Random&&) noexcept = default;
        /** @brief Move-assignment operator. */
        Random& operator=(Random&&) noexcept = default;

        // -------------------------------------------------------------------------
        // Shared instance
        // -------------------------------------------------------------------------

        /**
         * @brief Gets a thread-safe shared Random instance usable from any call site.
         *
         * C++ counterpart of .NET Random.Shared. The returned reference is valid
         * for the lifetime of the process. The static instance is initialized
         * thread-safely on first access (C++11 guarantee).
         */
        static Random& getSharedProperty();

        // -------------------------------------------------------------------------
        // Int overloads
        // -------------------------------------------------------------------------

    protected:
        /**
         * @brief Returns a random floating-point number between 0.0 and 1.0 (exclusive).
         *
         * C++ counterpart of .NET Random.Sample(). Override in a subclass to provide a
         * custom distribution; the base implementation delegates to NextDouble().
         */
        virtual double Sample() { return NextDouble(); }

    public:
        /**
         * @brief Returns a non-negative random integer in [0, int.MaxValue).
         * @return A 32-bit signed integer ≥ 0 and < Int32.MaxValue.
         */
        virtual intcs Next();

        /**
         * @brief Returns a non-negative random integer in [0, maxValue).
         * @param maxValue Exclusive upper bound; must be ≥ 0.
         * @throws System::ArgumentOutOfRangeException if maxValue < 0.
         */
        virtual intcs Next(intcs maxValue);

        /**
         * @brief Returns a random integer in [minValue, maxValue).
         * @param minValue Inclusive lower bound.
         * @param maxValue Exclusive upper bound; must be ≥ minValue.
         * @throws System::ArgumentOutOfRangeException if minValue > maxValue.
         */
        virtual intcs Next(intcs minValue, intcs maxValue);

        // -------------------------------------------------------------------------
        // Int64 overloads
        // -------------------------------------------------------------------------

        /**
         * @brief Returns a non-negative random 64-bit integer in [0, Int64.MaxValue).
         * @return A 64-bit signed integer ≥ 0 and < Int64.MaxValue.
         */
        virtual longcs NextInt64();

        /**
         * @brief Returns a non-negative random 64-bit integer in [0, maxValue).
         * @param maxValue Exclusive upper bound; must be ≥ 0.
         * @throws System::ArgumentOutOfRangeException if maxValue < 0.
         */
        virtual longcs NextInt64(longcs maxValue);

        /**
         * @brief Returns a random 64-bit integer in [minValue, maxValue).
         * @param minValue Inclusive lower bound.
         * @param maxValue Exclusive upper bound; must be ≥ minValue.
         * @throws System::ArgumentOutOfRangeException if minValue > maxValue.
         */
        virtual longcs NextInt64(longcs minValue, longcs maxValue);

        // -------------------------------------------------------------------------
        // Generic integer overloads
        // -------------------------------------------------------------------------

        /**
         * @brief Returns a non-negative random integer of type T in [0, T::max].
         *
         * C++ counterpart of .NET Random.NextInteger<T>(). T must be an integral type.
         * Unlike Next(), the return range is inclusive of T::max.
         *
         * Matches .NET's own generic-math implementation: draws the minimum number of random
         * bytes needed to cover T's bit-width via NextBytes(), masks the top byte so an
         * unsigned reinterpretation of those bytes never sets bits beyond T::max's actual bit
         * pattern, and rejects/retries if the assembled value still exceeds T::max (needed
         * because for a signed T, T::max doesn't use the full bit pattern of its storage).
         * @tparam T An integral type (int, long, uint32_t, etc.).
         */
        template<typename T>
        T NextInteger() {
            static_assert(std::is_integral_v<T>, "T must be an integral type");
            using UType = std::make_unsigned_t<T>;
            constexpr T maxValue = std::numeric_limits<T>::max();
            if constexpr (maxValue == T{0}) {
                return T{0};
            } else {
                constexpr int bitLength = std::numeric_limits<T>::digits;
                constexpr int byteCount = (bitLength + 7) / 8;
                constexpr int topBits = bitLength % 8;
                constexpr uint8_t topMask = topBits == 0 ? uint8_t{0xFF} : static_cast<uint8_t>((1 << topBits) - 1);

                std::array<uint8_t, static_cast<size_t>(byteCount)> bytes{};
                for (;;) {
                    NextBytes(Span<bytecs>(bytes.data(), byteCount));
                    bytes[static_cast<size_t>(byteCount) - 1] &= topMask;
                    UType value{};
                    for (int i = byteCount - 1; i >= 0; --i)
                        value = static_cast<UType>((value << 8) | bytes[static_cast<size_t>(i)]);
                    if (value <= static_cast<UType>(maxValue)) return static_cast<T>(value);
                }
            }
        }

        /**
         * @brief Returns a non-negative random integer of type T in [0, maxValue).
         *
         * Matches .NET's own fast-path dispatch: types whose full range always fits within
         * Int32 (int8/uint8/int16/uint16/int32) route through Next(int); wider types whose
         * runtime value fits within Int64 route through NextInt64(long).
         * @param maxValue Exclusive upper bound; must be ≥ 0.
         * @throws System::ArgumentOutOfRangeException if maxValue < 0.
         * @throws System::NotSupportedException if T is wider than 64 bits, or is a 64-bit
         *         unsigned type whose @p maxValue exceeds Int64.MaxValue.
         */
        template<typename T>
        T NextInteger(T maxValue) {
            static_assert(std::is_integral_v<T>, "T must be an integral type");
            if (maxValue < T{0})
                throw ArgumentOutOfRangeException("maxValue");
            if (maxValue == T{0}) return T{0};

            if constexpr (sizeof(T) < sizeof(intcs) || (sizeof(T) == sizeof(intcs) && std::is_signed_v<T>)) {
                return static_cast<T>(Next(static_cast<intcs>(maxValue)));
            } else if constexpr (sizeof(T) <= sizeof(longcs)) {
                auto maxAsLong = static_cast<std::make_unsigned_t<longcs>>(std::numeric_limits<longcs>::max());
                if (static_cast<std::make_unsigned_t<T>>(maxValue) > maxAsLong)
                    throw NotSupportedException("NextInteger<T>(maxValue) does not support a maxValue exceeding Int64.MaxValue.");
                return static_cast<T>(NextInt64(static_cast<longcs>(maxValue)));
            } else {
                throw NotSupportedException("NextInteger<T>(maxValue) is not supported for integer types wider than 64 bits.");
            }
        }

        /**
         * @brief Returns a random integer of type T in [minValue, maxValue).
         * @param minValue Inclusive lower bound.
         * @param maxValue Exclusive upper bound; must be ≥ minValue.
         * @throws System::ArgumentOutOfRangeException if minValue > maxValue.
         * @throws System::NotSupportedException if `maxValue - minValue` overflows T, or T is
         *         unsupported per the single-argument NextInteger(T) overload above.
         */
        template<typename T>
        T NextInteger(T minValue, T maxValue) {
            static_assert(std::is_integral_v<T>, "T must be an integral type");
            if (minValue > maxValue)
                throw ArgumentOutOfRangeException("minValue");
            if (minValue == maxValue) return minValue;
            if constexpr (std::is_signed_v<T>) {
                using UType = std::make_unsigned_t<T>;
                UType range = static_cast<UType>(static_cast<UType>(maxValue) - static_cast<UType>(minValue));
                if (range > static_cast<UType>(std::numeric_limits<T>::max()))
                    throw NotSupportedException("NextInteger<T>(minValue, maxValue) does not support a range wider than T::max.");
                return static_cast<T>(NextInteger<T>(static_cast<T>(range)) + minValue);
            } else {
                return static_cast<T>(NextInteger<T>(static_cast<T>(maxValue - minValue)) + minValue);
            }
        }

        // -------------------------------------------------------------------------
        // Floating-point
        // -------------------------------------------------------------------------

        /**
         * @brief Returns a random floating-point value of type T in [0.0, 1.0).
         *
         * C++ counterpart of .NET Random.NextBinaryFloat<T>().
         * Supported types: float → NextSingle(); all other floating-point types → NextDouble().
         * @tparam T A floating-point type (float, double, long double).
         */
        template<typename T>
        T NextBinaryFloat() {
            static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");
            if constexpr (std::is_same_v<T, float>) return NextSingle();
            else                                     return static_cast<T>(NextDouble());
        }

        /**
         * @brief Returns a random single-precision float in [0.0f, 1.0f).
         * @return A float ≥ 0.0f and < 1.0f.
         */
        virtual float NextSingle();

        /**
         * @brief Returns a random double-precision float in [0.0, 1.0).
         * @return A double ≥ 0.0 and < 1.0.
         */
        virtual double NextDouble();

        // -------------------------------------------------------------------------
        // Byte buffers
        // -------------------------------------------------------------------------

        /**
         * @brief Fills a byte vector with random values.
         * @param buffer Vector to fill; its size determines the number of bytes generated.
         */
        virtual void NextBytes(std::vector<bytecs>& buffer);

        /**
         * @brief Fills a Span<byte> with random values.
         *
         * C++ counterpart of .NET Random.NextBytes(Span<byte>).
         * @param buffer Span over mutable storage to fill with random bytes.
         */
        virtual void NextBytes(Span<bytecs> buffer);

        // -------------------------------------------------------------------------
        // Collection utilities
        // -------------------------------------------------------------------------

        /**
         * @brief Performs an in-place Fisher-Yates shuffle of a Span.
         *
         * C++ counterpart of .NET Random.Shuffle<T>(Span<T>).
         * @tparam T Element type.
         * @param values Span over the elements to shuffle in place.
         */
        template<typename T>
        void Shuffle(Span<T> values) {
            for (intcs i = 0; i < values.getLengthProperty() - 1; ++i) {
                intcs j = Next(i, values.getLengthProperty());
                std::swap(values[i], values[j]);
            }
        }

        /**
         * @brief Performs an in-place Fisher-Yates shuffle of a vector.
         *
         * C++ counterpart of .NET Random.Shuffle<T>(T[]).
         * Real .NET's array overload delegates entirely to the Span overload
         * (`Shuffle(new Span<T>(...))`) rather than implementing its own algorithm -- there is
         * only one shuffle algorithm in real .NET. This previously implemented an independent,
         * opposite-direction traditional Fisher-Yates loop (i from n-1 down to 1, j=Next(i+1)),
         * which produces a genuinely different permutation than Shuffle(Span<T>) for the same
         * seed and input size, breaking this file's carefully-verified seeded-determinism
         * guarantee (see the class-level doc comment) for any caller mixing the two overloads
         * or comparing against real .NET's own array-shuffle output.
         * @tparam T Element type.
         * @param values The vector to shuffle in place.
         */
        template<typename T>
        void Shuffle(std::vector<T>& values) {
            Shuffle(Span<T>(values.data(), static_cast<intcs>(values.size())));
        }

        /**
         * @brief Returns a vector of length @p length filled with elements chosen
         * randomly (with replacement) from @p choices.
         *
         * C++ counterpart of .NET Random.GetItems<T>(ReadOnlySpan<T>, int).
         * @tparam T Element type.
         * @param choices Non-empty source collection to sample from.
         * @param length  Number of elements to pick.
         * @return A new vector of size @p length.
         * @throws ArgumentException if @p choices is empty.
         * @throws ArgumentOutOfRangeException if @p length is negative.
         */
        template<typename T>
        std::vector<T> GetItems(const std::vector<T>& choices, intcs length) {
            return GetItems(ReadOnlySpan<T>(choices.data(), static_cast<intcs>(choices.size())), length);
        }

        /**
         * @brief Returns a vector of @p length elements chosen randomly (with replacement)
         * from a ReadOnlySpan.
         *
         * C++ counterpart of .NET Random.GetItems<T>(ReadOnlySpan<T>, int).
         * @tparam T Element type.
         * @param choices Non-empty source span to sample from.
         * @param length  Number of elements to pick.
         * @return A new vector of size @p length.
         * @throws ArgumentException if @p choices is empty.
         * @throws ArgumentOutOfRangeException if @p length is negative.
         */
        template<typename T>
        std::vector<T> GetItems(ReadOnlySpan<T> choices, intcs length) {
            if (length < 0) throw ArgumentOutOfRangeException("length", "'length' must be a non-negative value.");
            std::vector<T> result(static_cast<std::size_t>(length));
            GetItems(choices, Span<T>(result.data(), length));
            return result;
        }

        /**
         * @brief Fills @p destination with elements chosen randomly (with replacement)
         * from @p choices.
         *
         * C++ counterpart of .NET Random.GetItems<T>(ReadOnlySpan<T>, Span<T>).
         * @tparam T Element type.
         * @param choices     Non-empty source span to sample from.
         * @param destination Span to fill; its length determines how many items are picked.
         * @throws ArgumentException if @p choices is empty.
         */
        template<typename T>
        void GetItems(ReadOnlySpan<T> choices, Span<T> destination) {
            if (choices.getLengthProperty() == 0) throw ArgumentException("Span may not be empty.", "choices");
            for (intcs i = 0; i < destination.getLengthProperty(); ++i)
                destination[i] = choices[Next(choices.getLengthProperty())];
        }

        // -------------------------------------------------------------------------
        // String utilities
        // -------------------------------------------------------------------------

        /**
         * @brief Returns a random string of @p length characters drawn from @p choices.
         *
         * C++ counterpart of .NET Random.GetString(ReadOnlySpan<char>, int).
         * @param choices Non-empty string of characters to sample from.
         * @param length  Number of characters in the returned string.
         * @return A new std::string of size @p length.
         * @throws ArgumentException if @p choices is empty.
         * @throws ArgumentOutOfRangeException if @p length is negative.
         */
        std::string GetString(const std::string& choices, intcs length);

        /**
         * @brief Returns a random hexadecimal string of the specified length.
         *
         * C++ counterpart of .NET Random.GetHexString(int, bool).
         * @param stringLength Number of hex characters to generate.
         * @param lowercase    If true, uses lowercase hex digits (a–f); otherwise uppercase (A–F).
         * @return A std::string of @p stringLength hex characters.
         */
        std::string GetHexString(intcs stringLength, bool lowercase = false);

        /**
         * @brief Fills @p destination with random hexadecimal characters.
         *
         * C++ counterpart of .NET Random.GetHexString(Span<char>, bool).
         * @param destination Span of char to fill with hex characters.
         * @param lowercase   If true, uses lowercase (a–f); otherwise uppercase (A–F).
         */
        void GetHexString(Span<char> destination, bool lowercase = false);
    };

} // namespace System
