// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/IPNetwork.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/HashCode.hpp"

namespace System::Net {

    using System::Net::Sockets::AddressFamily;

    namespace {
        void applyPrefixMask(std::vector<bytecs>& bytes, intcs prefixLength) {
            intcs fullBytes = prefixLength / 8;
            intcs remBits = prefixLength % 8;
            size_t firstClearedIndex = static_cast<size_t>(fullBytes) + (remBits ? 1u : 0u);
            for (size_t i = firstClearedIndex; i < bytes.size(); ++i) bytes[i] = 0;
            if (remBits > 0 && static_cast<size_t>(fullBytes) < bytes.size()) {
                bytecs mask = static_cast<bytecs>(0xFF << (8 - remBits));
                bytes[static_cast<size_t>(fullBytes)] = static_cast<bytecs>(bytes[static_cast<size_t>(fullBytes)] & mask);
            }
        }
    }

    intcs IPNetwork::getMaxPrefixLength(const IPAddress& baseAddress) {
        return baseAddress.getAddressFamilyProperty() == AddressFamily::InterNetwork ? 32 : 128;
    }

    IPAddress IPNetwork::clearNonPrefixBits(const IPAddress& baseAddress, intcs prefixLength) {
        auto bytes = baseAddress.GetAddressBytes();
        applyPrefixMask(bytes, prefixLength);
        return IPAddress(bytes);
    }

    IPNetwork::IPNetwork(const IPAddress& baseAddress, intcs prefixLength) {
        if (prefixLength < 0 || prefixLength > getMaxPrefixLength(baseAddress)) {
            throw System::ArgumentOutOfRangeException("prefixLength");
        }

        baseAddress_ = clearNonPrefixBits(baseAddress, prefixLength);
        prefixLength_ = prefixLength;
    }

    bool IPNetwork::Contains(const IPAddress& address) const {
        AddressFamily networkFamily = baseAddress_.getAddressFamilyProperty();

        IPAddress toCompare = address;
        if (address.getAddressFamilyProperty() != networkFamily) {
            if (networkFamily == AddressFamily::InterNetwork && address.getIsIPv4MappedToIPv6Property()) {
                toCompare = address.MapToIPv4();
            } else {
                return false;
            }
        }

        auto candidateBytes = toCompare.GetAddressBytes();
        applyPrefixMask(candidateBytes, prefixLength_);
        return IPAddress(candidateBytes) == baseAddress_;
    }

    std::string IPNetwork::ToString() const {
        return baseAddress_.ToString() + "/" + std::to_string(prefixLength_);
    }

    intcs IPNetwork::GetHashCode() const {
        return System::HashCode::Combine(baseAddress_.GetHashCode(), prefixLength_);
    }

    IPNetwork IPNetwork::Parse(const std::string& s) {
        IPNetwork result;
        if (!TryParse(s, result)) {
            throw System::FormatException("An invalid IP network was specified: " + s);
        }
        return result;
    }

    bool IPNetwork::TryParse(const std::string& s, IPNetwork& result) {
        size_t sepIndex = s.rfind('/');
        if (sepIndex == std::string::npos) return false;

        std::string addressPart = s.substr(0, sepIndex);
        std::string prefixPart = s.substr(sepIndex + 1);
        if (prefixPart.empty()) return false;
        for (char c : prefixPart) {
            if (c < '0' || c > '9') return false;
        }

        IPAddress address;
        if (!IPAddress::TryParse(addressPart, address)) return false;

        long prefixLength;
        try {
            prefixLength = std::stol(prefixPart);
        } catch (...) {
            return false;
        }
        if (prefixLength > getMaxPrefixLength(address)) return false;

        result = IPNetwork(address, static_cast<intcs>(prefixLength));
        return true;
    }

} // namespace System::Net
