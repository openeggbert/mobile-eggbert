// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/NetworkInformation/PhysicalAddress.hpp"
#include "System/Convert.hpp"
#include "System/FormatException.hpp"

namespace System::Net::NetworkInformation {

    namespace {
        // Returns the hex nibble value of c, or -1 if c is not a hex digit.
        int hexNibble(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }
    }

    const PhysicalAddress PhysicalAddress::None(std::vector<SharpRuntime::bytecs>{});

    SharpRuntime::intcs PhysicalAddress::GetHashCode() const {
        if (hash_ == 0) {
            SharpRuntime::intcs hash = 0;
            size_t i = 0;
            size_t size = address_.size() & ~static_cast<size_t>(3);

            for (; i < size; i += 4) {
                SharpRuntime::intcs word = static_cast<unsigned char>(address_[i])
                    | (static_cast<unsigned char>(address_[i + 1]) << 8)
                    | (static_cast<unsigned char>(address_[i + 2]) << 16)
                    | (static_cast<unsigned char>(address_[i + 3]) << 24);
                hash ^= word;
            }

            if ((address_.size() & 3) != 0) {
                SharpRuntime::intcs remnant = 0;
                int shift = 0;
                for (; i < address_.size(); ++i) {
                    remnant |= static_cast<unsigned char>(address_[i]) << shift;
                    shift += 8;
                }
                hash ^= remnant;
            }

            if (hash == 0) hash = 1;
            hash_ = hash;
        }
        return hash_;
    }

    bool PhysicalAddress::Equals(const PhysicalAddress& other) const {
        return address_.size() == other.address_.size() &&
               GetHashCode() == other.GetHashCode() &&
               address_ == other.address_;
    }

    std::string PhysicalAddress::ToString() const {
        // Verified against PhysicalAddress.cs: real .NET's ToString() is
        // `Convert.ToHexString(_address)` (the uppercase form) -- Convert::ToHexString
        // now produces uppercase directly (ticket 248 fixed a naming/casing swap between
        // ToHexString and ToHexStringLower), so this no longer needs the old, incorrectly
        // named ToHexStringUpper.
        return System::Convert::ToHexString(address_);
    }

    bool PhysicalAddress::tryGetValidSegmentLength(const std::string& address, char delimiter, SharpRuntime::intcs& value) {
        value = -1;
        int segments = 1;
        SharpRuntime::intcs validSegmentLength = 0;
        for (size_t i = 0; i < address.size(); ++i) {
            if (address[i] == delimiter) {
                if (validSegmentLength == 0) {
                    validSegmentLength = static_cast<SharpRuntime::intcs>(i);
                } else if ((static_cast<SharpRuntime::intcs>(i) - (segments - 1)) % validSegmentLength != 0) {
                    return false;
                }
                ++segments;
            }
        }
        if (segments * validSegmentLength != 12) return false;
        value = validSegmentLength;
        return true;
    }

    bool PhysicalAddress::TryParse(const std::string& address, PhysicalAddress& value) {
        SharpRuntime::intcs validSegmentLength;
        char delimiter = '\0';
        std::vector<SharpRuntime::bytecs> buffer;

        if (address.find('-') != std::string::npos) {
            if ((address.size() + 1) % 3 != 0) return false;
            delimiter = '-';
            buffer.assign((address.size() + 1) / 3, 0);
            validSegmentLength = 2;
        } else if (address.find(':') != std::string::npos) {
            delimiter = ':';
            if (!tryGetValidSegmentLength(address, ':', validSegmentLength)) return false;
            if (validSegmentLength != 2 && validSegmentLength != 4) return false;
            buffer.assign(6, 0);
        } else if (address.find('.') != std::string::npos) {
            delimiter = '.';
            if (!tryGetValidSegmentLength(address, '.', validSegmentLength)) return false;
            if (validSegmentLength != 4) return false;
            buffer.assign(6, 0);
        } else {
            if (address.size() % 2 != 0) return false;
            validSegmentLength = static_cast<SharpRuntime::intcs>(address.size());
            buffer.assign(address.size() / 2, 0);
        }

        SharpRuntime::intcs validCount = 0;
        size_t j = 0;
        for (size_t i = 0; i < address.size(); ++i) {
            char c = address[i];
            int nibble = hexNibble(c);
            if (nibble < 0) {
                if (delimiter == c && validCount == validSegmentLength) {
                    validCount = 0;
                    continue;
                }
                return false;
            }

            if (validCount >= validSegmentLength) return false;

            if (validCount % 2 == 0) {
                buffer[j] = static_cast<SharpRuntime::bytecs>(nibble << 4);
            } else {
                buffer[j++] |= static_cast<SharpRuntime::bytecs>(nibble);
            }
            ++validCount;
        }

        if (validCount < validSegmentLength) return false;

        value = PhysicalAddress(buffer);
        return true;
    }

    PhysicalAddress PhysicalAddress::Parse(const std::string& address) {
        PhysicalAddress result = None;
        if (!TryParse(address, result)) {
            throw System::FormatException("An invalid physical address was specified: '" + address + "'.");
        }
        return result;
    }

} // namespace System::Net::NetworkInformation
