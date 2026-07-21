// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>

namespace System {

/**
 * @brief Provides a way to access a method in a lightweight, unmanaged representation.
 *
 * C++ counterpart of .NET System.RuntimeMethodHandle. In C++ there is no CLR
 * method metadata, so this wraps a zero intptr_t and all methods are stubs.
 */
struct RuntimeMethodHandle {
    std::intptr_t value_ = 0;

    /** @brief Initializes a default (zero) handle. */
    RuntimeMethodHandle() = default;

    /** @brief Initializes a handle with the given pointer-sized value. */
    explicit RuntimeMethodHandle(std::intptr_t value) : value_(value) {}

    /** @brief Creates a RuntimeMethodHandle from an IntPtr value. */
    [[nodiscard]] static RuntimeMethodHandle FromIntPtr(std::intptr_t value) {
        return RuntimeMethodHandle{value};
    }

    /** @brief Converts a RuntimeMethodHandle to its underlying IntPtr value. */
    [[nodiscard]] static std::intptr_t ToIntPtr(RuntimeMethodHandle handle) {
        return handle.value_;
    }

    /**
     * @brief Gets a function pointer to the method represented by this handle.
     *
     * Always returns 0 in this stub implementation.
     */
    [[nodiscard]] std::intptr_t GetFunctionPointer() const noexcept { return 0; }

    /** @brief Indicates whether this handle equals another. */
    [[nodiscard]] bool Equals(const RuntimeMethodHandle& other) const noexcept {
        return value_ == other.value_;
    }

    /** @brief Returns a hash code derived from the underlying value. */
    [[nodiscard]] int GetHashCode() const noexcept {
        return static_cast<int>(value_);
    }

    bool operator==(const RuntimeMethodHandle& o) const noexcept { return Equals(o); }
    bool operator!=(const RuntimeMethodHandle& o) const noexcept { return !Equals(o); }
};

} // namespace System
