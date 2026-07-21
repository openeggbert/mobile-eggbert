// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstddef>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a type that can be used to index a collection from the
     * start or from the end.
     *
     * C++ counterpart of .NET System.Index. Used to support C#'s "from end" index
     * syntax (e.g. <c>someArray[^1]</c>) when porting indexing expressions.
     */
    class Index {
        intcs value_ = 0;
        bool fromEnd_ = false;

    public:
        /** @brief Initializes an Index that refers to the first element of a collection. */
        Index() = default;

        /**
         * @brief Initializes an Index with the specified value, optionally from the end.
         *
         * Not marked explicit: mirrors .NET's implicit `operator Index(int)` conversion,
         * which always constructs a from-the-start index.
         * @param value   The index value (must be non-negative).
         * @param fromEnd If true, the index is measured from the end of the collection. If the
         *                Index is constructed from the end, index value 1 means pointing at the
         *                last element and index value 0 means pointing at beyond the last element.
         * @throws System::ArgumentOutOfRangeException if @p value is negative.
         */
        Index(intcs value, bool fromEnd = false) : value_(value), fromEnd_(fromEnd) {
            if (value < 0) throw System::ArgumentOutOfRangeException("value", "value must be non-negative");
        }

        /** @brief Returns true if the index is from the end of the collection. */
        [[nodiscard]] bool getIsFromEndProperty() const noexcept { return fromEnd_; }

        /** @brief Returns the index value. */
        [[nodiscard]] intcs getValueProperty() const noexcept { return value_; }

        /**
         * @brief Calculates the offset from the start using the given collection length.
         *
         * C++ counterpart of .NET Index.GetOffset(int). For performance reasons, matching
         * .NET, this does not validate @p length or the returned offset against negative
         * values or against @p length — callers that misuse the result will get an
         * out-of-range failure from whatever consumes the offset, same as .NET.
         * @param length The length of the collection that the Index will be used with.
         * @return The zero-based offset into the collection (unvalidated).
         */
        [[nodiscard]] intcs GetOffset(intcs length) const noexcept {
            return fromEnd_ ? length - value_ : value_;
        }

        /**
         * @brief Indicates whether the current Index is equal to another Index.
         *
         * C++ counterpart of .NET Index.Equals(Index).
         */
        [[nodiscard]] bool Equals(const Index& other) const noexcept {
            return value_ == other.value_ && fromEnd_ == other.fromEnd_;
        }

        /**
         * @brief Returns the hash code for this instance.
         *
         * C++ counterpart of .NET Index.GetHashCode().
         */
        [[nodiscard]] intcs GetHashCode() const noexcept {
            return fromEnd_ ? ~value_ : value_;
        }

        /**
         * @brief Converts the value of the current Index to its equivalent string representation.
         *
         * C++ counterpart of .NET Index.ToString(). Examples: "3", "^1".
         */
        [[nodiscard]] std::string ToString() const {
            return fromEnd_ ? "^" + std::to_string(value_) : std::to_string(value_);
        }

        /** @brief Returns true if the two Index values are equal. */
        friend bool operator==(const Index& lhs, const Index& rhs) noexcept { return lhs.Equals(rhs); }

        /** @brief Returns true if the two Index values are not equal. */
        friend bool operator!=(const Index& lhs, const Index& rhs) noexcept { return !lhs.Equals(rhs); }

        /** @brief Creates an Index from the start of a collection at the specified value. */
        static Index FromStart(intcs value) { return Index(value, false); }

        /** @brief Creates an Index from the end of a collection at the specified value. */
        static Index FromEnd(intcs value)   { return Index(value, true); }

        /** @brief Returns an Index that points to the first element (index 0). */
        static Index Start() { return Index(0, false); }

        /** @brief Returns an Index that points past the last element (from end, offset 0). */
        static Index End()   { return Index(0, true); }
    };

} // namespace System
