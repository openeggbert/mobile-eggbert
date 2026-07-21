// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "System/IO/Hashing/Crc32ParameterSet.hpp"
#include "System/IO/Hashing/NonCryptographicHashAlgorithm.hpp"

namespace System::IO::Hashing {

    /**
     * @brief Provides an implementation of the CRC-32 algorithm.
     *
     * By default, this implementation uses the ITU-T V.42 / IEEE 802.3 parameter set, but other
     * parameter sets can also be specified. For methods that return byte arrays or that write
     * into destination buffers, this implementation emits the answer in the byte order that
     * maintains the CRC residue relationship (CRC(message concat CRC(message)) is a fixed
     * value); for the default parameters this stable output is the little-endian representation
     * of the CRC.
     *
     * C++ counterpart of .NET System.IO.Hashing.Crc32.
     */
    class Crc32 final : public NonCryptographicHashAlgorithm {
    private:
        static constexpr intcs Size = 4;

        uintcs crc_;
        std::shared_ptr<Crc32ParameterSet> parameterSet_;

        Crc32(uintcs crc, std::shared_ptr<Crc32ParameterSet> parameterSet);

    protected:
        void GetCurrentHashCore(bytecs* destination) const override;
        void GetHashAndResetCore(bytecs* destination) override;

    public:
        /** Initializes a new Crc32 instance using the ITU-T V.42 / IEEE 802.3 parameters. */
        Crc32();
        /**
         * @brief Initializes a new Crc32 instance using the specified parameters.
         * @throws System::ArgumentNullException if @p parameterSet is null.
         */
        explicit Crc32(std::shared_ptr<Crc32ParameterSet> parameterSet);

        /** Gets the parameter set used by this instance. */
        [[nodiscard]] const std::shared_ptr<Crc32ParameterSet>& getParameterSetProperty() const { return parameterSet_; }

        /** Returns a clone of the current instance, with a copy of the current instance's internal state. */
        [[nodiscard]] Crc32 Clone() const { return Crc32(crc_, parameterSet_); }

        using NonCryptographicHashAlgorithm::Append;
        void Append(const bytecs* source, intcs length) override;
        void Reset() override;

        /** Gets the current computed hash value without modifying accumulated state. */
        [[nodiscard]] uintcs GetCurrentHashAsUInt32() const;

        /** Computes the CRC-32 hash of the provided data, using the ITU-T V.42 / IEEE 802.3 parameters. */
        [[nodiscard]] static std::vector<bytecs> Hash(const bytecs* source, intcs length);
        /** Computes the CRC-32 hash value for the provided data using the specified parameter set. */
        [[nodiscard]] static std::vector<bytecs> Hash(const std::shared_ptr<Crc32ParameterSet>& parameterSet,
                                                       const bytecs* source, intcs length);
        /** Computes the CRC-32 hash of the provided data into @p destination, using the ITU-T V.42 / IEEE 802.3 parameters. */
        static intcs Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength);
        /** Computes the CRC-32 hash of the provided data into @p destination, using the specified parameters. */
        static intcs Hash(const std::shared_ptr<Crc32ParameterSet>& parameterSet,
                           const bytecs* source, intcs length, bytecs* destination, intcs destinationLength);
        /** Attempts to compute the CRC-32 hash, using the ITU-T V.42 / IEEE 802.3 parameters, into @p destination. */
        static bool TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten);
        /** Attempts to compute the CRC-32 hash, using the specified parameter set, into @p destination. */
        static bool TryHash(const std::shared_ptr<Crc32ParameterSet>& parameterSet,
                             const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten);
        /** Computes the CRC-32 hash of the provided data, using the ITU-T V.42 / IEEE 802.3 parameters. */
        [[nodiscard]] static uintcs HashToUInt32(const bytecs* source, intcs length);
        /** Computes the CRC-32 hash of the provided data, using the specified parameters. */
        [[nodiscard]] static uintcs HashToUInt32(const std::shared_ptr<Crc32ParameterSet>& parameterSet,
                                                  const bytecs* source, intcs length);
    };

} // namespace System::IO::Hashing
