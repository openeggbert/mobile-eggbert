// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IndexOutOfRangeException.hpp"

namespace System {

    using SharpRuntime::intcs;

    // Forward-declare ReadOnlySpan so Span can reference it.
    template<typename T> class ReadOnlySpan;

    /**
     * @brief A type-safe, non-owning view over a contiguous region of mutable memory.
     *
     * C++ counterpart of .NET System.Span&lt;T&gt;.
     * The caller is responsible for ensuring the underlying storage outlives the span.
     * Unlike arrays, a Span can point to stack, heap, or any contiguous native memory.
     *
     * @tparam T The type of elements in the span.
     */
    template<typename T>
    class Span {
        T*    ptr_    = nullptr;
        intcs length_ = 0;
    public:
        // -----------------------------------------------------------------------
        // Constructors
        // -----------------------------------------------------------------------

        /**
         * @brief Constructs an empty Span (null pointer, zero length).
         *
         * C++ counterpart of .NET Span&lt;T&gt; default constructor.
         */
        Span() = default;

        /**
         * @brief Constructs a Span over @p length elements starting at @p ptr.
         *
         * C++ counterpart of .NET Span&lt;T&gt;(void*, int).
         * @param ptr    Pointer to the first element.
         * @param length Number of elements.
         */
        Span(T* ptr, intcs length) : ptr_(ptr), length_(length) {}

        /**
         * @brief Constructs a Span covering the entire contents of a vector.
         * @param v The source vector; must outlive this span.
         */
        explicit Span(std::vector<T>& v)
            : ptr_(v.data()), length_(static_cast<intcs>(v.size())) {}

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the number of elements in the span.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.Length.
         */
        [[nodiscard]] intcs getLengthProperty() const noexcept { return length_; }

        /**
         * @brief Gets a value indicating whether this span is empty.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.IsEmpty.
         */
        [[nodiscard]] bool getIsEmptyProperty() const noexcept { return length_ == 0; }

        /**
         * @brief Returns an empty Span&lt;T&gt;.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.Empty.
         */
        [[nodiscard]] static Span<T> Empty() noexcept { return {}; }

        // -----------------------------------------------------------------------
        // Element access
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a reference to the element at the specified index.
         * @param i The zero-based index.
         * @throws System::IndexOutOfRangeException if @p i is out of bounds.
         */
        T& operator[](intcs i) {
            if (i < 0 || i >= length_) throw System::IndexOutOfRangeException();
            return ptr_[i];
        }

        /**
         * @brief Returns a const reference to the element at the specified index.
         * @param i The zero-based index.
         * @throws System::IndexOutOfRangeException if @p i is out of bounds.
         */
        const T& operator[](intcs i) const {
            if (i < 0 || i >= length_) throw System::IndexOutOfRangeException();
            return ptr_[i];
        }

        /** @brief Returns a pointer to the first element. */
        [[nodiscard]] T* getPointer() noexcept { return ptr_; }
        /** @brief Returns a const pointer to the first element. */
        [[nodiscard]] const T* getPointer() const noexcept { return ptr_; }

        // -----------------------------------------------------------------------
        // Iteration
        // -----------------------------------------------------------------------

        T*       begin() noexcept       { return ptr_; }
        T*       end()   noexcept       { return ptr_ + length_; }
        const T* begin() const noexcept { return ptr_; }
        const T* end()   const noexcept { return ptr_ + length_; }

        // -----------------------------------------------------------------------
        // Slicing
        // -----------------------------------------------------------------------

        /**
         * @brief Forms a slice starting at @p start extending to the end of this span.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.Slice(int).
         * @param start The zero-based index at which to begin the slice.
         * @throws System::ArgumentOutOfRangeException if @p start is out of range.
         */
        [[nodiscard]] Span<T> Slice(intcs start) const {
            if (start < 0 || start > length_) throw System::ArgumentOutOfRangeException("start");
            return Span<T>(ptr_ + start, length_ - start);
        }

