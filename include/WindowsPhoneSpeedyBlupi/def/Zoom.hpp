/**
 * @file Zoom.hpp
 * @brief Defines the ZoomCheat enumeration for the MODERN-only debug zoom cheat.
 *
 * @details Available only when the MODERN preprocessor define is active. The zoom level
 * controls the render scale of the world, allowing the visible area to be scaled down
 * for debugging purposes.
 */

#pragma once

#ifdef MODERN
namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Zoom levels for the MODERN-only zoom cheat.
     *
     * @details The value represents the render scale of the world viewport.
     * Lower scale values show a wider view of the level at the cost of visual size.
     * This enum is only compiled in MODERN builds and is not part of the original game.
     *
     * @note Only available in MODERN builds. Not present in LEGACY mode.
     */
    enum class ZoomCheat
    {
        Zoom100,  ///< @brief 100% render scale (normal view, 640x480 logical area visible).
        Zoom50,   ///< @brief 50% render scale (visible width approximately 2x wider).
        Zoom25,   ///< @brief 25% render scale (visible width approximately 4x wider).
        Zoom12    ///< @brief 12.5% render scale (visible width approximately 8x wider).
    };
}
#endif
