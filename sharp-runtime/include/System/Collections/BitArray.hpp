// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include <stdexcept>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/IEnumerator.hpp"

namespace System::Collections {

    using SharpRuntime::intcs;

/**
 * @brief Manages a compact array of bit values, represented as Booleans.
 *
 * C++ counterpart of .NET System.Collections.BitArray.
 * Backed by std::vector<bool>.
 */
class BitArray {
    std::vector<bool> bits_;

    void requireSameLength(const BitArray& other) const {
        if (bits_.size() != other.bits_.size())
            throw System::ArgumentException("Array lengths must be the same.");
    }

    // .NET's real bounds check (BitArray.cs: `(uint)index >= (uint)_bitLength`) throws
    // System::ArgumentOutOfRangeException; using std::vector<bool>::at() here throws raw
    // std::out_of_range instead, which code catching the .NET-matching exception type won't
    // catch.
    void checkIndex(intcs index) const {
        if (static_cast<uint32_t>(index) >= static_cast<uint32_t>(bits_.size()))
            throw System::ArgumentOutOfRangeException("index");
    }

public:
    // -----------------------------------------------------------------------
    // Constructors
    // -----------------------------------------------------------------------

    /**
     * @brief Constructs a BitArray of the given length with all bits set to @p defaultValue.
     *
     * C++ counterpart of .NET BitArray(int) and BitArray(int, bool). Real .NET validates
     * `ArgumentOutOfRangeException.ThrowIfNegative(length)` before allocating.
     *
     * Without this check, a negative @p length cast to size_t wraps to a huge value passed
     * directly to std::vector<bool>'s constructor -- confirmed via a standalone ASan repro
     * that this is not merely "throws the wrong exception type" but a genuine, exploitable
     * heap-buffer-overflow: std::vector<bool>'s internal bit-to-word-count calculation
     * overflows for a size_t(-1)-scale request, silently allocating far less backing storage
     * than the reported size() implies, so any subsequent element access (even bits_[0]) is a
     * wild out-of-bounds write into unrelated heap memory (ASan: "heap-buffer-overflow...
     * WRITE of size 1").
     * @throws System::ArgumentOutOfRangeException if @p length is negative.
     */
    explicit BitArray(intcs length, bool defaultValue = false) {
        if (length < 0) throw System::ArgumentOutOfRangeException("length", "Non-negative number required.");
        bits_.assign(static_cast<size_t>(length), defaultValue);
    }

    /**
     * @brief Constructs a BitArray from a vector of bool values.
     *
     * C++ counterpart of .NET BitArray(bool[]).
     */
    explicit BitArray(const std::vector<bool>& values) : bits_(values) {}

    /**
     * @brief Constructs a BitArray from a vector of bytes, unpacking each byte into 8 bits (LSB-first).
     *
     * C++ counterpart of .NET BitArray(byte[]).
     */
    explicit BitArray(const std::vector<SharpRuntime::bytecs>& bytes) {
        bits_.reserve(bytes.size() * 8);
        for (auto b : bytes)
            for (int i = 0; i < 8; ++i)
                bits_.push_back((b >> i) & 1);
    }

    /**
     * @brief Constructs a BitArray from a vector of int values, unpacking each int into 32 bits (LSB-first).
     *
     * C++ counterpart of .NET BitArray(int[]).
     */
    explicit BitArray(const std::vector<SharpRuntime::intcs>& values) {
        bits_.reserve(values.size() * 32);
        for (auto v : values)
            for (int i = 0; i < 32; ++i)
                bits_.push_back((static_cast<uint32_t>(v) >> i) & 1u);
    }

    // -----------------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------------

    /**
     * @brief Gets the number of bits in the BitArray.
     *
     * C++ counterpart of .NET BitArray.Length.
     */
    [[nodiscard]] intcs getLengthProperty() const { return static_cast<intcs>(bits_.size()); }

    /**
     * @brief Sets the number of bits in the BitArray, growing or truncating as needed.
     *
     * C++ counterpart of .NET BitArray.Length setter. New bits introduced by growth are false.
     * @throws System::ArgumentOutOfRangeException if @p length is negative.
     */
    void setLengthProperty(intcs length) {
        if (length < 0) throw System::ArgumentOutOfRangeException("value", "Non-negative number required.");
        bits_.resize(static_cast<size_t>(length), false);
    }

