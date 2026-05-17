#pragma once

#include "WindowsPhoneSpeedyBlupi/ConfigDef.hpp"
//#define LEGACY

#ifndef LEGACY
#define MODERN
#endif

namespace WindowsPhoneSpeedyBlupi
{
#ifdef LEGACY
    /** True when the legacy (original behaviour) mode is compiled in. */
    static constexpr bool LEGACY_ENABLED = true;
#else
    /** False when the modern port mode is compiled in. */
    static constexpr bool LEGACY_ENABLED = false;
#endif

    /**
     * @brief Compile-time configuration constants for the game port.
     *
     * All fields are static constexpr and selected at compile time via the LEGACY or
     * MODERN preprocessor defines. Exactly one of those defines must be active.
     *
     * - LEGACY mode: reproduces the exact original game behaviour at 20 FPS with no
     *   touch-button visibility adaptation.
     * - MODERN mode: allows higher FPS and enables touch-button auto-hiding on
     *   non-touchscreen devices. All frame-count values must be passed through
     *   ScaleTime() / ScaleDiv() to remain correct at the configured FPS.
     *
     * @note Do not mix LEGACY and MODERN defines in the same translation unit.
     */
    struct Config
    {

#ifdef MODERN
        /** Target frame rate for this build. All timer values scale relative to Fps20. */
        static constexpr Fps FPS = Fps::Fps20;
        
        // Please do not change
        static constexpr int CURRENT_FPS = static_cast<int>(FPS);

        /**
         * @brief Ratio of the configured FPS to the original 20 FPS.
         *
         * Used to convert original frame-count delays to the current FPS.
         * At FPS=20 this is 1.0; at FPS=60 this is 3.0.
         * Multiply any original frame-count value by TIME_SCALE to get the
         * equivalent count at the current FPS.
         */
        // Please do not change
        static constexpr double TIME_SCALE =
            static_cast<double>(CURRENT_FPS) / static_cast<double>(ORIGINAL_FPS);

        /**
         * @brief Ratio of the original 20 FPS to the configured FPS.
         *
         * Used to scale per-frame movement and speed values so that the total
         * distance covered per second remains constant regardless of FPS.
         * At FPS=20 this is 1.0; at FPS=60 this is 0.333…
         * Multiply any original per-frame speed by SPEED_SCALE before applying it.
         */
        // Please do not change
        static constexpr double SPEED_SCALE =
            static_cast<double>(ORIGINAL_FPS) / static_cast<double>(CURRENT_FPS);

        /**
         * @brief Integer resolution scale factor (derived from ResolutionScale enum).
         *
         * Currently always 1 in production. Reserved for future high-DPI support.
         */
        static constexpr int RESOLUTION_SCALE = static_cast<int>(ResolutionScale::ScaleResolution1);

        /**
         * @brief When true, on-screen touch buttons are hidden on non-touchscreen devices.
         *
         * In MODERN mode this is true so that keyboard/mouse users do not see
         * redundant on-screen buttons. In LEGACY mode this is always false.
         */
        static constexpr bool TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE = true;

        /**
         * @brief Enables verbose console logging for input events when true.
         *
         * Useful for diagnosing input mapping issues during porting work.
         * Must be false in release builds.
         */
        static constexpr bool INPUT_DETAILED_DEBUGGING_ENABLED = false;

        /**
         * @brief Scales a frame-count timer value to the current FPS.
         *
         * Original gameplay timers are expressed in 20 FPS frames. Pass any such
         * constant through ScaleTime() before storing it so that timeouts and
         * delays behave identically at any supported FPS.
         *
         * At FPS=20: returns @p value unchanged.
         * At FPS=60: returns @p value * 3.
         *
         * @param value Frame count at 20 FPS.
         * @return Equivalent frame count at the configured FPS.
         */
        static constexpr int ScaleTime(int value)
        {
            if constexpr (FPS == Fps::Fps20)
            {
                return value;
            }
            else
            {
                return (value * CURRENT_FPS + ORIGINAL_FPS / 2) / ORIGINAL_FPS;
            }
        }

        /**
         * @brief Scales an integer animation frame divisor to the current FPS.
         *
         * Animation phases are often advanced by dividing a raw frame counter.
         * Apply ScaleDiv() to the divisor to keep the visual animation speed
         * constant regardless of the configured FPS.
         *
         * At FPS=20: returns @p value unchanged.
         * At FPS=60: returns @p value * 3.
         *
         * @param value Divisor at 20 FPS.
         * @return Equivalent divisor at the configured FPS.
         */
        static constexpr int ScaleDiv(int value)
        {
            return ScaleTime(value);
        }

        /**
         * @brief Scales an integer pixel/size value by the asset resolution scale.
         *
         * Use this to convert 1x sprite sheet coordinates (cell widths, heights,
         * gaps, offsets) to the corresponding pixel values in the loaded texture.
         * At RESOLUTION_SCALE=1 the value is returned unchanged.
         *
         * @param value Size in 1x sprite-sheet pixels.
         * @return Equivalent size in the loaded texture pixels.
         */
        static constexpr int ScaleAsset(int value)
        {
            return value * RESOLUTION_SCALE;
        }

#endif

#ifdef LEGACY
        // These are legacy values. Please do not modify.

        // Please do not change
        static constexpr Fps FPS = Fps::Fps20;

        // Please do not change
        static constexpr int CURRENT_FPS = static_cast<int>(Fps::Fps20);

        // Please do not change
        static constexpr double TIME_SCALE = 1.0;
        // Please do not change
        static constexpr double SPEED_SCALE = 1.0;

        // Please do not change
        static constexpr int RESOLUTION_SCALE = static_cast<int>(ResolutionScale::ScaleResolution1);
        // Please do not change
        static constexpr bool TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE = false;
        // Please do not change
        static constexpr bool INPUT_DETAILED_DEBUGGING_ENABLED = false;

        // Please do not change
        static constexpr int ScaleTime(int value)
        {
            return value;
        }

        // Please do not change
        static constexpr int ScaleDiv(int value)
        {
            return value;
        }

        /** @copydoc Config::ScaleAsset (MODERN) */
        // Please do not change
        static constexpr int ScaleAsset(int value)
        {
            return value * RESOLUTION_SCALE;
        }

#endif

#ifdef LEGACY
        static_assert(FPS == Fps::Fps20, "LEGACY mode must use Fps20");
        static_assert(CURRENT_FPS == static_cast<int>(Fps::Fps20), "LEGACY mode CURRENT_FPS must be 20");
        static_assert(TIME_SCALE == 1.0, "LEGACY mode TIME_SCALE must be 1.0");
        static_assert(SPEED_SCALE == 1.0, "LEGACY mode SPEED_SCALE must be 1.0");
        static_assert(RESOLUTION_SCALE == static_cast<int>(ResolutionScale::ScaleResolution1),
                      "LEGACY mode RESOLUTION_SCALE must be 1");
        static_assert(TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE == false,
                      "LEGACY mode must not hide touch buttons");
        static_assert(INPUT_DETAILED_DEBUGGING_ENABLED == false, "LEGACY mode must not enable input debugging");
#endif
    };
}
