// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {

    /**
     * @brief Indicates that the COM threading model for an application is
     * single-threaded apartment (STA).
     *
     * C++ counterpart of .NET System.STAThreadAttribute.
     * This is a marker attribute; it carries no data and has no effect in the
     * C++ port (COM threading models are Windows-specific).
     */
    class STAThreadAttribute final : public Attribute {
    public:
        /**
         * @brief Initializes a new instance of STAThreadAttribute.
         *
         * C++ counterpart of .NET STAThreadAttribute().
         */
        STAThreadAttribute() = default;
    };

} // namespace System
