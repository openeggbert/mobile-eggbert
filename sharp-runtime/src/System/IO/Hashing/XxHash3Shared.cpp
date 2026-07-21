// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/XxHash3Shared.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <cstring>

namespace System::IO::Hashing::Detail::XxHash3Shared {

    const bytecs DefaultSecret[SecretLengthBytes] = {
        0xb8, 0xfe, 0x6c, 0x39, 0x23, 0xa4, 0x4b, 0xbe, // DefaultSecretUInt64_0
        0x7c, 0x01, 0x81, 0x2c, 0xf7, 0x21, 0xad, 0x1c, // DefaultSecretUInt64_1
        0xde, 0xd4, 0x6d, 0xe9, 0x83, 0x90, 0x97, 0xdb, // DefaultSecretUInt64_2
        0x72, 0x40, 0xa4, 0xa4, 0xb7, 0xb3, 0x67, 0x1f, // DefaultSecretUInt64_3
        0xcb, 0x79, 0xe6, 0x4e, 0xcc, 0xc0, 0xe5, 0x78, // DefaultSecretUInt64_4
        0x82, 0x5a, 0xd0, 0x7d, 0xcc, 0xff, 0x72, 0x21, // DefaultSecretUInt64_5
        0xb8, 0x08, 0x46, 0x74, 0xf7, 0x43, 0x24, 0x8e, // DefaultSecretUInt64_6
        0xe0, 0x35, 0x90, 0xe6, 0x81, 0x3a, 0x26, 0x4c, // DefaultSecretUInt64_7
        0x3c, 0x28, 0x52, 0xbb, 0x91, 0xc3, 0x00, 0xcb, // DefaultSecretUInt64_8
        0x88, 0xd0, 0x65, 0x8b, 0x1b, 0x53, 0x2e, 0xa3, // DefaultSecretUInt64_9
        0x71, 0x64, 0x48, 0x97, 0xa2, 0x0d, 0xf9, 0x4e, // DefaultSecretUInt64_10
        0x38, 0x19, 0xef, 0x46, 0xa9, 0xde, 0xac, 0xd8, // DefaultSecretUInt64_11
        0xa8, 0xfa, 0x76, 0x3f, 0xe3, 0x9c, 0x34, 0x3f, // DefaultSecretUInt64_12
        0xf9, 0xdc, 0xbb, 0xc7, 0xc7, 0x0b, 0x4f, 0x1d, // DefaultSecretUInt64_13
        0x8a, 0x51, 0xe0, 0x4b, 0xcd, 0xb4, 0x59, 0x31, // DefaultSecretUInt64_14
        0xc8, 0x9f, 0x7e, 0xc9, 0xd9, 0x78, 0x73, 0x64, // DefaultSecretUInt64_15
        0xea, 0xc5, 0xac, 0x83, 0x34, 0xd3, 0xeb, 0xc3,
        0xc5, 0x81, 0xa0, 0xff, 0xfa, 0x13, 0x63, 0xeb,
        0x17, 0x0d, 0xdd, 0x51, 0xb7, 0xf0, 0xda, 0x49,
        0xd3, 0x16, 0x55, 0x26, 0x29, 0xd4, 0x68, 0x9e,
        0x2b, 0x16, 0xbe, 0x58, 0x7d, 0x47, 0xa1, 0xfc,
        0x8f, 0xf8, 0xb8, 0xd1, 0x7a, 0xd0, 0x31, 0xce,
        0x45, 0xcb, 0x3a, 0x8f, 0x95, 0x16, 0x04, 0x28,
        0xaf, 0xd7, 0xfb, 0xca, 0xbb, 0x4b, 0x40, 0x7e,
    };

    uintcs ReadUInt32LE(const bytecs* data) {
        uintcs v;
        std::memcpy(&v, data, sizeof(v));
        return v;
    }

    ulongcs ReadUInt64LE(const bytecs* data) {
        ulongcs v;
        std::memcpy(&v, data, sizeof(v));
        return v;
    }

