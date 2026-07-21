// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <functional>

#include "System/EventArgs.hpp"

namespace System::ComponentModel
{
    class CancelEventArgs : public System::EventArgs
    {
    public:
        bool Cancel = false;

        CancelEventArgs() = default;
        explicit CancelEventArgs(bool cancel) : Cancel(cancel) {}
    };

    /** @brief Represents the method that will handle the event raised when canceling an event. */
    using CancelEventHandler = std::function<void(void*, CancelEventArgs&)>;
}
