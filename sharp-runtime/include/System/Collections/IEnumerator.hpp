// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Collections
{
    /**
     * @brief Supports simple iteration over a non-generic collection.
     *
     * C++ counterpart of the .NET System.Collections.IEnumerator interface.
     */
    class IEnumerator
    {
    public:
        virtual ~IEnumerator() = default;

    /**
     * @brief Advances the enumerator to the next element of the collection.
     *
     * C++ counterpart of .NET IEnumerator.MoveNext().
     * @return true if the enumerator was advanced; false if past the end.
     */
    virtual bool MoveNext() = 0;

    /**
     * @brief Sets the enumerator to its initial position, before the first element.
     *
     * C++ counterpart of .NET IEnumerator.Reset().
     */
    virtual void Reset() = 0;

    /**
     * @brief Gets the current element in the collection.
     *
     * C++ counterpart of .NET IEnumerator.Current.
     * @return Pointer to the current element; cast to the appropriate type.
     */
    [[nodiscard]] virtual void* getCurrentProperty() const = 0;
    };
}
