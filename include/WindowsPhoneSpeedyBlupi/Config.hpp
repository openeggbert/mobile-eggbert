#pragma once

//#define LEGACY

#ifndef LEGACY
#define MODERN
#endif

namespace WindowsPhoneSpeedyBlupi
{
    enum class ResolutionScale
    {
        ScaleResolution1 = 1,
        ScaleResolution2 = 2,
        ScaleResolution4 = 4
    };

    inline ResolutionScale RESOLUTION_SCALE_DEFAULT = ResolutionScale::ScaleResolution1;

    enum class Fps
    {
        Fps20 = 20,
        Fps30 = 30,
        Fps60 = 60,
        Fps120 = 120,
        Fps144 = 144,
    };

    inline Fps FPS_DEFAULT = Fps::Fps20;

#ifdef LEGACY
    static constexpr bool LEGACY_ENABLED = true;
#else
    static constexpr bool LEGACY_ENABLED = false;
#endif

#ifdef MODERN
    static constexpr Fps FPS = Fps::Fps20;
    static constexpr double TIME_SCALE = static_cast<double>(FPS) / static_cast<double>(Fps::Fps20);
    static constexpr double SPEED_SCALE = static_cast<double>(Fps::Fps20) / static_cast<double>(FPS);
    static constexpr intcs RESOLUTION_SCALE = static_cast<intcs>(ResolutionScale::ScaleResolution1);
    static constexpr bool TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE = true;
    static constexpr bool INPUT_DETAILED_DEBUGGING_ENABLED = false;
#endif

#ifdef LEGACY
    // These are legacy values. Please, do not modify.
    static constexpr Fps FPS = Fps::Fps20;
    static constexpr double TIME_SCALE = static_cast<double>(FPS) / static_cast<double>(Fps::Fps20);
    static constexpr double SPEED_SCALE = static_cast<double>(Fps::Fps20) / static_cast<double>(FPS);
    static constexpr intcs RESOLUTION_SCALE = static_cast<intcs>(ResolutionScale::ResolutionScale);
    static constexpr bool TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE = false;
    static constexpr bool INPUT_DETAILED_DEBUGGING_ENABLED = false;
#endif
}
