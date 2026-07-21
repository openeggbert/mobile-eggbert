// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/EventArgs.hpp"

namespace System {

    /**
     * @brief Provides data for loader resolution events, such as the
     * AppDomain.TypeResolve, AppDomain.ResourceResolve and AppDomain.AssemblyResolve events.
     *
     * C++ counterpart of .NET System.ResolveEventArgs.
     * Because C++ has no runtime Assembly type, the requesting assembly is represented
     * as a name string rather than an Assembly object.
     */
    class ResolveEventArgs : public EventArgs {
        std::string name_;
        std::string requestingAssemblyName_;

    public:
        /**
         * @brief Initializes a new instance of the ResolveEventArgs class with the specified name.
         * @param name The name of an item to resolve.
         */
        explicit ResolveEventArgs(const std::string& name) : name_(name) {}

        /**
         * @brief Initializes a new instance of the ResolveEventArgs class with the specified
         * name and the assembly whose dependency is being resolved.
         * @param name               The name of an item to resolve.
         * @param requestingAssembly The name of the assembly whose dependency is being resolved.
         */
        ResolveEventArgs(const std::string& name, const std::string& requestingAssembly)
            : name_(name), requestingAssemblyName_(requestingAssembly) {}

        /**
         * @brief Gets the name of the item to resolve.
         *
         * C++ counterpart of .NET ResolveEventArgs.Name.
         * @return The name of the item to resolve.
         */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /**
         * @brief Gets the name of the assembly whose dependency is being resolved.
         *
         * C++ counterpart of .NET ResolveEventArgs.RequestingAssembly.
         * Returns an empty string if no requesting assembly was specified.
         * @return The name of the requesting assembly, or an empty string.
         */
        [[nodiscard]] const std::string& getRequestingAssemblyNameProperty() const {
            return requestingAssemblyName_;
        }
    };

} // namespace System
