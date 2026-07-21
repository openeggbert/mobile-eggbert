// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <bit>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Collections::Specialized {

/**
 * @brief A compact structure that stores Boolean values and small integers as a 32-bit integer.
 *
 * C++ counterpart of .NET System.Collections.Specialized.BitVector32.
 * Provides two usage modes: per-bit Boolean flags (using CreateMask) and
 * multi-bit integer sections (using CreateSection).
 */
struct BitVector32 {

    /**
     * @brief Represents a contiguous range of bits within a BitVector32.
     *
     * C++ counterpart of .NET System.Collections.Specialized.BitVector32.Section.
     * Created via BitVector32::CreateSection.
     */
    struct Section {
        friend struct BitVector32;

    private:
        SharpRuntime::shortcs mask_;    ///< Bit mask for this section.
        SharpRuntime::shortcs offset_;  ///< Bit offset of this section within the 32-bit value.

        // Matches real .NET's `internal Section(short, short)` constructor: only
        // BitVector32::CreateSection may construct a Section. Real .NET's offset/mask fields
        // are also private, exposed only via the read-only Mask/Offset properties below.
        // A publicly-constructible Section would let client code build one with an
        // out-of-range offset (e.g. 100), bypassing CreateSection's `offset >= 32` check --
        // operator[](Section)/set(Section, int) would then shift a 32-bit value by an amount
        // >= 32, which is undefined behavior in C++ (confirmed via a standalone UBSan repro).
        Section(SharpRuntime::shortcs mask, SharpRuntime::shortcs offset) : mask_(mask), offset_(offset) {}

    public:
        /**
         * @brief Gets the bit mask for this section.
         *
         * C++ counterpart of .NET BitVector32.Section.Mask.
         * @return The mask (equivalent to .NET short).
         */
        [[nodiscard]] SharpRuntime::shortcs getMaskProperty()   const { return mask_; }

        /**
         * @brief Gets the bit offset of this section within the 32-bit value.
         *
         * C++ counterpart of .NET BitVector32.Section.Offset.
         * @return The offset (equivalent to .NET short).
         */
        [[nodiscard]] SharpRuntime::shortcs getOffsetProperty() const { return offset_; }

        /**
         * @brief Determines whether this Section equals another Section.
         *
         * C++ counterpart of .NET BitVector32.Section.Equals(BitVector32.Section).
         * @param other The Section to compare.
         * @return true if both mask and offset match.
         */
        [[nodiscard]] bool Equals(const Section& other) const {
            return mask_ == other.mask_ && offset_ == other.offset_;
        }

        /**
         * @brief Returns a hash code for this Section.
         *
         * C++ counterpart of .NET BitVector32.Section.GetHashCode().
         * @return A hash based on mask and offset.
         */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            return static_cast<SharpRuntime::intcs>(mask_) ^ (static_cast<SharpRuntime::intcs>(offset_) << 16);
        }

        /**
         * @brief Returns a string representation of this Section.
         *
         * C++ counterpart of .NET BitVector32.Section.ToString().
         * @return A string in the form "Section{0xMASK, 0xOFFSET}" (lowercase hex, no leading
         *         zeros), matching real .NET's `$"Section{{0x{Mask:x}, 0x{Offset:x}}}"` exactly.
         *         An earlier version of this method used decimal "mask=N, offset=N" labels
         *         instead -- a real format mismatch versus the .NET reference, not just a
         *         paraphrase, since it changed both the field labels and the number base.
         */
        [[nodiscard]] std::string ToString() const {
            return "Section{0x" + toLowerHex(mask_) + ", 0x" + toLowerHex(offset_) + "}";
        }

        /**
         * @brief Returns a string representation of the given Section.
         *
         * C++ counterpart of .NET BitVector32.Section.ToString(BitVector32.Section).
         * @param section The Section to convert.
         * @return The same string as @p section.ToString().
         */
        static std::string ToString(const Section& section) { return section.ToString(); }

    private:
        // Matches .NET's "x" format specifier: lowercase hex digits, no leading zeros, "0" for
        // zero. mask_/offset_ are always non-negative in practice (CreateSection only ever
        // builds a Section with a small positive mask and a non-negative offset < 32).
        static std::string toLowerHex(SharpRuntime::shortcs value) {
            if (value == 0) return "0";
            static constexpr char kDigits[] = "0123456789abcdef";
            std::string s;
            auto v = static_cast<uint32_t>(value);
            while (v > 0) { s = kDigits[v & 0xF] + s; v >>= 4; }
            return s;
        }