    namespace {
        void WriteUInt64LE(bytecs* data, ulongcs value) {
            std::memcpy(data, &value, sizeof(value));
        }
    }

    ulongcs XorShift(ulongcs value, intcs shift) {
        return value ^ (value >> shift);
    }

    ulongcs Multiply32To64(uintcs v1, uintcs v2) {
        return static_cast<ulongcs>(v1) * static_cast<ulongcs>(v2);
    }

    ulongcs Avalanche(ulongcs hash) {
        hash = XorShift(hash, 37);
        hash *= 0x165667919E3779F9ull;
        hash = XorShift(hash, 32);
        return hash;
    }

    ulongcs XxHash64Avalanche(ulongcs hash) {
        hash ^= hash >> 33;
        hash *= Prime64_2;
        hash ^= hash >> 29;
        hash *= Prime64_3;
        hash ^= hash >> 32;
        return hash;
    }

    ulongcs Rrmxmx(ulongcs hash, uintcs length) {
        hash ^= ((hash << 49) | (hash >> 15)) ^ ((hash << 24) | (hash >> 40));
        hash *= 0x9FB21C651E98DF25ull;
        hash ^= (hash >> 35) + length;
        hash *= 0x9FB21C651E98DF25ull;
        return XorShift(hash, 28);
    }

    // Portable 64x64->128 multiply (no compiler-specific 128-bit type required).
    ulongcs Multiply64To128(ulongcs left, ulongcs right, ulongcs& lower) {
        ulongcs lowerLow = Multiply32To64(static_cast<uintcs>(left), static_cast<uintcs>(right));
        ulongcs higherLow = Multiply32To64(static_cast<uintcs>(left >> 32), static_cast<uintcs>(right));
        ulongcs lowerHigh = Multiply32To64(static_cast<uintcs>(left), static_cast<uintcs>(right >> 32));
        ulongcs higherHigh = Multiply32To64(static_cast<uintcs>(left >> 32), static_cast<uintcs>(right >> 32));

        ulongcs cross = (lowerLow >> 32) + (higherLow & 0xFFFFFFFFull) + lowerHigh;
        ulongcs upper = (higherLow >> 32) + (cross >> 32) + higherHigh;
        lower = (cross << 32) | (lowerLow & 0xFFFFFFFFull);
        return upper;
    }

    ulongcs Multiply64To128ThenFold(ulongcs left, ulongcs right) {
        ulongcs lower;
        ulongcs upper = Multiply64To128(left, right, lower);
        return lower ^ upper;
    }

    ulongcs Mix16Bytes(const bytecs* source, ulongcs secretLow, ulongcs secretHigh, ulongcs seed) {
        return Multiply64To128ThenFold(
            ReadUInt64LE(source) ^ (secretLow + seed),
            ReadUInt64LE(source + 8) ^ (secretHigh - seed));
    }

    void DeriveSecretFromSeed(bytecs* destinationSecret, ulongcs seed) {
        for (intcs i = 0; i < SecretLengthBytes; i += 16) {
            WriteUInt64LE(destinationSecret + i, ReadUInt64LE(DefaultSecret + i) + seed);
            WriteUInt64LE(destinationSecret + i + 8, ReadUInt64LE(DefaultSecret + i + 8) - seed);
        }
    }

    void InitializeAccumulators(ulongcs* accumulators) {
        accumulators[0] = Prime32_3;
        accumulators[1] = Prime64_1;
        accumulators[2] = Prime64_2;
        accumulators[3] = Prime64_3;
        accumulators[4] = Prime64_4;
        accumulators[5] = Prime32_2;
        accumulators[6] = Prime64_5;
        accumulators[7] = Prime32_1;
    }

