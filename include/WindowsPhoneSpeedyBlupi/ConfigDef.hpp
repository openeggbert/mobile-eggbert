#pragma once

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Selects the rendering resolution multiplier for the game viewport.
     *
     * Used to scale the base 640x480 logical resolution up for high-DPI displays.
     * Only ScaleResolution1 (1:1) is currently active in production builds.
     */
    enum class ResolutionScale
    {
        ScaleResolution1 = 1, ///< Native 640x480 logical resolution (default).
        ScaleResolution2 = 2, ///< 2x upscale (1280x960 logical).
        ScaleResolution4 = 4 ///< 4x upscale (2560x1920 logical).
    };

    /** Default resolution scale used when no explicit scale is configured. */
    // Please do not change
    inline constexpr ResolutionScale RESOLUTION_SCALE_DEFAULT = ResolutionScale::ScaleResolution1;

    /**
     * @brief Target update rates supported by the compile-time configuration.
     *
     * Fps20 is the original stable timing. Higher values are experimental and require
     * gameplay code to preserve the original real-time behavior using Config::ScaleTime(),
     * Config::ScaleDiv() and Config::SPEED_SCALE where appropriate.
     */
    enum class Fps
    {
        Fps20 = 20, ///< Original game speed (default).
        Fps30 = 30,
        Fps60 = 60,
        Fps90 = 90,
        Fps120 = 120,
        Fps144 = 144,
    };

    /** Default FPS used when no explicit frame rate is configured. */
    // Please do not change
    inline constexpr Fps FPS_DEFAULT = Fps::Fps20;
    static constexpr int ORIGINAL_FPS = static_cast<int>(Fps::Fps20);
}
