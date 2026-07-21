// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Index.hpp"

namespace System {

    /**
     * @brief Represents a range that has start and end indexes.
     *
     * C++ counterpart of .NET System.Range. Used with Index to express slices of
     * collections. The range [start, end) is half-open: start is inclusive, end is
     * exclusive.
     */
    struct Range {
    private:
        Index start_;
        Index end_;

        static std::string indexToString(const Index& idx) {
            std::string s = idx.getIsFromEndProperty() ? "^" : "";
            s += std::to_string(idx.getValueProperty());
            return s;
        }

        static bool indexEquals(const Index& a, const Index& b) noexcept {
            return a.getValueProperty() == b.getValueProperty() &&
                   a.getIsFromEndProperty() == b.getIsFromEndProperty();
        }

    public:
        /**
         * @brief Constructs a Range that covers the entire collection (equivalent to Range.All).
         */
        Range() : start_(Index::Start()), end_(Index::End()) {}

        /**
         * @brief Constructs a Range with explicit start and end indexes.
         * @param start Inclusive start index.
         * @param end   Exclusive end index.
         */
        Range(Index start, Index end) : start_(start), end_(end) {}

        /** @brief Returns the inclusive start index of the range. */
        [[nodiscard]] const Index& getStartProperty() const noexcept { return start_; }

        /** @brief Returns the exclusive end index of the range. */
        [[nodiscard]] const Index& getEndProperty() const noexcept { return end_; }

        /**
         * @brief Returns a Range that covers all elements of a collection (0..^0).
         *
         * C++ counterpart of .NET Range.All.
         */
        [[nodiscard]] static Range getAllProperty() { return Range(Index::Start(), Index::End()); }

        /**
         * @brief Returns a Range that starts at the specified index and ends at the end of the collection.
         * @param start Inclusive start index.
         */
        [[nodiscard]] static Range StartAt(Index start) { return Range(start, Index::End()); }

        /**
         * @brief Returns a Range from the beginning of the collection to the specified end index.
         * @param end Exclusive end index.
         */
        [[nodiscard]] static Range EndAt(Index end) { return Range(Index::Start(), end); }

        /** @brief Holds the resolved offset and length for a given collection size. */
        struct OffsetAndLength {
            /** @brief Zero-based start offset. */
            SharpRuntime::intcs Offset;
            /** @brief Number of elements covered. */
            SharpRuntime::intcs Length;
        };

        /**
         * @brief Calculates the start offset and length of the range relative to a collection of the given size.
         *
         * C++ counterpart of .NET Range.GetOffsetAndLength(int). Unlike Index.GetOffset, this
         * method does validate: it throws if the resolved end index exceeds @p length or if the
         * resolved start index exceeds the resolved end index (matching .NET's unsigned-compare
         * checks, which also reject negative offsets).
         * @param length Total number of elements in the collection.
         * @return Resolved OffsetAndLength value.
         * @throws System::ArgumentOutOfRangeException if the resolved range falls outside [0, length].
         */
        [[nodiscard]] OffsetAndLength GetOffsetAndLength(SharpRuntime::intcs length) const {
            SharpRuntime::intcs start = start_.GetOffset(length);
            SharpRuntime::intcs end   = end_.GetOffset(length);
            if (static_cast<unsigned>(end) > static_cast<unsigned>(length) ||
                static_cast<unsigned>(start) > static_cast<unsigned>(end)) {
                throw System::ArgumentOutOfRangeException("length");
            }
            return { start, end - start };
        }

        /**
         * @brief Returns true if this range equals the specified other range.
         *
         * C++ counterpart of .NET Range.Equals(Range).
         * @param other The range to compare.
         */
        [[nodiscard]] bool Equals(const Range& other) const noexcept {
            return indexEquals(start_, other.start_) && indexEquals(end_, other.end_);
        }

        /**
         * @brief Returns a hash code for this range.
         *
         * C++ counterpart of .NET Range.GetHashCode().
         */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const noexcept {
            SharpRuntime::intcs h1 = start_.getValueProperty() ^ (start_.getIsFromEndProperty() ? 0x10000 : 0);
            SharpRuntime::intcs h2 = end_.getValueProperty()   ^ (end_.getIsFromEndProperty()   ? 0x20000 : 0);
            return h1 ^ (h2 * 397);
        }

        /**
         * @brief Returns a string representation of this range in C# range expression syntax.
         *
         * C++ counterpart of .NET Range.ToString(). Examples: "0..^0", "1..5", "^2..^0".
         */
        [[nodiscard]] std::string ToString() const {
            return indexToString(start_) + ".." + indexToString(end_);
        }

        /** @brief Returns true if the two ranges are equal. */
        friend bool operator==(const Range& lhs, const Range& rhs) noexcept { return lhs.Equals(rhs); }

        /** @brief Returns true if the two ranges are not equal. */
        friend bool operator!=(const Range& lhs, const Range& rhs) noexcept { return !lhs.Equals(rhs); }
    };

} // namespace System
