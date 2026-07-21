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
 * @brief Represents a last-in, first-out (LIFO) collection of objects.
 *
 * C++ counterpart of .NET System.Collections.Generic.Stack<T>.
 * Backed by std::deque<T> (the same container std::stack<T> defaults to internally, chosen
 * directly here so the type can support iteration/enumeration); provides O(1) Push, Pop, and
 * Peek.
 *
 * @note GetEnumerator()'s Enumerator detects structural modification (Push/Pop/Clear) during
 * iteration via a version counter, matching .NET's InvalidOperationException fail-fast contract
 * (see List<T>/ArrayList's Enumerator in this codebase for the same established pattern).
 * Directly using begin()/end() STL iterators still follows plain std::deque<T> invalidation
 * rules, not .NET's version-checked contract -- only the GetEnumerator()-returned Enumerator is
 * fail-fast. Enumeration order matches real .NET's Stack<T>: top-to-bottom (most-recently-pushed
 * first, i.e. pop order) -- the reverse of begin()/end()'s underlying push order.
 *
 * @tparam T The type of elements in the stack.
 */
template<typename T>
class Stack {
    std::deque<T> stack_;
    intcs version_ = 0;

    class Enumerator : public IEnumerator<T>
    {
        const Stack<T>* stack_;
        intcs version_;
        intcs index_;
    public:
        explicit Enumerator(const Stack<T>* stack)
            : stack_(stack), version_(stack->version_), index_(static_cast<intcs>(stack->stack_.size())) {}
        bool MoveNext() override {
            if (version_ != stack_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            return --index_ >= 0;
        }
        void Reset() override {
            if (version_ != stack_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            index_ = static_cast<intcs>(stack_->stack_.size());
        }
        [[nodiscard]] const T& Current() const override { return stack_->stack_[static_cast<size_t>(index_)]; }
    };

public:
    /** @brief Initializes a new empty Stack<T>. */
    Stack() = default;

    /**
     * @brief Gets the number of elements contained in the Stack.
     *
     * C++ counterpart of .NET Stack<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(stack_.size()); }

    /**
     * @brief Inserts an object at the top of the Stack (copy).
     *
     * C++ counterpart of .NET Stack<T>.Push(T).
     * @param item The object to push onto the stack.
     */
    void Push(const T& item) { stack_.push_back(item); ++version_; }

    /**
     * @brief Inserts an object at the top of the Stack (move).
     *
     * C++ counterpart of .NET Stack<T>.Push(T).
     * @param item The object to push onto the stack (moved).
     */
    void Push(T&& item) { stack_.push_back(std::move(item)); ++version_; }

    /**
     * @brief Removes and returns the object at the top of the Stack.
     *
     * C++ counterpart of .NET Stack<T>.Pop().
     * @return The object removed from the top of the Stack.
     * @throws System::InvalidOperationException if the Stack is empty.
     */
    T Pop() {
        if (stack_.empty()) throw System::InvalidOperationException("Stack empty.");
        T val = std::move(stack_.back());
        stack_.pop_back();
        ++version_;
        return val;
    }

    /**
     * @brief Returns the object at the top of the Stack without removing it.
     *
     * C++ counterpart of .NET Stack<T>.Peek().
     * @return A const reference to the object at the top.
     * @throws System::InvalidOperationException if the Stack is empty.
     */
    [[nodiscard]] const T& Peek() const {
        if (stack_.empty()) throw System::InvalidOperationException("Stack empty.");
        return stack_.back();
    }

    /**
     * @brief Tries to remove and return the object at the top of the Stack.
     *
     * C++ counterpart of .NET Stack<T>.TryPop(out T).
     * @param result Receives the popped object if successful.
     * @return true if an element was removed; false if the Stack was empty.
     */
    bool TryPop(T& result) {
        if (stack_.empty()) return false;
        result = std::move(stack_.back());
        stack_.pop_back();
        ++version_;
        return true;
    }

    /**
     * @brief Tries to return the object at the top of the Stack without removing it.
     *
     * C++ counterpart of .NET Stack<T>.TryPeek(out T).
     * @param result Receives the top object if successful.
     * @return true if the Stack is non-empty; otherwise false.
     */
    bool TryPeek(T& result) const {
        if (stack_.empty()) return false;
        result = stack_.back();
        return true;
    }

    /**
     * @brief Determines whether the Stack contains the specified element.
     *
     * C++ counterpart of .NET Stack<T>.Contains(T).
     * @param item The object to locate.
     * @return true if the element is found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const {
        return std::find(stack_.begin(), stack_.end(), item) != stack_.end();
    }

    /**
     * @brief Removes all objects from the Stack.
     *
     * C++ counterpart of .NET Stack<T>.Clear().
     */
    void Clear() { stack_.clear(); ++version_; }

    /**
     * @brief Copies the Stack elements to a new vector in top-first order.
     *
     * C++ counterpart of .NET Stack<T>.ToArray().
     * @return A std::vector<T> where element 0 is the top of the stack.
     */
    [[nodiscard]] std::vector<T> ToArray() const {
        return std::vector<T>(stack_.rbegin(), stack_.rend());
    }

    /**
     * @brief Returns a version-checked enumerator over the Stack in top-to-bottom (pop) order.
     *
     * C++ counterpart of .NET Stack<T>.GetEnumerator(). Throws InvalidOperationException from
     * MoveNext()/Reset() if the Stack is structurally modified during enumeration.
     * @return A newly allocated IEnumerator<T>*; caller owns the returned object.
     */
    IEnumerator<T>* GetEnumerator() {
        return new Enumerator(this);
    }

    /**
     * @brief STL-interop mutable begin iterator (not part of the .NET API, not version-checked).
     * Iterates in PUSH order (bottom-to-top) -- the reverse of .NET enumeration order; use
     * GetEnumerator() for .NET-matching top-to-bottom order.
     */
    auto begin() { return stack_.begin(); }
    /** @brief STL-interop mutable end iterator (not part of the .NET API, not version-checked). */
    auto end()   { return stack_.end(); }
    /** @brief STL-interop const begin iterator (not part of the .NET API, not version-checked). */
    [[nodiscard]] auto begin() const { return stack_.cbegin(); }
    /** @brief STL-interop const end iterator (not part of the .NET API, not version-checked). */
    [[nodiscard]] auto end()   const { return stack_.cend(); }
};

} // namespace System::Collections::Generic
