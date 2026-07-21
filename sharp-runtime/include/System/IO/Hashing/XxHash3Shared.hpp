// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO::Hashing::Detail {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using SharpRuntime::uintcs;
    using SharpRuntime::ulongcs;

    /**
     * @brief Internal implementation shared by XxHash3 and XxHash128.
     *
     * This is a direct, scalar-only port of .NET's internal XxHashShared class (the
     * SIMD/vectorized code paths are pure performance optimizations that produce identical
     * results to the scalar path, so they are not ported). Not part of the public API.
     */
    namespace XxHash3Shared {

        constexpr intcs StripeLengthBytes = 64;
        constexpr intcs SecretLengthBytes = 192;
        constexpr intcs SecretLastAccStartBytes = 7;
        constexpr intcs SecretConsumeRateBytes = 8;
        constexpr intcs SecretMergeAccsStartBytes = 11;
        constexpr intcs NumStripesPerBlock = (SecretLengthBytes - StripeLengthBytes) / SecretConsumeRateBytes;
        constexpr intcs AccumulatorCount = StripeLengthBytes / 8;
        constexpr intcs MidSizeMaxBytes = 240;
        constexpr intcs InternalBufferLengthBytes = 256;
        constexpr intcs InternalBufferStripes = InternalBufferLengthBytes / StripeLengthBytes;

        constexpr ulongcs Prime64_1 = 0x9E3779B185EBCA87ull;
        constexpr ulongcs Prime64_2 = 0xC2B2AE3D27D4EB4Full;
        constexpr ulongcs Prime64_3 = 0x165667B19E3779F9ull;
        constexpr ulongcs Prime64_4 = 0x85EBCA77C2B2AE63ull;
        constexpr ulongcs Prime64_5 = 0x27D4EB2F165667C5ull;

        constexpr uintcs Prime32_1 = 0x9E3779B1u;
        constexpr uintcs Prime32_2 = 0x85EBCA77u;
        constexpr uintcs Prime32_3 = 0xC2B2AE3Du;
        constexpr uintcs Prime32_4 = 0x27D4EB2Fu;
        constexpr uintcs Prime32_5 = 0x165667B1u;

        // Cast of DefaultSecret byte[] => ulong[0..15] (little-endian).
        constexpr ulongcs DefaultSecretUInt64_0 = 0xBE4BA423396CFEB8ull;
        constexpr ulongcs DefaultSecretUInt64_1 = 0x1CAD21F72C81017Cull;
        constexpr ulongcs DefaultSecretUInt64_2 = 0xDB979083E96DD4DEull;
        constexpr ulongcs DefaultSecretUInt64_3 = 0x1F67B3B7A4A44072ull;
        constexpr ulongcs DefaultSecretUInt64_4 = 0x78E5C0CC4EE679CBull;
        constexpr ulongcs DefaultSecretUInt64_5 = 0x2172FFCC7DD05A82ull;
        constexpr ulongcs DefaultSecretUInt64_6 = 0x8E2443F7744608B8ull;
        constexpr ulongcs DefaultSecretUInt64_7 = 0x4C263A81E69035E0ull;
        constexpr ulongcs DefaultSecretUInt64_8 = 0xCB00C391BB52283Cull;
        constexpr ulongcs DefaultSecretUInt64_9 = 0xA32E531B8B65D088ull;
        constexpr ulongcs DefaultSecretUInt64_10 = 0x4EF90DA297486471ull;
        constexpr ulongcs DefaultSecretUInt64_11 = 0xD8ACDEA946EF1938ull;
        constexpr ulongcs DefaultSecretUInt64_12 = 0x3F349CE33F76FAA8ull;
        constexpr ulongcs DefaultSecretUInt64_13 = 0x1D4F0BC7C7BBDCF9ull;
        constexpr ulongcs DefaultSecretUInt64_14 = 0x3159B4CD4BE0518Aull;
        constexpr ulongcs DefaultSecretUInt64_15 = 0x647378D9C97E9FC8ull;

        // Cast of DefaultSecret offset by 3 bytes, byte[] => ulong[0..13] (little-endian).
        constexpr ulongcs DefaultSecret3UInt64_0 = 0x81017CBE4BA42339ull;
        constexpr ulongcs DefaultSecret3UInt64_1 = 0x6DD4DE1CAD21F72Cull;
        constexpr ulongcs DefaultSecret3UInt64_2 = 0xA44072DB979083E9ull;
        constexpr ulongcs DefaultSecret3UInt64_3 = 0xE679CB1F67B3B7A4ull;
        constexpr ulongcs DefaultSecret3UInt64_4 = 0xD05A8278E5C0CC4Eull;
        constexpr ulongcs DefaultSecret3UInt64_5 = 0x4608B82172FFCC7Dull;
        constexpr ulongcs DefaultSecret3UInt64_6 = 0x9035E08E2443F774ull;
        constexpr ulongcs DefaultSecret3UInt64_7 = 0x52283C4C263A81E6ull;
        constexpr ulongcs DefaultSecret3UInt64_8 = 0x65D088CB00C391BBull;
        constexpr ulongcs DefaultSecret3UInt64_9 = 0x486471A32E531B8Bull;
        constexpr ulongcs DefaultSecret3UInt64_10 = 0xEF19384EF90DA297ull;
        constexpr ulongcs DefaultSecret3UInt64_11 = 0x76FAA8D8ACDEA946ull;
        constexpr ulongcs DefaultSecret3UInt64_12 = 0xBBDCF93F349CE33Full;
        constexpr ulongcs DefaultSecret3UInt64_13 = 0xE0518A1D4F0BC7C7ull;

        /** The default secret for when no seed is provided (same as a custom secret derived from a seed of 0). */
        extern const bytecs DefaultSecret[SecretLengthBytes];

        /** Streaming state shared by XxHash3 and XxHash128. Plain-data struct; copyable via assignment (for Clone()). */
        struct State {
            ulongcs Accumulators[AccumulatorCount];
            bytecs Secret[SecretLengthBytes];
            bytecs Buffer[InternalBufferLengthBytes];
            uintcs BufferedCount = 0;
            ulongcs StripesProcessedInCurrentBlock = 0;
            ulongcs TotalLength = 0;
            ulongcs Seed = 0;
        };

        void Initialize(State& state, ulongcs seed);
        void ResetState(State& state);
        void Append(State& state, const bytecs* source, intcs length);
        void CopyAccumulators(const State& state, ulongcs* accumulators);
        void DigestLong(const State& state, ulongcs* accumulators, const bytecs* secret);

        void InitializeAccumulators(ulongcs* accumulators);
        void HashInternalLoop(ulongcs* accumulators, const bytecs* source, uintcs length, const bytecs* secret);
        void DeriveSecretFromSeed(bytecs* destinationSecret, ulongcs seed);
        ulongcs MergeAccumulators(const ulongcs* accumulators, const bytecs* secret, ulongcs start);

        ulongcs Mix16Bytes(const bytecs* source, ulongcs secretLow, ulongcs secretHigh, ulongcs seed);
        ulongcs Multiply32To64(uintcs v1, uintcs v2);
        ulongcs Avalanche(ulongcs hash);
        /** The classic XxHash64 avalanche (3 xor-shift/multiply rounds) — distinct from Avalanche() above (2 rounds). */
        ulongcs XxHash64Avalanche(ulongcs hash);
        ulongcs Rrmxmx(ulongcs hash, uintcs length);
        ulongcs Multiply64To128(ulongcs left, ulongcs right, ulongcs& lower);
        ulongcs Multiply64To128ThenFold(ulongcs left, ulongcs right);
        ulongcs XorShift(ulongcs value, intcs shift);
        uintcs ReadUInt32LE(const bytecs* data);
        ulongcs ReadUInt64LE(const bytecs* data);

    } // namespace XxHash3Shared

} // namespace System::IO::Hashing::Detail
