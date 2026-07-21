// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Span.hpp"
#include "System/ArraySegment.hpp"
#include "System/ReadOnlyMemory.hpp"
#include "System/Buffers/MemoryHandle.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a contiguous region of arbitrary memory.
     *
     * C++ counterpart of .NET System.Memory&lt;T&gt;.
     * Unlike .NET, this implementation is backed by a non-owning pointer into a
     * std::vector&lt;T&gt; together with an offset and a count.  There is no GC and
     * therefore no Pin()/MemoryHandle; use getSpanProperty() to obtain a
     * mutable view over the region.
     *
     * @tparam T The element type.
     */
    template<typename T>
    class Memory {
        std::vector<T>* data_   = nullptr;
        intcs           offset_ = 0;
        intcs           length_ = 0;

    public:
        // -----------------------------------------------------------------------
        // Constructors
        // -----------------------------------------------------------------------

        /**
         * @brief Initializes a default (empty) Memory&lt;T&gt;.
         *
         * C++ counterpart of .NET Memory&lt;T&gt; default value.
         */
        Memory() = default;

        /**
         * @brief Initializes a Memory&lt;T&gt; over the entirety of @p array.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;(T[]).
         * @param array The underlying vector.
         */
        explicit Memory(std::vector<T>& array)
            : data_(&array), offset_(0),
              length_(static_cast<intcs>(array.size())) {}

        /**
         * @brief Initializes a Memory&lt;T&gt; from an ArraySegment&lt;T&gt;.
         *
         * C++ counterpart of .NET implicit operator Memory&lt;T&gt;(ArraySegment&lt;T&gt;).
         * @param segment The source segment.
         */
        Memory(const ArraySegment<T>& segment)
            : data_(segment.getArrayProperty()),
              offset_(segment.getOffsetProperty()),
              length_(segment.getCountProperty()) {}

        /**
         * @brief Initializes a Memory&lt;T&gt; over a sub-range of @p array.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;(T[], int, int).
         * @param array  The underlying vector.
         * @param start  Zero-based index of the first element.
         * @param length Number of elements in the region.
         * @throws System::ArgumentOutOfRangeException if start or length are out of bounds.
         */
        Memory(std::vector<T>& array, intcs start, intcs length)
            : data_(&array), offset_(start), length_(length)
        {
            // start+length (both intcs/int32) can itself signed-overflow for large start/length,
            // silently bypassing this check instead of throwing -- confirmed real UB via a
            // standalone UBSan repro on the identical pattern in Span<T>::Slice (ticket 265);
            // fixed the same way real .NET's Span<T>.Slice(int,int) does: unsigned comparison,
            // subtraction instead of addition.
            auto sz = static_cast<intcs>(array.size());
            if (static_cast<SharpRuntime::uintcs>(start) > static_cast<SharpRuntime::uintcs>(sz) ||
                static_cast<SharpRuntime::uintcs>(length) > static_cast<SharpRuntime::uintcs>(sz - start))
                throw System::ArgumentOutOfRangeException("start");
        }

        // -----------------------------------------------------------------------
        // Static factory
        // -----------------------------------------------------------------------

        /**
         * @brief Returns an empty Memory&lt;T&gt;.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.Empty.
         */
        [[nodiscard]] static Memory<T> getEmpty() { return Memory<T>{}; }

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the number of elements in the memory region.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.Length.
         */
        [[nodiscard]] intcs getLengthProperty() const noexcept { return length_; }

        /**
         * @brief Returns true if the memory region is empty (Length == 0).
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.IsEmpty.
         */
        [[nodiscard]] bool getIsEmptyProperty() const noexcept { return length_ == 0; }

        /**
         * @brief Returns a Span&lt;T&gt; over this memory region.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.Span.
         */
        [[nodiscard]] Span<T> getSpanProperty() const noexcept {
            if (data_ == nullptr) return Span<T>{};
            return Span<T>(data_->data() + offset_, length_);
        }

        // -----------------------------------------------------------------------
        // Slice
        // -----------------------------------------------------------------------

        /**
         * @brief Forms a slice from the specified position to the end.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.Slice(int).
         * @param start Zero-based start index within this region.
         * @throws System::ArgumentOutOfRangeException if @p start is out of range.
         */
        [[nodiscard]] Memory<T> Slice(intcs start) const
        {
            if (start < 0 || start > length_)
                throw System::ArgumentOutOfRangeException("start");
            if (data_ == nullptr) return Memory<T>{};
            return Memory<T>(*data_, offset_ + start, length_ - start);
        }

        /**
         * @brief Forms a slice of the specified length starting at @p start.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.Slice(int, int).
         * @param start  Zero-based start index within this region.
         * @param length Number of elements in the slice.
         * @throws System::ArgumentOutOfRangeException if parameters are out of range.
         */
        [[nodiscard]] Memory<T> Slice(intcs start, intcs length) const
        {
            // See the (vector&, start, length) constructor above: same overflow-bypasses-the-
            // check bug, same fix.
            if (static_cast<SharpRuntime::uintcs>(start) > static_cast<SharpRuntime::uintcs>(length_) ||
                static_cast<SharpRuntime::uintcs>(length) > static_cast<SharpRuntime::uintcs>(length_ - start))
                throw System::ArgumentOutOfRangeException("start");
            if (data_ == nullptr) return Memory<T>{};
            return Memory<T>(*data_, offset_ + start, length);
        }

        // -----------------------------------------------------------------------
        // CopyTo / TryCopyTo
        // -----------------------------------------------------------------------

        /**
         * @brief Copies the contents into @p destination.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.CopyTo(Memory&lt;T&gt;).
         * @throws System::ArgumentException if destination is shorter than this region.
         */
        void CopyTo(Memory<T>& destination) const
        {
            if (length_ > destination.length_)
                throw System::ArgumentException("Memory<T>::CopyTo: destination too short");
            if (data_ == nullptr || length_ == 0) return;
            std::copy(data_->data() + offset_,
                      data_->data() + offset_ + length_,
                      destination.data_->data() + destination.offset_);
        }

        /**
         * @brief Attempts to copy the contents into @p destination.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.TryCopyTo(Memory&lt;T&gt;).
         * @return true on success; false if the destination is too short.
         */
        [[nodiscard]] bool TryCopyTo(Memory<T>& destination) const noexcept
        {
            if (length_ > destination.length_) return false;
            if (data_ == nullptr || length_ == 0) return true;
            std::copy(data_->data() + offset_,
                      data_->data() + offset_ + length_,
                      destination.data_->data() + destination.offset_);
            return true;
        }

        // -----------------------------------------------------------------------
        // Pin
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a handle exposing a raw pointer to this memory region.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.Pin(). .NET's Pin() prevents the
         * garbage collector from moving the backing array while the handle is alive;
         * this port has no GC and the backing std::vector never moves on its own, so
         * no actual pinning occurs - this exists for API/porting compatibility. As
         * with Span/ToArray, the returned pointer is only valid as long as the
         * backing vector is not reallocated (e.g. via push_back/resize) elsewhere.
         */
        [[nodiscard]] Buffers::MemoryHandle Pin() const noexcept {
            if (data_ == nullptr) return Buffers::MemoryHandle{};
            return Buffers::MemoryHandle(static_cast<void*>(data_->data() + offset_));
        }

        // -----------------------------------------------------------------------
        // ToArray
        // -----------------------------------------------------------------------

        /**
         * @brief Copies the region's elements into a new vector and returns it.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.ToArray().
         */
        [[nodiscard]] std::vector<T> ToArray() const
        {
            if (data_ == nullptr || length_ == 0) return {};
            return std::vector<T>(data_->begin() + offset_,
                                  data_->begin() + offset_ + length_);
        }

        // -----------------------------------------------------------------------
        // Equality
        // -----------------------------------------------------------------------

        /**
         * @brief Returns true if both instances point to the same vector at the
         * same offset with the same length.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.Equals(Memory&lt;T&gt;).
         */
        [[nodiscard]] bool Equals(const Memory<T>& other) const noexcept
        {
            return data_   == other.data_   &&
                   offset_ == other.offset_ &&
                   length_ == other.length_;
        }

        bool operator==(const Memory<T>& other) const noexcept { return Equals(other); }
        bool operator!=(const Memory<T>& other) const noexcept { return !Equals(other); }

        // -----------------------------------------------------------------------
        // GetHashCode / ToString
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a hash code based on the backing pointer, offset, and length.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.GetHashCode().
         */
        [[nodiscard]] intcs GetHashCode() const noexcept
        {
            size_t h = std::hash<void*>{}(data_);
            h ^= std::hash<intcs>{}(offset_) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<intcs>{}(length_)  + 0x9e3779b9 + (h << 6) + (h >> 2);
            return static_cast<intcs>(h & 0x7fffffff);
        }

        /**
         * @brief Returns a string describing this memory region.
         *
         * C++ counterpart of .NET Memory&lt;T&gt;.ToString().
         */
        [[nodiscard]] std::string ToString() const
        {
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << "System.Memory<T>[" << length_ << "]";
            return oss.str();
        }

        // -----------------------------------------------------------------------
        // Conversion to ReadOnlyMemory<T>
        // -----------------------------------------------------------------------

        /**
         * @brief Converts this Memory&lt;T&gt; to a ReadOnlyMemory&lt;T&gt;.
         *
         * C++ counterpart of .NET implicit operator ReadOnlyMemory&lt;T&gt;(Memory&lt;T&gt;).
         */
        operator ReadOnlyMemory<T>() const noexcept {
            if (data_ == nullptr) return ReadOnlyMemory<T>{};
            return ReadOnlyMemory<T>(data_->data() + offset_, length_);
        }
    };

} // namespace System
