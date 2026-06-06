/**
 * @file ConfigDef.hpp
 * @brief Defines the ResolutionScale and Fps enumerations and their default constants used by Config.
 *
 * @details These definitions are shared between LEGACY and MODERN build configurations and must
 * not be conditionally compiled. They are included by Config.hpp before the LEGACY/MODERN
 * preprocessor guards take effect.
 */

#pragma once

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Selects the rendering resolution multiplier for the game viewport.
     *
     * @details Used to scale the base 640x480 logical resolution up for high-DPI displays.
     * Only ScaleResolution1 (1:1) is currently active in production builds.
     * Higher values are reserved for future high-DPI support.
     */
    enum class ResolutionScale
    {
        ScaleResolution1 = 1, ///< @brief Native 640x480 logical resolution (default, 1:1 pixel mapping).
        ScaleResolution2 = 2, ///< @brief 2x upscale — 1280x960 logical resolution.
        ScaleResolution4 = 4  ///< @brief 4x upscale — 2560x1920 logical resolution.
    };

    /**
     * @brief Default resolution scale used when no explicit scale is configured.
     *
     * @details Always ScaleResolution1 (native 640x480). Do not change this constant.
     */
    // Please do not change
    inline constexpr ResolutionScale RESOLUTION_SCALE_DEFAULT = ResolutionScale::ScaleResolution1;

    /**
     * @brief Target update rates supported by the compile-time configuration.
     *
     * @details Fps20 is the original stable timing from the Windows Phone release.
     * Higher values are experimental and require gameplay code to preserve the original
     * real-time behaviour using Config::ScaleTime(), Config::ScaleDiv(), and
     * Config::SPEED_SCALE where appropriate.
     *
     * @warning Using frame rates other than Fps20 without applying the scale helpers
     *          will cause gameplay to run at incorrect speeds.
     */
    enum class Fps
    {
        Fps20  = 20,  ///< @brief Original 20 FPS game speed (default, stable).
        Fps30  = 30,  ///< @brief 30 FPS (experimental, requires ScaleTime/ScaleDiv/SPEED_SCALE).
        Fps60  = 60,  ///< @brief 60 FPS (experimental, requires ScaleTime/ScaleDiv/SPEED_SCALE).
        Fps90  = 90,  ///< @brief 90 FPS (experimental, requires ScaleTime/ScaleDiv/SPEED_SCALE).
        Fps120 = 120, ///< @brief 120 FPS (experimental, requires ScaleTime/ScaleDiv/SPEED_SCALE).
        Fps144 = 144, ///< @brief 144 FPS (experimental, requires ScaleTime/ScaleDiv/SPEED_SCALE).
    };

    /**
     * @brief Default FPS used when no explicit frame rate is configured.
     *
     * @details Always Fps20. Do not change this constant.
     */
    // Please do not change
    inline constexpr Fps FPS_DEFAULT = Fps::Fps20;

    /**
     * @brief The original game's update rate as a plain integer (20).
     *
     * @details Used as the denominator in TIME_SCALE and SPEED_SCALE calculations.
     * All ScaleTime()/ScaleDiv() computations are relative to this value. Do not change.
     */
    static constexpr int ORIGINAL_FPS = static_cast<int>(Fps::Fps20);
}
