// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/IEnumerable.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Defines size, enumerators, and synchronization methods for all non-generic collections.
 *
 * C++ counterpart of .NET System.Collections.ICollection.
 * Because C++ has no runtime Array type, CopyTo accepts a void* destination pointer.
 */
class ICollection : public IEnumerable {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~ICollection() = default;

    /**
     * @brief Gets the number of elements contained in the collection.
     *
     * C++ counterpart of .NET ICollection.Count.
     */
    [[nodiscard]] virtual intcs getCountProperty() const = 0;

    /**
     * @brief Copies the elements of the collection into an array starting at the given index.
     *
     * C++ counterpart of .NET ICollection.CopyTo(Array, int).
     * @param array Pointer to the destination array buffer.
     * @param index Zero-based index at which copying begins.
     */
    virtual void CopyTo(void* array, intcs index) = 0;

    /**
     * @brief Gets an object that can be used to synchronize access to the collection.
     *
     * C++ counterpart of .NET ICollection.SyncRoot.
     * @return A pointer that can be used as a synchronization lock.
     */
    [[nodiscard]] virtual const void* getSyncRootProperty() const { return this; }

    /**
     * @brief Gets a value indicating whether access to the collection is synchronized (thread-safe).
     *
     * C++ counterpart of .NET ICollection.IsSynchronized.
     */
    [[nodiscard]] virtual bool getIsSynchronizedProperty() const { return false; }
};

} // namespace System::Collections
