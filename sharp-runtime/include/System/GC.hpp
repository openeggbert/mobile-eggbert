// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/GCCollectionMode.hpp"
#include "System/GCGenerationInfo.hpp"
#include "System/GCNotificationStatus.hpp"
#include "System/TimeSpan.hpp"

namespace System {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * @brief Specifies the kind of garbage collection.
     *
     * C++ counterpart of .NET System.GCKind.
     */
    enum class GCKind {
        Any          = 0, /**< Any kind of collection. */
        Ephemeral    = 1, /**< A gen0 or gen1 collection. */
        FullBlocking = 2, /**< A blocking gen2 collection. */
        Background   = 3, /**< A background collection. */
    };

    /**
     * @brief Provides memory usage information about the garbage collector.
     *
     * C++ counterpart of .NET System.GCMemoryInfo.
     * All properties return zero/empty in this stub implementation.
     */
    struct GCMemoryInfo {
        /** @brief Gets the total available memory for the GC to use. */
        [[nodiscard]] longcs getTotalAvailableMemoryBytesProperty() const noexcept { return 0LL; }
        /** @brief Gets the memory load reported by the OS. */
        [[nodiscard]] longcs    getMemoryLoadBytesProperty()           const noexcept { return 0LL; }
        /** @brief Gets the heap size. */
        [[nodiscard]] longcs    getHeapSizeBytesProperty()             const noexcept { return 0LL; }
        /** @brief Gets the fragmented space in the heap. */
        [[nodiscard]] longcs    getFragmentedBytesProperty()           const noexcept { return 0LL; }
        /** @brief Gets the high memory load threshold. */
        [[nodiscard]] longcs    getHighMemoryLoadThresholdBytesProperty() const noexcept { return 0LL; }
        /** @brief Gets the total committed memory. */
        [[nodiscard]] longcs    getTotalCommittedBytesProperty()       const noexcept { return 0LL; }
        /** @brief Gets the index of the collection that produced this info. */
        [[nodiscard]] longcs    getIndexProperty()                     const noexcept { return 0LL; }
        /** @brief Gets the generation this info belongs to. */
        [[nodiscard]] intcs     getGenerationProperty()                const noexcept { return 0; }
        /** @brief Returns true when this info was produced by a concurrent GC. */
        [[nodiscard]] bool      getConcurrentProperty()                const noexcept { return false; }
        /** @brief Returns true when this info was produced by a compacting GC. */
        [[nodiscard]] bool      getCompactedProperty()                 const noexcept { return false; }
        /** @brief Gets the number of pinned objects the GC observed. */
        [[nodiscard]] longcs    getPinnedObjectsCountProperty()        const noexcept { return 0LL; }
        /** @brief Gets the number of objects ready for finalization this GC observed. */
        [[nodiscard]] longcs    getFinalizationPendingCountProperty()  const noexcept { return 0LL; }
        /** @brief Gets the number of bytes promoted to the next generation. */
        [[nodiscard]] longcs    getPromotedBytesProperty()             const noexcept { return 0LL; }
        /** @brief Gets the percentage of time spent paused for this GC, relative to its duration. */
        [[nodiscard]] double    getPauseTimePercentageProperty()       const noexcept { return 0.0; }
        /** @brief Gets per-generation size/fragmentation info; always empty in this port. */
        [[nodiscard]] std::vector<GCGenerationInfo> getGenerationInfoProperty() const {
            return {};
        }
        /** @brief Gets the pause durations observed for this GC; always empty in this port. */
        [[nodiscard]] std::vector<TimeSpan> getPauseDurationsProperty() const { return {}; }
        /** @brief Gets the pause duration for this GC. Convenience alias kept for existing callers. */
        [[nodiscard]] TimeSpan  getPauseDurationProperty()             const noexcept { return TimeSpan::Zero; }
    };

    /**
     * @brief Controls the system garbage collector — a service that automatically
     * reclaims unused memory.
     *
     * C++ counterpart of .NET System.GC.
     * All methods are no-op stubs or return zero/false because the C++ runtime
     * uses deterministic RAII rather than a tracing GC. The class exists so that
     * ported game code that calls GC.Collect() or GC.SuppressFinalize() compiles
     * and links without modification.
     *
     * @note AllocateArray<T>/AllocateUninitializedArray<T> and GetConfigurationVariables()
     * are intentionally omitted: the first two return a GC-tracked managed array with no
     * C++ equivalent (use a native array or std::vector<T> directly instead), and the third
     * returns an IReadOnlyDictionary<string, object> describing real GC configuration, which
     * does not exist in this port.
     */
    class GC {
    public:
        /** @brief Deleted constructor — all members are static. */
        GC() = delete;