    /**
     * @brief Gets the number of elements (bits) in the BitArray.
     *
     * C++ counterpart of .NET BitArray.Count.
     */
    [[nodiscard]] intcs getCountProperty()  const { return getLengthProperty(); }

    /**
     * @brief Returns false; BitArray is never read-only.
     *
     * C++ counterpart of .NET BitArray.IsReadOnly.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const { return false; }

    /**
     * @brief Returns false; BitArray is not thread-safe.
     *
     * C++ counterpart of .NET BitArray.IsSynchronized.
     */
    [[nodiscard]] bool getIsSynchronizedProperty() const { return false; }

    /**
     * @brief Gets an object that can be used to synchronize access to the BitArray.
     *
     * C++ counterpart of .NET BitArray.SyncRoot.
     */
    [[nodiscard]] const void* getSyncRootProperty() const { return this; }

    // -----------------------------------------------------------------------
    // Element access
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the value of the bit at the given index.
     *
     * C++ counterpart of .NET BitArray.Get(int).
     * @throws System::ArgumentOutOfRangeException if @p index is negative or >= Length.
     */
    [[nodiscard]] bool Get(intcs index) const { checkIndex(index); return bits_[static_cast<size_t>(index)]; }

    /**
     * @brief Sets the bit at the given index to the specified value.
     *
     * C++ counterpart of .NET BitArray.Set(int, bool).
     * @throws System::ArgumentOutOfRangeException if @p index is negative or >= Length.
     */
    void Set(intcs index, bool value)         { checkIndex(index); bits_[static_cast<size_t>(index)] = value; }

    /**
     * @brief Sets all bits in the BitArray to the specified value.
     *
     * C++ counterpart of .NET BitArray.SetAll(bool).
     */
    void SetAll(bool value)                   { std::fill(bits_.begin(), bits_.end(), value); }

    /**
     * @brief Returns the value of the bit at index @p i.
     *
     * C++ counterpart of .NET BitArray[int].
     * @throws System::ArgumentOutOfRangeException if @p i is negative or >= Length.
     */
    bool operator[](intcs i) const { checkIndex(i); return bits_[static_cast<size_t>(i)]; }

    // -----------------------------------------------------------------------
    // Bitwise operations
    // -----------------------------------------------------------------------

    /**
     * @brief Performs a bitwise AND of this BitArray with @p other in place.
     *
     * C++ counterpart of .NET BitArray.And(BitArray).
     * @throws System::ArgumentException if @p other's length differs from this BitArray's length.
     */
    BitArray& And(const BitArray& other) {
        requireSameLength(other);
        for (size_t i = 0; i < bits_.size(); ++i) bits_[i] = bits_[i] && other.bits_[i];
        return *this;
    }

    /**
     * @brief Performs a bitwise OR of this BitArray with @p other in place.
     *
     * C++ counterpart of .NET BitArray.Or(BitArray).
     * @throws System::ArgumentException if @p other's length differs from this BitArray's length.
     */
    BitArray& Or(const BitArray& other) {
        requireSameLength(other);
        for (size_t i = 0; i < bits_.size(); ++i) bits_[i] = bits_[i] || other.bits_[i];
        return *this;
    }

    /**
     * @brief Performs a bitwise XOR of this BitArray with @p other in place.
     *
     * C++ counterpart of .NET BitArray.Xor(BitArray).
     * @throws System::ArgumentException if @p other's length differs from this BitArray's length.
     */
    BitArray& Xor(const BitArray& other) {
        requireSameLength(other);
        for (size_t i = 0; i < bits_.size(); ++i) bits_[i] = bits_[i] != other.bits_[i];
        return *this;
    }

    /**
     * @brief Inverts all bit values in this BitArray in place.
     *
     * C++ counterpart of .NET BitArray.Not().
     */
    BitArray& Not() {
        for (size_t i = 0; i < bits_.size(); ++i) bits_[i] = !bits_[i];
        return *this;
    }

    /**
     * @brief Shifts all bits left (toward higher index) by @p count positions in place.
     *
     * C++ counterpart of .NET BitArray.LeftShift(int).
     * Vacated low positions are set to false.
     * @throws System::ArgumentOutOfRangeException if @p count is negative.
     */
    BitArray& LeftShift(intcs count) {
        if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        intcs len = static_cast<intcs>(bits_.size());
        for (intcs i = len - 1; i >= 0; --i)
            bits_[static_cast<size_t>(i)] = (i >= count) ? static_cast<bool>(bits_[static_cast<size_t>(i - count)]) : false;
        return *this;
    }

