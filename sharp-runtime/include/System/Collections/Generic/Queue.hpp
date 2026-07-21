// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <deque>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/Generic/IEnumerator.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a first-in, first-out (FIFO) collection of objects.
 *
 * C++ counterpart of .NET System.Collections.Generic.Queue<T>.
 * Backed by std::deque<T> (the same container std::queue<T> defaults to internally, chosen
 * directly here so the type can support iteration/enumeration); provides O(1) amortized
 * Enqueue and Dequeue.
 *
 * @note GetEnumerator()'s Enumerator detects structural modification (Enqueue/Dequeue/Clear)
 * during iteration via a version counter, matching .NET's InvalidOperationException fail-fast
 * contract (see List<T>/ArrayList's Enumerator in this codebase for the same established
 * pattern). Directly using begin()/end() STL iterators still follows plain std::deque<T>
 * invalidation rules, not .NET's version-checked contract -- only the GetEnumerator()-returned
 * Enumerator is fail-fast. Enumeration order matches real .NET's Queue<T>: front-to-back
 * (oldest-to-newest, i.e. dequeue order).
 *
 * @tparam T The type of elements in the queue.
 */
template<typename T>
class Queue {
    std::deque<T> queue_;
    intcs version_ = 0;

    class Enumerator : public IEnumerator<T>
    {
        const Queue<T>* queue_;
        intcs version_;
        intcs index_ = -1;
    public:
        explicit Enumerator(const Queue<T>* queue) : queue_(queue), version_(queue->version_) {}
        bool MoveNext() override {
            if (version_ != queue_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            return ++index_ < static_cast<intcs>(queue_->queue_.size());
        }
        void Reset() override {
            if (version_ != queue_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            index_ = -1;
        }
        [[nodiscard]] const T& Current() const override { return queue_->queue_[static_cast<size_t>(index_)]; }
    };

public:
    /** @brief Initializes a new empty Queue<T>. */
    Queue() = default;

    /**
     * @brief Gets the number of elements contained in the Queue.
     *
     * C++ counterpart of .NET Queue<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(queue_.size()); }

    /**
     * @brief Adds an object to the end of the Queue (copy).
     *
     * C++ counterpart of .NET Queue<T>.Enqueue(T).
     * @param item The object to add.
     */
    void Enqueue(const T& item) { queue_.push_back(item); ++version_; }

    /**
     * @brief Adds an object to the end of the Queue (move).
     *
     * C++ counterpart of .NET Queue<T>.Enqueue(T).
     * @param item The object to add (moved).
     */
    void Enqueue(T&& item) { queue_.push_back(std::move(item)); ++version_; }

    /**
     * @brief Removes and returns the object at the beginning of the Queue.
     *
     * C++ counterpart of .NET Queue<T>.Dequeue().
     * @return The object that was removed from the beginning of the Queue.
     * @throws System::InvalidOperationException if the Queue is empty.
     */
    T Dequeue() {
        if (queue_.empty()) throw System::InvalidOperationException("Queue empty.");
        T val = std::move(queue_.front());
        queue_.pop_front();
        ++version_;
        return val;
    }

    /**
     * @brief Returns the object at the beginning of the Queue without removing it.
     *
     * C++ counterpart of .NET Queue<T>.Peek().
     * @return A const reference to the object at the front.
     * @throws System::InvalidOperationException if the Queue is empty.
     */
    [[nodiscard]] const T& Peek() const {
        if (queue_.empty()) throw System::InvalidOperationException("Queue empty.");
        return queue_.front();
    }

    /**
     * @brief Tries to remove and return the object at the beginning of the Queue.
     *
     * C++ counterpart of .NET Queue<T>.TryDequeue(out T).
     * @param result Receives the dequeued object if successful.
     * @return true if an element was removed; false if the Queue was empty.
     */
    bool TryDequeue(T& result) {
        if (queue_.empty()) return false;
        result = std::move(queue_.front());
        queue_.pop_front();
        ++version_;
        return true;
    }

    /**
     * @brief Tries to return the object at the beginning of the Queue without removing it.
     *
     * C++ counterpart of .NET Queue<T>.TryPeek(out T).
     * @param result Receives the front object if successful.
     * @return true if the Queue is non-empty; otherwise false.
     */
    bool TryPeek(T& result) const {
        if (queue_.empty()) return false;
        result = queue_.front();
        return true;
    }

    /**
     * @brief Determines whether the Queue contains the specified element.
     *
     * C++ counterpart of .NET Queue<T>.Contains(T).
     * @param item The object to locate.
     * @return true if the element is found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const {
        return std::find(queue_.begin(), queue_.end(), item) != queue_.end();
    }

    /**
     * @brief Removes all objects from the Queue.
     *
     * C++ counterpart of .NET Queue<T>.Clear().
     */
    void Clear() { queue_.clear(); ++version_; }

    /**
     * @brief Copies the Queue elements to a new vector in FIFO order.
     *
     * C++ counterpart of .NET Queue<T>.ToArray().
     * @return A std::vector<T> containing all elements in queue order.
     */
    [[nodiscard]] std::vector<T> ToArray() const {
        return std::vector<T>(queue_.begin(), queue_.end());
    }

    /**
     * @brief Returns a version-checked enumerator over the Queue in front-to-back (dequeue) order.
     *
     * C++ counterpart of .NET Queue<T>.GetEnumerator(). Throws InvalidOperationException from
     * MoveNext()/Reset() if the Queue is structurally modified during enumeration.
     * @return A newly allocated IEnumerator<T>*; caller owns the returned object.
     */
    IEnumerator<T>* GetEnumerator() {
        return new Enumerator(this);
    }

    /** @brief STL-interop mutable begin iterator (not part of the .NET API, not version-checked). */
    auto begin() { return queue_.begin(); }
    /** @brief STL-interop mutable end iterator (not part of the .NET API, not version-checked). */
    auto end()   { return queue_.end(); }
    /** @brief STL-interop const begin iterator (not part of the .NET API, not version-checked). */
    [[nodiscard]] auto begin() const { return queue_.cbegin(); }
    /** @brief STL-interop const end iterator (not part of the .NET API, not version-checked). */
    [[nodiscard]] auto end()   const { return queue_.cend(); }
};

} // namespace System::Collections::Generic