    namespace {
        void Accumulate512(ulongcs* accumulators, const bytecs* source, const bytecs* secret) {
            for (intcs i = 0; i < AccumulatorCount; i++) {
                ulongcs sourceVal = ReadUInt64LE(source + (8 * i));
                ulongcs sourceKey = sourceVal ^ ReadUInt64LE(secret + (i * 8));

                accumulators[i ^ 1] += sourceVal; // swap adjacent lanes
                accumulators[i] += Multiply32To64(static_cast<uintcs>(sourceKey), static_cast<uintcs>(sourceKey >> 32));
            }
        }

        void ScrambleAccumulators(ulongcs* accumulators, const bytecs* secret) {
            for (intcs i = 0; i < AccumulatorCount; i++) {
                ulongcs xorShift = XorShift(accumulators[i], 47);
                ulongcs xorWithKey = xorShift ^ ReadUInt64LE(secret + i * 8);
                accumulators[i] = xorWithKey * Prime32_1;
            }
        }

        // Optimized version of looping over Accumulate512.
        void Accumulate(ulongcs* accumulators, const bytecs* source, const bytecs* secret,
                         intcs stripesToProcess, bool scramble = false, intcs blockCount = 1) {
            const bytecs* secretForAccumulate = secret;
            const bytecs* secretForScramble = secret + (SecretLengthBytes - StripeLengthBytes);

            for (intcs j = 0; j < blockCount; j++) {
                const bytecs* stripeSecret = secretForAccumulate;
                const bytecs* stripeSource = source + static_cast<std::ptrdiff_t>(j) * stripesToProcess * StripeLengthBytes;
                for (intcs i = 0; i < stripesToProcess; i++) {
                    Accumulate512(accumulators, stripeSource, stripeSecret);
                    stripeSource += StripeLengthBytes;
                    stripeSecret += SecretConsumeRateBytes;
                }

                if (scramble) {
                    ScrambleAccumulators(accumulators, secretForScramble);
                }
            }
        }

        void ConsumeStripes(ulongcs* accumulators, ulongcs& stripesSoFar, ulongcs stripesPerBlock,
                            const bytecs* source, ulongcs stripes, const bytecs* secret) {
            ulongcs stripesToEndOfBlock = stripesPerBlock - stripesSoFar;
            if (stripesToEndOfBlock <= stripes) {
                ulongcs stripesAfterBlock = stripes - stripesToEndOfBlock;
                Accumulate(accumulators, source, secret + (static_cast<intcs>(stripesSoFar) * SecretConsumeRateBytes),
                           static_cast<intcs>(stripesToEndOfBlock));
                ScrambleAccumulators(accumulators, secret + (SecretLengthBytes - StripeLengthBytes));
                Accumulate(accumulators, source + (static_cast<intcs>(stripesToEndOfBlock) * StripeLengthBytes), secret,
                           static_cast<intcs>(stripesAfterBlock));
                stripesSoFar = stripesAfterBlock;
            } else {
                Accumulate(accumulators, source, secret + (static_cast<intcs>(stripesSoFar) * SecretConsumeRateBytes),
                           static_cast<intcs>(stripes));
                stripesSoFar += stripes;
            }
        }
    }

    void HashInternalLoop(ulongcs* accumulators, const bytecs* source, uintcs length, const bytecs* secret) {
        constexpr intcs StripesPerBlock = (SecretLengthBytes - StripeLengthBytes) / SecretConsumeRateBytes;
        constexpr intcs BlockLen = StripeLengthBytes * StripesPerBlock;
        intcs blocksNum = static_cast<intcs>((length - 1) / BlockLen);

        Accumulate(accumulators, source, secret, StripesPerBlock, true, blocksNum);
        intcs offset = BlockLen * blocksNum;

        intcs stripesNumber = static_cast<intcs>((length - 1 - static_cast<uintcs>(offset)) / StripeLengthBytes);
        Accumulate(accumulators, source + offset, secret, stripesNumber);
        Accumulate512(accumulators, source + length - StripeLengthBytes,
                      secret + (SecretLengthBytes - StripeLengthBytes - SecretLastAccStartBytes));
    }

