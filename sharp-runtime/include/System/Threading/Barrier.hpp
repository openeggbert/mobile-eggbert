// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Threading/BarrierPostPhaseException.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * @brief Enables multiple tasks to cooperatively work on an algorithm in parallel through
     * multiple phases.
     *
     * @note Two behaviors verified against Barrier.cs's FinishPhase()/SignalAndWait():
     * (1) real .NET rejects (InvalidOperationException) a call to SignalAndWait/
     * AddParticipant(s)/RemoveParticipant(s) made from within the post-phase action on the same
     * thread, rather than allowing it to reenter; (2) every participant thread -- not just the
     * one that happened to trigger the phase transition -- observes and rethrows a
     * BarrierPostPhaseException if the post-phase action faulted. This port previously had
     * neither: FinishPhase() ran the action while still holding `mutex_` with no reentrancy
     * check, so a reentrant call from within the action self-deadlocked on the non-recursive
     * std::mutex instead of throwing; and only the triggering thread's FinishPhase() call ever
     * saw the caught exception, so every other participant silently resumed as if the phase had
     * completed successfully.
     */
    class Barrier {
        intcs participantCount_;
        intcs remainingCount_;
        longcs phaseCount_ = 0;
        std::function<void(Barrier&)> postPhaseAction_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        std::exception_ptr lastPostPhaseException_;
        std::atomic<std::thread::id> actionCallerId_;
        bool disposed_ = false;

        void ThrowIfCalledFromPostPhaseAction() const {
            if (actionCallerId_.load() == std::this_thread::get_id())
                throw System::InvalidOperationException(
                    "This method may not be called from within the postPhaseAction.");
        }

        // Verified against Barrier.cs: SignalAndWait/AddParticipants/RemoveParticipants all
        // call ObjectDisposedException.ThrowIf(_disposed, this) as their first check. This port
        // previously had no disposed_ flag at all -- Dispose() was a true no-op and every method
        // remained fully usable after disposal.
        void ThrowIfDisposed() const {
            if (disposed_) throw System::ObjectDisposedException("Barrier");
        }

    public:
        /** Constructs a Barrier with the specified number of participants and an optional post-phase action. */
        explicit Barrier(intcs participantCount, std::function<void(Barrier&)> postPhaseAction = nullptr)
            : participantCount_(participantCount), remainingCount_(participantCount),
              postPhaseAction_(std::move(postPhaseAction)) {
            if (participantCount < 0 || participantCount > 32767)
                throw System::ArgumentOutOfRangeException("participantCount");
        }

        /** Returns the total number of participants. */
        [[nodiscard]] intcs  getParticipantCountProperty() const { return participantCount_; }
        /** Returns the current phase number. */
        [[nodiscard]] longcs getCurrentPhaseNumberProperty() const { std::unique_lock lock(mutex_); return phaseCount_; }

        /**
         * @brief Signals that a participant has reached the barrier and blocks until all participants have arrived.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         * @throws System::InvalidOperationException if called from within the post-phase action.
         * @throws BarrierPostPhaseException if the post-phase action threw during this phase.
         */
        void SignalAndWait() {
            ThrowIfDisposed();
            ThrowIfCalledFromPostPhaseAction();
            std::unique_lock lock(mutex_);
            if (participantCount_ == 0)
                throw System::InvalidOperationException("The barrier has no registered participants.");
            --remainingCount_;
            if (remainingCount_ == 0) {
                FinishPhase(lock); // throws directly if the post-phase action faulted
            } else {
                longcs myPhase = phaseCount_;
                cv_.wait(lock, [this, myPhase]{ return phaseCount_ > myPhase; });
                if (lastPostPhaseException_)
                    throw BarrierPostPhaseException(lastPostPhaseException_);
            }
        }

        /**
         * @brief Notifies the barrier that there will be one additional participant; returns new participant count.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         * @throws System::InvalidOperationException if called from within the post-phase action.
         */
        intcs AddParticipant() {
            ThrowIfDisposed();
            ThrowIfCalledFromPostPhaseAction();
            std::unique_lock lock(mutex_);
            if (participantCount_ >= 32767)
                throw System::InvalidOperationException("Adding the specified number of participants would cause the Barrier's participants count to exceed the maximum allowed.");
            ++participantCount_;
            ++remainingCount_;
            return participantCount_;
        }

        /**
         * @brief Notifies the barrier that there will be one fewer participant.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         * @throws System::InvalidOperationException if called from within the post-phase action.
         */
        void RemoveParticipant() {
            ThrowIfDisposed();
            ThrowIfCalledFromPostPhaseAction();
            std::unique_lock lock(mutex_);
            if (participantCount_ == 0)
                throw System::ArgumentOutOfRangeException("participantCount");
            if (remainingCount_ == 0)
                throw System::InvalidOperationException("The number of participants to remove would result in a negative number of remaining participants for this phase.");
            --participantCount_;
            if (--remainingCount_ == 0) {
                FinishPhase(lock);
            }
        }

        /**
         * @brief Releases resources used by the Barrier.
         * @throws System::InvalidOperationException if called from within the post-phase action.
         * @note Verified against Barrier.cs's Dispose()/Dispose(bool): real .NET checks the same
         * post-phase-action reentrancy guard as SignalAndWait/AddParticipants/RemoveParticipants
         * before disposing, and Dispose(bool) is idempotent. Calling this repeatedly is safe.
         */
        void Dispose() {
            ThrowIfCalledFromPostPhaseAction();
            disposed_ = true;
        }

    private:
        /**
         * @brief Advances to the next phase, invoking the post-phase action.
         * @throws BarrierPostPhaseException on the calling (triggering) thread if the action
         * threw; other participants blocked in SignalAndWait() observe the same failure via
         * lastPostPhaseException_ once they wake.
         */
        void FinishPhase(std::unique_lock<std::mutex>& lock) {
            ++phaseCount_;
            remainingCount_ = participantCount_;
            if (postPhaseAction_) {
                actionCallerId_.store(std::this_thread::get_id());
                try {
                    postPhaseAction_(*this);
                    lastPostPhaseException_ = nullptr;
                } catch (...) {
                    lastPostPhaseException_ = std::current_exception();
                }
                actionCallerId_.store(std::thread::id());
            } else {
                lastPostPhaseException_ = nullptr;
            }
            cv_.notify_all();
            (void)lock;
            if (lastPostPhaseException_)
                throw BarrierPostPhaseException(lastPostPhaseException_);
        }
    };

} // namespace System::Threading
