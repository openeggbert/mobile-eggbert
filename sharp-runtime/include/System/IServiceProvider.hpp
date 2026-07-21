// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <typeinfo>

namespace System
{
    /** @brief Provides access to services identified by their runtime type. */
    class IServiceProvider
    {
    public:
        virtual ~IServiceProvider() = default;

        /**
         * @brief Gets a service object for the requested type.
         *
         * @param type Runtime type info of the requested service.
         * @return Pointer to the service object, or nullptr if not registered.
         */
        [[nodiscard]] virtual void* GetService(const std::type_info& type) const = 0;
    };
}
