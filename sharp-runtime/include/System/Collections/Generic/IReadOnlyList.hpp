// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/IReadOnlyCollection.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a read-only collection of elements that can be accessed by index.
 *
 * C++ counterpart of .NET System.Collections.Generic.IReadOnlyList<T>.
 * Extends IReadOnlyCollection<T> with a positional indexer; the list cannot be
 * modified through this interface.
 *
 * @tparam T The type of elements in the read-only list.
 */
template<typename T>
class IReadOnlyList : public IReadOnlyCollection<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IReadOnlyList() = default;

    /**
     * @brief Gets the element at the specified zero-based index.
     *
     * C++ counterpart of .NET IReadOnlyList<T>.Item[int] getter.
     * @param index The zero-based index of the element to get.
     * @return A const reference to the element at the specified index.
     */
    [[nodiscard]] virtual const T& operator[](intcs index) const = 0;
};

} // namespace System::Collections::Generic
