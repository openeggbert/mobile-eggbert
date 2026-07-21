// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <thread>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Threading {

    /** Provides support for spin-based waiting. */
    class SpinWait {
        SharpRuntime::intcs count_ = 0;
        static constexpr SharpRuntime::intcs YieldThreshold = 10;

    public:
        /** Returns the number of times SpinOnce has been called on this instance. */
        [[nodiscard]] SharpRuntime::intcs getCountProperty() const noexcept { return count_; }
        /** Returns true when the next call to SpinOnce will yield the processor. */
        [[nodiscard]] bool getNextSpinWillYieldProperty() const noexcept { return count_ >= YieldThreshold; }

        /** Performs a single spin; yields the processor when the spin count exceeds the threshold. */
        void SpinOnce() {
            if (count_ >= YieldThreshold)
                std::this_thread::yield();
            ++count_;
        }

        /** Performs a single spin, ignoring sleep1Threshold. */
        void SpinOnce(SharpRuntime::intcs /*sleep1Threshold*/) { SpinOnce(); }

        /** Resets the spin counter to zero. */
        void Reset() noexcept { count_ = 0; }

        /** Spins in a loop until condition returns true. */
        static void SpinUntil(std::function<bool()> condition) {
            SpinWait sw;
            while (!condition()) sw.SpinOnce();
        }

        /**
         * @brief Spins until condition returns true or the timeout elapses; returns true on success.
         *
         * Verified against SpinWait.cs: -1 (Timeout.Infinite) waits indefinitely (the 0-arg
         * SpinUntil(condition) overload delegates to this one with Timeout.Infinite). A
         * deadline computed as now()+milliseconds(-1) would already be in the past, so -1
         * must be special-cased to skip the deadline check entirely.
         */
        static bool SpinUntil(std::function<bool()> condition, SharpRuntime::intcs millisecondsTimeout) {
            if (millisecondsTimeout == -1) {
                SpinWait sw;
                while (!condition()) sw.SpinOnce();
                return true;
            }
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(millisecondsTimeout);
            SpinWait sw;
            while (!condition()) {
                if (std::chrono::steady_clock::now() >= deadline) return false;
                sw.SpinOnce();
            }
            return true;
        }
    };

} // namespace System::Threading
