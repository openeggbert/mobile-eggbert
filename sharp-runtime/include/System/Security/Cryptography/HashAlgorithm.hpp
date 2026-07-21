// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Security/Cryptography/CryptographicUnexpectedOperationException.hpp"

namespace System::Security::Cryptography {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief Represents the base class from which all implementations of cryptographic hash
     * algorithms must derive.
     *
     * C++ counterpart of .NET System.Security.Cryptography.HashAlgorithm.
     *
     * @note Reduced scope: does not implement `ICryptoTransform` (out of scope here, see
     * `plan.sqlite3`) and has no `Stream`-based `ComputeHash(Stream)`/`ComputeHashAsync`
     * overloads — callers read a stream into a buffer themselves and call `ComputeHash(buffer)`.
     */
    class HashAlgorithm {
        bool disposed_ = false;

    protected:
        intcs hashSizeValue_ = 0;
        std::vector<bytecs> hashValue_;
        intcs state_ = 0;

        /** @brief Feeds @p count bytes starting at @p offset in @p array into the running hash state. */
        virtual void HashCore(const std::vector<bytecs>& array, intcs offset, intcs count) = 0;

        /** @brief Finalizes the hash computation and returns the digest, without resetting state. */
        virtual std::vector<bytecs> HashFinal() = 0;

    public:
        virtual ~HashAlgorithm() = default;

        /** @return The size, in bits, of the computed hash code. */
        [[nodiscard]] virtual intcs getHashSizeProperty() const { return hashSizeValue_; }

        /**
         * @return The value of the computed hash, or empty if ComputeHash() hasn't been called.
         * @throws CryptographicUnexpectedOperationException if a TransformBlock-style operation
         * is in progress (this reduced port never leaves that state, since TransformBlock/
         * TransformFinalBlock — the ICryptoTransform surface — are out of scope).
         */
        [[nodiscard]] virtual std::vector<bytecs> getHashProperty() const {
            System::ObjectDisposedException::ThrowIf(disposed_, "HashAlgorithm");
            if (state_ != 0) {
                throw CryptographicUnexpectedOperationException("Hash must be finalized before the hash value is retrieved.");
            }
            return hashValue_;
        }

        /** @brief Computes the hash of the entire buffer. */
        [[nodiscard]] std::vector<bytecs> ComputeHash(const std::vector<bytecs>& buffer) {
            System::ObjectDisposedException::ThrowIf(disposed_, "HashAlgorithm");
            HashCore(buffer, 0, static_cast<intcs>(buffer.size()));
            return captureHashCodeAndReinitialize();
        }

        /** @brief Computes the hash of @p count bytes starting at @p offset in @p buffer. */
        [[nodiscard]] std::vector<bytecs> ComputeHash(const std::vector<bytecs>& buffer, intcs offset, intcs count) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(offset, "offset");
            if (count < 0 || count > static_cast<intcs>(buffer.size())) {
                throw System::ArgumentException("Value was invalid.", "count");
            }
            if (static_cast<intcs>(buffer.size()) - count < offset) {
                throw System::ArgumentException("Offset and length were out of bounds for the array.", "offset");
            }
            System::ObjectDisposedException::ThrowIf(disposed_, "HashAlgorithm");

            HashCore(buffer, offset, count);
            return captureHashCodeAndReinitialize();
        }

        /**
         * @brief Attempts to compute the hash of @p source into @p destination.
         * @return false without throwing if @p destination is too small.
         */
        bool TryComputeHash(const std::vector<bytecs>& source, std::vector<bytecs>& destination, intcs& bytesWritten) {
            System::ObjectDisposedException::ThrowIf(disposed_, "HashAlgorithm");
            if (static_cast<intcs>(destination.size()) < hashSizeValue_ / 8) {
                bytesWritten = 0;
                return false;
            }
            HashCore(source, 0, static_cast<intcs>(source.size()));
            std::vector<bytecs> final = HashFinal();
            std::copy(final.begin(), final.end(), destination.begin());
            bytesWritten = static_cast<intcs>(final.size());
            hashValue_.clear();
            Initialize();
            return true;
        }

        /** @brief Resets the hash algorithm to its initial state. */
        virtual void Initialize() = 0;

        /** @brief Releases resources used by this instance. */
        virtual void Dispose() { disposed_ = true; }

    private:
        std::vector<bytecs> captureHashCodeAndReinitialize() {
            hashValue_ = HashFinal();
            std::vector<bytecs> result = hashValue_;
            Initialize();
            return result;
        }
    };

} // namespace System::Security::Cryptography
