// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/XxHash3.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::uintcs;
    using namespace Detail::XxHash3Shared;

    namespace {
        ulongcs HashLength1To3(const bytecs* source, uintcs length, ulongcs seed) {
            bytecs c1 = *source;
            bytecs c2 = source[length >> 1];
            bytecs c3 = source[length - 1];

            uintcs combined = (static_cast<uintcs>(c1) << 16) | (static_cast<uintcs>(c2) << 24) | c3 | (length << 8);

            constexpr uintcs SecretXor = static_cast<uintcs>(DefaultSecretUInt64_0) ^ static_cast<uintcs>(DefaultSecretUInt64_0 >> 32);
            return XxHash64Avalanche(combined ^ (SecretXor + seed));
        }

        ulongcs HashLength4To8(const bytecs* source, uintcs length, ulongcs seed) {
            // seed ^= (ulong)ReverseEndianness((uint)seed) << 32
            uintcs seedLow32 = static_cast<uintcs>(seed);
            uintcs reversed32 = ((seedLow32 & 0x000000FFu) << 24) | ((seedLow32 & 0x0000FF00u) << 8) |
                                ((seedLow32 & 0x00FF0000u) >> 8)  | ((seedLow32 & 0xFF000000u) >> 24);
            seed ^= static_cast<ulongcs>(reversed32) << 32;

            uintcs inputLow = ReadUInt32LE(source);
            uintcs inputHigh = ReadUInt32LE(source + length - 4);

            constexpr ulongcs SecretXor = DefaultSecretUInt64_1 ^ DefaultSecretUInt64_2;
            ulongcs bitflip = SecretXor - seed;
            ulongcs input64 = inputHigh + (static_cast<ulongcs>(inputLow) << 32);

            return Rrmxmx(input64 ^ bitflip, length);
        }

        ulongcs HashLength9To16(const bytecs* source, uintcs length, ulongcs seed) {
            constexpr ulongcs SecretXorL = DefaultSecretUInt64_3 ^ DefaultSecretUInt64_4;
            constexpr ulongcs SecretXorR = DefaultSecretUInt64_5 ^ DefaultSecretUInt64_6;
            ulongcs bitflipLow = SecretXorL + seed;
            ulongcs bitflipHigh = SecretXorR - seed;

            ulongcs inputLow = ReadUInt64LE(source) ^ bitflipLow;
            ulongcs inputHigh = ReadUInt64LE(source + length - 8) ^ bitflipHigh;

            ulongcs reversedLow =
                ((inputLow & 0x00000000FFFFFFFFull) << 32) | (inputLow >> 32);
            reversedLow =
                ((reversedLow & 0xFFFF0000FFFF0000ull) >> 16) | ((reversedLow & 0x0000FFFF0000FFFFull) << 16);
            reversedLow =
                ((reversedLow & 0xFF00FF00FF00FF00ull) >> 8) | ((reversedLow & 0x00FF00FF00FF00FFull) << 8);

            return Avalanche(
                length +
                reversedLow +
                inputHigh +
                Multiply64To128ThenFold(inputLow, inputHigh));
        }

        ulongcs HashLength0To16(const bytecs* source, uintcs length, ulongcs seed) {
            if (length > 8) return HashLength9To16(source, length, seed);
            if (length >= 4) return HashLength4To8(source, length, seed);
            if (length != 0) return HashLength1To3(source, length, seed);

            constexpr ulongcs SecretXor = DefaultSecretUInt64_7 ^ DefaultSecretUInt64_8;
            return XxHash64Avalanche(seed ^ SecretXor);
        }

        ulongcs HashLength17To128(const bytecs* source, uintcs length, ulongcs seed) {
            ulongcs hash = static_cast<ulongcs>(length) * Prime64_1;

            switch ((length - 1) / 32) {
                default: // case 3
                    hash += Mix16Bytes(source + 48, DefaultSecretUInt64_12, DefaultSecretUInt64_13, seed);
                    hash += Mix16Bytes(source + length - 64, DefaultSecretUInt64_14, DefaultSecretUInt64_15, seed);
                    [[fallthrough]];
                case 2:
                    hash += Mix16Bytes(source + 32, DefaultSecretUInt64_8, DefaultSecretUInt64_9, seed);
                    hash += Mix16Bytes(source + length - 48, DefaultSecretUInt64_10, DefaultSecretUInt64_11, seed);
                    [[fallthrough]];
                case 1:
                    hash += Mix16Bytes(source + 16, DefaultSecretUInt64_4, DefaultSecretUInt64_5, seed);
                    hash += Mix16Bytes(source + length - 32, DefaultSecretUInt64_6, DefaultSecretUInt64_7, seed);
                    [[fallthrough]];
                case 0:
                    hash += Mix16Bytes(source, DefaultSecretUInt64_0, DefaultSecretUInt64_1, seed);
                    hash += Mix16Bytes(source + length - 16, DefaultSecretUInt64_2, DefaultSecretUInt64_3, seed);
                    break;
            }

            return Avalanche(hash);
        }

        ulongcs HashLength129To240(const bytecs* source, uintcs length, ulongcs seed) {
            ulongcs hash = static_cast<ulongcs>(length) * Prime64_1;

            hash += Mix16Bytes(source + (16 * 0), DefaultSecretUInt64_0, DefaultSecretUInt64_1, seed);
            hash += Mix16Bytes(source + (16 * 1), DefaultSecretUInt64_2, DefaultSecretUInt64_3, seed);
            hash += Mix16Bytes(source + (16 * 2), DefaultSecretUInt64_4, DefaultSecretUInt64_5, seed);
            hash += Mix16Bytes(source + (16 * 3), DefaultSecretUInt64_6, DefaultSecretUInt64_7, seed);
            hash += Mix16Bytes(source + (16 * 4), DefaultSecretUInt64_8, DefaultSecretUInt64_9, seed);
            hash += Mix16Bytes(source + (16 * 5), DefaultSecretUInt64_10, DefaultSecretUInt64_11, seed);
            hash += Mix16Bytes(source + (16 * 6), DefaultSecretUInt64_12, DefaultSecretUInt64_13, seed);
            hash += Mix16Bytes(source + (16 * 7), DefaultSecretUInt64_14, DefaultSecretUInt64_15, seed);

            hash = Avalanche(hash);

            switch ((length - (16 * 8)) / 16) {
                default: // case 7
                    hash += Mix16Bytes(source + (16 * 14), DefaultSecret3UInt64_12, DefaultSecret3UInt64_13, seed);
                    [[fallthrough]];
                case 6:
                    hash += Mix16Bytes(source + (16 * 13), DefaultSecret3UInt64_10, DefaultSecret3UInt64_11, seed);
                    [[fallthrough]];
                case 5:
                    hash += Mix16Bytes(source + (16 * 12), DefaultSecret3UInt64_8, DefaultSecret3UInt64_9, seed);
                    [[fallthrough]];
                case 4:
                    hash += Mix16Bytes(source + (16 * 11), DefaultSecret3UInt64_6, DefaultSecret3UInt64_7, seed);
                    [[fallthrough]];
                case 3:
                    hash += Mix16Bytes(source + (16 * 10), DefaultSecret3UInt64_4, DefaultSecret3UInt64_5, seed);
                    [[fallthrough]];
                case 2:
                    hash += Mix16Bytes(source + (16 * 9), DefaultSecret3UInt64_2, DefaultSecret3UInt64_3, seed);
                    [[fallthrough]];
                case 1:
                    hash += Mix16Bytes(source + (16 * 8), DefaultSecret3UInt64_0, DefaultSecret3UInt64_1, seed);
                    [[fallthrough]];
                case 0:
                    hash += Mix16Bytes(source + length - 16, 0x7378D9C97E9FC831ull, 0xEBD33483ACC5EA64ull, seed);
                    break;
            }

            return Avalanche(hash);
        }

        ulongcs HashLengthOver240(const bytecs* source, uintcs length, ulongcs seed) {
            const bytecs* secret = DefaultSecret;
            bytecs customSecret[SecretLengthBytes];
            if (seed != 0) {
                DeriveSecretFromSeed(customSecret, seed);
                secret = customSecret;
            }

            ulongcs accumulators[AccumulatorCount];
            InitializeAccumulators(accumulators);

            HashInternalLoop(accumulators, source, length, secret);

            return MergeAccumulators(accumulators, secret + 11, static_cast<ulongcs>(length) * Prime64_1);
        }

        ulongcs HashToUInt64Impl(const bytecs* source, intcs length, ulongcs seed) {
            // Same fix, same rationale as Detail::XxHash3Shared::Append: a negative length here
            // wraps to a huge value once cast to unsigned (confirmed via a standalone repro),
            // and would be treated as a request to read that many bytes from source deep inside
            // the length-dispatch logic below -- a severe out-of-bounds read, not just abstract
            // UB. This is the single choke point for all four one-shot public entry points
            // (Hash x2, TryHash, HashToUInt64).
            if (length < 0)
                throw System::ArgumentOutOfRangeException("length", "Non-negative number required.");
            uintcs len = static_cast<uintcs>(length);
            if (len <= 16) return HashLength0To16(source, len, seed);
            if (len <= 128) return HashLength17To128(source, len, seed);
            if (len <= MidSizeMaxBytes) return HashLength129To240(source, len, seed);
            return HashLengthOver240(source, len, seed);
        }
    }

    XxHash3::XxHash3() : XxHash3(0) {}

    XxHash3::XxHash3(longcs seed) : NonCryptographicHashAlgorithm(Size) {
        Initialize(state_, static_cast<ulongcs>(seed));
    }

    XxHash3::XxHash3(const State& state) : NonCryptographicHashAlgorithm(Size), state_(state) {}

    void XxHash3::Reset() {
        ResetState(state_);
    }

    void XxHash3::Append(const bytecs* source, intcs length) {
        Detail::XxHash3Shared::Append(state_, source, length);
    }

    void XxHash3::GetCurrentHashCore(bytecs* destination) const {
        ulongcs hash = GetCurrentHashAsUInt64();
        for (int i = 0; i < 8; ++i) destination[i] = static_cast<bytecs>(hash >> (56 - 8 * i));
    }

    ulongcs XxHash3::GetCurrentHashAsUInt64() const {
        if (state_.TotalLength > MidSizeMaxBytes) {
            ulongcs accumulators[AccumulatorCount];
            CopyAccumulators(state_, accumulators);

            const bytecs* secret = state_.Secret;
            DigestLong(state_, accumulators, secret);
            return MergeAccumulators(accumulators, secret + SecretMergeAccsStartBytes, state_.TotalLength * Prime64_1);
        }

        return HashToUInt64Impl(state_.Buffer, static_cast<intcs>(state_.TotalLength), state_.Seed);
    }

    std::vector<bytecs> XxHash3::Hash(const bytecs* source, intcs length) {
        return Hash(source, length, 0);
    }

    std::vector<bytecs> XxHash3::Hash(const bytecs* source, intcs length, longcs seed) {
        std::vector<bytecs> ret(static_cast<size_t>(Size));
        ulongcs hash = HashToUInt64(source, length, seed);
        for (int i = 0; i < 8; ++i) ret[static_cast<size_t>(i)] = static_cast<bytecs>(hash >> (56 - 8 * i));
        return ret;
    }

    intcs XxHash3::Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, longcs seed) {
        if (destinationLength < Size) {
            throw System::ArgumentException("Destination is too short.", "destination");
        }
        ulongcs hash = HashToUInt64(source, length, seed);
        for (int i = 0; i < 8; ++i) destination[i] = static_cast<bytecs>(hash >> (56 - 8 * i));
        return Size;
    }

    bool XxHash3::TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength,
                          intcs& bytesWritten, longcs seed) {
        if (destinationLength < Size) {
            bytesWritten = 0;
            return false;
        }
        ulongcs hash = HashToUInt64(source, length, seed);
        for (int i = 0; i < 8; ++i) destination[i] = static_cast<bytecs>(hash >> (56 - 8 * i));
        bytesWritten = Size;
        return true;
    }

    ulongcs XxHash3::HashToUInt64(const bytecs* source, intcs length, longcs seed) {
        return HashToUInt64Impl(source, length, static_cast<ulongcs>(seed));
    }

} // namespace System::IO::Hashing
