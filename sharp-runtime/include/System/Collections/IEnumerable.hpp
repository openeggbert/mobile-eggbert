// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Collections/IEnumerator.hpp"

namespace System::Collections
{
    /**
     * @brief Exposes an enumerator that provides simple iteration over a non-generic collection.
     *
     * C++ counterpart of the .NET System.Collections.IEnumerable interface.
     */
    class IEnumerable
    {
    public:
        virtual ~IEnumerable() = default;

        /** Returns an enumerator that iterates through the collection. */
        [[nodiscard]] virtual IEnumerator* GetEnumerator() = 0;
    };
}