        /**
         * @brief Forms a slice of @p length elements starting at @p start.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.Slice(int, int).
         * @param start  The zero-based index at which to begin the slice.
         * @param length The number of elements in the slice.
         * @throws System::ArgumentOutOfRangeException if the range is invalid.
         */
        [[nodiscard]] Span<T> Slice(intcs start, intcs length) const {
            // start+length (both intcs/int32) can itself signed-overflow for large start/length
            // -- confirmed real UB via a standalone UBSan repro before fixing, and worse than
            // "just UB": the wrapped (very negative) sum then compares as <= length_, silently
            // BYPASSING this bounds check entirely (e.g. start=INT32_MAX, length=10 on a
            // 10-element span). Real .NET's Span<T>.Slice(int,int) guards against exactly this
            // (see its own comment) by casting to unsigned and rearranging the comparison as a
            // subtraction instead of an addition -- length_-start cannot overflow once we know
            // 0 <= start <= length_, and casting to unsigned makes a negative start/length
            // compare as huge instead of silently passing.
            if (static_cast<SharpRuntime::uintcs>(start) > static_cast<SharpRuntime::uintcs>(length_) ||
                static_cast<SharpRuntime::uintcs>(length) > static_cast<SharpRuntime::uintcs>(length_ - start))
                throw System::ArgumentOutOfRangeException("start");
            return Span<T>(ptr_ + start, length);
        }

        // -----------------------------------------------------------------------
        // Copy, fill, and clear
        // -----------------------------------------------------------------------

        /**
         * @brief Copies the contents of this span into a destination span.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.CopyTo(Span&lt;T&gt;).
         * @param destination The span to copy items into.
         * @throws System::ArgumentException if the destination is shorter than this span.
         */
        void CopyTo(Span<T> destination) const {
            if (length_ > destination.getLengthProperty())
                throw System::ArgumentException("Destination is too short.");
            std::copy(ptr_, ptr_ + length_, destination.getPointer());
        }

        /**
         * @brief Attempts to copy this span into a destination span.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.TryCopyTo(Span&lt;T&gt;).
         * @param destination The span to copy items into.
         * @return true if the copy succeeded; false if the destination is too short.
         */
        [[nodiscard]] bool TryCopyTo(Span<T> destination) const noexcept {
            if (length_ > destination.getLengthProperty()) return false;
            std::copy(ptr_, ptr_ + length_, destination.getPointer());
            return true;
        }

        /**
         * @brief Copies the contents into a new vector.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.ToArray().
         * @return A new std::vector&lt;T&gt; containing the elements of this span.
         */
        [[nodiscard]] std::vector<T> ToArray() const {
            return std::vector<T>(ptr_, ptr_ + length_);
        }

        /**
         * @brief Fills all elements of this span with @p value.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.Fill(T).
         * @param value The value to assign to every element.
         */
        void Fill(const T& value) noexcept {
            std::fill(ptr_, ptr_ + length_, value);
        }

        /**
         * @brief Sets all elements to the default value of T.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.Clear().
         */
        void Clear() noexcept {
            std::fill(ptr_, ptr_ + length_, T{});
        }

        // -----------------------------------------------------------------------
        // Conversion
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a string representation of this span.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.ToString().
         * For @c char spans, returns the characters as a std::string.
         * For other types, returns a type descriptor.
         */
        [[nodiscard]] std::string ToString() const {
            if constexpr (std::is_same_v<T, char>) {
                return std::string(ptr_, static_cast<std::size_t>(length_));
            } else {
                return std::string("System.Span<")
                     + typeid(T).name() + ">["
                     + std::to_string(length_) + "]";
            }
        }

        /**
         * @brief Implicit conversion to ReadOnlySpan&lt;T&gt;.
         *
         * C++ counterpart of .NET implicit Span&lt;T&gt; → ReadOnlySpan&lt;T&gt; operator.
         */
        operator ReadOnlySpan<T>() const noexcept;  // defined after ReadOnlySpan

