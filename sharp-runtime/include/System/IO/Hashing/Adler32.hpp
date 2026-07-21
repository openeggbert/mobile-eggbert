// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/IO/Hashing/NonCryptographicHashAlgorithm.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::uintcs;

    /**
     * @brief Provides an implementation of the Adler-32 checksum algorithm, as specified in
     * RFC 1950.
     *
     * This algorithm produces a 32-bit checksum and is commonly used in data compression
     * formats such as zlib. It is not suitable for cryptographic purposes.
     *
     * C++ counterpart of .NET System.IO.Hashing.Adler32.
     */
    class Adler32 final : public NonCryptographicHashAlgorithm {
    private:
        static constexpr intcs Size = 4;
        static constexpr uintcs InitialState = 1u;

        uintcs adler_ = InitialState;

        explicit Adler32(uintcs adler);

    protected:
        void GetCurrentHashCore(bytecs* destination) const override;
        void GetHashAndResetCore(bytecs* destination) override;

    public:
        /** Initializes a new instance of the Adler32 class. */
        Adler32();

        /** Returns a clone of the current instance, with a copy of the current instance's internal state. */
        [[nodiscard]] Adler32 Clone() const { return Adler32(adler_); }

        using NonCryptographicHashAlgorithm::Append;
        void Append(const bytecs* source, intcs length) override;
        void Reset() override;

        /** Gets the current computed hash value without modifying accumulated state. */
        [[nodiscard]] uintcs GetCurrentHashAsUInt32() const { return adler_; }

        /** Computes the Adler-32 hash of the provided data. */
        [[nodiscard]] static std::vector<bytecs> Hash(const bytecs* source, intcs length);
        /** Attempts to compute the Adler-32 hash of the provided data into @p destination. */
        static bool TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten);
        /** Computes the Adler-32 hash of the provided data into @p destination. */
        static intcs Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength);
        /** Computes the Adler-32 hash of the provided data. */
        [[nodiscard]] static uintcs HashToUInt32(const bytecs* source, intcs length);
    };

} // namespace System::IO::Hashing
