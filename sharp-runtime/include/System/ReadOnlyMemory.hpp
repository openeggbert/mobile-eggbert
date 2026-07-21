// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ArraySegment.hpp"
#include "System/Buffers/MemoryHandle.hpp"
#include "System/Span.hpp"

namespace System {

    using SharpRuntime::intcs;

    // Forward declaration only (not a full #include) to avoid a circular dependency:
    // Memory.hpp already includes this header for its ReadOnlyMemory<T> conversion
    // operator. CopyTo/TryCopyTo below take Memory<T>& by reference and only need
    // Memory<T>'s definition at the point their template bodies are instantiated,
    // by which time the calling translation unit will already have Memory.hpp
    // included (it needs a concrete Memory<T> to pass in).
    template<typename T> class Memory;

    /**
     * @brief Represents a contiguous region of read-only memory.
     *
     * C++ counterpart of .NET System.ReadOnlyMemory<T>. Wraps a const pointer and
     * element count; does not own the data. Equivalent to a non-owning view over an
     * array or contiguous buffer.
     *
     * @tparam T Element type.
     */
    template<typename T>
    class ReadOnlyMemory {
        const T* ptr_ = nullptr;
        intcs    length_ = 0;

    public:
        /** @brief Constructs an empty ReadOnlyMemory. Equivalent to ReadOnlyMemory<T>.Empty. */
        constexpr ReadOnlyMemory() noexcept = default;

        /**
         * @brief Constructs from a raw const pointer and element count.
         * @param ptr    Pointer to the first element.
         * @param length Number of elements.
         */
        constexpr ReadOnlyMemory(const T* ptr, intcs length) noexcept
            : ptr_(ptr), length_(length) {}

        /**
         * @brief Constructs from a std::vector, borrowing its data without copying.
         *
         * The vector must outlive this ReadOnlyMemory.
         * @param v Source vector.
         */
        explicit ReadOnlyMemory(const std::vector<T>& v) noexcept
            : ptr_(v.data()), length_(static_cast<intcs>(v.size())) {}

        /**
         * @brief Constructs a ReadOnlyMemory<T> from an ArraySegment<T>.
         *
         * C++ counterpart of .NET implicit operator ReadOnlyMemory<T>(ArraySegment<T>).
         * @param segment The source segment.
         */
        ReadOnlyMemory(const ArraySegment<T>& segment) noexcept
            : ptr_(segment.getArrayProperty() ? segment.getArrayProperty()->data() + segment.getOffsetProperty() : nullptr),
              length_(segment.getCountProperty()) {}

        /**
         * @brief Returns the number of elements in this memory region.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.Length.
         */
        [[nodiscard]] intcs getLengthProperty() const noexcept { return length_; }

        /**
         * @brief Returns true if the memory region contains no elements.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.IsEmpty.
         */
        [[nodiscard]] bool getIsEmptyProperty() const noexcept { return length_ == 0; }

        /**
         * @brief Returns a ReadOnlySpan<T> over the memory region.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.Span.
         */
        [[nodiscard]] ReadOnlySpan<T> getSpanProperty() const noexcept {
            return ReadOnlySpan<T>(ptr_, length_);
        }

        /** @brief Returns a raw const pointer to the start of the memory region. */
        [[nodiscard]] const T* getPointer() const noexcept { return ptr_; }

        /**
         * @brief Returns an empty ReadOnlyMemory<T>.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.Empty.
         */
        [[nodiscard]] static ReadOnlyMemory<T> getEmptyProperty() noexcept { return {}; }

        /**
         * @brief Element access by index.
         * @throws System::ArgumentOutOfRangeException if index is out of bounds.
         */
        [[nodiscard]] const T& operator[](intcs index) const {
            if (index < 0 || index >= length_)
                throw System::ArgumentOutOfRangeException("index");
            return ptr_[index];
        }

