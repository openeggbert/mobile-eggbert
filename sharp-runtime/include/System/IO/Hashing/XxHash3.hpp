// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/IO/Hashing/NonCryptographicHashAlgorithm.hpp"
#include "System/IO/Hashing/XxHash3Shared.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::longcs;
    using SharpRuntime::ulongcs;

    /**
     * @brief Provides an implementation of the XXH3 hash algorithm for generating a 64-bit hash.
     *
     * For methods that persist the computed numerical hash value as bytes, the value is written
     * in big-endian byte order.
     *
     * C++ counterpart of .NET System.IO.Hashing.XxHash3. Based on the XXH3 implementation from
     * https://github.com/Cyan4973/xxHash.
     */
    class XxHash3 final : public NonCryptographicHashAlgorithm {
    private:
        static constexpr intcs Size = 8;

        Detail::XxHash3Shared::State state_;

        explicit XxHash3(const Detail::XxHash3Shared::State& state);

    protected:
        void GetCurrentHashCore(bytecs* destination) const override;

    public:
        /** Initializes a new instance of the XxHash3 class using the default seed value 0. */
        XxHash3();
        /** Initializes a new instance of the XxHash3 class using the specified seed. */
        explicit XxHash3(longcs seed);

        /** Returns a clone of the current instance, with a copy of the current instance's internal state. */
        [[nodiscard]] XxHash3 Clone() const { return XxHash3(state_); }

        void Reset() override;
        using NonCryptographicHashAlgorithm::Append;
        void Append(const bytecs* source, intcs length) override;

        /** Gets the current computed hash value without modifying accumulated state. */
        [[nodiscard]] ulongcs GetCurrentHashAsUInt64() const;

        /** Computes the XXH3 hash of the provided data, using the default seed of 0. */
        [[nodiscard]] static std::vector<bytecs> Hash(const bytecs* source, intcs length);
        /** Computes the XXH3 hash of the provided data using the provided seed. */
        [[nodiscard]] static std::vector<bytecs> Hash(const bytecs* source, intcs length, longcs seed);
        /** Computes the XXH3 hash of the provided data into @p destination using the optionally provided seed. */
        static intcs Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, longcs seed = 0);
        /** Attempts to compute the XXH3 hash of the provided data into @p destination using the optionally provided seed. */
        static bool TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength,
                             intcs& bytesWritten, longcs seed = 0);
        /** Computes the XXH3 hash of the provided data using the optionally provided seed. */
        [[nodiscard]] static ulongcs HashToUInt64(const bytecs* source, intcs length, longcs seed = 0);
    };

} // namespace System::IO::Hashing
