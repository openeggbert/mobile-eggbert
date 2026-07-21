// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/InvalidOperationException.hpp"

namespace System {

    /**
     * @brief Supports iterating over a string and reading its individual characters.
     *
     * C++ counterpart of .NET System.CharEnumerator (sealed).
     * Obtained by calling String.GetEnumerator() or String.ToCharArray() iteration.
     * Implements a forward-only cursor over a std::string.
     *
     * The enumerator starts before the first element (index -1).
     * Call MoveNext() to advance; Current throws if the cursor is out of range.
     */
    class CharEnumerator final {
        std::string str_;
        int index_ = -1;

    public:
        /**
         * @brief Constructs a CharEnumerator over the given string.
         *
         * C++ counterpart of the internal .NET CharEnumerator(string str) constructor.
         * @param str The string to enumerate.
         */
        explicit CharEnumerator(std::string str) : str_(std::move(str)) {}

        /**
         * @brief Advances the enumerator to the next character.
         *
         * C++ counterpart of .NET CharEnumerator.MoveNext().
         * @return true if the enumerator was successfully advanced; false if the
         *         enumerator has passed the end of the string.
         */
        bool MoveNext() noexcept {
            int next = index_ + 1;
            if (next < static_cast<int>(str_.size())) {
                index_ = next;
                return true;
            }
            index_ = static_cast<int>(str_.size());
            return false;
        }

        /**
         * @brief Gets the character at the current position of the enumerator.
         *
         * C++ counterpart of .NET CharEnumerator.Current.
         * @throws System::InvalidOperationException if the enumerator is positioned before the
         *         first element or after the last element.
         */
        [[nodiscard]] char getCurrentProperty() const {
            if (index_ < 0 || index_ >= static_cast<int>(str_.size()))
                throw InvalidOperationException(
                    "Enumeration has either not started or has already finished.");
            return str_[static_cast<std::string::size_type>(index_)];
        }

        /**
         * @brief Resets the enumerator to its initial position, before the first element.
         *
         * C++ counterpart of .NET CharEnumerator.Reset().
         */
        void Reset() noexcept { index_ = -1; }

        /**
         * @brief Releases the string reference held by this enumerator.
         *
         * C++ counterpart of .NET CharEnumerator.Dispose().
         */
        void Dispose() noexcept { str_.clear(); index_ = -1; }

        /**
         * @brief Creates a copy of the enumerator at its current position.
         *
         * C++ counterpart of .NET CharEnumerator.Clone().
         */
        [[nodiscard]] CharEnumerator Clone() const { return *this; }
    };

} // namespace System
