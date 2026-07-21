// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Collections::Immutable {

    using SharpRuntime::intcs;

    /** An immutable last-in, first-out (LIFO) stack. */
    template<typename T>
    class ImmutableStack {
        // Stored as vector; top is the last element (index size-1)
        using VecT = std::vector<T>;
        std::shared_ptr<const VecT> data_;

        explicit ImmutableStack(std::shared_ptr<const VecT> data) : data_(std::move(data)) {}

    public:
        /** Default-constructs an empty ImmutableStack. */
        ImmutableStack() : data_(std::make_shared<VecT>()) {}

        /** Returns an empty ImmutableStack. */
        static ImmutableStack<T> Empty() { return ImmutableStack<T>(); }

        /** Returns true if the stack contains no elements. */
        [[nodiscard]] bool getIsEmptyProperty() const { return data_->empty(); }
        /** Gets the number of elements in the stack. */
        [[nodiscard]] intcs getCountProperty()   const { return static_cast<intcs>(data_->size()); }

        /**
         * @brief Returns the element at the top of the stack without removing it.
         * @throws System::InvalidOperationException if the stack is empty.
         */
        [[nodiscard]] const T& Peek() const {
            if (data_->empty()) throw System::InvalidOperationException("Stack is empty.");
            return data_->back();
        }

        /** Returns a new stack with the given item added to the top. */
        [[nodiscard]] ImmutableStack<T> Push(const T& item) const {
            auto v = std::make_shared<VecT>(*data_);
            v->push_back(item);
            return ImmutableStack<T>(std::move(v));
        }

        /**
         * @brief Returns a new stack with the top element removed.
         * @throws System::InvalidOperationException if the stack is empty.
         */
        [[nodiscard]] ImmutableStack<T> Pop() const {
            if (data_->empty()) throw System::InvalidOperationException("Stack is empty.");
            auto v = std::make_shared<VecT>(*data_);
            v->pop_back();
            return ImmutableStack<T>(std::move(v));
        }

        /**
         * @brief Returns a new stack with the top element removed, and outputs that element via value.
         * @throws System::InvalidOperationException if the stack is empty.
         */
        [[nodiscard]] ImmutableStack<T> Pop(T& value) const {
            if (data_->empty()) throw System::InvalidOperationException("Stack is empty.");
            value = data_->back();
            auto v = std::make_shared<VecT>(*data_);
            v->pop_back();
            return ImmutableStack<T>(std::move(v));
        }

        /** Returns an empty ImmutableStack. */
        [[nodiscard]] ImmutableStack<T> Clear() const { return Empty(); }

        /** Returns an iterator to the top of the stack (iterates top-to-bottom). */
        auto begin() const { return data_->rbegin(); }
        /** Returns an iterator past the bottom of the stack. */
        auto end()   const { return data_->rend(); }
    };

} // namespace System::Collections::Immutable