        // -----------------------------------------------------------------------
        // Comparison operators
        // -----------------------------------------------------------------------

        /**
         * @brief Returns true if both spans point to the same memory and have the same length.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.operator ==.
         * Does NOT compare element contents.
         */
        [[nodiscard]] bool operator==(const Span<T>& o) const noexcept {
            return ptr_ == o.ptr_ && length_ == o.length_;
        }

        /**
         * @brief Returns true if the spans differ in pointer or length.
         *
         * C++ counterpart of .NET Span&lt;T&gt;.operator !=.
         */
        [[nodiscard]] bool operator!=(const Span<T>& o) const noexcept {
            return !(*this == o);
        }
    };

    /**
     * @brief Provides a type-safe, non-owning read-only view over a contiguous region of memory.
     *
     * C++ counterpart of .NET System.ReadOnlySpan&lt;T&gt;.
     * The caller is responsible for ensuring the underlying storage outlives the span.
     * Unlike Span&lt;T&gt;, the elements cannot be mutated through this view.
     */
    template<typename T>
    class ReadOnlySpan {
        const T* ptr_    = nullptr;
        intcs    length_ = 0;
    public:
        /**
         * @brief Constructs an empty ReadOnlySpan (null pointer, zero length).
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt; default constructor.
         */
        ReadOnlySpan() = default;

        /**
         * @brief Constructs a ReadOnlySpan over @p length elements starting at @p ptr.
         * @param ptr    Pointer to the first element.
         * @param length Number of elements.
         */
        ReadOnlySpan(const T* ptr, intcs length) : ptr_(ptr), length_(length) {}

        /**
         * @brief Constructs a ReadOnlySpan covering the entire contents of a vector.
         * @param v The source vector; must outlive this span.
         */
        explicit ReadOnlySpan(const std::vector<T>& v)
            : ptr_(v.data()), length_(static_cast<intcs>(v.size())) {}

        /**
         * @brief Constructs a ReadOnlySpan from a mutable Span.
         *
         * C++ counterpart of .NET implicit Span&lt;T&gt; → ReadOnlySpan&lt;T&gt; conversion.
         * @param span The source Span; must outlive this span.
         */
        ReadOnlySpan(const Span<T>& span)  // NOLINT(google-explicit-constructor)
            : ptr_(span.getPointer()), length_(span.getLengthProperty()) {}

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the number of elements in the read-only span.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.Length.
         */
        [[nodiscard]] intcs getLengthProperty() const noexcept { return length_; }

        /**
         * @brief Gets a value indicating whether this span is empty.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.IsEmpty.
         */
        [[nodiscard]] bool getIsEmptyProperty() const noexcept { return length_ == 0; }

        /**
         * @brief Returns a read-only span with zero elements.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.Empty.
         */
        [[nodiscard]] static ReadOnlySpan<T> Empty() noexcept { return {}; }

        // -----------------------------------------------------------------------
        // Element access
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a const reference to the element at the specified index.
         * @param i The zero-based index of the element.
         * @throws System::IndexOutOfRangeException if @p i is out of bounds.
         */
        const T& operator[](intcs i) const {
            if (i < 0 || i >= length_) throw System::IndexOutOfRangeException();
            return ptr_[i];
        }

        /** @brief Returns a const pointer to the first element. */
        [[nodiscard]] const T* getPointer() const noexcept { return ptr_; }

        /** @brief Const iterator to the first element. */
        const T* begin() const noexcept { return ptr_; }
        /** @brief Const iterator past the last element. */
        const T* end()   const noexcept { return ptr_ + length_; }

        // -----------------------------------------------------------------------
        // Slicing
        // -----------------------------------------------------------------------

        /**
         * @brief Forms a slice starting at @p start extending to the end of this span.
         * @param start The zero-based index at which to begin the slice.
         * @throws System::ArgumentOutOfRangeException if @p start is out of range.
         */
        [[nodiscard]] ReadOnlySpan<T> Slice(intcs start) const {
            if (start < 0 || start > length_) throw System::ArgumentOutOfRangeException("start");
            return ReadOnlySpan<T>(ptr_ + start, length_ - start);
        }

