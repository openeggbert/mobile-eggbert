// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <vector>
#include "System/ArgumentNullException.hpp"
#include "System/IProgress.hpp"

namespace System {

    /**
     * @brief Provides an IProgress<T> that invokes callbacks for each reported progress value.
     *
     * C++ counterpart of .NET System.Progress<T>. Any handler provided to the
     * constructor and handlers registered via addProgressChangedHandler() are invoked
     * synchronously when Report() is called.
     *
     * Note: .NET's Progress<T> marshals callbacks through SynchronizationContext.
     * This implementation calls handlers directly on the calling thread, which is
     * equivalent to running without a captured SynchronizationContext (thread-pool
     * semantics in .NET).
     *
     * @tparam T Specifies the type of the progress report value.
     */
    template<typename T>
    class Progress : public IProgress<T> {
        std::function<void(T)>              handler_;
        std::vector<std::function<void(T)>> progressChanged_;

    protected:
        /**
         * @brief Reports a progress change by invoking all registered handlers.
         *
         * C++ counterpart of .NET Progress<T>.OnReport(T value).
         * Override in a subclass to customize how progress notifications are delivered.
         * @param value The progress value to report.
         */
        virtual void OnReport(const T& value) {
            if (handler_) handler_(value);
            for (auto& h : progressChanged_) h(value);
        }

    public:
        /**
         * @brief Initializes a Progress<T> with no handler attached.
         *
         * C++ counterpart of .NET Progress<T>().
         */
        Progress() = default;

        /**
         * @brief Initializes a Progress<T> that invokes the specified handler on each Report().
         *
         * C++ counterpart of .NET Progress<T>(Action<T> handler).
         * @param handler Callback invoked with the reported value.
         * @throws ArgumentNullException if @p handler is empty/null, matching .NET.
         */
        explicit Progress(std::function<void(T)> handler)
            : handler_(std::move(handler)) {
            if (!handler_) throw ArgumentNullException("handler");
        }

        /**
         * @brief Reports a progress change, invoking all attached handlers.
         *
         * C++ counterpart of .NET IProgress<T>.Report(T value) as implemented by Progress<T>.
         * Delegates to OnReport().
         * @param value The progress value to report.
         */
        void Report(const T& value) override { OnReport(value); }

        /**
         * @brief Attaches an additional handler to be called on each Report().
         *
         * C++ equivalent of subscribing to the .NET Progress<T>.ProgressChanged event.
         * Multiple handlers are called in the order they were registered.
         * @param handler Callback to append to the handler list.
         */
        void addProgressChangedHandler(std::function<void(T)> handler) {
            progressChanged_.push_back(std::move(handler));
        }
    };

} // namespace System
