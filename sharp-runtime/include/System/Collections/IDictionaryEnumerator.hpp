// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Collections/IEnumerator.hpp"
#include "System/Collections/DictionaryEntry.hpp"

namespace System::Collections {

/**
 * @brief Enumerates the elements of a non-generic dictionary.
 *
 * C++ counterpart of .NET System.Collections.IDictionaryEnumerator.
 * Extends IEnumerator; getCurrentProperty() returns a DictionaryEntry as void*.
 */
class IDictionaryEnumerator : public IEnumerator {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IDictionaryEnumerator() = default;

    /**
     * @brief Gets both the key and the value of the current dictionary entry.
     *
     * C++ counterpart of .NET IDictionaryEnumerator.Entry.
     */
    [[nodiscard]] virtual DictionaryEntry getEntryProperty() const = 0;

    /**
     * @brief Gets the key of the current dictionary entry.
     *
     * C++ counterpart of .NET IDictionaryEnumerator.Key.
     * @throws System::InvalidOperationException if the enumerator is positioned before the
     *         first element or after the last element (matches .NET's real contract; concrete
     *         implementations are responsible for enforcing this).
     */
    [[nodiscard]] virtual const void* getKeyProperty() const = 0;

    /**
     * @brief Gets the value of the current dictionary entry.
     *
     * C++ counterpart of .NET IDictionaryEnumerator.Value.
     */
    [[nodiscard]] virtual const void* getValueProperty() const = 0;
};

} // namespace System::Collections
