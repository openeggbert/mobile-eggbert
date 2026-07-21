// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <deque>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/ICollection.hpp"
#include "System/Collections/IEnumerator.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Represents a non-generic last-in, first-out (LIFO) collection of objects.
 *
 * C++ counterpart of .NET System.Collections.Stack.
 * Elements are stored as void* pointers. Prefer Generic::Stack&lt;T&gt; for type-safe code.
 */
class Stack : public ICollection {
public:
    /** @brief Constructs an empty Stack. */
    Stack() = default;

    /**
     * @brief Constructs an empty Stack with a reserved capacity hint.
     *
     * C++ counterpart of .NET Stack(int initialCapacity).
     * @param initialCapacity Capacity hint; ignored in this implementation.
     */
    explicit Stack(intcs /*initialCapacity*/) {}

    /**
     * @brief Constructs a Stack populated with all elements from @p col.
     *
     * C++ counterpart of .NET Stack(ICollection col).
     * Each element is the raw void* returned by IEnumerator::getCurrentProperty().
     * @param col Source collection.
     */
    explicit Stack(ICollection& col) {
        IEnumerator* e = col.GetEnumerator();
        if (e) { while (e->MoveNext()) s_.push_back(e->getCurrentProperty()); delete e; }
    }

    // -----------------------------------------------------------------------
    // ICollection
    // -----------------------------------------------------------------------

    /**
     * @brief Gets the number of elements in the Stack.
     *
     * C++ counterpart of .NET Stack.Count.
     */
    [[nodiscard]] intcs getCountProperty() const override {
        return static_cast<intcs>(s_.size());
    }

    /**
     * @brief Copies all elements into a void*[] destination array (top-to-bottom order).
     *
     * C++ counterpart of .NET Stack.CopyTo(Array, int).
     * @param array Pointer to a void*[] buffer.
     * @param index Zero-based index at which copying begins.
     */
    void CopyTo(void* array, intcs index) override {
        auto** dest = static_cast<void**>(array);
        intcs i = index;
        for (auto it = s_.rbegin(); it != s_.rend(); ++it) dest[i++] = *it;
    }

    /** @brief Returns false; Stack is not synchronized. */
    [[nodiscard]] bool getIsSynchronizedProperty() const override { return false; }

    /** @brief Returns a synchronization root for this Stack. */
    [[nodiscard]] const void* getSyncRootProperty() const override { return this; }

    /**
     * @brief Returns a heap-allocated enumerator over the Stack (top-to-bottom); caller takes ownership.
     *
     * C++ counterpart of .NET Stack.GetEnumerator().
     * The enumerator throws InvalidOperationException if the Stack is modified
     * (Push/Pop/Clear) while enumeration is in progress, matching .NET.
     */
    IEnumerator* GetEnumerator() override { return new Enumerator(this); }

    // -----------------------------------------------------------------------
    // Stack operations
    // -----------------------------------------------------------------------

    /**
     * @brief Inserts an object at the top of the Stack.
     *
     * C++ counterpart of .NET Stack.Push(object?).
     * @param item The object to push.
     */
    void Push(void* item) { s_.push_back(item); ++version_; }

    /**
     * @brief Removes and returns the object at the top of the Stack.
     *
     * C++ counterpart of .NET Stack.Pop().
     * @throws System::InvalidOperationException if the Stack is empty.
     */
    void* Pop() {
        if (s_.empty()) throw System::InvalidOperationException("Stack empty.");
        void* v = s_.back(); s_.pop_back(); ++version_; return v;
    }

    /**
     * @brief Returns the object at the top of the Stack without removing it.
     *
     * C++ counterpart of .NET Stack.Peek().
     * @throws System::InvalidOperationException if the Stack is empty.
     */
    [[nodiscard]] void* Peek() const {
        if (s_.empty()) throw System::InvalidOperationException("Stack empty.");
        return s_.back();
    }

    /**
     * @brief Determines whether an element is in the Stack.
     *
     * C++ counterpart of .NET Stack.Contains(object?).
     * @param item The object to locate.
     * @return true if @p item is found; otherwise false.
     */
    [[nodiscard]] bool Contains(void* item) const {
        for (void* p : s_) if (p == item) return true;
        return false;
    }

    /**
     * @brief Removes all objects from the Stack.
     *
     * C++ counterpart of .NET Stack.Clear().
     */
    void Clear() { s_.clear(); ++version_; }

    /**
     * @brief Copies the Stack elements to a new void*[] array (top element first).
     *
     * C++ counterpart of .NET Stack.ToArray().
     * @return Vector of void* elements with top element first.
     */
    [[nodiscard]] std::vector<void*> ToArray() const {
        return std::vector<void*>(s_.rbegin(), s_.rend());
    }

    /**
     * @brief Returns a shallow copy of the Stack.
     *
     * C++ counterpart of .NET Stack.Clone().
     */
    [[nodiscard]] Stack Clone() const {
        Stack copy;
        copy.s_ = s_;
        return copy;
    }

private:
    std::deque<void*> s_;
    intcs version_ = 0;

    /**
     * @brief Enumerates the elements of a Stack from top to bottom.
     *
     * C++ counterpart of .NET Stack's private StackEnumerator. Detects concurrent
     * modification via a version counter, matching .NET's fail-fast enumeration contract.
     */
    class Enumerator : public IEnumerator {
        const Stack* s_;
        intcs version_;
        intcs index_;   // index from the top (0 == top element); -1 before start / after end
        bool started_ = false;

    public:
        explicit Enumerator(const Stack* s) : s_(s), version_(s->version_), index_(-1) {}

        bool MoveNext() override {
            if (version_ != s_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            if (index_ + 1 >= static_cast<intcs>(s_->s_.size())) { started_ = true; index_ = -1; return false; }
            ++index_;
            started_ = true;
            return true;
        }

        void Reset() override {
            if (version_ != s_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            index_ = -1;
            started_ = false;
        }

        [[nodiscard]] void* getCurrentProperty() const override {
            if (!started_ || index_ < 0)
                throw System::InvalidOperationException("Enumeration has either not started or has already finished.");
            return s_->s_[s_->s_.size() - 1 - static_cast<size_t>(index_)];
        }
    };
};

} // namespace System::Collections
