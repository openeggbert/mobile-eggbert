// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <future>
#include <thread>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/Threading/IThreadPoolWorkItem.hpp"
#include "System/Threading/RegisteredWaitHandle.hpp"
#include "System/Threading/WaitCallback.hpp"
#include "System/Threading/WaitHandle.hpp"
#include "System/Threading/WaitOrTimerCallback.hpp"
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
#  include "System/PlatformNotSupportedException.hpp"
#endif

namespace System::Threading {

    using SharpRuntime::intcs;

    /** Provides a pool of threads that can be used to execute tasks, post work items, and other operations. */
    class ThreadPool {
    public:
        /** Prevents instantiation — all members are static. */
        ThreadPool() = delete;

        /** Queues the callback for execution on a detached thread; returns true on success. */
        static bool QueueUserWorkItem(std::function<void()> callBack) {
            if (!callBack)
                throw System::ArgumentNullException("callBack");
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
            throw System::PlatformNotSupportedException("ThreadPool::QueueUserWorkItem requires pthreads (not available in Emscripten single-threaded build)");
#else
            std::thread(std::move(callBack)).detach();
            return true;
#endif
        }

        /** Queues the callback with a state argument for execution on a detached thread; returns true on success. */
        static bool QueueUserWorkItem(std::function<void(void*)> callBack, void* state) {
            if (!callBack)
                throw System::ArgumentNullException("callBack");
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
            (void)state;
            throw System::PlatformNotSupportedException("ThreadPool::QueueUserWorkItem requires pthreads (not available in Emscripten single-threaded build)");
#else
            std::thread([callBack, state]{ callBack(state); }).detach();
            return true;
#endif
        }

        /** Retrieves the minimum number of threads the pool maintains. */
        static void GetMinThreads(int& workerThreads, int& completionPortThreads) {
            workerThreads = 1;
            completionPortThreads = 1;
        }

        /** Retrieves the maximum number of concurrent threads. */
        static void GetMaxThreads(int& workerThreads, int& completionPortThreads) {
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
            workerThreads = 1;
            completionPortThreads = 1;
#else
            workerThreads = static_cast<int>(std::thread::hardware_concurrency()) * 2;
            completionPortThreads = workerThreads;
#endif
        }

        /** Sets the minimum number of threads the pool maintains; always returns true in this stub. */
        static bool SetMinThreads(int /*workerThreads*/, int /*completionPortThreads*/) { return true; }
        /** Sets the maximum number of concurrent threads; always returns true in this stub. */
        static bool SetMaxThreads(int /*workerThreads*/, int /*completionPortThreads*/) { return true; }

        /** Queues an IThreadPoolWorkItem's Execute() for execution on a detached thread; returns true on success. */
        static bool UnsafeQueueUserWorkItem(IThreadPoolWorkItem* callBack, bool /*preferLocal*/) {
            if (!callBack)
                throw System::ArgumentNullException("callBack");
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
            throw System::PlatformNotSupportedException("ThreadPool::UnsafeQueueUserWorkItem requires pthreads (not available in Emscripten single-threaded build)");
#else
            std::thread([callBack]{ callBack->Execute(); }).detach();
            return true;
#endif
        }

        /**
         * @brief Registers a wait on waitObject; callBack is invoked when it is signalled or the timeout elapses.
         * @note Deliberate simplification: implemented via a dedicated background thread that polls
         * waitObject rather than true OS-level thread-pool wait registration.
         */
        static RegisteredWaitHandle RegisterWaitForSingleObject(
            WaitHandle* waitObject, WaitOrTimerCallback callBack, void* state,
            intcs millisecondsTimeOutInterval, bool executeOnlyOnce) {
            return RegisteredWaitHandle(waitObject, std::move(callBack), state, millisecondsTimeOutInterval, executeOnlyOnce);
        }
    };

} // namespace System::Threading
