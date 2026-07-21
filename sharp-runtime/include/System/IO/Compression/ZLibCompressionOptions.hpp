// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IO/Compression/ZLibCompressionStrategy.hpp"

namespace System::IO::Compression {

    using SharpRuntime::intcs;

    /**
     * @brief Provides compression options to be used with ZLibStream, DeflateStream, and
     * GZipStream.
     *
     * C++ counterpart of .NET System.IO.Compression.ZLibCompressionOptions.
     */
    class ZLibCompressionOptions {
    private:
        intcs compressionLevel_ = -1;
        ZLibCompressionStrategy strategy_ = ZLibCompressionStrategy::Default;
        intcs windowLog_ = -1;

    public:
        /** The minimum window log (base-2 logarithm of the window size) for zlib compression. */
        static constexpr intcs MinWindowLog = 8;
        /** The maximum window log (base-2 logarithm of the window size) for zlib compression. */
        static constexpr intcs MaxWindowLog = 15;
        /** The default window log (base-2 logarithm of the window size) for zlib compression. */
        static constexpr intcs DefaultWindowLog = MaxWindowLog;
        /** The minimum compression level accepted by CompressionLevel. */
        static constexpr intcs MinQuality = 0;
        /** The maximum compression level accepted by CompressionLevel. */
        static constexpr intcs MaxQuality = 9;

        ZLibCompressionOptions() = default;

        /**
         * @brief Gets or sets the compression level for a compression stream.
         *
         * Accepts any value between -1 and 9 (inclusive): 0 gives no compression, 1 gives best
         * speed, 9 gives best compression, and -1 requests the default compression level
         * (currently equivalent to 6). Default value is -1.
         * @throws System::ArgumentOutOfRangeException if @p value is not -1 and outside [0, 9].
         */
        [[nodiscard]] intcs getCompressionLevelProperty() const { return compressionLevel_; }
        void setCompressionLevelProperty(intcs value) {
            if (value != -1) {
                if (value < MinQuality)
                    throw System::ArgumentOutOfRangeException("value", "Value must be greater than or equal to " + std::to_string(MinQuality) + ".");
                if (value > MaxQuality)
                    throw System::ArgumentOutOfRangeException("value", "Value must be less than or equal to " + std::to_string(MaxQuality) + ".");
            }
            compressionLevel_ = value;
        }

        /**
         * @brief Gets or sets the compression algorithm for a compression stream.
         * @throws System::ArgumentOutOfRangeException if @p value is not a valid ZLibCompressionStrategy value.
         */
        [[nodiscard]] ZLibCompressionStrategy getCompressionStrategyProperty() const { return strategy_; }
        void setCompressionStrategyProperty(ZLibCompressionStrategy value) {
            if (static_cast<intcs>(value) < static_cast<intcs>(ZLibCompressionStrategy::Default))
                throw System::ArgumentOutOfRangeException("value", "Value must be greater than or equal to Default.");
            if (static_cast<intcs>(value) > static_cast<intcs>(ZLibCompressionStrategy::Fixed))
                throw System::ArgumentOutOfRangeException("value", "Value must be less than or equal to Fixed.");
            strategy_ = value;
        }

        /**
         * @brief Gets or sets the base-2 logarithm of the window size for a compression stream.
         *
         * Accepts -1 or any value between 8 and 15 (inclusive). -1 requests the default window
         * log (currently equivalent to 15, a 32KB window). Default value is -1.
         * @throws System::ArgumentOutOfRangeException if @p value is not -1 and outside [8, 15].
         */
        [[nodiscard]] intcs getWindowLogProperty() const { return windowLog_; }
        void setWindowLogProperty(intcs value) {
            if (value != -1) {
                if (value < MinWindowLog)
                    throw System::ArgumentOutOfRangeException("value", "Value must be greater than or equal to " + std::to_string(MinWindowLog) + ".");
                if (value > MaxWindowLog)
                    throw System::ArgumentOutOfRangeException("value", "Value must be less than or equal to " + std::to_string(MaxWindowLog) + ".");
            }
            windowLog_ = value;
        }
    };

} // namespace System::IO::Compression
