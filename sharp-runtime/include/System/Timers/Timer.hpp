// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/EventHandler.hpp"
#include "System/TimeSpan.hpp"
#include "System/Timers/ElapsedEventArgs.hpp"

namespace System::Threading {
    class Timer;
}

namespace System::Timers {

    /**
     * @brief Generates recurring events in an application, raising Elapsed at a configurable interval.
     *
     * C++ counterpart of .NET System.Timers.Timer. Built on System::Threading::Timer.
     *
     * @note Reduced scope: does not derive from System::ComponentModel::Component (no designer/
     * ISite integration is meaningful here) and does not implement `SynchronizingObject`
     * (`ISynchronizeInvoke`-based marshaling is a WinForms/UI-thread concept with no equivalent
     * in this runtime — `Elapsed` is always raised directly from the timer's background thread,
     * matching how `System::Threading::Timer` itself already works here).
     *
     * @note Lifetime contract: the background timer callback captures `this` directly (see
     * Timer.cpp's `startTimerThread()`), and the underlying `System::Threading::Timer::Dispose()`
     * detaches its thread rather than joining it. Destroying (or `Close()`-ing) a `Timer` does
     * not guarantee an in-flight callback invocation has finished — a callback that is already
     * executing when the `Timer` is destroyed can dereference a dangling `this`. Callers must
     * ensure a `Timer` outlives any callback it might still be dispatching, e.g. by not
     * destroying it from within its own `Elapsed` handler and by providing external
     * synchronization if destruction can race with a pending tick. This is the same class of
     * this-capture-into-background-thread hazard documented on `Socket`'s async methods and
     * `ClientWebSocket`'s Send/ReceiveAsync — not redesigned here for the same reason: fixing it
     * needs a broader ownership change (e.g. `enable_shared_from_this`/`weak_ptr` in the
     * callback), out of scope for a single audit pass.
     */
    class Timer {
        double interval_ = 100;
        bool enabled_ = false;
        bool autoReset_ = true;
        bool initializing_ = false;
        bool delayedEnable_ = false;
        std::unique_ptr<System::Threading::Timer> timer_;
        std::shared_ptr<int> cookie_;

        void updateTimer();
        void startTimerThread();

    public:
        /** @brief Event handler collection for the Elapsed event. */
        System::EventHandler<ElapsedEventArgs> Elapsed;

        /** @brief Constructs with a default 100ms interval, initially disabled. */
        Timer();

        /**
         * @brief Constructs with the given interval, in milliseconds.
         * @throws System::ArgumentException if @p interval is not in (0, INT32_MAX].
         */
        explicit Timer(double interval);

        /** @brief Constructs with the given interval. */
        explicit Timer(System::TimeSpan interval);

        ~Timer();

        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;

        /** @return true if Elapsed is raised repeatedly (every Interval) while Enabled. */
        [[nodiscard]] bool getAutoResetProperty() const { return autoReset_; }
        /** @brief Sets whether Elapsed is raised repeatedly while Enabled. */
        void setAutoResetProperty(bool value);

        /** @return true if the timer is currently running. */
        [[nodiscard]] bool getEnabledProperty() const { return enabled_; }
        /** @brief Starts (true) or stops (false) the timer. */
        void setEnabledProperty(bool value);

        /** @return The interval, in milliseconds, at which to raise Elapsed. */
        [[nodiscard]] double getIntervalProperty() const { return interval_; }
        /** @brief Sets the interval, in milliseconds. @throws System::ArgumentException if not positive. */
        void setIntervalProperty(double value);

        /** @brief Starts raising the Elapsed event (equivalent to setEnabledProperty(true)). */
        void Start() { setEnabledProperty(true); }
        /** @brief Stops raising the Elapsed event (equivalent to setEnabledProperty(false)). */
        void Stop() { setEnabledProperty(false); }

        /** @brief Stops the timer and prepares for a batch of property changes (see EndInit()). */
        void BeginInit();
        /** @brief Applies any Enabled value set since BeginInit() and resumes normal operation. */
        void EndInit();

        /** @brief Stops and releases the underlying timer resources. */
        void Close();
        /** @brief Equivalent to Close(). */
        void Dispose() { Close(); }
    };

} // namespace System::Timers
