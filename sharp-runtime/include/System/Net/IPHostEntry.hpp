// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Net/IPAddress.hpp"

namespace System::Net {

    /**
     * @brief Provides a container class for Internet host address information.
     *
     * C++ counterpart of .NET System.Net.IPHostEntry.
     */
    class IPHostEntry {
        std::string hostName_;
        std::vector<std::string> aliases_;
        std::vector<IPAddress> addressList_;

    public:
        IPHostEntry() = default;

        /** @return The DNS name of the host. */
        [[nodiscard]] const std::string& getHostNameProperty() const { return hostName_; }
        /** Sets the DNS name of the host. */
        void setHostNameProperty(const std::string& value) { hostName_ = value; }

        /** @return The list of aliases associated with the host. */
        [[nodiscard]] const std::vector<std::string>& getAliasesProperty() const { return aliases_; }
        /** Sets the list of aliases associated with the host. */
        void setAliasesProperty(const std::vector<std::string>& value) { aliases_ = value; }

        /** @return The list of IP addresses associated with the host. */
        [[nodiscard]] const std::vector<IPAddress>& getAddressListProperty() const { return addressList_; }
        /** Sets the list of IP addresses associated with the host. */
        void setAddressListProperty(const std::vector<IPAddress>& value) { addressList_ = value; }
    };

} // namespace System::Net