        // -----------------------------------------------------------------------
        // Collect overloads
        // -----------------------------------------------------------------------

        /**
         * @brief Forces an immediate garbage collection of all generations.
         *
         * C++ counterpart of .NET GC.Collect().
         * No-op in this port.
         */
        static void Collect() {}

        /**
         * @brief Forces a garbage collection from generation 0 through @p generation.
         *
         * C++ counterpart of .NET GC.Collect(int).
         * No-op in this port.
         */
        static void Collect(intcs generation) { (void)generation; }

        /**
         * @brief Forces a garbage collection with the specified mode.
         *
         * C++ counterpart of .NET GC.Collect(int, GCCollectionMode).
         * No-op in this port.
         */
        static void Collect(intcs generation, GCCollectionMode mode) {
            (void)generation; (void)mode;
        }

        /**
         * @brief Forces a garbage collection with the specified mode and blocking behaviour.
         *
         * C++ counterpart of .NET GC.Collect(int, GCCollectionMode, bool).
         * No-op in this port.
         */
        static void Collect(intcs generation, GCCollectionMode mode, bool blocking) {
            (void)generation; (void)mode; (void)blocking;
        }

        /**
         * @brief Forces a garbage collection with the specified mode, blocking, and compacting.
         *
         * C++ counterpart of .NET GC.Collect(int, GCCollectionMode, bool, bool).
         * No-op in this port.
         */
        static void Collect(intcs generation, GCCollectionMode mode, bool blocking, bool compacting) {
            (void)generation; (void)mode; (void)blocking; (void)compacting;
        }

        // -----------------------------------------------------------------------
        // Memory info
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the total memory (in bytes) currently thought to be allocated.
         *
         * C++ counterpart of .NET GC.GetTotalMemory(bool).
         * Always returns 0 in this port.
         */
        [[nodiscard]] static longcs GetTotalMemory(bool forceFullCollection) {
            (void)forceFullCollection;
            return 0LL;
        }

        /**
         * @brief Gets the total bytes allocated on the managed heap since the process started.
         *
         * C++ counterpart of .NET GC.GetTotalAllocatedBytes(bool).
         * Always returns 0 in this port.
         */
        [[nodiscard]] static longcs GetTotalAllocatedBytes(bool precise = false) {
            (void)precise;
            return 0LL;
        }

        /**
         * @brief Gets the total bytes allocated on the managed heap for the current thread.
         *
         * C++ counterpart of .NET GC.GetAllocatedBytesForCurrentThread().
         * Always returns 0 in this port.
         */
        [[nodiscard]] static longcs GetAllocatedBytesForCurrentThread() { return 0LL; }

        /**
         * @brief Returns the total time spent pausing for GC since the process started.
         *
         * C++ counterpart of .NET GC.GetTotalPauseDuration().
         * Always returns TimeSpan::Zero in this port.
         */
        [[nodiscard]] static TimeSpan GetTotalPauseDuration() { return TimeSpan::Zero; }

        /**
         * @brief Returns GC memory info for the most recent collection of any kind.
         *
         * C++ counterpart of .NET GC.GetGCMemoryInfo().
         * Returns an all-zero stub in this port.
         */
        [[nodiscard]] static GCMemoryInfo GetGCMemoryInfo() { return GCMemoryInfo{}; }

        /**
         * @brief Returns GC memory info for the most recent collection of the specified kind.
         *
         * C++ counterpart of .NET GC.GetGCMemoryInfo(GCKind).
         * Returns an all-zero stub in this port.
         */
        [[nodiscard]] static GCMemoryInfo GetGCMemoryInfo(GCKind kind) {
            (void)kind;
            return GCMemoryInfo{};
        }

        // -----------------------------------------------------------------------
        // Generation info
        // -----------------------------------------------------------------------

        /**
         * @brief Gets the maximum number of generations the GC supports.
         *
         * C++ counterpart of .NET GC.MaxGeneration.
         * Returns 2 (matching .NET's 3-generation model: gen0, gen1, gen2).
         */
        [[nodiscard]] static intcs getMaxGenerationProperty() { return 2; }

        /** @brief Alias kept for backward compatibility. */
        [[nodiscard]] static intcs MaxGeneration() { return 2; }

