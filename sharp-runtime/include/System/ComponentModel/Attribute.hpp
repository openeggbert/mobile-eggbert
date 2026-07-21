// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::ComponentModel
{
    /**
     * @brief Base class for custom attributes.
     *
     * Minimal stub — C++ counterpart of System.Attribute.
     */
    class Attribute
    {
    public:
        virtual ~Attribute() = default;
    };
}
