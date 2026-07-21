// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/ReadOnlyMemory.hpp"

namespace System::Buffers {

/**
 * @brief Represents a node in a linked list of ReadOnlyMemory&lt;T&gt; segments.
 *
 * C++ counterpart of .NET System.Buffers.ReadOnlySequenceSegment&lt;T&gt;.
 * Subclasses fill in the protected fields via the set-property helpers.
 * A ReadOnlySequence&lt;T&gt; can be constructed from a chain of these segments to
 * represent non-contiguous memory.
 *
 * @tparam T The element type.
 */
template<typename T>
class ReadOnlySequenceSegment {
protected:
    System::ReadOnlyMemory<T>     memory_;
    ReadOnlySequenceSegment<T>*   next_         = nullptr;
    long long                     runningIndex_ = 0;

public:
    virtual ~ReadOnlySequenceSegment() = default;

    /**
     * @brief Gets the memory for the current node.
     * C++ counterpart of .NET ReadOnlySequenceSegment&lt;T&gt;.Memory.
     */
    [[nodiscard]] const System::ReadOnlyMemory<T>& getMemoryProperty() const noexcept {
        return memory_;
    }

    /**
     * @brief Gets the next node in the linked list, or nullptr if this is the last.
     * C++ counterpart of .NET ReadOnlySequenceSegment&lt;T&gt;.Next.
     */
    [[nodiscard]] ReadOnlySequenceSegment<T>* getNextProperty() const noexcept {
        return next_;
    }

    /**
     * @brief Gets the sum of node lengths before this node (its start offset in the sequence).
     * C++ counterpart of .NET ReadOnlySequenceSegment&lt;T&gt;.RunningIndex.
     */
    [[nodiscard]] long long getRunningIndexProperty() const noexcept {
        return runningIndex_;
    }

protected:
    /** @brief Sets the memory for this segment. */
    void setMemoryProperty(System::ReadOnlyMemory<T> memory) { memory_ = std::move(memory); }
    /** @brief Sets the next segment. */
    void setNextProperty(ReadOnlySequenceSegment<T>* next) { next_ = next; }
    /** @brief Sets the running index. */
    void setRunningIndexProperty(long long index) { runningIndex_ = index; }
};

} // namespace System::Buffers
