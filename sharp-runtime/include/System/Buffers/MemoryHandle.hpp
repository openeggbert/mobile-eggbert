// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/IDisposable.hpp"

namespace System::Buffers {

// Forward declaration to break circular dependency with IPinnable.
struct IPinnable;

/**
 * @brief A handle for pinned memory.
 *
 * C++ counterpart of .NET System.Buffers.MemoryHandle.
 * Wraps a raw pointer to pinned memory and optionally holds a reference to an
 * IPinnable that must be unpinned on disposal. Because C++ RAII is deterministic,
 * callers should call Dispose() explicitly (or let the destructor do it).
 */
struct MemoryHandle : System::IDisposable {
    void*      pointer_  = nullptr;
    IPinnable* pinnable_ = nullptr;

    /** @brief Constructs a default (null) MemoryHandle. */
    MemoryHandle() = default;

    /**
     * @brief Constructs a MemoryHandle from a raw pointer and optional IPinnable.
     * @param pointer  Pointer to the pinned memory.
     * @param pinnable The IPinnable that pinned the memory, or nullptr.
     */
    explicit MemoryHandle(void* pointer, IPinnable* pinnable = nullptr)
        : pointer_(pointer), pinnable_(pinnable) {}

    /**
     * @brief Gets the pointer to the pinned memory.
     * @return Raw pointer to pinned memory, or nullptr.
     */
    [[nodiscard]] void* getPointerProperty() const noexcept { return pointer_; }

    /**
     * @brief Frees the pinned handle and releases IPinnable.
     *
     * Defined after IPinnable is complete (see IPinnable.hpp).
     */
    void Dispose() override;
};

} // namespace System::Buffers
