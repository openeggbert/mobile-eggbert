// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/ComponentModel/IChangeTracking.hpp"

namespace System::ComponentModel
{
    /** @brief Provides support for rolling back the changes. */
    class IRevertibleChangeTracking : public IChangeTracking
    {
    public:
        /** @brief Resets the object's state to unchanged by rejecting the modifications. */
        virtual void RejectChanges() = 0;
    };
}
