// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Collections/IEnumerable.hpp"
#include "System/Collections/Generic/IEnumerator.hpp"

namespace System::Collections::Generic {

/**
 * @brief Exposes an enumerator that provides iteration over a strongly typed collection.
 *
 * C++ counterpart of .NET System.Collections.Generic.IEnumerable<T>.
 * Extends the non-generic System::Collections::IEnumerable with a typed GetEnumerator().
 *
 * @tparam T The type of objects to enumerate.
 */
template<typename T>
class IEnumerable : public System::Collections::IEnumerable {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IEnumerable() override = default;

    /**
     * @brief Returns a typed enumerator that iterates through the collection.
     *
     * C++ counterpart of .NET IEnumerable<T>.GetEnumerator().
     * Covariant override: IEnumerator<T> derives from System::Collections::IEnumerator.
     * @return A heap-allocated IEnumerator<T>; caller takes ownership.
     */
    [[nodiscard]] virtual IEnumerator<T>* GetEnumerator() override = 0;
};

} // namespace System::Collections::Generic
