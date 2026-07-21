// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief An opaque token capturing the lock state to be restored by ReaderWriterLock.
     *
     * C++ counterpart of .NET System.Threading.LockCookie. In .NET this is an opaque struct
     * only ever produced/consumed by ReaderWriterLock; the fields here (reader/writer recursion
     * levels captured at ReleaseLock/UpgradeToWriterLock time) are an implementation detail of
     * this port's ReaderWriterLock, not part of the public contract.
     */
    struct LockCookie {
        /** @brief Constructs a default (empty) cookie. */
        LockCookie() = default;

        /** @brief Returns true if @p other captures the same lock state. */
        [[nodiscard]] bool Equals(const LockCookie& other) const {
            return readerLevel_ == other.readerLevel_ && writerLevel_ == other.writerLevel_;
        }
        /** @brief Equality operator; see Equals. */
        bool operator==(const LockCookie& other) const { return Equals(other); }
        /** @brief Inequality operator; see Equals. */
        bool operator!=(const LockCookie& other) const { return !Equals(other); }
        /** @brief Returns a hash code derived from the captured recursion levels. */
        [[nodiscard]] intcs GetHashCode() const {
            return static_cast<intcs>(readerLevel_ * 31 + writerLevel_);
        }

        // Internal to this port's ReaderWriterLock implementation.
        intcs readerLevel_ = 0;
        intcs writerLevel_ = 0;
    };

} // namespace System::Threading
