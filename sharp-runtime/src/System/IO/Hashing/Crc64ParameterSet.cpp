// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/Crc64ParameterSet.hpp"

namespace System::IO::Hashing {

    namespace {
        ulongcs ReverseBits(ulongcs value) {
            value = ((value & 0xAAAAAAAAAAAAAAAAull) >> 1) | ((value & 0x5555555555555555ull) << 1);
            value = ((value & 0xCCCCCCCCCCCCCCCCull) >> 2) | ((value & 0x3333333333333333ull) << 2);
            value = ((value & 0xF0F0F0F0F0F0F0F0ull) >> 4) | ((value & 0x0F0F0F0F0F0F0F0Full) << 4);

            value = ((value & 0xFF00FF00FF00FF00ull) >> 8) | ((value & 0x00FF00FF00FF00FFull) << 8);
            value = ((value & 0xFFFF0000FFFF0000ull) >> 16) | ((value & 0x0000FFFF0000FFFFull) << 16);
            return (value >> 32) | (value << 32);
        }

        std::vector<ulongcs> GenerateLookupTable(ulongcs polynomial, bool reflectInput) {
            std::vector<ulongcs> table(256, 0ull);

            if (!reflectInput) {
                ulongcs crc = 0x8000000000000000ull;
                for (intcs i = 1; i < 256; i <<= 1) {
                    crc = (crc & 0x8000000000000000ull) ? (crc << 1) ^ polynomial : crc << 1;
                    for (intcs j = 0; j < i; ++j) {
                        table[static_cast<size_t>(i + j)] = crc ^ table[static_cast<size_t>(j)];
                    }
                }
            } else {
                for (intcs i = 1; i < 256; ++i) {
                    ulongcs r = ReverseBits(static_cast<ulongcs>(i));
                    constexpr ulongcs LastBit = 0x8000000000000000ull;
                    for (intcs j = 0; j < 8; ++j) {
                        r = (r & LastBit) ? (r << 1) ^ polynomial : r << 1;
                    }
                    table[static_cast<size_t>(i)] = ReverseBits(r);
                }
            }

            return table;
        }
    }

    Crc64ParameterSet::Crc64ParameterSet(ulongcs polynomial, ulongcs initialValue, ulongcs finalXorValue, bool reflectValues)
        : polynomial_(polynomial), initialValue_(initialValue), finalXorValue_(finalXorValue), reflectValues_(reflectValues),
          lookupTable_(GenerateLookupTable(polynomial, reflectValues)) {}

    std::shared_ptr<Crc64ParameterSet> Crc64ParameterSet::Create(
        ulongcs polynomial, ulongcs initialValue, ulongcs finalXorValue, bool reflectValues)
    {
        return std::shared_ptr<Crc64ParameterSet>(new Crc64ParameterSet(polynomial, initialValue, finalXorValue, reflectValues));
    }

    const std::shared_ptr<Crc64ParameterSet>& Crc64ParameterSet::getCrc64Property() {
        static const std::shared_ptr<Crc64ParameterSet> instance =
            Create(0x42F0E1EBA9EA3693ull, 0x0000000000000000ull, 0x0000000000000000ull, false);
        return instance;
    }

    const std::shared_ptr<Crc64ParameterSet>& Crc64ParameterSet::getNvmeProperty() {
        static const std::shared_ptr<Crc64ParameterSet> instance =
            Create(0xAD93D23594C93659ull, 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull, true);
        return instance;
    }

    ulongcs Crc64ParameterSet::Update(ulongcs value, const bytecs* source, intcs length) const {
        ulongcs crc = value;
        if (reflectValues_) {
            for (intcs i = 0; i < length; ++i) {
                const bytecs idx = static_cast<bytecs>(crc ^ source[i]);
                crc = lookupTable_[idx] ^ (crc >> 8);
            }
        } else {
            for (intcs i = 0; i < length; ++i) {
                const bytecs idx = static_cast<bytecs>((crc >> 56) ^ source[i]);
                crc = lookupTable_[idx] ^ (crc << 8);
            }
        }
        return crc;
    }

    void Crc64ParameterSet::WriteCrcToSpan(ulongcs crc, bytecs* destination) const {
        if (reflectValues_) {
            for (int i = 0; i < 8; ++i) destination[i] = static_cast<bytecs>(crc >> (8 * i));
        } else {
            for (int i = 0; i < 8; ++i) destination[i] = static_cast<bytecs>(crc >> (56 - 8 * i));
        }
    }

} // namespace System::IO::Hashing
