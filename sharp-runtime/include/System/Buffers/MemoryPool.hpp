// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <limits>
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Buffers/IMemoryOwner.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

    /**
     * @brief Provides a resource pool that enables reusing instances of T arrays.
     *
     * C++ counterpart of .NET System.Buffers.MemoryPool&lt;T&gt;.
     * Subclasses provide pool-specific allocation strategies. The shared instance
     * provides a simple heap-backed default implementation.
     *
     * @tparam T The type of items in the memory pool.
     */
    template<typename T>
    class MemoryPool {
    public:
        /** Matches .NET's Array.MaxLength (Array.cs), the ceiling MaxBufferSize/Rent() validate against. */
        static constexpr intcs MaxArrayLength = 0x7FFFFFC7;

        /** @brief Virtual destructor. */
        virtual ~MemoryPool() = default;

        /**
         * @brief Returns the maximum buffer size supported by this pool.
         * @return Maximum number of elements a single Rent() may return.
         */
        [[nodiscard]] virtual intcs getMaxBufferSizeProperty() const noexcept {
            return MaxArrayLength;
        }

        /**
         * @brief Rents a memory block with at least @p minBufferSize elements.
         * @param minBufferSize Minimum number of elements; -1 (the default) means
         *        implementation choice. Any other negative value, or a value greater
         *        than getMaxBufferSizeProperty(), throws.
         * @return An IMemoryOwner&lt;T&gt; representing the rented block.
         * @throws System::ArgumentOutOfRangeException if @p minBufferSize is negative
         *         (and not -1) or exceeds getMaxBufferSizeProperty().
         */
        virtual std::unique_ptr<IMemoryOwner<T>> Rent(intcs minBufferSize = -1) = 0;

        /**
         * @brief Returns the shared MemoryPool instance.
         *
         * The shared pool allocates directly from the heap without pooling.
         * @return Reference to the process-wide shared MemoryPool&lt;T&gt;.
         */
        [[nodiscard]] static MemoryPool<T>& Shared();
    };

    // -----------------------------------------------------------------------
    // Implementation helpers (defined after MemoryPool is complete)
    // -----------------------------------------------------------------------

    /** @brief Simple heap-backed IMemoryOwner implementation. */
    template<typename T>
    class MemoryPoolHeapOwner_ : public IMemoryOwner<T> {
        std::vector<T> buf_;
    public:
        explicit MemoryPoolHeapOwner_(intcs size) : buf_(static_cast<std::size_t>(size)) {}
        System::Memory<T> getMemoryProperty() override { return System::Memory<T>(buf_); }
        void Dispose() override { buf_.clear(); buf_.shrink_to_fit(); }
    };

    /** @brief Concrete default pool implementation returned by Shared(). */
    template<typename T>
    class DefaultMemoryPool_ : public MemoryPool<T> {
    public:
        std::unique_ptr<IMemoryOwner<T>> Rent(intcs minBufferSize = -1) override {
            // Matches ArrayMemoryPool<T>.Rent (ArrayMemoryPool.cs): only exactly -1 means
            // "use the implementation default"; any other negative value is invalid. The
            // default itself is sized in *bytes* (~4096), not elements, unlike the prior
            // implementation here which always rented 4096 elements regardless of sizeof(T).
            if (minBufferSize == -1) {
                minBufferSize = 1 + (4095 / static_cast<intcs>(sizeof(T)));
            } else if (minBufferSize < 0 || minBufferSize > MemoryPool<T>::MaxArrayLength) {
                throw System::ArgumentOutOfRangeException("minimumBufferSize");
            }
            return std::make_unique<MemoryPoolHeapOwner_<T>>(minBufferSize);
        }
    };

    template<typename T>
    MemoryPool<T>& MemoryPool<T>::Shared() {
        static DefaultMemoryPool_<T> instance;
        return instance;
    }

} // namespace System::Buffers
