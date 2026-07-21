// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/Adler32.hpp"
#include "System/ArgumentException.hpp"

namespace System::IO::Hashing {

    namespace {
        // Largest prime smaller than 65536.
        constexpr uintcs ModBase = 65521u;
        // Largest n such that 255n(n+1)/2 + (n+1)(ModBase-1) <= 2^32-1.
        constexpr intcs NMax = 5552;

        uintcs Update(uintcs adler, const bytecs* source, intcs length) {
            if (length == 0) return adler;

            uintcs s1 = adler & 0xFFFFu;
            uintcs s2 = (adler >> 16) & 0xFFFFu;

            while (length > 0) {
                const intcs k = length < NMax ? length : NMax;
                for (intcs i = 0; i < k; ++i) {
                    s1 += source[i];
                    s2 += s1;
                }
                s1 %= ModBase;
                s2 %= ModBase;
                source += k;
                length -= k;
            }

            return (s2 << 16) | s1;
        }

        void WriteUInt32BigEndian(uintcs value, bytecs* destination) {
            destination[0] = static_cast<bytecs>(value >> 24);
            destination[1] = static_cast<bytecs>(value >> 16);
            destination[2] = static_cast<bytecs>(value >> 8);
            destination[3] = static_cast<bytecs>(value);
        }
    }

    Adler32::Adler32() : NonCryptographicHashAlgorithm(Size) {}

    Adler32::Adler32(uintcs adler) : NonCryptographicHashAlgorithm(Size), adler_(adler) {}

    void Adler32::Append(const bytecs* source, intcs length) {
        adler_ = Update(adler_, source, length);
    }

    void Adler32::Reset() {
        adler_ = InitialState;
    }

    void Adler32::GetCurrentHashCore(bytecs* destination) const {
        WriteUInt32BigEndian(adler_, destination);
    }

    void Adler32::GetHashAndResetCore(bytecs* destination) {
        WriteUInt32BigEndian(adler_, destination);
        adler_ = InitialState;
    }

    std::vector<bytecs> Adler32::Hash(const bytecs* source, intcs length) {
        std::vector<bytecs> ret(static_cast<size_t>(Size));
        WriteUInt32BigEndian(HashToUInt32(source, length), ret.data());
        return ret;
    }

    bool Adler32::TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        if (destinationLength < Size) {
            bytesWritten = 0;
            return false;
        }
        WriteUInt32BigEndian(HashToUInt32(source, length), destination);
        bytesWritten = Size;
        return true;
    }

    intcs Adler32::Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength) {
        if (destinationLength < Size) {
            throw System::ArgumentException("Destination is too short.", "destination");
        }
        WriteUInt32BigEndian(HashToUInt32(source, length), destination);
        return Size;
    }

    uintcs Adler32::HashToUInt32(const bytecs* source, intcs length) {
        return Update(InitialState, source, length);
    }

} // namespace System::IO::Hashing
