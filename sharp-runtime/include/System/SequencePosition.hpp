// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    /**
     * @brief Represents a position in a non-contiguous set of memory.
     *
     * C++ counterpart of .NET System.SequencePosition.
     * A position is defined by a segment object pointer and an integer offset within
     * that segment. For single-segment sequences, the object pointer is typically
     * null and the integer is an absolute byte offset.
     */
    struct SequencePosition {
        /** @brief The segment object (may be null for single-segment sequences). */
        void* object_ = nullptr;
        /** @brief The integer offset within the segment. */
        SharpRuntime::intcs integer_ = 0;

        /** @brief Default constructor — represents position zero with no segment. */
        SequencePosition() = default;

        /**
         * @brief Constructs a SequencePosition from a segment object and integer offset.
         * @param object  Pointer to the segment object (may be null).
         * @param integer Integer offset within the segment.
         */
        SequencePosition(void* object, SharpRuntime::intcs integer) noexcept
            : object_(object), integer_(integer) {}

        /** @brief Returns the segment object pointer. */
        [[nodiscard]] void* GetObject() const noexcept { return object_; }
        /** @brief Returns the integer offset. */
        [[nodiscard]] SharpRuntime::intcs GetInteger() const noexcept { return integer_; }

        /** @brief Returns true if both positions refer to the same location. */
        [[nodiscard]] bool operator==(const SequencePosition& o) const noexcept {
            return object_ == o.object_ && integer_ == o.integer_;
        }
        /** @brief Returns true if the positions differ. */
        [[nodiscard]] bool operator!=(const SequencePosition& o) const noexcept {
            return !(*this == o);
        }
    };

} // namespace System
