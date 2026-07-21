// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {

    /**
     * @brief Indicates whether a program element is compliant with the Common
     * Language Specification (CLS).
     *
     * C++ counterpart of .NET System.CLSCompliantAttribute.
     */
    class CLSCompliantAttribute : public Attribute {
        bool isCompliant_;
    public:
        /**
         * @brief Initializes a new instance indicating the CLS compliance of the target element.
         *
         * C++ counterpart of .NET CLSCompliantAttribute(bool isCompliant).
         * @param isCompliant true if the element is CLS-compliant; otherwise, false.
         */
        explicit CLSCompliantAttribute(bool isCompliant) : isCompliant_(isCompliant) {}

        /**
         * @brief Gets a value indicating whether the program element is CLS-compliant.
         *
         * C++ counterpart of .NET CLSCompliantAttribute.IsCompliant.
         */
        [[nodiscard]] bool getIsCompliantProperty() const { return isCompliant_; }
    };

} // namespace System
