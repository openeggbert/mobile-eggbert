// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Security/Principal/IIdentity.hpp"

namespace System::Security::Principal {

    /**
     * @brief Represents a generic user or "any" identity.
     *
     * C++ counterpart of .NET System.Security.Principal.GenericIdentity.
     *
     * @note .NET's real `GenericIdentity` derives from `System.Security.Claims.ClaimsIdentity`
     * (adding itself as a claim, exposing a `Claims` enumeration, etc.). That whole claims-based
     * identity system (`System.Security.Claims`) is a separate, not-yet-reviewed namespace in
     * this codebase's port plan, so this is a standalone reduced-scope implementation covering
     * just the practical `IIdentity` surface (Name/AuthenticationType/IsAuthenticated) that game
     * code actually needs — no claims enumeration.
     */
    class GenericIdentity : public IIdentity {
        std::string name_;
        std::string authenticationType_;

    public:
        /** @brief Constructs with the given name and an empty authentication type. */
        explicit GenericIdentity(const std::string& name) : name_(name) {}

        /** @brief Constructs with the given name and authentication type. */
        GenericIdentity(const std::string& name, const std::string& authenticationType)
            : name_(name), authenticationType_(authenticationType) {}

        [[nodiscard]] std::optional<std::string> getNameProperty() const override { return name_; }
        [[nodiscard]] std::optional<std::string> getAuthenticationTypeProperty() const override {
            return authenticationType_;
        }
        [[nodiscard]] bool getIsAuthenticatedProperty() const override { return !name_.empty(); }
    };

} // namespace System::Security::Principal
