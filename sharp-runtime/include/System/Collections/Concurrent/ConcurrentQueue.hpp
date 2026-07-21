// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <mutex>
#include <queue>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Concurrent/IProducerConsumerCollection.hpp"
#include "System/Collections/Generic/IEnumerable.hpp"
#include "System/Collections/Generic/IEnumerator.hpp"

namespace System::Collections::Concurrent {

using SharpRuntime::intcs;

/**
 * @brief A thread-safe first-in, first-out (FIFO) collection.
 *
 * C++ counterpart of .NET System.Collections.Concurrent.ConcurrentQueue&lt;T&gt;.
 * Wraps std::queue&lt;T&gt; with a std::mutex; simpler (and less scalable under heavy
 * contention) than .NET's lock-free segmented-array implementation, but observably
 * equivalent. Enumeration takes a point-in-time snapshot under lock, matching .NET's
 * documented weakly-consistent (not exactly moment-in-time, never throws on concurrent
 * modification) enumeration contract.
 */
template<typename T>
class ConcurrentQueue : public IProducerConsumerCollection<T> {
    mutable std::mutex mutex_;
    std::queue<T>      queue_;

    class SnapshotEnumerator : public System::Collections::Generic::IEnumerator<T> {
        std::vector<T> items_;
        intcs index_ = -1;
    public:
        explicit SnapshotEnumerator(std::vector<T> items) : items_(std::move(items)) {}
        bool MoveNext() override { return ++index_ < static_cast<intcs>(items_.size()); }
        void Reset() override { index_ = -1; }
        [[nodiscard]] const T& Current() const override { return items_[static_cast<size_t>(index_)]; }
    };

public:
    /** @brief Default-constructs an empty ConcurrentQueue. */
    ConcurrentQueue() = default;

    /**
     * @brief Constructs a ConcurrentQueue populated with all elements from @p collection, in order.
     * C++ counterpart of .NET ConcurrentQueue(IEnumerable&lt;T&gt;).
     */
    explicit ConcurrentQueue(System::Collections::Generic::IEnumerable<T>& collection) {
        auto* e = collection.GetEnumerator();
        if (e) { while (e->MoveNext()) queue_.push(e->Current()); delete e; }
    }

    /** @brief Thread-safely adds item to the back of the queue. */
    void Enqueue(const T& item) {
        std::lock_guard<std::mutex> lk(mutex_);
        queue_.push(item);
    }

    /** @brief Thread-safely removes and returns the front element; returns false if empty. */
    bool TryDequeue(T& result) {
        std::lock_guard<std::mutex> lk(mutex_);
        if (queue_.empty()) return false;
        result = queue_.front();
        queue_.pop();
        return true;
    }

    /** @brief Thread-safely returns the front element without removing it; returns false if empty. */
    bool TryPeek(T& result) const {
        std::lock_guard<std::mutex> lk(mutex_);
        if (queue_.empty()) return false;
        result = queue_.front();
        return true;
    }

    /**
     * @brief Attempts to add an object to the collection.
     * C++ counterpart of .NET IProducerConsumerCollection&lt;T&gt;.TryAdd(T) — always succeeds for a queue.
     */
    bool TryAdd(const T& item) override { Enqueue(item); return true; }

    /**
     * @brief Attempts to remove and return an object from the collection (equivalent to TryDequeue).
     * C++ counterpart of .NET IProducerConsumerCollection&lt;T&gt;.TryTake(out T).
     */
    bool TryTake(T& item) override { return TryDequeue(item); }

    /** @brief Returns true if the queue contains no elements (thread-safe). */
    [[nodiscard]] bool getIsEmptyProperty() const override {
        std::lock_guard<std::mutex> lk(mutex_);
        return queue_.empty();
    }

    /** @brief Gets the number of elements in the queue (thread-safe). */
    [[nodiscard]] intcs getCountProperty() const override {
        std::lock_guard<std::mutex> lk(mutex_);
        return static_cast<intcs>(queue_.size());
    }

    /** @brief Thread-safely removes all elements from the queue. */
    void Clear() {
        std::lock_guard<std::mutex> lk(mutex_);
        while (!queue_.empty()) queue_.pop();
    }

    /**
     * @brief Copies the queue's elements (front-to-back) into @p array starting at @p index.
     * C++ counterpart of .NET ConcurrentQueue&lt;T&gt;.CopyTo(T[], int).
     * @throws System::ArgumentOutOfRangeException if @p index is negative.
     * @throws System::ArgumentException if @p array is not large enough.
     */
    void CopyTo(std::vector<T>& array, intcs index) override {
        std::lock_guard<std::mutex> lk(mutex_);
        if (index < 0) throw System::ArgumentOutOfRangeException("index");
        if (static_cast<size_t>(index) + queue_.size() > array.size())
            throw System::ArgumentException("Destination array was not long enough.");
        std::queue<T> copy = queue_;
        intcs i = index;
        while (!copy.empty()) { array[static_cast<size_t>(i++)] = copy.front(); copy.pop(); }
    }

    /**
     * @brief Copies the queue's elements (front-to-back) into a new vector.
     * C++ counterpart of .NET ConcurrentQueue&lt;T&gt;.ToArray().
     */
    [[nodiscard]] std::vector<T> ToArray() const override {
        std::lock_guard<std::mutex> lk(mutex_);
        std::vector<T> result;
        result.reserve(queue_.size());
        std::queue<T> copy = queue_;
        while (!copy.empty()) { result.push_back(copy.front()); copy.pop(); }
        return result;
    }

    /**
     * @brief Returns an enumerator over a point-in-time snapshot of the queue (front-to-back).
     * C++ counterpart of .NET ConcurrentQueue&lt;T&gt;.GetEnumerator().
     * @return Heap-allocated enumerator; caller takes ownership.
     */
    [[nodiscard]] System::Collections::Generic::IEnumerator<T>* GetEnumerator() override {
        return new SnapshotEnumerator(ToArray());
    }
};

} // namespace System::Collections::Concurrent
