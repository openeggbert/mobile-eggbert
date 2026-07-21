// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Defines a provider for progress updates.
     *
     * C++ counterpart of .NET System.IProgress<T>. Implement this interface
     * to receive notifications about the progress of an operation.
     *
     * @note .NET declares this interface contravariant (`IProgress<in T>`), so
     *   `IProgress<Base>` is usable wherever `IProgress<Derived>` is expected.
     *   C++ class templates have no equivalent variance mechanism, so
     *   `IProgress<Derived>` and `IProgress<Base>` are unrelated types here.
     * @tparam T The type of progress update value.
     */
    template<typename T>
    class IProgress {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IProgress() = default;

        /**
         * @brief Reports a progress update.
         *
         * C++ counterpart of .NET IProgress<T>.Report(T value).
         * @param value The value of the updated progress.
         */
        virtual void Report(const T& value) = 0;
    };

} // namespace System