        /**
         * @brief Returns the current GC generation of the object pointed to by @p obj.
         *
         * C++ counterpart of .NET GC.GetGeneration(object).
         * Always returns 0 in this port.
         */
        [[nodiscard]] static intcs GetGeneration(void*) { return 0; }

        /**
         * @brief Returns the number of times GC has occurred for the specified generation.
         *
         * C++ counterpart of .NET GC.CollectionCount(int).
         * Always returns 0 in this port.
         */
        [[nodiscard]] static intcs CollectionCount(intcs generation) {
            (void)generation;
            return 0;
        }

        // -----------------------------------------------------------------------
        // Finalizer control
        // -----------------------------------------------------------------------

        /**
         * @brief Requests that the system not call the finalizer for @p obj.
         *
         * C++ counterpart of .NET GC.SuppressFinalize(object).
         * No-op in this port.
         */
        static void SuppressFinalize(void*) {}

        /**
         * @brief Requests that the system call the finalizer for @p obj,
         * even if SuppressFinalize was previously called.
         *
         * C++ counterpart of .NET GC.ReRegisterForFinalize(object).
         * No-op in this port.
         */
        static void ReRegisterForFinalize(void*) {}

        /**
         * @brief Suspends the current thread until the finalizer thread has
         * emptied its queue.
         *
         * C++ counterpart of .NET GC.WaitForPendingFinalizers().
         * No-op in this port.
         */
        static void WaitForPendingFinalizers() {}

        // -----------------------------------------------------------------------
        // KeepAlive
        // -----------------------------------------------------------------------

        /**
         * @brief References @p obj, making it ineligible for GC from the start of
         * the current method until this call.
         *
         * C++ counterpart of .NET GC.KeepAlive(object).
         * No-op in this port.
         */
        template<typename T>
        static void KeepAlive(const T&) {}

        // -----------------------------------------------------------------------
        // Memory pressure
        // -----------------------------------------------------------------------

        /**
         * @brief Informs the runtime of a large allocation of unmanaged memory.
         *
         * C++ counterpart of .NET GC.AddMemoryPressure(long).
         * No-op in this port.
         */
        static void AddMemoryPressure(longcs bytesAllocated) { (void)bytesAllocated; }

        /**
         * @brief Informs the runtime that unmanaged memory has been released.
         *
         * C++ counterpart of .NET GC.RemoveMemoryPressure(long).
         * No-op in this port.
         */
        static void RemoveMemoryPressure(longcs bytesAllocated) { (void)bytesAllocated; }

        /**
         * @brief Updates the runtime on the current memory limit.
         *
         * C++ counterpart of .NET GC.RefreshMemoryLimit().
         * No-op in this port.
         */
        static void RefreshMemoryLimit() {}

        // -----------------------------------------------------------------------
        // No-GC region
        // -----------------------------------------------------------------------

        /**
         * @brief Attempts to disallow garbage collection during the execution of
         * a critical path.
         *
         * C++ counterpart of .NET GC.TryStartNoGCRegion(long).
         * Always returns false in this port (no GC to suppress).
         */
        [[nodiscard]] static bool TryStartNoGCRegion(longcs totalSize) {
            (void)totalSize;
            return false;
        }

        /**
         * @brief Attempts to disallow garbage collection, optionally disallowing a
         * full blocking collection, during the execution of a critical path.
         *
         * C++ counterpart of .NET GC.TryStartNoGCRegion(long, bool).
         * Always returns false in this port (no GC to suppress).
         */
        [[nodiscard]] static bool TryStartNoGCRegion(longcs totalSize, bool disallowFullBlockingGC) {
            (void)totalSize; (void)disallowFullBlockingGC;
            return false;
        }

        /**
         * @brief Attempts to disallow garbage collection, specifying the amount of
         * memory to reserve in the large object heap, during a critical path.
         *
         * C++ counterpart of .NET GC.TryStartNoGCRegion(long, long).
         * Always returns false in this port (no GC to suppress).
         */
        [[nodiscard]] static bool TryStartNoGCRegion(longcs totalSize, longcs lohSize) {
            (void)totalSize; (void)lohSize;
            return false;
        }

        /**
         * @brief Attempts to disallow garbage collection, specifying the large object
         * heap size and whether to disallow a full blocking collection.
         *
         * C++ counterpart of .NET GC.TryStartNoGCRegion(long, long, bool).
         * Always returns false in this port (no GC to suppress).
         */
        [[nodiscard]] static bool TryStartNoGCRegion(longcs totalSize, longcs lohSize,
                                                      bool disallowFullBlockingGC) {
            (void)totalSize; (void)lohSize; (void)disallowFullBlockingGC;
            return false;
        }

