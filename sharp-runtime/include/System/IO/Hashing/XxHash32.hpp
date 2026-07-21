// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/IO/Hashing/NonCryptographicHashAlgorithm.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::uintcs;

    /**
     * @brief xxHash32 — fast 32-bit non-cryptographic hash (xxHash by Yann Collet).
     *
     * C++ counterpart of .NET System.IO.Hashing.XxHash32.
     */
    class XxHash32 final : public NonCryptographicHashAlgorithm {
    private:
        static constexpr intcs Size = 4;
        static constexpr uintcs Prime1 = 0x9E3779B1u;
        static constexpr uintcs Prime2 = 0x85EBCA77u;
        static constexpr uintcs Prime3 = 0xC2B2AE3Du;
        static constexpr uintcs Prime4 = 0x27D4EB2Fu;
        static constexpr uintcs Prime5 = 0x165667B1u;

        uintcs seed_;
        uintcs v1_, v2_, v3_, v4_;
        SharpRuntime::ulongcs totalLength_ = 0;
        bytecs buf_[16] = {};
        intcs  bufLen_  = 0;

        void initState();
        static uintcs rotl32(uintcs v, int n);
        static uintcs round(uintcs acc, uintcs input);
        void processBlock(const bytecs* block);

        XxHash32(uintcs seed, uintcs v1, uintcs v2, uintcs v3, uintcs v4,
                 SharpRuntime::ulongcs totalLength, const bytecs* buf, intcs bufLen);

    protected:
        void GetCurrentHashCore(bytecs* destination) const override;

    public:
        /** Constructs the hasher with an optional seed value. */
        explicit XxHash32(intcs seed = 0);

        /** Returns a clone of the current instance, with a copy of the current instance's internal state. */
        [[nodiscard]] XxHash32 Clone() const;

        void Reset() override;
        using NonCryptographicHashAlgorithm::Append;
        void Append(const bytecs* source, intcs length) override;

        /** Returns the current hash as a native 32-bit integer, without modifying accumulated state. */
        [[nodiscard]] uintcs GetCurrentHashAsUInt32() const;

        /** Computes the xxHash32 hash of the provided data. */
        [[nodiscard]] static std::vector<bytecs> Hash(const bytecs* source, intcs length, intcs seed = 0);
        /** Computes the xxHash32 hash of the provided data into @p destination. */
        static intcs Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs seed = 0);
        /** Attempts to compute the xxHash32 hash of the provided data into @p destination. */
        static bool TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength,
                             intcs& bytesWritten, intcs seed = 0);
        /** Computes the xxHash32 hash of the provided data. */
        [[nodiscard]] static uintcs HashToUInt32(const bytecs* source, intcs length, intcs seed = 0);
    };

} // namespace System::IO::Hashing
