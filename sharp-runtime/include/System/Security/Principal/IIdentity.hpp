// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>

namespace System::Security::Principal {

    /**
     * @brief Defines the basic functionality of an identity object.
     *
     * C++ counterpart of .NET System.Security.Principal.IIdentity.
     */
    class IIdentity {
    public:
        virtual ~IIdentity() = default;

        /** @return The name of the identity, or empty if unauthenticated/unavailable. */
        [[nodiscard]] virtual std::optional<std::string> getNameProperty() const = 0;

        /** @return The type of authentication used, or empty if not authenticated. */
        [[nodiscard]] virtual std::optional<std::string> getAuthenticationTypeProperty() const = 0;

        /** @return true if the user has been authenticated. */
        [[nodiscard]] virtual bool getIsAuthenticatedProperty() const = 0;
    };

} // namespace System::Security::Principal