        /**
         * @brief Forms a slice of @p length elements starting at @p start.
         * @param start  The zero-based index at which to begin the slice.
         * @param length The number of elements in the slice.
         * @throws System::ArgumentOutOfRangeException if the range is invalid.
         */
        [[nodiscard]] ReadOnlySpan<T> Slice(intcs start, intcs length) const {
            // See Span<T>::Slice(intcs,intcs)'s doc-comment: start+length can itself overflow
            // and silently bypass this check (confirmed real UB via a standalone UBSan repro);
            // this mirrors that fix.
            if (static_cast<SharpRuntime::uintcs>(start) > static_cast<SharpRuntime::uintcs>(length_) ||
                static_cast<SharpRuntime::uintcs>(length) > static_cast<SharpRuntime::uintcs>(length_ - start))
                throw System::ArgumentOutOfRangeException("start");
            return ReadOnlySpan<T>(ptr_ + start, length);
        }

        // -----------------------------------------------------------------------
        // Copy and conversion
        // -----------------------------------------------------------------------

        /**
         * @brief Copies the contents of this read-only span into a destination Span.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.CopyTo(Span&lt;T&gt;).
         * @param destination The span to copy items into.
         * @throws System::ArgumentException if the destination is shorter than this span.
         */
        void CopyTo(Span<T> destination) const {
            if (length_ > destination.getLengthProperty())
                throw System::ArgumentException("Destination is too short.");
            std::copy(ptr_, ptr_ + length_, destination.getPointer());
        }

        /**
         * @brief Attempts to copy this read-only span into a destination Span.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.TryCopyTo(Span&lt;T&gt;).
         * @param destination The span to copy items into.
         * @return true if the copy succeeded; false if the destination is too short.
         */
        [[nodiscard]] bool TryCopyTo(Span<T> destination) const noexcept {
            if (length_ > destination.getLengthProperty()) return false;
            std::copy(ptr_, ptr_ + length_, destination.getPointer());
            return true;
        }

        /**
         * @brief Copies the contents of this read-only span into a new vector.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.ToArray().
         * @return A new std::vector&lt;T&gt; containing the elements of this span.
         */
        [[nodiscard]] std::vector<T> ToArray() const {
            return std::vector<T>(ptr_, ptr_ + length_);
        }

        /**
         * @brief Returns a string representation of this span.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.ToString().
         * For char spans, returns the characters as a string.
         * For other types, returns a type descriptor.
         */
        [[nodiscard]] std::string ToString() const {
            if constexpr (std::is_same_v<T, char>) {
                return std::string(ptr_, static_cast<std::size_t>(length_));
            } else {
                return std::string("System.ReadOnlySpan<")
                     + typeid(T).name() + ">["
                     + std::to_string(length_) + "]";
            }
        }

        // -----------------------------------------------------------------------
        // Comparison operators
        // -----------------------------------------------------------------------

        /**
         * @brief Returns true if both spans point to the same memory and have the same length.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.operator ==.
         * Does NOT compare element contents.
         */
        [[nodiscard]] bool operator==(const ReadOnlySpan<T>& o) const noexcept {
            return ptr_ == o.ptr_ && length_ == o.length_;
        }

        /**
         * @brief Returns true if the spans differ in pointer or length.
         *
         * C++ counterpart of .NET ReadOnlySpan&lt;T&gt;.operator !=.
         */
        [[nodiscard]] bool operator!=(const ReadOnlySpan<T>& o) const noexcept {
            return !(*this == o);
        }
    };

    // Out-of-line definition: Span<T> → ReadOnlySpan<T> conversion (ReadOnlySpan now complete).
    template<typename T>
    inline Span<T>::operator ReadOnlySpan<T>() const noexcept {
        return ReadOnlySpan<T>(ptr_, length_);
    }

} // namespace System
