// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/Span.hpp"
#include "System/Range.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    /**
     * @brief Enables enumerating each split within a ReadOnlySpan<T> divided by one or more separators.
     *
     * C++ counterpart of .NET System.MemoryExtensions.SpanSplitEnumerator<T>.
     * Supports range-based for loops; Current returns a Range into the source span.
     * Two split modes: single-element separator and any-of (span of separators).
     *
     * @tparam T The element type; must support operator==.
     */
    template<typename T>
    class SpanSplitEnumerator {
    public:
        /** @brief The separator-matching strategy: a single element, any of a set, or an exact subsequence. */
        enum class Mode { SingleElement, AnyOf, Sequence };

    private:
        ReadOnlySpan<T> source_;
        T               singleSep_{};
        std::vector<T>  separators_;    // used for AnyOf
        std::vector<T>  sequence_;      // used for Sequence
        Mode            mode_;

        SharpRuntime::intcs startCurrent_ = 0;
        SharpRuntime::intcs endCurrent_   = 0;
        SharpRuntime::intcs startNext_    = 0;
        bool                done_         = false;

        // ----- helpers -----
        SharpRuntime::intcs findSingle(SharpRuntime::intcs from) const {
            for (SharpRuntime::intcs i = from; i < source_.getLengthProperty(); ++i)
                if (source_[i] == singleSep_) return i - from;
            return -1;
        }

        SharpRuntime::intcs findAny(SharpRuntime::intcs from) const {
            for (SharpRuntime::intcs i = from; i < source_.getLengthProperty(); ++i)
                for (const T& sep : separators_)
                    if (source_[i] == sep) return i - from;
            return -1;
        }

        SharpRuntime::intcs findSequence(SharpRuntime::intcs from) const {
            SharpRuntime::intcs slen = static_cast<SharpRuntime::intcs>(sequence_.size());
            SharpRuntime::intcs srclen = source_.getLengthProperty();
            for (SharpRuntime::intcs i = from; i <= srclen - slen; ++i) {
                bool match = true;
                for (SharpRuntime::intcs j = 0; j < slen && match; ++j)
                    match = (source_[i + j] == sequence_[j]);
                if (match) return i - from;
            }
            return -1;
        }

    public:
        /** @brief Constructs an enumerator that splits on a single separator element. */
        SpanSplitEnumerator(ReadOnlySpan<T> source, const T& separator)
            : source_(source), singleSep_(separator), mode_(Mode::SingleElement) {}

        /** @brief Constructs an enumerator that splits on any element from separators. */
        SpanSplitEnumerator(ReadOnlySpan<T> source, const std::vector<T>& separators,
                            bool treatAsAny = true)
            : source_(source), mode_(treatAsAny ? Mode::AnyOf : Mode::Sequence) {
            if (treatAsAny) separators_ = separators;
            else            sequence_   = separators;
        }

        /** @brief Returns the current Range (indices into the source span). */
        [[nodiscard]] Range getCurrentProperty() const {
            return Range(Index(startCurrent_), Index(endCurrent_));
        }

        /** @brief Returns the slice of the source span for the current range. */
        [[nodiscard]] ReadOnlySpan<T> getCurrentSpan() const {
            return source_.Slice(startCurrent_, endCurrent_ - startCurrent_);
        }

        /**
         * @brief Advances to the next segment.
         * @return true if a segment is available; false if enumeration is finished.
         */
        bool MoveNext() {
            if (done_) return false;

            SharpRuntime::intcs sepIdx = -1, sepLen = 1;
            if      (mode_ == Mode::SingleElement) sepIdx = findSingle(startNext_);
            else if (mode_ == Mode::AnyOf)         sepIdx = findAny(startNext_);
            else {
                sepIdx = findSequence(startNext_);
                sepLen = static_cast<SharpRuntime::intcs>(sequence_.size());
            }

            startCurrent_ = startNext_;
            if (sepIdx >= 0) {
                endCurrent_ = startCurrent_ + sepIdx;
                startNext_  = endCurrent_ + sepLen;
            } else {
                endCurrent_ = startNext_ = source_.getLengthProperty();
                done_ = true;
            }
            return true;
        }

        // ---- range-based for support ----

        struct Iterator {
            SpanSplitEnumerator* e;
            bool                 valid;

            bool operator!=(const Iterator& other) const { return valid != other.valid; }
            Iterator& operator++() { valid = e->MoveNext(); return *this; }
            ReadOnlySpan<T> operator*() const { return e->getCurrentSpan(); }
        };

        /** @brief Allows use in a range-based for loop. */
        Iterator begin() { bool ok = MoveNext(); return {this, ok}; }
        /** @brief Sentinel end iterator. */
        Iterator end()   { return {this, false}; }

        /** @brief Returns an enumerator (self) for explicit GetEnumerator() pattern. */
        SpanSplitEnumerator& GetEnumerator() { return *this; }
    };

} // namespace System
