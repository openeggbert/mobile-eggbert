// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::intcs;

// Forward declaration to avoid circular include with ModuleHandle.hpp
struct ModuleHandle;

/**
 * @brief Represents a type using an internal metadata token.
 *
 * C++ counterpart of .NET System.RuntimeTypeHandle. In C++ there is no
 * CLR type system, so this is a thin wrapper around an intptr_t value;
 * all reflection-dependent methods are stubs.
 */
struct RuntimeTypeHandle {
    std::intptr_t value_ = 0;

    /** @brief Initializes a default (zero) handle. */
    RuntimeTypeHandle() = default;

    /** @brief Initializes a handle with the given pointer-sized value. */
    explicit RuntimeTypeHandle(std::intptr_t value) : value_(value) {}

    /** @brief Creates a RuntimeTypeHandle from an IntPtr value. */
    [[nodiscard]] static RuntimeTypeHandle FromIntPtr(std::intptr_t value) {
        return RuntimeTypeHandle{value};
    }

    /** @brief Converts a RuntimeTypeHandle to its underlying IntPtr value. */
    [[nodiscard]] static std::intptr_t ToIntPtr(RuntimeTypeHandle handle) {
        return handle.value_;
    }

    /** @brief Gets the pointer-sized value of this handle. */
    [[nodiscard]] std::intptr_t getValueProperty() const noexcept { return value_; }

    /**
     * @brief Gets the module in which the type represented by this handle is defined.
     *
     * Returns ModuleHandle::EmptyHandle (stub — no CLR metadata available).
     */
    [[nodiscard]] ModuleHandle GetModuleHandle() const;

    /** @brief Indicates whether this handle equals another. */
    [[nodiscard]] bool Equals(const RuntimeTypeHandle& other) const noexcept {
        return value_ == other.value_;
    }

    /** @brief Returns a hash code derived from the underlying value. */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        return static_cast<intcs>(value_);
    }

    bool operator==(const RuntimeTypeHandle& o) const noexcept { return Equals(o); }
    bool operator!=(const RuntimeTypeHandle& o) const noexcept { return !Equals(o); }
};

} // namespace System

// Deferred: include ModuleHandle so that GetModuleHandle() can be defined.
#include "System/ModuleHandle.hpp"

namespace System {
    inline ModuleHandle RuntimeTypeHandle::GetModuleHandle() const {
        return ModuleHandle::EmptyHandle;
    }
} // namespace System
