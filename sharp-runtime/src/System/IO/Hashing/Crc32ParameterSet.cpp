// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/Hashing/Crc32ParameterSet.hpp"

namespace System::IO::Hashing {

    namespace {
        uintcs ReverseBits(uintcs value) {
            value = ((value & 0xAAAAAAAAu) >> 1) | ((value & 0x55555555u) << 1);
            value = ((value & 0xCCCCCCCCu) >> 2) | ((value & 0x33333333u) << 2);
            value = ((value & 0xF0F0F0F0u) >> 4) | ((value & 0x0F0F0F0Fu) << 4);
            return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) |
                   ((value & 0x00FF0000u) >> 8)  | ((value & 0xFF000000u) >> 24);
        }

        std::vector<uintcs> GenerateLookupTable(uintcs polynomial, bool reflectInput) {
            std::vector<uintcs> table(256, 0u);

            if (!reflectInput) {
                uintcs crc = 0x80000000u;
                for (intcs i = 1; i < 256; i <<= 1) {
                    crc = (crc & 0x80000000u) ? (crc << 1) ^ polynomial : crc << 1;
                    for (intcs j = 0; j < i; ++j) {
                        table[static_cast<size_t>(i + j)] = crc ^ table[static_cast<size_t>(j)];
                    }
                }
            } else {
                for (intcs i = 1; i < 256; ++i) {
                    uintcs r = ReverseBits(static_cast<uintcs>(i));
                    constexpr uintcs LastBit = 0x80000000u;
                    for (intcs j = 0; j < 8; ++j) {
                        r = (r & LastBit) ? (r << 1) ^ polynomial : r << 1;
                    }
                    table[static_cast<size_t>(i)] = ReverseBits(r);
                }
            }

            return table;
        }
    }

    Crc32ParameterSet::Crc32ParameterSet(uintcs polynomial, uintcs initialValue, uintcs finalXorValue, bool reflectValues)
        : polynomial_(polynomial), initialValue_(initialValue), finalXorValue_(finalXorValue), reflectValues_(reflectValues),
          lookupTable_(GenerateLookupTable(polynomial, reflectValues)) {}

    std::shared_ptr<Crc32ParameterSet> Crc32ParameterSet::Create(
        uintcs polynomial, uintcs initialValue, uintcs finalXorValue, bool reflectValues)
    {
        return std::shared_ptr<Crc32ParameterSet>(new Crc32ParameterSet(polynomial, initialValue, finalXorValue, reflectValues));
    }

    const std::shared_ptr<Crc32ParameterSet>& Crc32ParameterSet::getCrc32Property() {
        static const std::shared_ptr<Crc32ParameterSet> instance =
            Create(0x04c11db7u, 0xffffffffu, 0xffffffffu, true);
        return instance;
    }

    const std::shared_ptr<Crc32ParameterSet>& Crc32ParameterSet::getCrc32CProperty() {
        static const std::shared_ptr<Crc32ParameterSet> instance =
            Create(0x1edc6f41u, 0xffffffffu, 0xffffffffu, true);
        return instance;
    }

    uintcs Crc32ParameterSet::Update(uintcs value, const bytecs* source, intcs length) const {
        uintcs crc = value;
        if (reflectValues_) {
            for (intcs i = 0; i < length; ++i) {
                const bytecs idx = static_cast<bytecs>(crc ^ source[i]);
                crc = lookupTable_[idx] ^ (crc >> 8);
            }
        } else {
            for (intcs i = 0; i < length; ++i) {
                const bytecs idx = static_cast<bytecs>((crc >> 24) ^ source[i]);
                crc = lookupTable_[idx] ^ (crc << 8);
            }
        }
        return crc;
    }

    void Crc32ParameterSet::WriteCrcToSpan(uintcs crc, bytecs* destination) const {
        if (reflectValues_) {
            destination[0] = static_cast<bytecs>(crc);
            destination[1] = static_cast<bytecs>(crc >> 8);
            destination[2] = static_cast<bytecs>(crc >> 16);
            destination[3] = static_cast<bytecs>(crc >> 24);
        } else {
            destination[0] = static_cast<bytecs>(crc >> 24);
            destination[1] = static_cast<bytecs>(crc >> 16);
            destination[2] = static_cast<bytecs>(crc >> 8);
            destination[3] = static_cast<bytecs>(crc);
        }
    }

} // namespace System::IO::Hashing
