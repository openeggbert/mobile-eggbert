/**
 * @file Config.hpp
 * @brief Compile-time configuration constants for the Speedy Blupi port (LEGACY and MODERN modes).
 *
 * @details Exactly one of LEGACY or MODERN must be defined. LEGACY reproduces the original
 * 20 FPS behaviour unchanged. MODERN enables higher frame rates and touch-button auto-hiding,
 * and requires all frame-count values to be passed through ScaleTime()/ScaleDiv().
 *
 * @note Do not mix LEGACY and MODERN defines in the same translation unit.
 */

#pragma once

#include "WindowsPhoneSpeedyBlupi/ConfigDef.hpp"
//#define LEGACY

#ifndef LEGACY
#define MODERN
#endif

namespace WindowsPhoneSpeedyBlupi
{
#ifdef LEGACY
    /** @brief True when the legacy (original behaviour) mode is compiled in. */
    static constexpr bool LEGACY_ENABLED = true;
#else
    /** @brief False when the modern port mode is compiled in. */
    static constexpr bool LEGACY_ENABLED = false;
#endif

    /**
     * @struct Config
     * @brief Compile-time configuration constants for the game port.
     *
     * @details All fields are static constexpr and selected at compile time via the LEGACY or
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
        /**
         * @brief Target frame rate for this build.
         *
         * @details All timer and divisor values scale relative to the original Fps20.
         * Change this value (and recompile) to run at a different frame rate.
         * Do not change at runtime.
         */
        static constexpr Fps FPS = Fps::Fps20;

        /**
         * @brief Integer representation of FPS, derived from the Fps enum.
         *
         * @details Provided for arithmetic convenience. Always equals static_cast<int>(FPS).
         * Do not change directly — it is derived from FPS.
         */
        // Please do not change
        static constexpr int CURRENT_FPS = static_cast<int>(FPS);

        /**
         * @brief Ratio of the configured FPS to the original 20 FPS.
         *
         * @details Used to convert original frame-count delays to the current FPS.
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
         * @details Used to scale per-frame movement and speed values so that the total
         * distance covered per second remains constant regardless of FPS.
         * At FPS=20 this is 1.0; at FPS=60 this is 0.333...
         * Multiply any original per-frame speed value by SPEED_SCALE before applying it.
         */
        // Please do not change
        static constexpr double SPEED_SCALE =
            static_cast<double>(ORIGINAL_FPS) / static_cast<double>(CURRENT_FPS);

        /**
         * @brief Integer resolution scale factor derived from the ResolutionScale enum.
         *
         * @details Currently always 1 in production. Reserved for future high-DPI support.
         * Use ScaleAsset() to apply this factor to sprite-sheet pixel measurements.
         */
        static constexpr int RESOLUTION_SCALE = static_cast<int>(ResolutionScale::ScaleResolution1);

        /**
         * @brief When true, on-screen touch buttons are hidden on non-touchscreen devices.
         *
         * @details In MODERN mode this is true so that keyboard/mouse users do not see
         * redundant on-screen buttons. In LEGACY mode this is always false.
         */
        static constexpr bool TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE = true;

        /**
         * @brief Enables verbose console logging for input events when true.
         *
         * @details Useful for diagnosing input mapping issues during porting work.
         *
         * @warning Must be false in release builds. Setting this to true generates
         *          significant console output that may affect performance.
         */
        static constexpr bool INPUT_DETAILED_DEBUGGING_ENABLED = false;

        /**
         * @brief Scales a frame-count timer value from 20 FPS to the current FPS.
         *
         * @details Original gameplay timers are expressed in 20 FPS frames. Pass any such
         * constant through ScaleTime() before storing it so that timeouts and
         * delays behave identically at any supported FPS.
         *
         * At FPS=20: returns @p value unchanged.
         * At FPS=60: returns @p value * 3.
         *
         * @param[in] value Frame count expressed at 20 FPS.
         * @return Equivalent frame count at the currently configured FPS.
         */
        static constexpr int ScaleTime(int value)
        {
            if (FPS == Fps::Fps20)
            {
                return value;
            }
            else
            {
                return (value * CURRENT_FPS + ORIGINAL_FPS / 2) / ORIGINAL_FPS;
            }
        }

        /**
         * @brief Scales an integer animation frame divisor from 20 FPS to the current FPS.
         *
         * @details Animation phases are often advanced by dividing a raw frame counter.
         * Apply ScaleDiv() to the divisor to keep the visual animation speed
         * constant regardless of the configured FPS.
         *
         * At FPS=20: returns @p value unchanged.
         * At FPS=60: returns @p value * 3.
         *
         * @param[in] value Frame divisor expressed at 20 FPS.
         * @return Equivalent divisor at the currently configured FPS.
         */
        static constexpr int ScaleDiv(int value)
        {
            return ScaleTime(value);
        }

        /**
         * @brief Scales an integer pixel/size value by the asset resolution scale factor.
         *
         * @details Use this to convert 1x sprite-sheet coordinates (cell widths, heights,
         * gaps, offsets) to the corresponding pixel values in the loaded texture.
         * At RESOLUTION_SCALE=1 the value is returned unchanged.
         *
         * @param[in] value Size in 1x sprite-sheet pixels.
         * @return Equivalent size in the loaded texture pixels.
         */
        static constexpr int ScaleAsset(int value)
        {
            return value * RESOLUTION_SCALE;
        }

#endif

#ifdef LEGACY
        // These are legacy values. Please do not modify.

        /** @brief Target frame rate locked to Fps20 in LEGACY mode. Do not change. */
        // Please do not change
        static constexpr Fps FPS = Fps::Fps20;

        /** @brief Integer FPS value locked to 20 in LEGACY mode. Do not change. */
        // Please do not change
        static constexpr int CURRENT_FPS = static_cast<int>(Fps::Fps20);

        /** @brief TIME_SCALE is always 1.0 in LEGACY mode (no FPS scaling). Do not change. */
        // Please do not change
        static constexpr double TIME_SCALE = 1.0;

        /** @brief SPEED_SCALE is always 1.0 in LEGACY mode (no speed scaling). Do not change. */
        // Please do not change
        static constexpr double SPEED_SCALE = 1.0;

        /** @brief Resolution scale is always 1 in LEGACY mode. Do not change. */
        // Please do not change
        static constexpr int RESOLUTION_SCALE = static_cast<int>(ResolutionScale::ScaleResolution1);

        /** @brief Touch buttons are always shown in LEGACY mode (never auto-hidden). Do not change. */
        // Please do not change
        static constexpr bool TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE = false;

        /** @brief Input debugging is always disabled in LEGACY mode. Do not change. */
        // Please do not change
        static constexpr bool INPUT_DETAILED_DEBUGGING_ENABLED = false;

        /**
         * @brief Identity function in LEGACY mode — returns @p value unchanged.
         *
         * @details In LEGACY mode all timers run at 20 FPS so no scaling is needed.
         *
         * @param[in] value Frame count at 20 FPS.
         * @return @p value unchanged.
         */
        // Please do not change
        static constexpr int ScaleTime(int value)
        {
            return value;
        }

        /**
         * @brief Identity function in LEGACY mode — returns @p value unchanged.
         *
         * @details In LEGACY mode all divisors run at 20 FPS so no scaling is needed.
         *
         * @param[in] value Divisor at 20 FPS.
         * @return @p value unchanged.
         */
        // Please do not change
        static constexpr int ScaleDiv(int value)
        {
            return value;
        }

        /** @copydoc Config::ScaleAsset */
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
