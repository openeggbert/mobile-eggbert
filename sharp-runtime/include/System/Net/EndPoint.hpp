// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "System/Net/Sockets/AddressFamily.hpp"
#include "System/Net/SocketAddress.hpp"
#include "System/NotImplementedException.hpp"

namespace System::Net {

    /**
     * @brief Identifies a network address. Base class for concrete endpoint types such as
     * IPEndPoint and DnsEndPoint.
     *
     * C++ counterpart of .NET System.Net.EndPoint.
     *
     * @note Matches .NET's own base-class behavior: AddressFamily/Serialize/Create are not
     * abstract in .NET — they throw NotImplementedException by design unless a derived type
     * overrides them. That is reproduced here rather than making them pure virtual.
     */
    class EndPoint {
    public:
        virtual ~EndPoint() = default;

        /** @return The address family to which the endpoint belongs. */
        [[nodiscard]] virtual System::Net::Sockets::AddressFamily getAddressFamilyProperty() const {
            throw System::NotImplementedException("This property is not implemented by the base EndPoint class.");
        }

        /** Serializes endpoint information into a SocketAddress structure. */
        [[nodiscard]] virtual SocketAddress Serialize() const {
            throw System::NotImplementedException("This method is not implemented by the base EndPoint class.");
        }

        /** Creates an EndPoint instance from a SocketAddress structure. */
        [[nodiscard]] virtual std::shared_ptr<EndPoint> Create(const SocketAddress& /*socketAddress*/) const {
            throw System::NotImplementedException("This method is not implemented by the base EndPoint class.");
        }
    };

} // namespace System::Net
