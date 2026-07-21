// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/Collections/Generic/IEnumerable.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections::Concurrent {

/**
 * @brief Defines methods to manipulate thread-safe collections intended for producer/consumer usage.
 *
 * C++ counterpart of .NET System.Collections.Concurrent.IProducerConsumerCollection<T>.
 * All implementations of this interface must enable all members to be used concurrently
 * from multiple threads.
 *
 * @tparam T Specifies the type of elements in the collection.
 */
template<typename T>
class IProducerConsumerCollection : public System::Collections::Generic::IEnumerable<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IProducerConsumerCollection() = default;

    /**
     * @brief Attempts to add an object to the collection.
     *
     * C++ counterpart of .NET IProducerConsumerCollection<T>.TryAdd(T).
     * @param item The object to add to the collection.
     * @return true if the object was added successfully; otherwise false.
     */
    virtual bool TryAdd(const T& item) = 0;

    /**
     * @brief Attempts to remove and return an object from the collection.
     *
     * C++ counterpart of .NET IProducerConsumerCollection<T>.TryTake(out T).
     * @param item Receives the removed object if successful.
     * @return true if an object was removed and returned successfully; otherwise false.
     */
    virtual bool TryTake(T& item) = 0;

    /**
     * @brief Gets the number of elements contained in the collection.
     *
     * C++ counterpart of .NET ICollection.Count (via IProducerConsumerCollection).
     */
    [[nodiscard]] virtual SharpRuntime::intcs getCountProperty() const = 0;

    /**
     * @brief Gets a value indicating whether the collection is empty.
     *
     * C++ counterpart of .NET ConcurrentXxx.IsEmpty.
     */
    [[nodiscard]] virtual bool getIsEmptyProperty() const = 0;

    /**
     * @brief Copies the elements of the collection to a vector starting at the given index.
     *
     * C++ counterpart of .NET IProducerConsumerCollection<T>.CopyTo(T[], int).
     * @param array Destination vector (must be large enough).
     * @param index Zero-based index in array at which copying begins.
     */
    virtual void CopyTo(std::vector<T>& array, SharpRuntime::intcs index) = 0;

    /**
     * @brief Copies the elements contained in the collection to a new vector.
     *
     * C++ counterpart of .NET IProducerConsumerCollection<T>.ToArray().
     * @return A new vector containing all elements.
     */
    [[nodiscard]] virtual std::vector<T> ToArray() const = 0;
};

} // namespace System::Collections::Concurrent
