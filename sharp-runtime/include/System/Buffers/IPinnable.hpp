// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Buffers/MemoryHandle.hpp"

namespace System::Buffers {

    using SharpRuntime::intcs;

/**
 * @brief Provides a mechanism for pinning and unpinning objects.
 *
 * C++ counterpart of .NET System.Buffers.IPinnable. Since C++ does not have a
 * moving GC, pinning is a no-op in practice; this interface exists so that
 * ported code referencing IPinnable compiles without modification.
 */
struct IPinnable {
    virtual ~IPinnable() = default;

    /**
     * @brief Pins the object at the given element index and returns a handle.
     *
     * C++ counterpart of .NET IPinnable.Pin(int). The returned MemoryHandle
     * points to the pinned memory. Call Dispose() on the handle when done.
     * @param elementIndex Offset to the element at which the handle points.
     * @return A MemoryHandle pointing to the pinned memory.
     */
    virtual MemoryHandle Pin(intcs elementIndex) = 0;

    /**
     * @brief Indicates that the object no longer needs to be pinned.
     *
     * C++ counterpart of .NET IPinnable.Unpin(). After this call the GC is
     * free to move the object (no-op in C++).
     */
    virtual void Unpin() = 0;
};

} // namespace System::Buffers

// Deferred definition of MemoryHandle::Dispose (needs IPinnable to be complete).
namespace System::Buffers {
    inline void MemoryHandle::Dispose() {
        if (pinnable_) {
            pinnable_->Unpin();
            pinnable_ = nullptr;
        }
        pointer_ = nullptr;
    }
} // namespace System::Buffers