    ulongcs MergeAccumulators(const ulongcs* accumulators, const bytecs* secret, ulongcs start) {
        ulongcs result64 = start;

        result64 += Multiply64To128ThenFold(accumulators[0] ^ ReadUInt64LE(secret), accumulators[1] ^ ReadUInt64LE(secret + 8));
        result64 += Multiply64To128ThenFold(accumulators[2] ^ ReadUInt64LE(secret + 16), accumulators[3] ^ ReadUInt64LE(secret + 24));
        result64 += Multiply64To128ThenFold(accumulators[4] ^ ReadUInt64LE(secret + 32), accumulators[5] ^ ReadUInt64LE(secret + 40));
        result64 += Multiply64To128ThenFold(accumulators[6] ^ ReadUInt64LE(secret + 48), accumulators[7] ^ ReadUInt64LE(secret + 56));

        return Avalanche(result64);
    }

    void Initialize(State& state, ulongcs seed) {
        state.Seed = seed;

        if (seed == 0) {
            std::memcpy(state.Secret, DefaultSecret, SecretLengthBytes);
        } else {
            DeriveSecretFromSeed(state.Secret, seed);
        }

        ResetState(state);
    }

    void ResetState(State& state) {
        state.BufferedCount = 0;
        state.StripesProcessedInCurrentBlock = 0;
        state.TotalLength = 0;
        InitializeAccumulators(state.Accumulators);
    }

    void CopyAccumulators(const State& state, ulongcs* accumulators) {
        for (intcs i = 0; i < AccumulatorCount; i++) {
            accumulators[i] = state.Accumulators[i];
        }
    }

    void DigestLong(const State& state, ulongcs* accumulators, const bytecs* secret) {
        const bytecs* accumulateData;
        bytecs lastStripe[StripeLengthBytes];

        if (state.BufferedCount >= StripeLengthBytes) {
            uintcs stripes = (state.BufferedCount - 1) / StripeLengthBytes;
            ulongcs stripesSoFar = state.StripesProcessedInCurrentBlock;

            ConsumeStripes(accumulators, stripesSoFar, NumStripesPerBlock, state.Buffer, stripes, secret);

            accumulateData = state.Buffer + state.BufferedCount - StripeLengthBytes;
        } else {
            intcs catchupSize = StripeLengthBytes - static_cast<intcs>(state.BufferedCount);

            std::memcpy(lastStripe, state.Buffer + InternalBufferLengthBytes - catchupSize, static_cast<size_t>(catchupSize));
            std::memcpy(lastStripe + catchupSize, state.Buffer, state.BufferedCount);
            accumulateData = lastStripe;
        }

        Accumulate512(accumulators, accumulateData, secret + (SecretLengthBytes - StripeLengthBytes - SecretLastAccStartBytes));
    }

