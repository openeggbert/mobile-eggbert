// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a min-heap priority queue where lower priority values are dequeued first.
 *
 * C++ counterpart of .NET System.Collections.Generic.PriorityQueue<TElement,TPriority>.
 * Elements with lower TPriority values are returned before those with higher values,
 * matching .NET semantics where priority 0 is "highest priority".
 *
 * @tparam TElement  The type of the elements stored in the queue.
 * @tparam TPriority The type of the priority associated with each element (must support operator<).
 */
template<typename TElement, typename TPriority>
class PriorityQueue {
    struct Entry {
        TElement  element;
        TPriority priority;
        bool operator>(const Entry& o) const { return priority > o.priority; }
    };

    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> heap_;
    intcs count_ = 0;

public:
    /** @brief Initializes a new empty PriorityQueue. */
    PriorityQueue() = default;

    /**
     * @brief Gets the number of elements in the PriorityQueue.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return count_; }

    /**
     * @brief Adds the specified element with the given priority to the PriorityQueue.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.Enqueue(TElement, TPriority).
     * @param element  The element to add.
     * @param priority The priority associated with the element.
     */
    void Enqueue(const TElement& element, const TPriority& priority) {
        heap_.push({element, priority});
        ++count_;
    }

    /**
     * @brief Removes and returns the element with the lowest priority value.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.Dequeue().
     * @return The element with the lowest priority.
     * @throws System::InvalidOperationException if the queue is empty.
     */
    [[nodiscard]] TElement Dequeue() {
        if (heap_.empty()) throw System::InvalidOperationException("Queue empty.");
        TElement el = heap_.top().element;
        heap_.pop();
        --count_;
        return el;
    }

    /**
     * @brief Returns the element with the lowest priority value without removing it.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.Peek().
     * @return The element with the lowest priority.
     * @throws System::InvalidOperationException if the queue is empty.
     */
    [[nodiscard]] TElement Peek() const {
        if (heap_.empty()) throw System::InvalidOperationException("Queue empty.");
        return heap_.top().element;
    }

    /**
     * @brief Removes the element with the lowest priority and outputs it with its priority.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.TryDequeue(out TElement, out TPriority).
     * @param element  Receives the dequeued element if successful.
     * @param priority Receives the priority of the dequeued element if successful.
     * @return true if an element was removed; false if the queue was empty.
     */
    [[nodiscard]] bool TryDequeue(TElement& element, TPriority& priority) {
        if (heap_.empty()) return false;
        element  = heap_.top().element;
        priority = heap_.top().priority;
        heap_.pop();
        --count_;
        return true;
    }

    /**
     * @brief Returns the element with the lowest priority and its priority without removing them.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.TryPeek(out TElement, out TPriority).
     * @param element  Receives the front element if the queue is non-empty.
     * @param priority Receives the front priority if the queue is non-empty.
     * @return true if the queue is non-empty; otherwise false.
     */
    [[nodiscard]] bool TryPeek(TElement& element, TPriority& priority) const {
        if (heap_.empty()) return false;
        element  = heap_.top().element;
        priority = heap_.top().priority;
        return true;
    }

    /**
     * @brief Adds a sequence of element/priority pairs to the PriorityQueue.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.EnqueueRange(IEnumerable<(TElement,TPriority)>).
     * @param items A vector of (element, priority) pairs to enqueue.
     */
    void EnqueueRange(const std::vector<std::pair<TElement, TPriority>>& items) {
        for (const auto& p : items) Enqueue(p.first, p.second);
    }

    /**
     * @brief Removes all elements from the PriorityQueue.
     *
     * C++ counterpart of .NET PriorityQueue<TElement,TPriority>.Clear().
     */
    void Clear() {
        while (!heap_.empty()) heap_.pop();
        count_ = 0;
    }
};

} // namespace System::Collections::Generic
