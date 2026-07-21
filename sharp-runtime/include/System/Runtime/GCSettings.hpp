// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Runtime {

    /** Controls how the GC compacts the large-object heap. */
    enum class GCLargeObjectHeapCompactionMode {
        Default     = 1, ///< No compaction (default behaviour).
        CompactOnce = 2, ///< Compact the LOH on the next full GC.
    };

    /** Indicates the current garbage-collection latency mode. */
    enum class GCLatencyMode {
        Batch                = 0, ///< Optimise for throughput.
        Interactive          = 1, ///< Balance throughput and responsiveness (default).
        LowLatency           = 2, ///< Minimise GC pause times.
        SustainedLowLatency  = 3, ///< Minimise pauses for extended periods.
        NoGCRegion           = 4, ///< No-GC region is active.
    };

    /**
     * Provides global configuration settings for the garbage collector.
     * 
     * All members are static; the class cannot be instantiated.
     */
    class GCSettings {
        static GCLatencyMode latencyMode_;
        static GCLargeObjectHeapCompactionMode compactionMode_;
        static bool isServerGC_;

    public:
        GCSettings() = delete;

        /** @return The current GC latency mode. */
        [[nodiscard]] static GCLatencyMode getLatencyModeProperty()            { return latencyMode_; }

        /** Sets the GC latency mode. */
        static void setLatencyModeProperty(GCLatencyMode m)                    { latencyMode_ = m; }

        /** @return The current LOH compaction mode. */
        [[nodiscard]] static GCLargeObjectHeapCompactionMode getLargeObjectHeapCompactionModeProperty() {
            return compactionMode_;
        }

        /** Sets the LOH compaction mode. */
        static void setLargeObjectHeapCompactionModeProperty(GCLargeObjectHeapCompactionMode m) {
            compactionMode_ = m;
        }

        /** @return True if the server GC is enabled (always false in this stub). */
        [[nodiscard]] static bool getIsServerGCProperty() { return isServerGC_; }
    };

    inline GCLatencyMode GCSettings::latencyMode_ = GCLatencyMode::Interactive;
    inline GCLargeObjectHeapCompactionMode GCSettings::compactionMode_ = GCLargeObjectHeapCompactionMode::Default;
    inline bool GCSettings::isServerGC_ = false;

} // namespace System::Runtime