    public:

        /**
         * @brief Returns true if two Section values are equal.
         * @param a First Section.
         * @param b Second Section.
         * @return true if a.Equals(b).
         */
        friend bool operator==(const Section& a, const Section& b) { return a.Equals(b); }

        /**
         * @brief Returns true if two Section values are not equal.
         * @param a First Section.
         * @param b Second Section.
         * @return true if !a.Equals(b).
         */
        friend bool operator!=(const Section& a, const Section& b) { return !a.Equals(b); }
    };

private:
    uint32_t data_;

public:
    /**
     * @brief Default-constructs a BitVector32 with all bits cleared.
     *
     * C++ counterpart of the default BitVector32 state in .NET.
     */
    BitVector32() : data_(0) {}

    /**
     * @brief Constructs a BitVector32 from the given integer bit pattern.
     *
     * C++ counterpart of .NET BitVector32(int).
     * @param data The initial 32-bit value.
     */
    explicit BitVector32(SharpRuntime::intcs data) : data_(static_cast<uint32_t>(data)) {}

    /** @brief Copy-constructs a BitVector32. */
    BitVector32(const BitVector32&) = default;

    /** @brief Copy-assigns a BitVector32. */
    BitVector32& operator=(const BitVector32&) = default;

    /**
     * @brief Gets the internal 32-bit storage value.
     *
     * C++ counterpart of .NET BitVector32.Data.
     * @return The raw 32-bit integer data.
     */
    [[nodiscard]] SharpRuntime::intcs getDataProperty() const { return static_cast<SharpRuntime::intcs>(data_); }

    /**
     * @brief Returns true if all bits specified by the bit mask @p bit are set.
     *
     * C++ counterpart of .NET BitVector32.Item[int] getter.
     * @param bit A bitmask specifying the bit(s) to test.
     * @return true if all specified bits are set.
     */
    [[nodiscard]] bool operator[](SharpRuntime::intcs bit) const {
        return (data_ & static_cast<uint32_t>(bit)) == static_cast<uint32_t>(bit);
    }

    /**
     * @brief Sets or clears the bits specified by @p bit.
     *
     * C++ counterpart of .NET BitVector32.Item[int] setter.
     * @param bit   A bitmask specifying the bit(s) to modify.
     * @param value true to set the bits; false to clear them.
     */
    void set(SharpRuntime::intcs bit, bool value) {
        if (value) data_ |= static_cast<uint32_t>(bit);
        else       data_ &= ~static_cast<uint32_t>(bit);
    }

    /**
     * @brief Returns the integer value stored in the given @p section.
     *
     * C++ counterpart of .NET BitVector32.Item[BitVector32.Section] getter.
     * @param section The section describing the bit range.
     * @return The integer value extracted from the section.
     */
    [[nodiscard]] SharpRuntime::intcs operator[](const Section& section) const {
        return static_cast<SharpRuntime::intcs>((data_ >> section.offset_) & static_cast<uint16_t>(section.mask_));
    }

    /**
     * @brief Stores @p value in the given @p section.
     *
     * C++ counterpart of .NET BitVector32.Item[BitVector32.Section] setter.
     * @param section The section describing the bit range.
     * @param value   The integer value to store (must fit within the section's mask).
     */
    void set(const Section& section, SharpRuntime::intcs value) {
        data_ = (data_ & ~(static_cast<uint32_t>(static_cast<uint16_t>(section.mask_)) << section.offset_))
              | ((static_cast<uint32_t>(value) & static_cast<uint16_t>(section.mask_)) << section.offset_);
    }

    /**
     * @brief Creates the first bit mask (value 1) for use with this BitVector32.
     *
     * C++ counterpart of .NET BitVector32.CreateMask().
     * @return The integer 1.
     */
    static SharpRuntime::intcs CreateMask() { return CreateMask(0); }

    /**
     * @brief Creates the next bit mask by shifting @p previous one bit to the left.
     *
     * C++ counterpart of .NET BitVector32.CreateMask(int).
     * @param previous The previous mask; pass 0 to get the first mask.
     * @return The next mask in the sequence.
     * @throws System::InvalidOperationException if @p previous is already the last possible
     *         mask (@c int32_t minimum), since shifting further would overflow.
     */
    static SharpRuntime::intcs CreateMask(SharpRuntime::intcs previous) {
        if (previous == 0) return 1;
        if (previous == std::numeric_limits<SharpRuntime::intcs>::min())
            throw System::InvalidOperationException("Bit vector is full.");
        return static_cast<SharpRuntime::intcs>(static_cast<uint32_t>(previous) << 1);
    }

    /**
     * @brief Creates a Section that can hold a value in the range [0, maxValue].
     *
     * C++ counterpart of .NET BitVector32.CreateSection(short).
     * @param maxValue The maximum value the section must be able to hold; must be positive.
     * @return A Section starting at bit 0.
     * @throws System::ArgumentOutOfRangeException if @p maxValue is not positive.
     */
    static Section CreateSection(SharpRuntime::shortcs maxValue) { return CreateSection(maxValue, Section(0, 0)); }

    /**
     * @brief Creates a Section following @p previous that can hold a value in [0, maxValue].
     *
     * C++ counterpart of .NET BitVector32.CreateSection(short, BitVector32.Section).
     * @param maxValue The maximum value the section must be able to hold; must be positive.
     * @param previous The preceding Section; the new section starts just after it.
     * @return A new Section positioned after @p previous.
     * @throws System::ArgumentOutOfRangeException if @p maxValue is not positive.
     * @throws System::InvalidOperationException if the new section would start at or past bit 32.
     */
    static Section CreateSection(SharpRuntime::shortcs maxValue, const Section& previous) {
        if (maxValue <= 0)
            throw System::ArgumentOutOfRangeException("maxValue");

        auto offset = static_cast<SharpRuntime::shortcs>(
            previous.offset_ + std::popcount(static_cast<uint16_t>(previous.mask_)));
        if (offset >= 32)
            throw System::InvalidOperationException("Bit vector is full.");

        uint16_t mask = 0;
        auto v = static_cast<uint16_t>(maxValue);
        while (v > 0) { mask = static_cast<uint16_t>((mask << 1) | 1); v >>= 1; }
        return Section(static_cast<SharpRuntime::shortcs>(mask), offset);
    }

    /**
     * @brief Determines whether this BitVector32 equals @p other.
     *
     * C++ counterpart of .NET BitVector32.Equals(BitVector32).
     * @param other The BitVector32 to compare.
     * @return true if both have identical bit patterns.
     */
    [[nodiscard]] bool Equals(const BitVector32& other) const { return data_ == other.data_; }

    /**
     * @brief Returns a hash code for this BitVector32.
     *
     * C++ counterpart of .NET BitVector32.GetHashCode().
     * @return The raw 32-bit data cast to int.
     */
    [[nodiscard]] SharpRuntime::intcs GetHashCode() const { return static_cast<SharpRuntime::intcs>(data_); }

    /**
     * @brief Returns true if both BitVector32 values have identical bit patterns.
     * @param o The value to compare against.
     * @return true if equal.
     */
    bool operator==(const BitVector32& o) const { return data_ == o.data_; }

    /**
     * @brief Returns true if the BitVector32 values differ.
     * @param o The value to compare against.
     * @return true if not equal.
     */
    bool operator!=(const BitVector32& o) const { return data_ != o.data_; }

    /**
     * @brief Returns a string representation of the bit pattern.
     *
     * C++ counterpart of .NET BitVector32.ToString().
     * @return A string in the form "BitVector32{XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}" (32 bits, MSB first).
     */
    [[nodiscard]] std::string ToString() const {
        std::string s = "BitVector32{";
        for (int i = 31; i >= 0; --i) s += (data_ & (1u << i)) ? '1' : '0';
        return s + "}";
    }

    /**
     * @brief Returns a string representation of the given BitVector32.
     *
     * C++ counterpart of .NET BitVector32.ToString(BitVector32).
     * @param value The BitVector32 to convert.
     * @return The same string as @p value.ToString().
     */
    static std::string ToString(const BitVector32& value) { return value.ToString(); }
};

} // namespace System::Collections::Specialized
