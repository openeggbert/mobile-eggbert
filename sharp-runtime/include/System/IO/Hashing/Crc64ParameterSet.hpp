// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using SharpRuntime::ulongcs;

    /**
     * @brief Represents a set of parameters that define the behavior of a CRC-64 hash algorithm.
     *
     * The parameter-set instance precomputes a 256-entry lookup table to be used in the CRC
     * calculation; callers are expected to create a single instance and reuse it.
     *
     * C++ counterpart of .NET System.IO.Hashing.Crc64ParameterSet. See Crc32ParameterSet's doc
     * comment for why this port uses a single class with a runtime branch on
     * getReflectValuesProperty() instead of .NET's internal reflected/forward class hierarchy.
     */
    class Crc64ParameterSet {
    private:
        ulongcs polynomial_;
        ulongcs initialValue_;
        ulongcs finalXorValue_;
        bool reflectValues_;
        std::vector<ulongcs> lookupTable_;

        Crc64ParameterSet(ulongcs polynomial, ulongcs initialValue, ulongcs finalXorValue, bool reflectValues);

    public:
        /** Gets the polynomial value used for the CRC calculation. */
        [[nodiscard]] ulongcs getPolynomialProperty() const { return polynomial_; }
        /** Gets the initial value (seed) for the CRC calculation. */
        [[nodiscard]] ulongcs getInitialValueProperty() const { return initialValue_; }
        /** Gets the value to XOR with the final CRC result. */
        [[nodiscard]] ulongcs getFinalXorValueProperty() const { return finalXorValue_; }
        /**
         * @brief Gets whether the input and output bytes are most-significant-bit (MSB) first, or last.
         *
         * true if the MSB is the least significant bit of the last byte; false if the MSB is the
         * most significant bit of the first byte.
         */
        [[nodiscard]] bool getReflectValuesProperty() const { return reflectValues_; }

        /** Creates a new Crc64ParameterSet with the specified parameters. */
        [[nodiscard]] static std::shared_ptr<Crc64ParameterSet> Create(
            ulongcs polynomial, ulongcs initialValue, ulongcs finalXorValue, bool reflectValues);

        /** Gets the parameter set for the ECMA-182 variant of CRC-64. */
        [[nodiscard]] static const std::shared_ptr<Crc64ParameterSet>& getCrc64Property();
        /** Gets the parameter set used for CRC-64 in Non-Volatile Memory Express (NVMe). */
        [[nodiscard]] static const std::shared_ptr<Crc64ParameterSet>& getNvmeProperty();

        /** Updates @p value with @p length bytes from @p source using this parameter set's lookup table. */
        [[nodiscard]] ulongcs Update(ulongcs value, const bytecs* source, intcs length) const;

        /** Applies the final XOR to @p value. */
        [[nodiscard]] ulongcs Finalize(ulongcs value) const { return value ^ finalXorValue_; }

        /** Writes @p crc to @p destination (8 bytes) in the byte order matching getReflectValuesProperty(). */
        void WriteCrcToSpan(ulongcs crc, bytecs* destination) const;
    };

} // namespace System::IO::Hashing
