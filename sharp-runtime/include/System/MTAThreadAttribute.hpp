// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {

    /**
     * @brief Indicates that the COM threading model for an application is
     * multi-threaded apartment (MTA).
     *
     * C++ counterpart of .NET System.MTAThreadAttribute.
     * This is a marker attribute; it carries no data and has no effect in the
     * C++ port (COM threading models are Windows-specific).
     */
    class MTAThreadAttribute final : public Attribute {
    public:
        /**
         * @brief Initializes a new instance of MTAThreadAttribute.
         *
         * C++ counterpart of .NET MTAThreadAttribute().
         */
        MTAThreadAttribute() = default;
    };

} // namespace System
