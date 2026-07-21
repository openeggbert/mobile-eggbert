// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Runtime/InteropServices/PosixSignal.hpp"

namespace System::Runtime::InteropServices {

    /**
     * @brief Provides data for a PosixSignalRegistration event.
     *
     * C++ counterpart of .NET System.Runtime.InteropServices.PosixSignalContext.
     */
    class PosixSignalContext {
        PosixSignal signal_;
        bool cancel_ = false;

    public:
        /** @brief Initializes a new instance with the specified signal. */
        explicit PosixSignalContext(PosixSignal signal) : signal_(signal) {}

        /** @brief Gets the signal that occurred. */
        [[nodiscard]] PosixSignal getSignalProperty() const { return signal_; }

        /**
         * @brief Sets the signal that occurred.
         * Not part of the public .NET surface (real .NET's Signal setter is `internal`) -- exposed
         * here only so PosixSignalRegistration's dispatch loop can stamp the actual signal value
         * being delivered onto a shared context object before invoking each handler, matching real
         * .NET's own internal-setter mechanism for the same purpose.
         */
        void setSignalProperty(PosixSignal signal) { signal_ = signal; }

        /** @brief Gets whether to cancel the default handling of the signal. Default is false. */
        [[nodiscard]] bool getCancelProperty() const { return cancel_; }

        /** @brief Sets whether to cancel the default handling of the signal. */
        void setCancelProperty(bool value) { cancel_ = value; }
    };

} // namespace System::Runtime::InteropServices
