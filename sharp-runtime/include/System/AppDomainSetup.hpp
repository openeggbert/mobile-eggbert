// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/AppContext.hpp"

namespace System {

    /**
     * @brief Represents setup information for an application domain.
     *
     * C++ counterpart of .NET System.AppDomainSetup (sealed).
     * In .NET, this class configures application domains before they are created.
     * In sharp-runtime there is only one domain, so all properties delegate to
     * AppContext / AppDomain.
     */
    class AppDomainSetup final {
    public:
        AppDomainSetup() = default;

        /**
         * @brief Gets the name of the application base directory.
         *
         * C++ counterpart of .NET AppDomainSetup.ApplicationBase.
         * Delegates to AppContext.BaseDirectory.
         */
        [[nodiscard]] std::string getApplicationBaseProperty() const {
            return AppContext::getBaseDirectoryProperty();
        }

        /**
         * @brief Gets the name of the framework version targeted by the current application.
         *
         * C++ counterpart of .NET AppDomainSetup.TargetFrameworkName.
         * Always returns an empty string in sharp-runtime.
         */
        [[nodiscard]] std::string getTargetFrameworkNameProperty() const {
            return AppContext::getTargetFrameworkNameProperty();
        }
    };

} // namespace System
