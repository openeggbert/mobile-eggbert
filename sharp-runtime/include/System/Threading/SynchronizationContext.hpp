// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Threading/ThreadPool.hpp"

namespace System::Threading {

    /** A delegate type for sending or posting callbacks to a SynchronizationContext. */
    using SendOrPostCallback = std::function<void(void*)>;

    /**
     * @brief Provides the basic functionality required to propagate a synchronisation context
     * in various synchronisation models.
     *
     * C++ counterpart of .NET System.Threading.SynchronizationContext.
     *
     * @note Verified against SynchronizationContext.cs: (1) Post() queues the callback to the
     * thread pool (asynchronous), unlike Send() which runs it synchronously on the calling
     * thread -- this port's Post() previously ran the callback inline, identically to Send(),
     * defeating the entire purpose of the distinction. (2) Current/SetSynchronizationContext
     * read/write a per-thread field (real .NET: Thread.CurrentThread._synchronizationContext);
     * this port's getCurrentProperty() previously always returned nullptr regardless of what had been
     * set via SetSynchronizationContext(), which was itself a complete no-op -- the pair never
     * round-tripped at all.
     */
    class SynchronizationContext {
    public:
        /** Destroys the SynchronizationContext. */
        virtual ~SynchronizationContext() = default;

        /** Dispatches an asynchronous message to the context (queued to the thread pool by default). */
        virtual void Post(SendOrPostCallback d, void* state) {
            if (d) ThreadPool::QueueUserWorkItem(d, state);
        }

        /** Dispatches a synchronous message to the context (runs immediately on the calling thread). */
        virtual void Send(SendOrPostCallback d, void* state) {
            if (d) d(state);
        }

        /** Returns the synchronization context set for the current thread via SetSynchronizationContext(), or nullptr if none. */
        static SynchronizationContext* getCurrentProperty() { return CurrentSlot(); }

        /** Sets the synchronization context for the current thread. */
        static void SetSynchronizationContext(SynchronizationContext* syncContext) { CurrentSlot() = syncContext; }

    private:
        static SynchronizationContext*& CurrentSlot() {
            thread_local SynchronizationContext* current = nullptr;
            return current;
        }
    };

} // namespace System::Threading
