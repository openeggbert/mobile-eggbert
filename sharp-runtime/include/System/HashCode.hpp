// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>
#include "System/Collections/Generic/IEqualityComparer.hpp"
#include "System/Span.hpp"

namespace System {

    /**
     * @brief Provides a hash code accumulator that combines multiple values into
     * a single hash code.
     *
     * This is a C++ counterpart of .NET System.HashCode.
     * The internal algorithm is FNV-1a; exact hash values will differ from the
     * .NET xxHash32-based implementation, but the API contract is identical —
     * including .NET's deliberate non-determinism: like .NET, the initial state
     * is mixed with a random seed generated once per process, so Combine()/
     * ToHashCode() results are consistent within a run but differ across runs
     * (by design, to discourage persisting hash codes and to resist
     * hash-flooding). GetHashCode()/Equals(object) are intentionally not
     * ported: .NET only defines them to make HashCode itself throw
     * NotSupportedException if misused as a hashable/comparable value, a
     * concern that doesn't apply here since nothing in C++ implicitly calls
     * them on a plain struct.
     */
    class HashCode {
        static uint32_t GlobalSeed() noexcept {
            static const uint32_t seed = std::random_device{}();
            return seed;
        }

        uint32_t hash_ = 2166136261u ^ GlobalSeed(); // FNV offset basis, mixed with the per-process seed

        static constexpr uint32_t FNV_PRIME = 16777619u;

        void mix(uint32_t value) noexcept {
            hash_ ^= value;
            hash_ *= FNV_PRIME;
        }

    public:
        /**
         * @brief Adds a single value to the hash code.
         * @tparam T The type of the value to add.
         * @param value The value to add.
         */
        template<typename T>
        void Add(const T& value) noexcept {
            mix(static_cast<uint32_t>(std::hash<T>{}(value)));
        }

        /**
         * @brief Adds a single value to the hash code, using the specified comparer.
         * @tparam T The type of the value to add.
         * @param value The value to add.
         * @param comparer The comparer whose GetHashCode is used.
         */
        template<typename T>
        void Add(const T& value, const System::Collections::Generic::IEqualityComparer<T>& comparer) noexcept {
            mix(static_cast<uint32_t>(comparer.GetHashCode(value)));
        }

        /**
         * @brief Adds a span of bytes to the hash code.
         * @param data Pointer to the byte array.
         * @param length Number of bytes.
         */
        void AddBytes(const uint8_t* data, std::size_t length) noexcept {
            for (std::size_t i = 0; i < length; ++i)
                mix(static_cast<uint32_t>(data[i]));
        }

        /**
         * @brief Adds a vector of bytes to the hash code.
         * @param bytes The bytes to add.
         */
        void AddBytes(const std::vector<uint8_t>& bytes) noexcept {
            AddBytes(bytes.data(), bytes.size());
        }

        /**
         * @brief Adds a span of bytes to the hash code.
         * C++ counterpart of .NET HashCode.AddBytes(ReadOnlySpan&lt;byte&gt;).
         * @param value The span of bytes to add.
         */
        void AddBytes(const ReadOnlySpan<uint8_t>& value) noexcept {
            AddBytes(value.getPointer(), static_cast<std::size_t>(value.getLengthProperty()));
        }

        /**
         * @brief Returns the final hash code produced by the accumulated values.
         * @return The computed hash code.
         */
        [[nodiscard]] int ToHashCode() const noexcept {
            return static_cast<int>(hash_);
        }

        /** @brief Combines one value into a single hash code. */
        template<typename T1>
        static int Combine(const T1& v1) {
            HashCode hc; hc.Add(v1); return hc.ToHashCode();
        }
        /** @brief Combines two values into a single hash code. */
        template<typename T1, typename T2>
        static int Combine(const T1& v1, const T2& v2) {
            HashCode hc; hc.Add(v1); hc.Add(v2); return hc.ToHashCode();
        }
        /** @brief Combines three values into a single hash code. */
        template<typename T1, typename T2, typename T3>
        static int Combine(const T1& v1, const T2& v2, const T3& v3) {
            HashCode hc; hc.Add(v1); hc.Add(v2); hc.Add(v3); return hc.ToHashCode();
        }
        /** @brief Combines four values into a single hash code. */
        template<typename T1, typename T2, typename T3, typename T4>
        static int Combine(const T1& v1, const T2& v2, const T3& v3, const T4& v4) {
            HashCode hc; hc.Add(v1); hc.Add(v2); hc.Add(v3); hc.Add(v4); return hc.ToHashCode();
        }
        /** @brief Combines five values into a single hash code. */
        template<typename T1, typename T2, typename T3, typename T4, typename T5>
        static int Combine(const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) {
            HashCode hc; hc.Add(v1); hc.Add(v2); hc.Add(v3); hc.Add(v4); hc.Add(v5);
            return hc.ToHashCode();
        }
        /** @brief Combines six values into a single hash code. */
        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        static int Combine(const T1& v1, const T2& v2, const T3& v3, const T4& v4,
                           const T5& v5, const T6& v6) {
            HashCode hc; hc.Add(v1); hc.Add(v2); hc.Add(v3); hc.Add(v4); hc.Add(v5); hc.Add(v6);
            return hc.ToHashCode();
        }
        /** @brief Combines seven values into a single hash code. */
        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
        static int Combine(const T1& v1, const T2& v2, const T3& v3, const T4& v4,
                           const T5& v5, const T6& v6, const T7& v7) {
            HashCode hc; hc.Add(v1); hc.Add(v2); hc.Add(v3); hc.Add(v4);
            hc.Add(v5); hc.Add(v6); hc.Add(v7);
            return hc.ToHashCode();
        }
        /** @brief Combines eight values into a single hash code. */
        template<typename T1, typename T2, typename T3, typename T4,
                 typename T5, typename T6, typename T7, typename T8>
        static int Combine(const T1& v1, const T2& v2, const T3& v3, const T4& v4,
                           const T5& v5, const T6& v6, const T7& v7, const T8& v8) {
            HashCode hc; hc.Add(v1); hc.Add(v2); hc.Add(v3); hc.Add(v4);
            hc.Add(v5); hc.Add(v6); hc.Add(v7); hc.Add(v8);
            return hc.ToHashCode();
        }
    };

} // namespace System
