// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "System/IO/Hashing/Crc64ParameterSet.hpp"
#include "System/IO/Hashing/NonCryptographicHashAlgorithm.hpp"

namespace System::IO::Hashing {

    /**
     * @brief Provides an implementation of the CRC-64 algorithm.
     *
     * By default, this implementation uses the ECMA-182 parameter set, but other parameter sets
     * (e.g. NVMe) can also be specified.
     *
     * C++ counterpart of .NET System.IO.Hashing.Crc64.
     */
    class Crc64 final : public NonCryptographicHashAlgorithm {
    private:
        static constexpr intcs Size = 8;

        ulongcs crc_;
        std::shared_ptr<Crc64ParameterSet> parameterSet_;

        Crc64(ulongcs crc, std::shared_ptr<Crc64ParameterSet> parameterSet);

    protected:
        void GetCurrentHashCore(bytecs* destination) const override;
        void GetHashAndResetCore(bytecs* destination) override;

    public:
        /** Initializes a new Crc64 instance using the ECMA-182 parameters. */
        Crc64();
        /**
         * @brief Initializes a new Crc64 instance using the specified parameters.
         * @throws System::ArgumentNullException if @p parameterSet is null.
         */
        explicit Crc64(std::shared_ptr<Crc64ParameterSet> parameterSet);

        /** Gets the parameter set used by this instance. */
        [[nodiscard]] const std::shared_ptr<Crc64ParameterSet>& getParameterSetProperty() const { return parameterSet_; }

        /** Returns a clone of the current instance, with a copy of the current instance's internal state. */
        [[nodiscard]] Crc64 Clone() const { return Crc64(crc_, parameterSet_); }

        using NonCryptographicHashAlgorithm::Append;
        void Append(const bytecs* source, intcs length) override;
        void Reset() override;

        /** Gets the current computed hash value without modifying accumulated state. */
        [[nodiscard]] ulongcs GetCurrentHashAsUInt64() const;

        /** Computes the CRC-64 hash of the provided data, using the ECMA-182 parameters. */
        [[nodiscard]] static std::vector<bytecs> Hash(const bytecs* source, intcs length);
        /** Computes the CRC-64 hash value for the provided data using the specified parameter set. */
        [[nodiscard]] static std::vector<bytecs> Hash(const std::shared_ptr<Crc64ParameterSet>& parameterSet,
                                                       const bytecs* source, intcs length);
        /** Computes the CRC-64 hash of the provided data into @p destination, using the ECMA-182 parameters. */
        static intcs Hash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength);
        /** Computes the CRC-64 hash of the provided data into @p destination, using the specified parameters. */
        static intcs Hash(const std::shared_ptr<Crc64ParameterSet>& parameterSet,
                           const bytecs* source, intcs length, bytecs* destination, intcs destinationLength);
        /** Attempts to compute the CRC-64 hash, using the ECMA-182 parameters, into @p destination. */
        static bool TryHash(const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten);
        /** Attempts to compute the CRC-64 hash, using the specified parameter set, into @p destination. */
        static bool TryHash(const std::shared_ptr<Crc64ParameterSet>& parameterSet,
                             const bytecs* source, intcs length, bytecs* destination, intcs destinationLength, intcs& bytesWritten);
        /** Computes the CRC-64 hash of the provided data, using the ECMA-182 parameters. */
        [[nodiscard]] static ulongcs HashToUInt64(const bytecs* source, intcs length);
        /** Computes the CRC-64 hash of the provided data, using the specified parameters. */
        [[nodiscard]] static ulongcs HashToUInt64(const std::shared_ptr<Crc64ParameterSet>& parameterSet,
                                                   const bytecs* source, intcs length);
    };

} // namespace System::IO::Hashing
