// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "System/Collections/Generic/IEnumerable.hpp"

namespace System::Collections::Immutable {

using System::Collections::Generic::IEnumerable;

/**
 * @brief Represents an immutable last-in, first-out (LIFO) collection.
 *
 * C++ counterpart of .NET System.Collections.Immutable.IImmutableStack<T>.
 * All mutating operations return a new instance; the original is unchanged.
 *
 * @tparam T The type of elements in the stack.
 */
template<typename T>
class IImmutableStack : public IEnumerable<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IImmutableStack() override = default;

    /**
     * @brief Gets a value indicating whether the stack is empty.
     *
     * C++ counterpart of .NET IImmutableStack<T>.IsEmpty.
     * @return true if the stack contains no elements; otherwise false.
     */
    [[nodiscard]] virtual bool getIsEmptyProperty() const = 0;

    /**
     * @brief Returns an empty immutable stack.
     *
     * C++ counterpart of .NET IImmutableStack<T>.Clear().
     * @return A shared_ptr to a new empty IImmutableStack.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableStack<T>> Clear() const = 0;

    /**
     * @brief Returns a new stack with the element pushed onto the top.
     *
     * C++ counterpart of .NET IImmutableStack<T>.Push(T).
     * @param value The element to push.
     * @return A shared_ptr to a new IImmutableStack with the element on top.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableStack<T>> Push(const T& value) const = 0;

    /**
     * @brief Returns a new stack with the top element removed.
     *
     * C++ counterpart of .NET IImmutableStack<T>.Pop().
     * @return A shared_ptr to a new IImmutableStack with the top element removed.
     * @throws System::InvalidOperationException if the stack is empty.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableStack<T>> Pop() const = 0;

    /**
     * @brief Returns the element at the top of the stack without removing it.
     *
     * C++ counterpart of .NET IImmutableStack<T>.Peek().
     * @return A const reference to the top element.
     * @throws System::InvalidOperationException if the stack is empty.
     */
    [[nodiscard]] virtual const T& Peek() const = 0;
};

} // namespace System::Collections::Immutable
