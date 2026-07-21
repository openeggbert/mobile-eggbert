// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/Buffers/IBufferWriter.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Memory.hpp"
#include "System/ReadOnlyMemory.hpp"
#include "System/Span.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a heap-based, array-backed output sink into which T data can be written.
     *
     * C++ counterpart of .NET System.Buffers.ArrayBufferWriter&lt;T&gt;.
     * Implements IBufferWriter&lt;T&gt; using an internal std::vector&lt;T&gt; that grows as needed.
     *
     * @tparam T The type of items in the ArrayBufferWriter.
     */
    template<typename T>
    class ArrayBufferWriter : public IBufferWriter<T> {
        std::vector<T> buffer_;
        intcs writtenCount_ = 0;

        void checkAndResizeBuffer(intcs sizeHint) {
            if (sizeHint < 0)
                throw System::ArgumentException("sizeHint must be non-negative", "sizeHint");
            if (sizeHint == 0) sizeHint = 1;
            if (sizeHint > getFreeCapacityProperty()) {
                intcs currentLength = static_cast<intcs>(buffer_.size());
                // Matches .NET: grow by the larger of sizeHint and the current length,
                // special-casing an empty buffer to jump straight to DefaultInitialBufferSize.
                intcs growBy = std::max(sizeHint, currentLength);
                if (currentLength == 0) growBy = std::max(growBy, DefaultInitialBufferSize);
                buffer_.resize(static_cast<std::size_t>(currentLength + growBy));
            }
        }

    public:
        /** @brief Default initial capacity used when growing an empty buffer. */
        static constexpr intcs DefaultInitialBufferSize = 256;

        /**
         * @brief Constructs an ArrayBufferWriter with zero initial capacity.
         * C++ counterpart of .NET ArrayBufferWriter() — the backing buffer starts
         * empty; DefaultInitialBufferSize is only applied on the first growth.
         */
        ArrayBufferWriter() = default;

        /**
         * @brief Constructs an ArrayBufferWriter with the specified initial capacity.
         * @param initialCapacity The minimum initial capacity of the backing buffer.
         * @throws System::ArgumentException if initialCapacity is zero or negative.
         */
        explicit ArrayBufferWriter(intcs initialCapacity) {
            if (initialCapacity <= 0)
                throw System::ArgumentException("initialCapacity must be positive", "initialCapacity");
            buffer_.resize(static_cast<std::size_t>(initialCapacity));
        }

        /**
         * @brief Returns a ReadOnlyMemory view over the data written so far.
         * @return Read-only view of the written portion of the buffer.
         */
        [[nodiscard]] System::ReadOnlyMemory<T> getWrittenMemoryProperty() const noexcept {
            return System::ReadOnlyMemory<T>(buffer_.data(), writtenCount_);
        }

        /**
         * @brief Returns a ReadOnlySpan view over the data written so far.
         * @return Read-only view of the written portion of the buffer.
         */
        [[nodiscard]] System::ReadOnlySpan<T> getWrittenSpanProperty() const noexcept {
            return System::ReadOnlySpan<T>(buffer_.data(), writtenCount_);
        }

        /**
         * @brief Returns the total number of elements written to the buffer.
         * @return Number of elements written.
         */
        [[nodiscard]] intcs getWrittenCountProperty() const noexcept { return writtenCount_; }

        /**
         * @brief Returns the total capacity of the backing buffer.
         * @return Total buffer capacity.
         */
        [[nodiscard]] intcs getCapacityProperty() const noexcept {
            return static_cast<intcs>(buffer_.size());
        }

        /**
         * @brief Returns the remaining space in the buffer (capacity minus written count).
         * @return Number of elements that can still be written without reallocation.
         */
        [[nodiscard]] intcs getFreeCapacityProperty() const noexcept {
            return getCapacityProperty() - writtenCount_;
        }

        /**
         * @brief Clears the written data, zeroing the buffer's content, and resets
         * the written count to zero.
         *
         * C++ counterpart of .NET ArrayBufferWriter&lt;T&gt;.Clear(). Slower than
         * ResetWrittenCount() since it also zeroes the buffer; use ResetWrittenCount()
         * if the contents don't need to be cleared.
         */
        void Clear() {
            std::fill(buffer_.begin(), buffer_.begin() + writtenCount_, T{});
            writtenCount_ = 0;
        }

        /**
         * @brief Resets the written count to zero without zeroing the buffer's content.
         * C++ counterpart of .NET ArrayBufferWriter&lt;T&gt;.ResetWrittenCount().
         */
        void ResetWrittenCount() noexcept { writtenCount_ = 0; }

        /**
         * @brief Notifies the writer that @p count elements have been written.
         * @param count Number of elements written to the span returned by GetSpan().
         * @throws System::ArgumentException if count is negative.
         * @throws InvalidOperationException if count would advance past the end of the buffer.
         */
        void Advance(intcs count) override {
            if (count < 0)
                throw System::ArgumentException("count must be non-negative", "count");
            if (writtenCount_ > getCapacityProperty() - count)
                throw InvalidOperationException("Cannot advance past the end of the buffer.");
            writtenCount_ += count;
        }

        /**
         * @brief Returns a writable Memory with at least @p sizeHint free elements.
         * @param sizeHint Minimum elements requested (0 means at least 1).
         * @return Writable Memory over the free portion of the buffer.
         */
        System::Memory<T> GetMemory(intcs sizeHint = 0) override {
            checkAndResizeBuffer(sizeHint);
            return System::Memory<T>(buffer_, writtenCount_, getFreeCapacityProperty());
        }

        /**
         * @brief Returns a writable Span with at least @p sizeHint free elements.
         * @param sizeHint Minimum elements requested (0 means at least 1).
         * @return Writable Span over the free portion of the buffer.
         */
        System::Span<T> GetSpan(intcs sizeHint = 0) override {
            checkAndResizeBuffer(sizeHint);
            return System::Span<T>(buffer_.data() + writtenCount_, getFreeCapacityProperty());
        }
    };

} // namespace System::Buffers
