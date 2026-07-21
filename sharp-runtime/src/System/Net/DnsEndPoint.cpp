// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/DnsEndPoint.hpp"
#include "System/Net/IPEndPoint.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <algorithm>
#include <cctype>

namespace System::Net {

    using System::Net::Sockets::AddressFamily;

    DnsEndPoint::DnsEndPoint(const std::string& host, intcs port)
        : DnsEndPoint(host, port, AddressFamily::Unspecified) {
    }

    DnsEndPoint::DnsEndPoint(const std::string& host, intcs port, AddressFamily addressFamily) {
        System::ArgumentException::ThrowIfNullOrEmpty(host, "host");
        System::ArgumentOutOfRangeException::ThrowIfLessThan(port, IPEndPoint::MinPort, "port");
        System::ArgumentOutOfRangeException::ThrowIfGreaterThan(port, IPEndPoint::MaxPort, "port");

        if (addressFamily != AddressFamily::InterNetwork &&
            addressFamily != AddressFamily::InterNetworkV6 &&
            addressFamily != AddressFamily::Unspecified) {
            throw System::ArgumentException("Address family not supported.", "addressFamily");
        }

        host_ = host;
        port_ = port;
        family_ = addressFamily;
    }

    std::string DnsEndPoint::ToString() const {
        std::string familyStr;
        switch (family_) {
            case AddressFamily::InterNetwork:   familyStr = "InterNetwork"; break;
            case AddressFamily::InterNetworkV6: familyStr = "InterNetworkV6"; break;
            case AddressFamily::Unspecified:    familyStr = "Unspecified"; break;
            default:                            familyStr = std::to_string(static_cast<int>(family_)); break;
        }
        return familyStr + "/" + host_ + ":" + std::to_string(port_);
    }

    bool DnsEndPoint::Equals(const DnsEndPoint& other) const {
        if (family_ != other.family_ || port_ != other.port_) return false;
        if (host_.size() != other.host_.size()) return false;
        return std::equal(host_.begin(), host_.end(), other.host_.begin(), [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
        });
    }

} // namespace System::Net
