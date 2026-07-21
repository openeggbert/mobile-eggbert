// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "System/Net/ICredentials.hpp"
#include "System/Net/ICredentialsByHost.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::intcs;

    /**
     * Provides credentials for password-based authentication schemes such as basic,
     * digest, NTLM and Kerberos.
     *
     * C++ counterpart of .NET System.Net.NetworkCredential.
     *
     * @note .NET's SecureString-based constructors/properties are omitted: this runtime has
     * no SecureString port (SecureString is itself a legacy/Windows-oriented API in .NET).
     *
     * @note GetCredential() returns shared_from_this(), so instances used through the
     * ICredentials/ICredentialsByHost interfaces must be held via std::shared_ptr
     * (e.g. std::make_shared<NetworkCredential>(...)) rather than as stack/value objects.
     */
    class NetworkCredential : public ICredentials, public ICredentialsByHost, public std::enable_shared_from_this<NetworkCredential> {
        std::string userName_;
        std::string password_;
        std::string domain_;

    public:
        /** Initializes a new instance with empty user name, password, and domain. */
        NetworkCredential() = default;

        /** Initializes a new instance with the specified user name and password. */
        NetworkCredential(const std::string& userName, const std::string& password)
            : userName_(userName), password_(password) {}

        /** Initializes a new instance with the specified user name, password, and domain. */
        NetworkCredential(const std::string& userName, const std::string& password, const std::string& domain)
            : userName_(userName), password_(password), domain_(domain) {}

        ~NetworkCredential() override = default;

        /** The user name associated with this credential. */
        [[nodiscard]] const std::string& getUserNameProperty() const { return userName_; }
        /** Sets the user name associated with this credential. */
        void setUserNameProperty(const std::string& value) { userName_ = value; }

        /** The password for the user name. */
        [[nodiscard]] const std::string& getPasswordProperty() const { return password_; }
        /** Sets the password for the user name. */
        void setPasswordProperty(const std::string& value) { password_ = value; }

        /** The machine name that verifies the credentials. Usually this is the host machine. */
        [[nodiscard]] const std::string& getDomainProperty() const { return domain_; }
        /** Sets the machine name that verifies the credentials. */
        void setDomainProperty(const std::string& value) { domain_ = value; }

        /** Returns this instance of the NetworkCredential class for a Uri and authentication type. */
        [[nodiscard]] std::shared_ptr<NetworkCredential> GetCredential(const System::Uri& /*uri*/, const std::string& /*authType*/) override {
            return shared_from_this();
        }

        /** Returns this instance of the NetworkCredential class for a host, port, and authentication type. */
        [[nodiscard]] std::shared_ptr<NetworkCredential> GetCredential(const std::string& /*host*/, intcs /*port*/, const std::string& /*authenticationType*/) override {
            return shared_from_this();
        }
    };

} // namespace System::Net
