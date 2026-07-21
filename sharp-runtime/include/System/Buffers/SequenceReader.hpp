// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Buffers/ReadOnlySequence.hpp"

namespace System::Buffers {

    using SharpRuntime::longcs;

    /**
     * @brief Provides forward-only, low-allocation reading of a ReadOnlySequence&lt;T&gt;.
     *
     * C++ counterpart of .NET System.Buffers.SequenceReader&lt;T&gt;.
     * Tracks a current read position within the sequence. Callers advance through
     * elements using TryRead(), IsNext(), Advance(), and related helpers.
     *
     * @tparam T The type of items in the underlying sequence.
     */
    template<typename T>
    class SequenceReader {
        const ReadOnlySequence<T>& sequence_;
        System::ReadOnlyMemory<T>  segment_;
        int consumed_ = 0;   // absolute offset from sequence start

        [[nodiscard]] int total() const noexcept {
            return static_cast<int>(sequence_.getLengthProperty());
        }

    public:
        /**
         * @brief Constructs a SequenceReader over the given sequence.
         * @param sequence The sequence to read from. Must outlive this reader.
         */
        explicit SequenceReader(const ReadOnlySequence<T>& sequence)
            : sequence_(sequence), segment_(sequence.First()), consumed_(0) {}

        /**
         * @brief Returns the number of elements consumed so far.
         * @return Consumed element count.
         */
        [[nodiscard]] longcs getConsumedProperty() const noexcept {
            return static_cast<longcs>(consumed_);
        }

        /**
         * @brief Returns the number of elements remaining to be read.
         * @return Remaining element count.
         */
        [[nodiscard]] longcs getRemainingProperty() const noexcept {
            return static_cast<longcs>(total() - consumed_);
        }

        /**
         * @brief Returns true if all elements have been consumed.
         * @return true if at end of sequence.
         */
        [[nodiscard]] bool getEndProperty() const noexcept {
            return consumed_ >= total();
        }

        /**
         * @brief Returns the current SequencePosition.
         * @return Current position within the underlying sequence.
         */
        [[nodiscard]] System::SequencePosition getPositionProperty() const noexcept {
            return sequence_.GetPosition(static_cast<longcs>(consumed_));
        }

        /**
         * @brief Tries to read the next element from the sequence.
         * @param value Receives the next element if successful.
         * @return true if an element was read; false if at end of sequence.
         */
        bool TryRead(T& value) {
            if (getEndProperty()) return false;
            value = segment_[consumed_];
            ++consumed_;
            return true;
        }

        /**
         * @brief Returns true if the next element equals @p next without advancing.
         * @param next The value to compare against the next element.
         * @return true if the next element equals @p next.
         */
        [[nodiscard]] bool IsNext(const T& next) const {
            if (getEndProperty()) return false;
            return segment_[consumed_] == next;
        }

        /**
         * @brief Advances the reader by @p count elements.
         * @param count Number of elements to skip.
         * @throws System::ArgumentOutOfRangeException if count exceeds remaining elements.
         */
        void Advance(longcs count) {
            if (count < 0 || count > getRemainingProperty())
                throw System::ArgumentOutOfRangeException("count");
            consumed_ += static_cast<int>(count);
        }

        /**
         * @brief Tries to advance past @p next if it is the next element.
         * @param next The element to match and skip.
         * @return true if the element was matched and skipped; false otherwise.
         */
        bool TryAdvancePast(const T& next) {
            if (!IsNext(next)) return false;
            ++consumed_;
            return true;
        }

        /**
         * @brief Moves the reader back the specified number of elements.
         *
         * C++ counterpart of .NET SequenceReader&lt;T&gt;.Rewind(long). Note this takes a
         * relative count, not an absolute position -- to reset to the very start, pass
         * getConsumedProperty().
         * @param count Number of elements to move back.
         * @throws System::ArgumentOutOfRangeException if count is negative or exceeds
         *         getConsumedProperty().
         */
        void Rewind(longcs count) {
            if (count < 0 || count > getConsumedProperty())
                throw System::ArgumentOutOfRangeException("count");
            consumed_ -= static_cast<int>(count);
        }

        // -----------------------------------------------------------------------
        // Additional API from SequenceReader.cs / SequenceReader.Search.cs
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the total length of the underlying sequence.
         * @return Total number of elements in the sequence.
         */
        [[nodiscard]] longcs getLengthProperty() const noexcept {
            return static_cast<longcs>(total());
        }

        /**
         * @brief Returns a reference to the underlying ReadOnlySequence.
         * C++ counterpart of .NET SequenceReader&lt;T&gt;.Sequence.
         */
        [[nodiscard]] const ReadOnlySequence<T>& getSequenceProperty() const noexcept {
            return sequence_;
        }

        /**
         * @brief Peeks at the next element without consuming it.
         * @param value Receives the next element if successful.
         * @return true if an element is available; false if at end.
         */
        [[nodiscard]] bool TryPeek(T& value) const {
            if (getEndProperty()) return false;
            value = segment_[consumed_];
            return true;
        }

        /**
         * @brief Tries to read all elements up to (but not including) @p delimiter.
         *
         * C++ counterpart of .NET SequenceReader&lt;T&gt;.TryReadTo(out ReadOnlySpan, T, bool).
         * Reads from the current position until @p delimiter is found. On success,
         * @p result contains the elements read; the reader is positioned just after
         * the delimiter (if @p advancePastDelimiter is true).
         *
         * @param result                Vector that receives the consumed elements.
         * @param delimiter             The delimiter to search for.
         * @param advancePastDelimiter  If true, also consume the delimiter itself.
         * @return true if @p delimiter was found; false if it was not found.
         */
        bool TryReadTo(std::vector<T>& result, const T& delimiter,
                       bool advancePastDelimiter = true) {
            result.clear();
            int start = consumed_;
            while (consumed_ < total()) {
                T val = segment_[consumed_];
                if (val == delimiter) {
                    if (advancePastDelimiter) ++consumed_;
                    return true;
                }
                result.push_back(val);
                ++consumed_;
            }
            // Delimiter not found — restore position.
            consumed_ = start;
            result.clear();
            return false;
        }

        /**
         * @brief Advances past consecutive elements equal to @p value.
         *
         * C++ counterpart of .NET SequenceReader&lt;T&gt;.AdvancePast(T).
         * @return Number of elements skipped.
         */
        longcs AdvancePast(const T& value) {
            longcs count = 0;
            while (consumed_ < total() && segment_[consumed_] == value) {
                ++consumed_;
                ++count;
            }
            return count;
        }

        /**
         * @brief Returns true if the next N elements equal those in @p next.
         *
         * C++ counterpart of .NET SequenceReader&lt;T&gt;.IsNext(ReadOnlySpan, bool).
         * @param next               Span of values to compare.
         * @param advancePast        If true and match succeeds, advance past the match.
         * @return true if the upcoming bytes match @p next.
         */
        [[nodiscard]] bool IsNext(const std::vector<T>& next, bool advancePast = false) {
            int n = static_cast<int>(next.size());
            if (total() - consumed_ < n) return false;
            for (int i = 0; i < n; ++i) {
                if (segment_[consumed_ + i] != next[i]) return false;
            }
            if (advancePast) consumed_ += n;
            return true;
        }
    };

} // namespace System::Buffers
