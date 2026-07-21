// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::ComponentModel
{
    /** @brief Provides functionality to commit or rollback changes to an object that is used as a data source. */
    class IEditableObject
    {
    public:
        virtual ~IEditableObject() = default;

        /** @brief Begins an edit on an object. */
        virtual void BeginEdit() = 0;

        /** @brief Pushes changes since the last BeginEdit into the underlying object. */
        virtual void EndEdit() = 0;

        /** @brief Discards changes since the last BeginEdit call. */
        virtual void CancelEdit() = 0;
    };
}
