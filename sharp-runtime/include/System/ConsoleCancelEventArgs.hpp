// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/EventArgs.hpp"
#include "System/ConsoleSpecialKey.hpp"

namespace System {

    /**
     * @brief Provides data for the Console::CancelKeyPress event.
     *
     * C++ counterpart of .NET System.ConsoleCancelEventArgs.
     * When a CTRL+C or CTRL+BREAK signal is received, the handler may set
     * getCancel to true to prevent the process from being terminated.
     */
    class ConsoleCancelEventArgs : public EventArgs {
        ConsoleSpecialKey type_;
        bool              cancel_ = false;

    public:
        /**
         * @brief Constructs ConsoleCancelEventArgs for the specified special key.
         * @param type The ConsoleSpecialKey value that caused the event.
         */
        explicit ConsoleCancelEventArgs(ConsoleSpecialKey type)
            : type_(type) {}

        /**
         * @brief Gets or indicates whether the current process should be terminated.
         * Set to true to suppress the default termination behavior.
         * @return true if cancellation is requested.
         */
        [[nodiscard]] bool getCancelProperty() const noexcept { return cancel_; }
        /** @brief Sets whether the process should cancel the break signal and continue running. */
        void setCancelProperty(bool v) noexcept { cancel_ = v; }

        /**
         * @brief Gets the ConsoleSpecialKey combination that interrupted the current process.
         * @return The special key that caused this event.
         */
        [[nodiscard]] ConsoleSpecialKey getSpecialKeyProperty() const noexcept { return type_; }
    };

    /**
     * @brief Represents the method that handles the Console::CancelKeyPress event.
     *
     * C++ counterpart of .NET System.ConsoleCancelEventHandler delegate.
     * @param sender The source of the event (typically nullptr).
     * @param e      A ConsoleCancelEventArgs containing the special key information.
     */
    using ConsoleCancelEventHandler = std::function<void(void* sender, ConsoleCancelEventArgs& e)>;

} // namespace System
