// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/XxHash128.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::uintcs;
    using namespace Detail::XxHash3Shared;

    namespace {
        // BinaryPrimitives.ReverseEndianness: byte-order swap (NOT a bit reversal).
        ulongcs ByteSwap64(ulongcs value) {
            value = ((value & 0xFF00FF00FF00FF00ull) >> 8) | ((value & 0x00FF00FF00FF00FFull) << 8);
            value = ((value & 0xFFFF0000FFFF0000ull) >> 16) | ((value & 0x0000FFFF0000FFFFull) << 16);
            return (value >> 32) | (value << 32);
        }

        uintcs ByteSwap32(uintcs value) {
            return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) |
                   ((value & 0x00FF0000u) >> 8)  | ((value & 0xFF000000u) >> 24);
        }

        uintcs RotateLeft32(uintcs value, int n) { return (value << n) | (value >> (32 - n)); }

        Hash128 HashLength1To3(const bytecs* source, uintcs length, ulongcs seed) {
            bytecs c1 = *source;
            bytecs c2 = source[length >> 1];
            bytecs c3 = source[length - 1];

            uintcs combinedl = (static_cast<uintcs>(c1) << 16) | (static_cast<uintcs>(c2) << 24) | c3 | (length << 8);
            uintcs combinedh = RotateLeft32(ByteSwap32(combinedl), 13);

            constexpr uintcs SecretXorL = static_cast<uintcs>(DefaultSecretUInt64_0) ^ static_cast<uintcs>(DefaultSecretUInt64_0 >> 32);
            constexpr uintcs SecretXorH = static_cast<uintcs>(DefaultSecretUInt64_1) ^ static_cast<uintcs>(DefaultSecretUInt64_1 >> 32);
            ulongcs bitflipl = SecretXorL + seed;
            ulongcs bitfliph = SecretXorH - seed;
            ulongcs keyedLo = combinedl ^ bitflipl;
            ulongcs keyedHi = combinedh ^ bitfliph;

            return Hash128(XxHash64Avalanche(keyedLo), XxHash64Avalanche(keyedHi));
        }

        Hash128 HashLength4To8(const bytecs* source, uintcs length, ulongcs seed) {
            // seed ^= (ulong)ReverseEndianness((uint)seed) << 32
            uintcs seedLow32 = static_cast<uintcs>(seed);
            uintcs byteSwapped32 = ((seedLow32 & 0x000000FFu) << 24) | ((seedLow32 & 0x0000FF00u) << 8) |
                                   ((seedLow32 & 0x00FF0000u) >> 8)  | ((seedLow32 & 0xFF000000u) >> 24);
            seed ^= static_cast<ulongcs>(byteSwapped32) << 32;

            uintcs inputLo = ReadUInt32LE(source);
            uintcs inputHi = ReadUInt32LE(source + length - 4);
            ulongcs input64 = inputLo + (static_cast<ulongcs>(inputHi) << 32);
            ulongcs bitflip = (DefaultSecretUInt64_2 ^ DefaultSecretUInt64_3) + seed;
            ulongcs keyed = input64 ^ bitflip;

            ulongcs m128Low;
            ulongcs m128High = Multiply64To128(keyed, Prime64_1 + (static_cast<ulongcs>(length) << 2), m128Low);

            m128High += (m128Low << 1);
            m128Low ^= (m128High >> 3);

            m128Low = XorShift(m128Low, 35);
            m128Low *= 0x9FB21C651E98DF25ull;
            m128Low = XorShift(m128Low, 28);
            m128High = Avalanche(m128High);

            return Hash128(m128Low, m128High);
        }

        Hash128 HashLength9To16(const bytecs* source, uintcs length, ulongcs seed) {
            ulongcs bitflipl = (DefaultSecretUInt64_4 ^ DefaultSecretUInt64_5) - seed;
            ulongcs bitfliph = (DefaultSecretUInt64_6 ^ DefaultSecretUInt64_7) + seed;
            ulongcs inputLo = ReadUInt64LE(source);
            ulongcs inputHi = ReadUInt64LE(source + length - 8);

            ulongcs m128Low;
            ulongcs m128High = Multiply64To128(inputLo ^ inputHi ^ bitflipl, Prime64_1, m128Low);

            m128Low += static_cast<ulongcs>(length - 1) << 54;
            inputHi ^= bitfliph;

            if constexpr (sizeof(void*) < sizeof(ulongcs)) {
                m128High += (inputHi & 0xFFFFFFFF00000000ull) + Multiply32To64(static_cast<uintcs>(inputHi), Prime32_2);
            } else {
                m128High += inputHi + Multiply32To64(static_cast<uintcs>(inputHi), Prime32_2 - 1);
            }

            m128Low ^= ByteSwap64(m128High);

            ulongcs h128Low;
            ulongcs h128High = Multiply64To128(m128Low, Prime64_2, h128Low);
            h128High += m128High * Prime64_2;

            h128Low = Avalanche(h128Low);
            h128High = Avalanche(h128High);
            return Hash128(h128Low, h128High);
        }

        Hash128 HashLength0To16(const bytecs* source, uintcs length, ulongcs seed) {
            if (length > 8) return HashLength9To16(source, length, seed);
            if (length >= 4) return HashLength4To8(source, length, seed);
            if (length != 0) return HashLength1To3(source, length, seed);

            constexpr ulongcs BitFlipL = DefaultSecretUInt64_8 ^ DefaultSecretUInt64_9;
            constexpr ulongcs BitFlipH = DefaultSecretUInt64_10 ^ DefaultSecretUInt64_11;
            return Hash128(XxHash64Avalanche(seed ^ BitFlipL), XxHash64Avalanche(seed ^ BitFlipH));
        }

        void Mix32Bytes(ulongcs& accLow, ulongcs& accHigh, const bytecs* input1, const bytecs* input2,
                        ulongcs secret1, ulongcs secret2, ulongcs secret3, ulongcs secret4, ulongcs seed) {
            accLow += Mix16Bytes(input1, secret1, secret2, seed);
            accLow ^= ReadUInt64LE(input2) + ReadUInt64LE(input2 + 8);
            accHigh += Mix16Bytes(input2, secret3, secret4, seed);
            accHigh ^= ReadUInt64LE(input1) + ReadUInt64LE(input1 + 8);
        }

        Hash128 AvalancheHash(ulongcs accLow, ulongcs accHigh, uintcs length, ulongcs seed) {
            ulongcs h128Low = accLow + accHigh;
            ulongcs h128High = (accLow * Prime64_1) + (accHigh * Prime64_4) + (static_cast<ulongcs>(length) - seed) * Prime64_2;
            h128Low = Avalanche(h128Low);
            h128High = 0ull - Avalanche(h128High);
            return Hash128(h128Low, h128High);
        }

        Hash128 HashLength17To128(const bytecs* source, uintcs length, ulongcs seed) {
            ulongcs accLow = static_cast<ulongcs>(length) * Prime64_1;
            ulongcs accHigh = 0;

            switch ((length - 1) / 32) {
                default: // case 3
                    Mix32Bytes(accLow, accHigh, source + 48, source + length - 64,
                               DefaultSecretUInt64_12, DefaultSecretUInt64_13, DefaultSecretUInt64_14, DefaultSecretUInt64_15, seed);
                    [[fallthrough]];
                case 2:
                    Mix32Bytes(accLow, accHigh, source + 32, source + length - 48,
                               DefaultSecretUInt64_8, DefaultSecretUInt64_9, DefaultSecretUInt64_10, DefaultSecretUInt64_11, seed);
                    [[fallthrough]];
                case 1:
                    Mix32Bytes(accLow, accHigh, source + 16, source + length - 32,
                               DefaultSecretUInt64_4, DefaultSecretUInt64_5, DefaultSecretUInt64_6, DefaultSecretUInt64_7, seed);
                    [[fallthrough]];
                case 0:
                    Mix32Bytes(accLow, accHigh, source, source + length - 16,
                               DefaultSecretUInt64_0, DefaultSecretUInt64_1, DefaultSecretUInt64_2, DefaultSecretUInt64_3, seed);
                    break;
            }

            return AvalancheHash(accLow, accHigh, length, seed);
        }

        Hash128 HashLength129To240(const bytecs* source, uintcs length, ulongcs seed) {
            ulongcs accLow = static_cast<ulongcs>(length) * Prime64_1;
            ulongcs accHigh = 0;

            Mix32Bytes(accLow, accHigh, source + (32 * 0), source + (32 * 0) + 16,
                       DefaultSecretUInt64_0, DefaultSecretUInt64_1, DefaultSecretUInt64_2, DefaultSecretUInt64_3, seed);
            Mix32Bytes(accLow, accHigh, source + (32 * 1), source + (32 * 1) + 16,
                       DefaultSecretUInt64_4, DefaultSecretUInt64_5, DefaultSecretUInt64_6, DefaultSecretUInt64_7, seed);
            Mix32Bytes(accLow, accHigh, source + (32 * 2), source + (32 * 2) + 16,
                       DefaultSecretUInt64_8, DefaultSecretUInt64_9, DefaultSecretUInt64_10, DefaultSecretUInt64_11, seed);
            Mix32Bytes(accLow, accHigh, source + (32 * 3), source + (32 * 3) + 16,
                       DefaultSecretUInt64_12, DefaultSecretUInt64_13, DefaultSecretUInt64_14, DefaultSecretUInt64_15, seed);

            accLow = Avalanche(accLow);
            accHigh = Avalanche(accHigh);

            uintcs bound = (length - (32 * 4)) / 32;
            if (bound != 0) {
                Mix32Bytes(accLow, accHigh, source + (32 * 4), source + (32 * 4) + 16,
                           DefaultSecret3UInt64_0, DefaultSecret3UInt64_1, DefaultSecret3UInt64_2, DefaultSecret3UInt64_3, seed);
                if (bound >= 2) {
                    Mix32Bytes(accLow, accHigh, source + (32 * 5), source + (32 * 5) + 16,
                               DefaultSecret3UInt64_4, DefaultSecret3UInt64_5, DefaultSecret3UInt64_6, DefaultSecret3UInt64_7, seed);
                    if (bound == 3) {
                        Mix32Bytes(accLow, accHigh, source + (32 * 6), source + (32 * 6) + 16,
                                   DefaultSecret3UInt64_8, DefaultSecret3UInt64_9, DefaultSecret3UInt64_10, DefaultSecret3UInt64_11, seed);
                    }
                }
            }
            Mix32Bytes(accLow, accHigh, source + length - 16, source + length - 32,
                       0x4F0BC7C7BBDCF93Full, 0x59B4CD4BE0518A1Dull, 0x7378D9C97E9FC831ull, 0xEBD33483ACC5EA64ull, 0ull - seed);

            return AvalancheHash(accLow, accHigh, length, seed);
        }

        Hash128 HashLengthOver240(const bytecs* source, uintcs length, ulongcs seed) {
            const bytecs* secret = DefaultSecret;
            bytecs customSecret[SecretLengthBytes];
            if (seed != 0) {
                DeriveSecretFromSeed(customSecret, seed);
                secret = customSecret;
            }

            ulongcs accumulators[AccumulatorCount];
            InitializeAccumulators(accumulators);

            HashInternalLoop(accumulators, source, length, secret);

            return Hash128(
                MergeAccumulators(accumulators, secret + SecretMergeAccsStartBytes, static_cast<ulongcs>(length) * Prime64_1),
                MergeAccumulators(accumulators, secret + SecretLengthBytes - AccumulatorCount * 8 - SecretMergeAccsStartBytes,
                                  ~(static_cast<ulongcs>(length) * Prime64_2)));
        }

        Hash128 HashToHash128Impl(const bytecs* source, intcs length, ulongcs seed) {
            // Same fix, same rationale as XxHash3's HashToUInt64Impl (ticket 355): a negative
            // length wraps to a huge value once cast to unsigned, and is then treated as a
            // request to read that many bytes from source deep inside the length-dispatch logic
            // below -- a severe out-of-bounds read, confirmed via a standalone ASan repro. This
            // is the single choke point for all one-shot public entry points (Hash x3, TryHash,
            // HashToHash128) -- unlike the streaming Append() path, which already validates via
            // Detail::XxHash3Shared::Append, this one-shot path was not covered by that fix.
            if (length < 0)
                throw System::ArgumentOutOfRangeException("length", "Non-negative number required.");
            uintcs len = static_cast<uintcs>(length);
            if (len <= 16) return HashLength0To16(source, len, seed);
            if (len <= 128) return HashLength17To128(source, len, seed);
            if (len <= MidSizeMaxBytes) return HashLength129To240(source, len, seed);
            return HashLengthOver240(source, len, seed);
        }

        void WriteBigEndian128(const Hash128& hash, bytecs* destination) {
            for (int i = 0; i < 8; ++i) destination[i] = static_cast<bytecs>(hash.High64 >> (56 - 8 * i));
            for (int i = 0; i < 8; ++i) destination[8 + i] = static_cast<bytecs>(hash.Low64 >> (56 - 8 * i));
        }
    }

    XxHash128::XxHash128() : XxHash128(0) {}

    XxHash128::XxHash128(longcs seed) : NonCryptographicHashAlgorithm(Size) {
        Initialize(state_, static_cast<ulongcs>(seed));
    }

    XxHash128::XxHash128(const State& state) : NonCryptographicHashAlgorithm(Size), state_(state) {}

    void XxHash128::Reset() {
        ResetState(state_);
    }

    void XxHash128::Append(const bytecs* source, intcs length) {
        Detail::XxHash3Shared::Append(state_, source, length);
    }

    void XxHash128::GetCurrentHashCore(bytecs* destination) const {
        WriteBigEndian128(GetCurrentHashAsHash128(), destination);
    }

    Hash128 XxHash128::GetCurrentHashAsHash128() const {
        if (state_.TotalLength > MidSizeMaxBytes) {
            ulongcs accumulators[AccumulatorCount];
            CopyAccumulators(state_, accumulators);

            const bytecs* secret = state_.Secret;
            DigestLong(state_, accumulators, secret);
            return Hash128(
                MergeAccumulators(accumulators, secret + SecretMergeAccsStartBytes, state_.TotalLength * Prime64_1),
                MergeAccumulators(accumulators, secret + SecretLengthBytes - AccumulatorCount * 8 - SecretMergeAccsStartBytes,
                                  ~(state_.TotalLength * Prime64_2)));
        }

        return HashToHash128Impl(state_.Buffer, static_cast<intcs>(state_.TotalLength), state_.Seed);
    }

    std::vector<bytecs> XxHash128::Hash(const bytecs* source, intcs length) {
        return Hash(source, length, 0);
    }

    std::vector<bytecs> XxHash128::Hash(const bytecs* source, intcs length, longcs seed) {
        std::vector<bytecs> ret(static_cast<size_t>(Size));
        Hash128 hash = HashToHash128(source, length, seed);
        WriteBigEndian128(hash, ret.data());
        return ret;
    }

    intcs XxHash128::Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, longcs seed) {
        if (destinationLength < Size) {
            throw System::ArgumentException("Destination is too short.", "destination");
        }
        WriteBigEndian128(HashToHash128(source, length, seed), destination);
        return Size;
    }

    bool XxHash128::TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength,
                            intcs& bytesWritten, longcs seed) {
        if (destinationLength < Size) {
            bytesWritten = 0;
            return false;
        }
        WriteBigEndian128(HashToHash128(source, length, seed), destination);
        bytesWritten = Size;
        return true;
    }

    Hash128 XxHash128::HashToHash128(const bytecs* source, intcs length, longcs seed) {
        return HashToHash128Impl(source, length, static_cast<ulongcs>(seed));
    }

} // namespace System::IO::Hashing
