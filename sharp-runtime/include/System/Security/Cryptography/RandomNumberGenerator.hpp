// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Security::Cryptography {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief Represents an abstract class from which all implementations of cryptographic random
     * number generators must derive.
     *
     * C++ counterpart of .NET System.Security.Cryptography.RandomNumberGenerator.
     *
     * @note Reduced scope: the template-heavy convenience helpers `GetItems<T>`/`GetString`/
     * `GetHexString`/`Shuffle<T>` are not reproduced — this port covers the core `GetBytes`/
     * `Fill`/`GetInt32` surface, which is what the overwhelming majority of callers need.
     * `Create(string)` (the CryptoConfig-based named-algorithm factory) is also omitted, matching
     * this runtime's stance on `CryptoConfig` (reflection-based, out of scope).
     */
    class RandomNumberGenerator {
    public:
        virtual ~RandomNumberGenerator() = default;

        /** @brief Fills the entire vector with cryptographically strong random bytes. */
        virtual void GetBytes(std::vector<bytecs>& data) = 0;

        /** @brief Fills @p count bytes starting at @p offset with cryptographically strong random bytes. */
        virtual void GetBytes(std::vector<bytecs>& data, intcs offset, intcs count) {
            verifyGetBytes(data, offset, count);
            if (count <= 0) return;
            if (offset == 0 && count == static_cast<intcs>(data.size())) {
                GetBytes(data);
                return;
            }
            std::vector<bytecs> temp(static_cast<size_t>(count));
            GetBytes(temp);
            std::copy(temp.begin(), temp.end(), data.begin() + offset);
        }

        /** @brief Fills the vector with non-zero cryptographically strong random bytes. */
        virtual void GetNonZeroBytes(std::vector<bytecs>& data) {
            std::vector<bytecs> temp(data.size());
            for (size_t i = 0; i < data.size();) {
                GetBytes(temp);
                for (size_t j = 0; j < temp.size() && i < data.size(); ++j) {
                    if (temp[j] != 0) {
                        data[i++] = temp[j];
                    }
                }
            }
        }

        /** @brief Releases resources used by this instance. */
        virtual void Dispose() {}

        /** @brief Creates a new instance of the default OS-backed random number generator. */
        [[nodiscard]] static std::shared_ptr<RandomNumberGenerator> Create();

        /** @brief Fills @p data with cryptographically strong random bytes using the default generator. */
        static void Fill(std::vector<bytecs>& data);

        /** @return A new vector of @p count cryptographically strong random bytes. */
        [[nodiscard]] static std::vector<bytecs> GetBytes(intcs count) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
            std::vector<bytecs> result(static_cast<size_t>(count));
            Fill(result);
            return result;
        }

        /** @return A random intcs in [fromInclusive, toExclusive). */
        [[nodiscard]] static intcs GetInt32(intcs fromInclusive, intcs toExclusive) {
            if (fromInclusive >= toExclusive) {
                throw System::ArgumentException("fromInclusive must be less than toExclusive.");
            }
            uint32_t range = static_cast<uint32_t>(toExclusive) - static_cast<uint32_t>(fromInclusive) - 1;
            if (range == 0) return fromInclusive;

            uint32_t mask = range;
            mask |= mask >> 1;
            mask |= mask >> 2;
            mask |= mask >> 4;
            mask |= mask >> 8;
            mask |= mask >> 16;

            uint32_t result;
            std::vector<bytecs> buffer(4);
            do {
                Fill(buffer);
                result = static_cast<uint32_t>(buffer[0]) | (static_cast<uint32_t>(buffer[1]) << 8) |
                         (static_cast<uint32_t>(buffer[2]) << 16) | (static_cast<uint32_t>(buffer[3]) << 24);
                result &= mask;
            } while (result > range);

            return static_cast<intcs>(result) + fromInclusive;
        }

        /** @return A random intcs in [0, toExclusive). */
        [[nodiscard]] static intcs GetInt32(intcs toExclusive) {
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(toExclusive, "toExclusive");
            return GetInt32(0, toExclusive);
        }

    protected:
        static void verifyGetBytes(const std::vector<bytecs>& data, intcs offset, intcs count) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(offset, "offset");
            System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
            if (count > static_cast<intcs>(data.size()) - offset) {
                throw System::ArgumentException("Offset and length were out of bounds for the array.");
            }
        }
    };

} // namespace System::Security::Cryptography
