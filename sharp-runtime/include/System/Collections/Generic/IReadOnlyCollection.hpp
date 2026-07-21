// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/IEnumerable.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a strongly-typed, read-only collection of elements.
 *
 * C++ counterpart of .NET System.Collections.Generic.IReadOnlyCollection<T>.
 * Extends IEnumerable<T> with a Count property; the collection cannot be modified
 * through this interface.
 *
 * @tparam T The type of the elements.
 */
template<typename T>
class IReadOnlyCollection : public IEnumerable<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IReadOnlyCollection() = default;

    /**
     * @brief Gets the number of elements in the collection.
     *
     * C++ counterpart of .NET IReadOnlyCollection<T>.Count.
     * @return The number of elements in the collection.
     */
    [[nodiscard]] virtual intcs getCountProperty() const = 0;
};

} // namespace System::Collections::Generic
