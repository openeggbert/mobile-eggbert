// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

    /** Provides a resource pool for reusing instances of arrays of type T. */
    template<typename T>
    class ArrayPool {
    public:
        /** Destroys the pool and releases associated resources. */
        virtual ~ArrayPool() = default;

        /**
         * @brief Retrieves a buffer from the pool with at least the specified minimum length.
         * @throws ArgumentOutOfRangeException if minimumLength is negative.
         */
        virtual std::vector<T> Rent(intcs minimumLength) = 0;
        /** Returns a rented buffer back to the pool, optionally clearing its contents first. */
        virtual void Return(std::vector<T>& array, bool clearArray = false) {
            if (clearArray) array.assign(array.size(), T{});
        }

        /** Returns a shared, process-wide ArrayPool instance. */
        static ArrayPool<T>& Shared();

        /**
         * @brief Creates a new ArrayPool with default capacity limits.
         *
         * C++ counterpart of .NET ArrayPool&lt;T&gt;.Create(). Returns a heap-backed
         * pool that allocates a new vector on each Rent call.
         */
        static std::unique_ptr<ArrayPool<T>> Create();

        /**
         * @brief Creates a new ArrayPool with specified capacity limits.
         *
         * C++ counterpart of .NET ArrayPool&lt;T&gt;.Create(int, int).
         * The capacity parameters are accepted for API compatibility but are
         * not used in this stub implementation.
         * @param maxArrayLength       Maximum allowed array length.
         * @param maxArraysPerBucket   Maximum number of arrays per bucket.
         */
        static std::unique_ptr<ArrayPool<T>> Create(intcs maxArrayLength, intcs maxArraysPerBucket);
    };

    // Defined after ArrayPool<T> is complete to avoid incomplete-type warning.
    /** Default shared ArrayPool implementation that allocates a new vector on each Rent call. */
    template<typename T>
    struct SharedArrayPool : ArrayPool<T> {
        /**
         * @brief Rents a vector of at least minimumLength elements.
         * @throws ArgumentOutOfRangeException if minimumLength is negative.
         */
        std::vector<T> Rent(intcs minimumLength) override {
            if (minimumLength < 0)
                throw ArgumentOutOfRangeException("minimumLength");
            return std::vector<T>(static_cast<size_t>(minimumLength));
        }
    };

    template<typename T>
    ArrayPool<T>& ArrayPool<T>::Shared() {
        static SharedArrayPool<T> instance;
        return instance;
    }

    template<typename T>
    std::unique_ptr<ArrayPool<T>> ArrayPool<T>::Create() {
        return std::make_unique<SharedArrayPool<T>>();
    }

    template<typename T>
    std::unique_ptr<ArrayPool<T>> ArrayPool<T>::Create(intcs /*maxArrayLength*/, intcs /*maxArraysPerBucket*/) {
        return std::make_unique<SharedArrayPool<T>>();
    }

} // namespace System::Buffers
