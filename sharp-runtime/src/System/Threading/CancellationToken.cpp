// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Threading/CancellationToken.hpp"
#include "System/OperationCanceledException.hpp"
#include "System/Threading/CancellationTokenRegistration.hpp"
namespace System::Threading {
    void CancellationToken::ThrowIfCancellationRequested() const {
        if (state_->cancelled.load())
            throw System::OperationCanceledException();
    }

    CancellationTokenRegistration CancellationToken::Register(std::function<void()> callback) {
        // The cancelled-check and callback-map mutation share state_->mutex with Cancel()'s
        // equivalent critical section, so there is no TOCTOU window where a registration could
        // be lost because Cancel() ran between an unlocked check and the map insert.
        bool alreadyCancelled = false;
        intcs id = -1;
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            if (state_->cancelled.load()) {
                alreadyCancelled = true;
            } else {
                id = state_->nextId++;
                state_->callbacks[id] = callback;
            }
        }
        if (alreadyCancelled) {
            if (callback) callback();
            return CancellationTokenRegistration();
        }
        return CancellationTokenRegistration(state_, id);
    }
} // namespace System::Threading