        /**
         * @brief Ends the no-GC region latency mode.
         *
         * C++ counterpart of .NET GC.EndNoGCRegion().
         * No-op in this port.
         */
        static void EndNoGCRegion() {}

        /**
         * @brief Registers a callback to invoke when a no-GC region allocation
         * reaches @p totalSize bytes.
         *
         * C++ counterpart of .NET GC.RegisterNoGCRegionCallback(long, Action).
         * No-op in this port.
         */
        static void RegisterNoGCRegionCallback(longcs totalSize,
                                               std::function<void()> callback) {
            (void)totalSize; (void)callback;
        }

        // -----------------------------------------------------------------------
        // Full GC notifications
        // -----------------------------------------------------------------------

        /**
         * @brief Specifies that a garbage collection notification should be raised
         * when conditions favor a full, blocking GC.
         *
         * C++ counterpart of .NET GC.RegisterForFullGCNotification(int, int).
         * No-op in this port.
         */
        static void RegisterForFullGCNotification(intcs maxGenerationThreshold,
                                                   intcs largeObjectHeapThreshold) {
            (void)maxGenerationThreshold; (void)largeObjectHeapThreshold;
        }

        /**
         * @brief Cancels the registration of a full GC notification.
         *
         * C++ counterpart of .NET GC.CancelFullGCNotification().
         * No-op in this port.
         */
        static void CancelFullGCNotification() {}

        /**
         * @brief Returns the status of a registered notification for a full GC approach.
         *
         * C++ counterpart of .NET GC.WaitForFullGCApproach(int).
         * Always returns GCNotificationStatus::NotApplicable in this port: since
         * RegisterForFullGCNotification() is a no-op (nothing is ever actually monitored),
         * claiming Succeeded (a full GC approach was genuinely detected) would misrepresent
         * that no such detection occurred. NotApplicable matches real .NET's own use of that
         * value for "this API doesn't apply to the current GC configuration".
         */
        [[nodiscard]] static GCNotificationStatus WaitForFullGCApproach(
            intcs millisecondsTimeout = -1)
        {
            (void)millisecondsTimeout;
            return GCNotificationStatus::NotApplicable;
        }

        /**
         * @brief Returns the status of a registered notification for a full GC approach,
         * using a TimeSpan timeout.
         *
         * C++ counterpart of .NET GC.WaitForFullGCApproach(TimeSpan).
         * Always returns GCNotificationStatus::NotApplicable in this port -- see the
         * millisecond-timeout overload's doc-comment for why.
         */
        [[nodiscard]] static GCNotificationStatus WaitForFullGCApproach(TimeSpan timeout) {
            (void)timeout;
            return GCNotificationStatus::NotApplicable;
        }

        /**
         * @brief Returns the status of a registered notification for a full GC completion.
         *
         * C++ counterpart of .NET GC.WaitForFullGCComplete(int).
         * Always returns GCNotificationStatus::NotApplicable in this port -- see
         * WaitForFullGCApproach(int)'s doc-comment for why.
         */
        [[nodiscard]] static GCNotificationStatus WaitForFullGCComplete(
            intcs millisecondsTimeout = -1)
        {
            (void)millisecondsTimeout;
            return GCNotificationStatus::NotApplicable;
        }

        /**
         * @brief Returns the status of a registered notification for a full GC completion,
         * using a TimeSpan timeout.
         *
         * C++ counterpart of .NET GC.WaitForFullGCComplete(TimeSpan).
         * Always returns GCNotificationStatus::NotApplicable in this port -- see
         * WaitForFullGCApproach(int)'s doc-comment for why.
         */
        [[nodiscard]] static GCNotificationStatus WaitForFullGCComplete(TimeSpan timeout) {
            (void)timeout;
            return GCNotificationStatus::NotApplicable;
        }

        // -----------------------------------------------------------------------
        // Legacy helper kept for source compatibility
        // -----------------------------------------------------------------------

        /**
         * @brief Returns total available memory bytes from GCMemoryInfo.
         *
         * Convenience wrapper kept for existing callers; prefer GetGCMemoryInfo().
         */
        [[nodiscard]] static longcs GetGCMemoryInfo_TotalAvailableMemoryBytes() {
            return 0LL;
        }
    };

} // namespace System
