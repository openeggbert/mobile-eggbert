// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "System/Collections/Generic/IEnumerable.hpp"

namespace System::Collections::Immutable {

using System::Collections::Generic::IEnumerable;

/**
 * @brief Represents an immutable first-in, first-out (FIFO) collection.
 *
 * C++ counterpart of .NET System.Collections.Immutable.IImmutableQueue<T>.
 * All mutating operations return a new instance; the original is unchanged.
 *
 * @tparam T The type of elements in the queue.
 */
template<typename T>
class IImmutableQueue : public IEnumerable<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IImmutableQueue() override = default;

    /**
     * @brief Gets a value indicating whether the queue is empty.
     *
     * C++ counterpart of .NET IImmutableQueue<T>.IsEmpty.
     * @return true if the queue contains no elements; otherwise false.
     */
    [[nodiscard]] virtual bool getIsEmptyProperty() const = 0;

    /**
     * @brief Returns an empty immutable queue.
     *
     * C++ counterpart of .NET IImmutableQueue<T>.Clear().
     * @return A shared_ptr to a new empty IImmutableQueue.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableQueue<T>> Clear() const = 0;

    /**
     * @brief Returns the element at the front of the queue without removing it.
     *
     * C++ counterpart of .NET IImmutableQueue<T>.Peek().
     * @return A const reference to the front element.
     * @throws System::InvalidOperationException if the queue is empty.
     */
    [[nodiscard]] virtual const T& Peek() const = 0;

    /**
     * @brief Returns a new queue with the element added to the back.
     *
     * C++ counterpart of .NET IImmutableQueue<T>.Enqueue(T).
     * @param value The element to add.
     * @return A shared_ptr to a new IImmutableQueue with the element enqueued.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableQueue<T>> Enqueue(const T& value) const = 0;

    /**
     * @brief Returns a new queue with the front element removed.
     *
     * C++ counterpart of .NET IImmutableQueue<T>.Dequeue().
     * @return A shared_ptr to a new IImmutableQueue with the front element removed.
     * @throws std::out_of_range if the queue is empty.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableQueue<T>> Dequeue() const = 0;
};

} // namespace System::Collections::Immutable
