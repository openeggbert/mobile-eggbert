// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/IO/Hashing/NonCryptographicHashAlgorithm.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::longcs;
    using SharpRuntime::ulongcs;

    /**
     * @brief xxHash64 — fast 64-bit non-cryptographic hash (xxHash by Yann Collet).
     *
     * C++ counterpart of .NET System.IO.Hashing.XxHash64.
     */
    class XxHash64 final : public NonCryptographicHashAlgorithm {
    private:
        static constexpr intcs Size = 8;
        static constexpr ulongcs Prime1 = 0x9E3779B185EBCA87ull;
        static constexpr ulongcs Prime2 = 0xC2B2AE3D27D4EB4Full;
        static constexpr ulongcs Prime3 = 0x165667B19E3779F9ull;
        static constexpr ulongcs Prime4 = 0x85EBCA77C2B2AE63ull;
        static constexpr ulongcs Prime5 = 0x27D4EB2F165667C5ull;

        ulongcs seed_;
        ulongcs v1_, v2_, v3_, v4_;
        ulongcs totalLength_ = 0;
        bytecs  buf_[32] = {};
        intcs   bufLen_  = 0;

        void initState();
        static ulongcs rotl64(ulongcs v, int n);
        static ulongcs round(ulongcs acc, ulongcs input);
        static ulongcs mergeAccumulator(ulongcs h64, ulongcs acc);
        void processBlock(const bytecs* block);

        XxHash64(ulongcs seed, ulongcs v1, ulongcs v2, ulongcs v3, ulongcs v4,
                 ulongcs totalLength, const bytecs* buf, intcs bufLen);

    protected:
        void GetCurrentHashCore(bytecs* destination) const override;

    public:
        /** Constructs the hasher with an optional seed value. */
        explicit XxHash64(longcs seed = 0);

        /** Returns a clone of the current instance, with a copy of the current instance's internal state. */
        [[nodiscard]] XxHash64 Clone() const;

        void Reset() override;
        using NonCryptographicHashAlgorithm::Append;
        void Append(const bytecs* source, intcs length) override;

        /** Returns the current hash as a native 64-bit integer, without modifying accumulated state. */
        [[nodiscard]] ulongcs GetCurrentHashAsUInt64() const;

        /** Computes the xxHash64 hash of the provided data. */
        [[nodiscard]] static std::vector<bytecs> Hash(const bytecs* source, intcs length, longcs seed = 0);
        /** Computes the xxHash64 hash of the provided data into @p destination. */
        static intcs Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, longcs seed = 0);
        /** Attempts to compute the xxHash64 hash of the provided data into @p destination. */
        static bool TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength,
                             intcs& bytesWritten, longcs seed = 0);
        /** Computes the xxHash64 hash of the provided data. */
        [[nodiscard]] static ulongcs HashToUInt64(const bytecs* source, intcs length, longcs seed = 0);
    };

} // namespace System::IO::Hashing
