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
    using SharpRuntime::uintcs;

    /**
     * @brief Represents a set of parameters that define the behavior of a CRC-32 hash algorithm.
     *
     * The parameter-set instance precomputes a 256-entry lookup table to be used in the CRC
     * calculation; callers are expected to create a single instance and reuse it.
     *
     * C++ counterpart of .NET System.IO.Hashing.Crc32ParameterSet. .NET splits the reflected
     * and forward (non-reflected) variants into separate internal class hierarchies to enable
     * per-variant intrinsics; this port keeps a single class with a runtime branch on
     * getReflectValuesProperty(), since that branching was purely an internal performance
     * detail, not part of the public contract.
     */
    class Crc32ParameterSet {
    private:
        uintcs polynomial_;
        uintcs initialValue_;
        uintcs finalXorValue_;
        bool reflectValues_;
        std::vector<uintcs> lookupTable_;

        Crc32ParameterSet(uintcs polynomial, uintcs initialValue, uintcs finalXorValue, bool reflectValues);

    public:
        /** Gets the polynomial value used for the CRC calculation. */
        [[nodiscard]] uintcs getPolynomialProperty() const { return polynomial_; }
        /** Gets the initial value (seed) for the CRC calculation. */
        [[nodiscard]] uintcs getInitialValueProperty() const { return initialValue_; }
        /** Gets the value to XOR with the final CRC result. */
        [[nodiscard]] uintcs getFinalXorValueProperty() const { return finalXorValue_; }
        /**
         * @brief Gets whether the input and output bytes are most-significant-bit (MSB) first, or last.
         *
         * true if the MSB is the least significant bit of the last byte; false if the MSB is the
         * most significant bit of the first byte.
         */
        [[nodiscard]] bool getReflectValuesProperty() const { return reflectValues_; }

        /** Creates a new Crc32ParameterSet with the specified parameters. */
        [[nodiscard]] static std::shared_ptr<Crc32ParameterSet> Create(
            uintcs polynomial, uintcs initialValue, uintcs finalXorValue, bool reflectValues);

        /** Gets the parameter set for the variant of CRC-32 as used in ITU-T V.42 and IEEE 802.3. */
        [[nodiscard]] static const std::shared_ptr<Crc32ParameterSet>& getCrc32Property();
        /** Gets the parameter set for the CRC-32C variant of CRC-32. */
        [[nodiscard]] static const std::shared_ptr<Crc32ParameterSet>& getCrc32CProperty();

        /** Updates @p value with @p length bytes from @p source using this parameter set's lookup table. */
        [[nodiscard]] uintcs Update(uintcs value, const bytecs* source, intcs length) const;

        /** Applies the final XOR to @p value. */
        [[nodiscard]] uintcs Finalize(uintcs value) const { return value ^ finalXorValue_; }

        /** Writes @p crc to @p destination (4 bytes) in the byte order matching getReflectValuesProperty(). */
        void WriteCrcToSpan(uintcs crc, bytecs* destination) const;
    };

} // namespace System::IO::Hashing
