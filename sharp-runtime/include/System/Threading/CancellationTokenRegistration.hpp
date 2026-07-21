// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <mutex>
#include <thread>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IDisposable.hpp"
#include "System/Threading/CancellationToken.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /** Represents a callback registration with a CancellationToken that can be deregistered. */
    class CancellationTokenRegistration : public System::IDisposable {
        std::shared_ptr<Detail::CancellationState> state_;
        intcs id_ = -1;

    public:
        /** Constructs an inactive CancellationTokenRegistration (nothing to unregister). */
        CancellationTokenRegistration() = default;
        /** Constructs a registration tracking the callback identified by @p id within @p state. */
        CancellationTokenRegistration(std::shared_ptr<Detail::CancellationState> state, intcs id)
            : state_(std::move(state)), id_(id) {}

        /**
         * @brief Deregisters the callback associated with this registration.
         *
         * If the target callback is currently executing (i.e. Cancel() removed it from the
         * pending map and is in the middle of invoking it on another thread), this method waits
         * until it completes before returning -- except when called from within the callback
         * itself (self-unregister), which would otherwise deadlock. Verified against
         * CancellationTokenRegistration.cs's Dispose()/WaitForCallbackIfNecessary: without this
         * wait, a caller that disposes a registration and then immediately tears down a resource
         * the callback references would race a still-in-flight callback invocation.
         */
        void Dispose() override {
            if (!state_) return;
            std::unique_lock<std::mutex> lock(state_->mutex);
            bool erased = state_->callbacks.erase(id_) != 0;
            if (!erased && state_->executingId == id_ && state_->executingThreadId != std::this_thread::get_id()) {
                state_->callbackFinished.wait(lock, [this] { return state_->executingId != id_; });
            }
            lock.unlock();
            state_.reset();
        }

        /** Attempts to deregister the associated callback; equivalent to Dispose. */
        void Unregister() { Dispose(); }

        /** Returns true if the registration has not been disposed and its callback is still pending. */
        [[nodiscard]] bool getIsActiveProperty() const {
            if (!state_) return false;
            std::lock_guard<std::mutex> lock(state_->mutex);
            return state_->callbacks.count(id_) != 0;
        }
    };

} // namespace System::Threading