    /**
     * @brief Shifts all bits right (toward lower index) by @p count positions in place.
     *
     * C++ counterpart of .NET BitArray.RightShift(int).
     * Vacated high positions are set to false.
     * @throws System::ArgumentOutOfRangeException if @p count is negative.
     */
    BitArray& RightShift(intcs count) {
        if (count < 0) throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        intcs len = static_cast<intcs>(bits_.size());
        for (intcs i = 0; i < len; ++i)
            bits_[static_cast<size_t>(i)] = (i + count < len) ? static_cast<bool>(bits_[static_cast<size_t>(i + count)]) : false;
        return *this;
    }

    // -----------------------------------------------------------------------
    // Query
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the number of bits set to true in the BitArray.
     *
     * C++ counterpart of .NET BitArray.PopCount().
     */
    [[nodiscard]] intcs PopCount() const {
        intcs count = 0;
        for (bool b : bits_) if (b) ++count;
        return count;
    }

    /**
     * @brief Returns true if every bit in the array is set to 1.
     *
     * C++ counterpart of .NET BitArray.HasAllSet().
     */
    [[nodiscard]] bool HasAllSet() const {
        for (bool b : bits_) if (!b) return false;
        return true;
    }

    /**
     * @brief Returns true if at least one bit in the array is set to 1.
     *
     * C++ counterpart of .NET BitArray.HasAnySet().
     */
    [[nodiscard]] bool HasAnySet() const {
        for (bool b : bits_) if (b) return true;
        return false;
    }

    // -----------------------------------------------------------------------
    // Copy / clone
    // -----------------------------------------------------------------------

    /**
     * @brief Copies all bits into a bool vector (resized to match).
     *
     * C++ counterpart of .NET BitArray.CopyTo(Array, int) for bool arrays.
     */
    void CopyTo(std::vector<bool>& dest) const { dest = bits_; }

    /**
     * @brief Copies bits packed into bytes (LSB-first per byte) into a byte vector.
     *
     * C++ counterpart of .NET BitArray.CopyTo(Array, int) for byte arrays.
     */
    void CopyTo(std::vector<SharpRuntime::bytecs>& dest) const {
        size_t byteCount = (bits_.size() + 7) / 8;
        dest.assign(byteCount, 0);
        for (size_t i = 0; i < bits_.size(); ++i)
            if (bits_[i]) dest[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
    }

    /**
     * @brief Returns a copy of this BitArray.
     *
     * C++ counterpart of .NET BitArray.Clone().
     */
    [[nodiscard]] BitArray Clone() const { return BitArray(bits_); }

    // -----------------------------------------------------------------------
    // Enumerator
    // -----------------------------------------------------------------------

    /**
     * @brief Enumerates over each bool value in the BitArray.
     *
     * getCurrentProperty() returns a void* pointing to an internal bool; cast to bool* to read.
     */
    class Enumerator : public IEnumerator {
        const BitArray* arr_;
        int             index_;
        mutable bool    current_;
    public:
        explicit Enumerator(const BitArray* arr) : arr_(arr), index_(-1), current_(false) {}
        bool MoveNext() override {
            int len = arr_->getLengthProperty();
            if (index_ + 1 < len) {
                ++index_;
                current_ = arr_->Get(index_);
                return true;
            }
            return false;
        }
        void Reset() override { index_ = -1; current_ = false; }
        /** @brief Returns pointer to an internal bool holding the current bit value. */
        void* getCurrentProperty() const override { return &current_; }
    };

    /**
     * @brief Returns a heap-allocated Enumerator; caller takes ownership.
     *
     * C++ counterpart of .NET BitArray.GetEnumerator().
     */
    [[nodiscard]] IEnumerator* GetEnumerator() { return new Enumerator(this); }

    // -----------------------------------------------------------------------
    // Range-based for support
    // -----------------------------------------------------------------------

    /** @brief Returns an iterator to the beginning of the bit sequence. */
    auto begin() const { return bits_.begin(); }
    /** @brief Returns an iterator past the end of the bit sequence. */
    auto end()   const { return bits_.end(); }
};

} // namespace System::Collections