    void Append(State& state, const bytecs* source, intcs length) {
        // Confirmed via a standalone repro that a negative length is a severe, immediate memory-
        // safety bug here, not just abstract UB: static_cast<size_t>(length) below (in the
        // small-input memcpy path a few lines down) wraps a negative intcs to a value near
        // SIZE_MAX (e.g. -1 -> 18446744073709551615 on a 64-bit build), turning the memcpy call
        // into an immediate crash / massive out-of-bounds read-and-write. Unlike .NET's
        // ReadOnlySpan<byte>, whose Length can never be negative through any normal
        // construction path, this port's raw-pointer-plus-intcs-length API has no such inherent
        // guarantee, so it must be validated explicitly at this shared entry point (used by both
        // XxHash3::Append and XxHash128::Append).
        if (length < 0)
            throw System::ArgumentOutOfRangeException("length", "Non-negative number required.");
        state.TotalLength += static_cast<ulongcs>(length);

        bytecs* buffer = state.Buffer;

        // Small input: just copy the data to the buffer.
        if (length <= InternalBufferLengthBytes - static_cast<intcs>(state.BufferedCount)) {
            std::memcpy(buffer + state.BufferedCount, source, static_cast<size_t>(length));
            state.BufferedCount += static_cast<uintcs>(length);
            return;
        }

        bytecs* secret = state.Secret;
        ulongcs* accumulators = state.Accumulators;

        // Internal buffer is partially filled (always, except at beginning). Complete it, then consume it.
        intcs sourceIndex = 0;
        if (state.BufferedCount != 0) {
            intcs loadSize = InternalBufferLengthBytes - static_cast<intcs>(state.BufferedCount);

            std::memcpy(buffer + state.BufferedCount, source, static_cast<size_t>(loadSize));
            sourceIndex = loadSize;

            ConsumeStripes(accumulators, state.StripesProcessedInCurrentBlock, NumStripesPerBlock, buffer,
                           InternalBufferStripes, secret);
            state.BufferedCount = 0;
        }

        // Large input to consume: ingest per full block.
        if (length - sourceIndex > NumStripesPerBlock * StripeLengthBytes) {
            ulongcs stripes = static_cast<ulongcs>(length - sourceIndex - 1) / StripeLengthBytes;

            // Join to current block's end.
            ulongcs stripesToEnd = static_cast<ulongcs>(NumStripesPerBlock) - state.StripesProcessedInCurrentBlock;
            Accumulate(accumulators, source + sourceIndex,
                       secret + (static_cast<intcs>(state.StripesProcessedInCurrentBlock) * SecretConsumeRateBytes),
                       static_cast<intcs>(stripesToEnd));
            ScrambleAccumulators(accumulators, secret + (SecretLengthBytes - StripeLengthBytes));
            state.StripesProcessedInCurrentBlock = 0;
            sourceIndex += static_cast<intcs>(stripesToEnd) * StripeLengthBytes;
            stripes -= stripesToEnd;

            // Consume entire blocks.
            while (stripes >= static_cast<ulongcs>(NumStripesPerBlock)) {
                Accumulate(accumulators, source + sourceIndex, secret, NumStripesPerBlock);
                ScrambleAccumulators(accumulators, secret + (SecretLengthBytes - StripeLengthBytes));
                sourceIndex += NumStripesPerBlock * StripeLengthBytes;
                stripes -= static_cast<ulongcs>(NumStripesPerBlock);
            }

            // Consume complete stripes in the last partial block.
            Accumulate(accumulators, source + sourceIndex, secret, static_cast<intcs>(stripes));
            sourceIndex += static_cast<intcs>(stripes) * StripeLengthBytes;
            state.StripesProcessedInCurrentBlock = stripes;

            // Copy the last stripe into the end of the buffer so it is available to GetCurrentHashCore
            // when processing the "stripe from the end".
            std::memcpy(buffer + InternalBufferLengthBytes - StripeLengthBytes, source + sourceIndex - StripeLengthBytes,
                       StripeLengthBytes);
        } else if (length - sourceIndex > InternalBufferLengthBytes) {
            // Content to consume <= block size. Consume source by a multiple of internal buffer size.
            do {
                ConsumeStripes(accumulators, state.StripesProcessedInCurrentBlock, NumStripesPerBlock,
                               source + sourceIndex, InternalBufferStripes, secret);
                sourceIndex += InternalBufferLengthBytes;
            } while (length - sourceIndex > InternalBufferLengthBytes);

            std::memcpy(buffer + InternalBufferLengthBytes - StripeLengthBytes, source + sourceIndex - StripeLengthBytes,
                       StripeLengthBytes);
        }

        // Buffer the remaining input.
        std::memcpy(buffer, source + sourceIndex, static_cast<size_t>(length - sourceIndex));
        state.BufferedCount = static_cast<uintcs>(length - sourceIndex);
    }

} // namespace System::IO::Hashing::Detail::XxHash3Shared
