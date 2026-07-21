// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Collections/IEnumerator.hpp"

namespace System::Collections::Generic {

/**
 * @brief Supports simple iteration over a strongly typed generic collection.
 *
 * C++ counterpart of .NET System.Collections.Generic.IEnumerator<T>.
 * Extends the non-generic System::Collections::IEnumerator with a typed Current() accessor.
 *
 * @tparam T The type of objects to enumerate.
 */
template<typename T>
class IEnumerator : public System::Collections::IEnumerator {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IEnumerator() override = default;

    /**
     * @brief Gets the element in the collection at the current position of the enumerator.
     *
     * C++ counterpart of .NET IEnumerator<T>.Current.
     * @return A const reference to the current element.
     */
    [[nodiscard]] virtual const T& Current() const = 0;

    /**
     * @brief Returns a void pointer to the current element (non-generic override).
     *
     * Bridges the non-generic System::Collections::IEnumerator::getCurrentProperty() to the
     * typed Current() accessor. Allows non-generic code to access the current element.
     */
    void* getCurrentProperty() const override {
        return const_cast<T*>(&Current());
    }
};

} // namespace System::Collections::Generic
