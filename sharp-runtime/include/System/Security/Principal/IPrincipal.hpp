// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "System/Security/Principal/IIdentity.hpp"

namespace System::Security::Principal {

    /**
     * @brief Defines the basic functionality of a principal object.
     *
     * C++ counterpart of .NET System.Security.Principal.IPrincipal.
     */
    class IPrincipal {
    public:
        virtual ~IPrincipal() = default;

        /** @return The identity of the current principal. */
        [[nodiscard]] virtual std::shared_ptr<IIdentity> getIdentityProperty() const = 0;

        /** @return true if the current principal belongs to @p role. */
        [[nodiscard]] virtual bool IsInRole(const std::string& role) const = 0;
    };

} // namespace System::Security::Principal
