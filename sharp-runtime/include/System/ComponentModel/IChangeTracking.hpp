// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::ComponentModel
{
    /** @brief Defines the mechanism for querying the object for changes and resetting of the changed status. */
    class IChangeTracking
    {
    public:
        virtual ~IChangeTracking() = default;

        /** @brief Gets a value indicating whether the object's content has changed since the last call to AcceptChanges(). */
        [[nodiscard]] virtual bool getIsChangedProperty() const = 0;

        /** @brief Resets the object's state to unchanged by accepting the modifications. */
        virtual void AcceptChanges() = 0;
    };
}