        /**
         * @brief Returns a slice of this memory starting at @p start with @p length elements.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.Slice(int, int).
         * @throws System::ArgumentOutOfRangeException if the slice extends outside the current range.
         */
        [[nodiscard]] ReadOnlyMemory<T> Slice(intcs start, intcs length) const {
            // start+length (both intcs/int32) can itself signed-overflow for large start/length,
            // silently bypassing this check instead of throwing -- confirmed real UB via a
            // standalone UBSan repro on the identical pattern in Span<T>::Slice (ticket 265);
            // fixed the same way real .NET's Span<T>.Slice(int,int) does: unsigned comparison,
            // subtraction instead of addition (length_-start can't overflow once start<=length_
            // is known).
            if (static_cast<SharpRuntime::uintcs>(start) > static_cast<SharpRuntime::uintcs>(length_) ||
                static_cast<SharpRuntime::uintcs>(length) > static_cast<SharpRuntime::uintcs>(length_ - start))
                throw System::ArgumentOutOfRangeException("start");
            return ReadOnlyMemory<T>(ptr_ + start, length);
        }

        /**
         * @brief Returns a slice from @p start to the end of the memory region.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.Slice(int).
         * @throws System::ArgumentOutOfRangeException if start is out of bounds.
         */
        [[nodiscard]] ReadOnlyMemory<T> Slice(intcs start) const {
            return Slice(start, length_ - start);
        }

        /**
         * @brief Copies the content into a new std::vector.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.ToArray().
         */
        [[nodiscard]] std::vector<T> ToArray() const {
            return std::vector<T>(ptr_, ptr_ + length_);
        }

        /**
         * @brief Copies the contents into @p destination.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.CopyTo(Memory<T>).
         * @throws System::ArgumentException if destination is shorter than this region.
         */
        void CopyTo(Memory<T>& destination) const {
            getSpanProperty().CopyTo(destination.getSpanProperty());
        }

        /**
         * @brief Attempts to copy the contents into @p destination.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.TryCopyTo(Memory<T>).
         * @return true on success; false if the destination is too short.
         */
        [[nodiscard]] bool TryCopyTo(Memory<T>& destination) const noexcept {
            return getSpanProperty().TryCopyTo(destination.getSpanProperty());
        }

        /**
         * @brief Returns a handle exposing a raw pointer to this memory region.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.Pin(). No actual pinning occurs -
         * this port has no GC - matching Memory<T>.Pin()'s deviation for the same reason.
         */
        [[nodiscard]] Buffers::MemoryHandle Pin() const noexcept {
            return Buffers::MemoryHandle(const_cast<void*>(static_cast<const void*>(ptr_)));
        }

        /**
         * @brief Returns a string description of this memory region.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.ToString().
         */
        [[nodiscard]] std::string ToString() const {
            return "System.ReadOnlyMemory<T>[" + std::to_string(length_) + "]";
        }

        /**
         * @brief Returns true if this ReadOnlyMemory<T> refers to the same region as @p other.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.Equals(ReadOnlyMemory<T>).
         * Two instances are equal when they point to the same start address with the same length.
         */
        [[nodiscard]] bool Equals(const ReadOnlyMemory<T>& other) const noexcept {
            return ptr_ == other.ptr_ && length_ == other.length_;
        }

        /**
         * @brief Returns a hash code based on the backing pointer and length.
         *
         * C++ counterpart of .NET ReadOnlyMemory<T>.GetHashCode().
         */
        [[nodiscard]] intcs GetHashCode() const noexcept {
            std::size_t h = std::hash<const void*>{}(ptr_);
            h ^= std::hash<intcs>{}(length_) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return static_cast<intcs>(h & 0x7fffffff);
        }

        /** @brief Returns true if the two instances refer to the same memory region. */
        friend bool operator==(const ReadOnlyMemory<T>& lhs, const ReadOnlyMemory<T>& rhs) noexcept {
            return lhs.Equals(rhs);
        }

        /** @brief Returns true if the two instances do not refer to the same memory region. */
        friend bool operator!=(const ReadOnlyMemory<T>& lhs, const ReadOnlyMemory<T>& rhs) noexcept {
            return !lhs.Equals(rhs);
        }
    };

} // namespace System
