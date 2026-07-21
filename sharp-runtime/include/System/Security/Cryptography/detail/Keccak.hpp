// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

// Not a public System::* API — an internal Keccak-f[1600] permutation and sponge construction
// shared by SHA3-256/384/512 (fixed-output, domain separator 0x06) and Shake128/256
// (extendable-output, domain separator 0x1F). Kept out of the public namespace so it isn't
// mistaken for a real, independently-portable System.Security.Cryptography type. Verified against
// the FIPS 202 reference algorithm (24 rounds, rotation offsets and round constants below are the
// standard published Keccak values).
namespace SharpRuntimeDetail::Keccak {

    inline uint64_t rotl(uint64_t x, unsigned n) { return n == 0 ? x : (x << n) | (x >> (64 - n)); }

    // Standard FIPS 202 round constants for the iota step (24 rounds).
    inline constexpr uint64_t kRoundConstants[24] = {
        0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
        0x000000000000808bULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
        0x000000000000008aULL, 0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
        0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
        0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
        0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL,
    };

    // Standard FIPS 202 rotation offsets, indexed [x][y] (5x5 lane grid).
    inline constexpr unsigned kRho[5][5] = {
        {0, 36, 3, 41, 18}, {1, 44, 10, 45, 2}, {62, 6, 43, 15, 61}, {28, 55, 25, 21, 56}, {27, 20, 39, 8, 14},
    };

    /** @brief The Keccak-f[1600] permutation over a 25-lane (200-byte) state. */
    inline void permute(std::array<uint64_t, 25>& a) {
        for (int round = 0; round < 24; ++round) {
            // Theta
            uint64_t c[5];
            for (int x = 0; x < 5; ++x) c[x] = a[x] ^ a[x + 5] ^ a[x + 10] ^ a[x + 15] ^ a[x + 20];
            uint64_t d[5];
            for (int x = 0; x < 5; ++x) d[x] = c[(x + 4) % 5] ^ rotl(c[(x + 1) % 5], 1);
            for (int x = 0; x < 5; ++x)
                for (int y = 0; y < 5; ++y) a[x + 5 * y] ^= d[x];

            // Rho + Pi
            std::array<uint64_t, 25> b{};
            for (int x = 0; x < 5; ++x)
                for (int y = 0; y < 5; ++y)
                    b[y + 5 * ((2 * x + 3 * y) % 5)] = rotl(a[x + 5 * y], kRho[x][y]);

            // Chi
            for (int x = 0; x < 5; ++x)
                for (int y = 0; y < 5; ++y)
                    a[x + 5 * y] = b[x + 5 * y] ^ ((~b[(x + 1) % 5 + 5 * y]) & b[(x + 2) % 5 + 5 * y]);

            // Iota
            a[0] ^= kRoundConstants[round];
        }
    }

    /**
     * @brief A Keccak sponge with a given byte rate, supporting incremental absorption and
     * squeezing (i.e. extendable-output). @p domainSeparator is the SHA-3 padding suffix (0x06
     * for the fixed-output SHA3-* family, 0x1F for the SHAKE* extendable-output family).
     */
    class Sponge {
        std::array<uint64_t, 25> state_{};
        std::array<uint8_t, 200> buffer_{};
        size_t bufferLen_ = 0;
        size_t rateBytes_;
        uint8_t domainSeparator_;
        bool squeezing_ = false;

        void absorbBlock() {
            for (size_t i = 0; i < rateBytes_; ++i) {
                size_t lane = i / 8, byteInLane = i % 8;
                state_[lane] ^= static_cast<uint64_t>(buffer_[i]) << (8 * byteInLane);
            }
            permute(state_);
        }

    public:
        explicit Sponge(size_t rateBytes, uint8_t domainSeparator)
            : rateBytes_(rateBytes), domainSeparator_(domainSeparator) {}

        void Reset() {
            state_.fill(0);
            buffer_.fill(0);
            bufferLen_ = 0;
            squeezing_ = false;
        }

        void Absorb(const uint8_t* data, size_t len) {
            while (len > 0) {
                size_t take = std::min(len, rateBytes_ - bufferLen_);
                std::memcpy(buffer_.data() + bufferLen_, data, take);
                bufferLen_ += take;
                data += take;
                len -= take;
                if (bufferLen_ == rateBytes_) {
                    absorbBlock();
                    bufferLen_ = 0;
                }
            }
        }

        /** @brief Pads and absorbs the final block, transitioning the sponge into squeeze mode. */
        void FinishAbsorbing() {
            std::memset(buffer_.data() + bufferLen_, 0, rateBytes_ - bufferLen_);
            buffer_[bufferLen_] |= domainSeparator_;
            buffer_[rateBytes_ - 1] |= 0x80;
            absorbBlock();
            bufferLen_ = 0;
            squeezing_ = true;
        }

        /** @brief Squeezes @p len output bytes into @p out. May be called repeatedly for XOFs. */
        void Squeeze(uint8_t* out, size_t len) {
            while (len > 0) {
                if (bufferLen_ == 0) {
                    for (size_t i = 0; i < rateBytes_; ++i) {
                        size_t lane = i / 8, byteInLane = i % 8;
                        buffer_[i] = static_cast<uint8_t>(state_[lane] >> (8 * byteInLane));
                    }
                }
                size_t take = std::min(len, rateBytes_ - bufferLen_);
                std::memcpy(out, buffer_.data() + bufferLen_, take);
                out += take;
                len -= take;
                bufferLen_ += take;
                if (bufferLen_ == rateBytes_) {
                    permute(state_);
                    bufferLen_ = 0;
                }
            }
        }

        [[nodiscard]] bool IsSqueezing() const { return squeezing_; }
    };

} // namespace SharpRuntimeDetail::Keccak
