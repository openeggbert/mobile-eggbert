// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <random>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

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
        // Int overloads
        // -------------------------------------------------------------------------

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

    };

} // namespace System
