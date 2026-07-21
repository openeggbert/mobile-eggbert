// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/Stream.hpp"

namespace System::IO::Hashing {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief Represents a non-cryptographic hash algorithm.
     *
     * C++ counterpart of .NET System.IO.Hashing.NonCryptographicHashAlgorithm. Operates on raw
     * pointer+length buffers in place of .NET's ReadOnlySpan<byte>/Span<byte>; async Append is
     * not ported (no async infrastructure in this runtime).
     */
    class NonCryptographicHashAlgorithm {
    private:
        intcs hashLengthInBytes_;

    protected:
        /**
         * @brief Called from constructors in derived classes to initialize the base class.
         * @throws System::ArgumentOutOfRangeException if @p hashLengthInBytes is less than 1.
         */
        explicit NonCryptographicHashAlgorithm(intcs hashLengthInBytes);

        /**
         * @brief Writes the computed hash value to @p destination then clears the accumulated state.
         *
         * @p destination is guaranteed to be exactly getHashLengthInBytesProperty() bytes.
         * Default implementation calls GetCurrentHashCore() followed by Reset().
         */
        virtual void GetHashAndResetCore(bytecs* destination);

        /**
         * @brief Writes the computed hash value to @p destination without modifying accumulated state.
         *
         * @p destination is guaranteed to be exactly getHashLengthInBytesProperty() bytes.
         */
        virtual void GetCurrentHashCore(bytecs* destination) const = 0;

        /** Throws System::ArgumentException("Destination is too short.", "destination"). */
        [[noreturn]] static void ThrowDestinationTooShort();

    public:
        virtual ~NonCryptographicHashAlgorithm() = default;

        /** Gets the number of bytes produced from this hash algorithm. */
        [[nodiscard]] intcs getHashLengthInBytesProperty() const { return hashLengthInBytes_; }

        /** Appends the contents of @p source (@p length bytes) to the data already processed for the current hash computation. */
        virtual void Append(const bytecs* source, intcs length) = 0;

        /** Appends the contents of @p source to the data already processed for the current hash computation. */
        void Append(const std::vector<bytecs>& source);

        /** Appends the entire contents of @p stream to the data already processed for the current hash computation. */
        void Append(System::IO::Stream& stream);

        /** Resets the hash computation to the initial state. */
        virtual void Reset() = 0;

        /** Gets the current computed hash value without modifying accumulated state. */
        [[nodiscard]] std::vector<bytecs> GetCurrentHash() const;

        /**
         * @brief Attempts to write the computed hash value to @p destination without modifying accumulated state.
         * @return true if @p destinationLength is at least getHashLengthInBytesProperty().
         */
        bool TryGetCurrentHash(bytecs* destination, intcs destinationLength, intcs& bytesWritten) const;

        /**
         * @brief Writes the computed hash value to @p destination without modifying accumulated state.
         * @throws System::ArgumentException if @p destinationLength is shorter than getHashLengthInBytesProperty().
         */
        intcs GetCurrentHash(bytecs* destination, intcs destinationLength) const;

        /** Gets the current computed hash value and clears the accumulated state. */
        std::vector<bytecs> GetHashAndReset();

        /**
         * @brief Attempts to write the computed hash value to @p destination, clearing the accumulated state on success.
         * @return true if @p destinationLength is at least getHashLengthInBytesProperty().
         */
        bool TryGetHashAndReset(bytecs* destination, intcs destinationLength, intcs& bytesWritten);

        /**
         * @brief Writes the computed hash value to @p destination then clears the accumulated state.
         * @throws System::ArgumentException if @p destinationLength is shorter than getHashLengthInBytesProperty().
         */
        intcs GetHashAndReset(bytecs* destination, intcs destinationLength);
    };

} // namespace System::IO::Hashing
