// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/NonCryptographicHashAlgorithm.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::IO::Hashing {

    NonCryptographicHashAlgorithm::NonCryptographicHashAlgorithm(intcs hashLengthInBytes)
        : hashLengthInBytes_(hashLengthInBytes)
    {
        if (hashLengthInBytes < 1) {
            throw System::ArgumentOutOfRangeException("hashLengthInBytes");
        }
    }

    void NonCryptographicHashAlgorithm::GetHashAndResetCore(bytecs* destination) {
        GetCurrentHashCore(destination);
        Reset();
    }

    void NonCryptographicHashAlgorithm::ThrowDestinationTooShort() {
        throw System::ArgumentException("Destination is too short.", "destination");
    }

    void NonCryptographicHashAlgorithm::Append(const std::vector<bytecs>& source) {
        Append(source.data(), static_cast<intcs>(source.size()));
    }

    void NonCryptographicHashAlgorithm::Append(System::IO::Stream& stream) {
        bytecs buffer[65536];
        intcs n;
        while ((n = stream.Read(buffer, 0, static_cast<intcs>(sizeof(buffer)))) > 0) {
            Append(buffer, n);
        }
    }

    std::vector<bytecs> NonCryptographicHashAlgorithm::GetCurrentHash() const {
        std::vector<bytecs> ret(static_cast<size_t>(hashLengthInBytes_));
        GetCurrentHashCore(ret.data());
        return ret;
    }

    bool NonCryptographicHashAlgorithm::TryGetCurrentHash(bytecs* destination, intcs destinationLength, intcs& bytesWritten) const {
        if (destinationLength < hashLengthInBytes_) {
            bytesWritten = 0;
            return false;
        }
        GetCurrentHashCore(destination);
        bytesWritten = hashLengthInBytes_;
        return true;
    }

    intcs NonCryptographicHashAlgorithm::GetCurrentHash(bytecs* destination, intcs destinationLength) const {
        if (destinationLength < hashLengthInBytes_) {
            ThrowDestinationTooShort();
        }
        GetCurrentHashCore(destination);
        return hashLengthInBytes_;
    }

    std::vector<bytecs> NonCryptographicHashAlgorithm::GetHashAndReset() {
        std::vector<bytecs> ret(static_cast<size_t>(hashLengthInBytes_));
        GetHashAndResetCore(ret.data());
        return ret;
    }

    bool NonCryptographicHashAlgorithm::TryGetHashAndReset(bytecs* destination, intcs destinationLength, intcs& bytesWritten) {
        if (destinationLength < hashLengthInBytes_) {
            bytesWritten = 0;
            return false;
        }
        GetHashAndResetCore(destination);
        bytesWritten = hashLengthInBytes_;
        return true;
    }

    intcs NonCryptographicHashAlgorithm::GetHashAndReset(bytecs* destination, intcs destinationLength) {
        if (destinationLength < hashLengthInBytes_) {
            ThrowDestinationTooShort();
        }
        GetHashAndResetCore(destination);
        return hashLengthInBytes_;
    }

} // namespace System::IO::Hashing
