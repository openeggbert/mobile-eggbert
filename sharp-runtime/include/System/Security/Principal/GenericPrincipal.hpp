// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>
#include "System/ArgumentNullException.hpp"
#include "System/Security/Principal/IPrincipal.hpp"

namespace System::Security::Principal {

    /**
     * @brief Represents a generic principal, backed by a fixed list of role names.
     *
     * C++ counterpart of .NET System.Security.Principal.GenericPrincipal.
     *
     * @note .NET's real `GenericPrincipal` derives from `System.Security.Claims.ClaimsPrincipal`
     * (adding the identity and roles as claims, so `IsInRole` also consults any claims already on
     * the wrapped identity). That whole claims-based system (`System.Security.Claims`) is a
     * separate, not-yet-reviewed namespace here, so this is a standalone reduced-scope
     * implementation: role membership is checked only against the explicit @p roles list passed
     * to the constructor, not any claim on the identity.
     */
    class GenericPrincipal : public IPrincipal {
        std::shared_ptr<IIdentity> identity_;
        std::vector<std::string> roles_;

        static bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            return a.size() == b.size() &&
                   std::equal(a.begin(), a.end(), b.begin(),
                              [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
        }

    public:
        /**
         * @brief Constructs from @p identity and the list of role names it belongs to.
         * @throws System::ArgumentNullException if @p identity is null, matching .NET's
         *         `ArgumentNullException.ThrowIfNull(identity)` (GenericPrincipal.cs) --
         *         without this check, a null identity_ would silently propagate into
         *         getIdentityProperty() and crash a later, unrelated caller instead of
         *         failing fast here at construction.
         */
        GenericPrincipal(std::shared_ptr<IIdentity> identity, const std::vector<std::string>& roles)
            : identity_(std::move(identity)), roles_(roles) {
            if (!identity_) throw System::ArgumentNullException("identity");
        }

        [[nodiscard]] std::shared_ptr<IIdentity> getIdentityProperty() const override { return identity_; }

        [[nodiscard]] bool IsInRole(const std::string& role) const override {
            return std::any_of(roles_.begin(), roles_.end(),
                                [&](const std::string& r) { return equalsIgnoreCase(r, role); });
        }
    };

} // namespace System::Security::Principal
