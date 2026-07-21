// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>

namespace System {

/**
 * @brief Provides a way to access a field in a lightweight, unmanaged representation.
 *
 * C++ counterpart of .NET System.RuntimeFieldHandle. In C++ there is no CLR
 * field metadata, so this wraps a zero intptr_t and all methods are stubs.
 */
struct RuntimeFieldHandle {
    std::intptr_t value_ = 0;

    /** @brief Initializes a default (zero) handle. */
    RuntimeFieldHandle() = default;

    /** @brief Initializes a handle with the given pointer-sized value. */
    explicit RuntimeFieldHandle(std::intptr_t value) : value_(value) {}

    /** @brief Creates a RuntimeFieldHandle from an IntPtr value. */
    [[nodiscard]] static RuntimeFieldHandle FromIntPtr(std::intptr_t value) {
        return RuntimeFieldHandle{value};
    }

    /** @brief Converts a RuntimeFieldHandle to its underlying IntPtr value. */
    [[nodiscard]] static std::intptr_t ToIntPtr(RuntimeFieldHandle handle) {
        return handle.value_;
    }

    /** @brief Indicates whether this handle equals another. */
    [[nodiscard]] bool Equals(const RuntimeFieldHandle& other) const noexcept {
        return value_ == other.value_;
    }

    /** @brief Returns a hash code derived from the underlying value. */
    [[nodiscard]] int GetHashCode() const noexcept {
        return static_cast<int>(value_);
    }

    bool operator==(const RuntimeFieldHandle& o) const noexcept { return Equals(o); }
    bool operator!=(const RuntimeFieldHandle& o) const noexcept { return !Equals(o); }
};

} // namespace System
