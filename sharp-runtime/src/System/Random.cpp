// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 5/28/25.
//

#include "System/Random.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System {

    // -----------------------------------------------------------------------
    // Knuth "subtractive" generator -- exact port of .NET's Random.CompatPrng
    // (Random.CompatImpl.cs). See the class doc-comment in Random.hpp for why
    // this specific algorithm was chosen over a faster generic PRNG.
    //
    // C#'s default arithmetic is *unchecked*: int subtraction/addition silently wraps
    // modulo 2^32 on overflow rather than throwing, and this algorithm's intermediate
    // values (e.g. the raw, never-corrected seedArray_[55] seed) rely on that wraparound
    // to reach the correct final state. Plain int32_t arithmetic would invoke C++ signed-
    // overflow UB for the same inputs, so every subtraction below is done in uint32_t
    // (where wraparound is guaranteed, defined behavior) and cast back with static_cast,
    // which C++20 guarantees reproduces the identical two's-complement bit pattern C#
    // produces -- i.e. bit-for-bit the same result, not just "no crash".
    // -----------------------------------------------------------------------

    namespace {
        inline int32_t wrappingSub(int32_t a, int32_t b) {
            return static_cast<int32_t>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
        }
    }

    void Random::initializeSeed(intcs seed) {
        int32_t subtraction = (seed == std::numeric_limits<int32_t>::min())
            ? std::numeric_limits<int32_t>::max()
            : std::abs(static_cast<int32_t>(seed));
        int32_t mj = 161803398 - subtraction;
        seedArray_[55] = mj;
        int32_t mk = 1;

        int ii = 0;
        for (int i = 1; i < 55; i++) {
            // The range [1..55] is special (Knuth) and so we're wasting the 0'th position.
            ii += 21;
            if (ii >= 55) ii -= 55;

            seedArray_[static_cast<size_t>(ii)] = mk;
            mk = wrappingSub(mj, mk);
            if (mk < 0) mk += std::numeric_limits<int32_t>::max();

            mj = seedArray_[static_cast<size_t>(ii)];
        }

        for (int k = 1; k < 5; k++) {
            for (int i = 1; i < 56; i++) {
                int n = i + 30;
                if (n >= 55) n -= 55;

                seedArray_[static_cast<size_t>(i)] =
                    wrappingSub(seedArray_[static_cast<size_t>(i)], seedArray_[static_cast<size_t>(1 + n)]);
                if (seedArray_[static_cast<size_t>(i)] < 0)
                    seedArray_[static_cast<size_t>(i)] += std::numeric_limits<int32_t>::max();
            }
        }

        inext_ = 0;
        inextp_ = 21;
    }

    int32_t Random::internalSample() {
        int locINext = inext_;
        if (++locINext >= 56) locINext = 1;

        int locINextp = inextp_;
        if (++locINextp >= 56) locINextp = 1;

        int32_t retVal = wrappingSub(seedArray_[static_cast<size_t>(locINext)], seedArray_[static_cast<size_t>(locINextp)]);

        if (retVal == std::numeric_limits<int32_t>::max()) retVal--;
        if (retVal < 0) retVal += std::numeric_limits<int32_t>::max();

        seedArray_[static_cast<size_t>(locINext)] = retVal;
        inext_ = locINext;
        inextp_ = locINextp;

        return retVal;
    }

    double Random::getSampleForLargeRange() {
        // The distribution of the double returned by prngSample() is not good enough for a
        // large range. If we used it directly for a range like [int.MinValue..int.MaxValue),
        // we'd end up getting even numbers only.
        int32_t result = internalSample();

        // Can't use addition here: the distribution would be bad if we did. Decide the sign
        // based on a second sample instead.
        if (internalSample() % 2 == 0) result = -result;

        double d = result;
        d += std::numeric_limits<int32_t>::max() - 1; // range [0..2*int.MaxValue-1)
        d /= 2.0 * static_cast<double>(std::numeric_limits<int32_t>::max()) - 1;
        return d;
    }

    // -----------------------------------------------------------------------
    // Construction
    // -----------------------------------------------------------------------

    Random::Random() {
        initializeSeed(static_cast<intcs>(std::random_device{}()));
    }

    Random::Random(intcs seed) {
        initializeSeed(seed);
    }

    // -----------------------------------------------------------------------
    // Int32
    // -----------------------------------------------------------------------

    intcs Random::Next()
    {
        return internalSample();
    }

    intcs Random::Next(intcs maxValue)
    {
        if (maxValue < 0)
        {
            throw ArgumentOutOfRangeException("maxValue");
        }

        return static_cast<intcs>(prngSample() * static_cast<double>(maxValue));
    }

    intcs Random::Next(intcs minValue, intcs maxValue)
    {
        if (minValue > maxValue)
        {
            throw ArgumentOutOfRangeException("minValue");
        }

        int64_t range = static_cast<int64_t>(maxValue) - static_cast<int64_t>(minValue);
        if (range <= std::numeric_limits<int32_t>::max())
        {
            return static_cast<intcs>(prngSample() * static_cast<double>(range)) + minValue;
        }
        return static_cast<intcs>(static_cast<int64_t>(getSampleForLargeRange() * static_cast<double>(range)) + minValue);
    }

} // namespace System
