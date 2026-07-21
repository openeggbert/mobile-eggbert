// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <typeinfo>

namespace System {

    /**
     * @brief Provides a mechanism for retrieving an object to control formatting.
     *
     * C++ counterpart of .NET System.IFormatProvider.
     * Implement GetFormat() to supply culture-specific or custom formatting objects.
     */
    class IFormatProvider {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IFormatProvider() = default;
        /**
         * @brief Returns a formatting object for the given type, or nullptr if not supported.
         * @param formatType The type of formatting object to retrieve.
         * @return Pointer to the formatting object, or nullptr.
         */
        [[nodiscard]] virtual void* GetFormat(const std::type_info& formatType) const = 0;
    };

} // namespace System
