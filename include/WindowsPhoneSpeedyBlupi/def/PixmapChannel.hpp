/**
 * @file PixmapChannel.hpp
 * @brief Defines the PixmapChannel enumeration identifying sprite-sheet (texture atlas) slots in the rendering subsystem.
 *
 * @details Each value maps to one Texture2D loaded by Pixmap::LoadContent(). Values mirror the
 * CH* integer constants in Def for backward compatibility. Gaps exist at indices 7 and 8 (unused).
 * Do not use channel values as raw array indices without verifying Pixmap::GetBitmap().
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using PixmapChannelUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief Identifies which sprite sheet (texture atlas) to use for a draw call.
     *
     * Each channel corresponds to one Texture2D loaded by Pixmap::LoadContent().
     * The Pixmap class maps channel values to the appropriate bitmap and applies
     * the correct icon grid layout (icon size, gap, grid dimensions) per channel.
     *
     * These constants mirror the CH* integer constants defined in Def for backward
     * compatibility. When adding new sprite sheets, add a corresponding entry here
     * and update Pixmap::GetBitmap() and the icon-grid parameters.
     *
     * @note Do not treat channel values as array indices without verifying the
     *       mapping in Pixmap::GetBitmap(). Gaps exist (e.g., 7 and 8 are unused).
     * @note This is rendering/resource code. Channel values must not be used to
     *       drive gameplay logic.
     */
    enum class PixmapChannel : PixmapChannelUnderlying
    {
        PixmapChannel0          = 0,  ///< @brief Reserved / unused channel 0.
        Object                  = 1,  ///< @brief Moving objects sprite sheet (bitmapObject).
        Blupi                   = 2,  ///< @brief Main Blupi character sprite sheet (bitmapBlupi).
        Background              = 3,  ///< @brief World/decor background tiles (bitmapBackground via BackgroundCache).
        Button                  = 4,  ///< @brief UI buttons sprite sheet (bitmapButton).
        Jauge                   = 5,  ///< @brief HUD gauge/status bar sprite sheet (bitmapJauge).
        Text                    = 6,  ///< @brief Font/text glyph sprite sheet (bitmapText).
        Explosion               = 9,  ///< @brief Explosion particle sprite sheet (bitmapExplo).
        Element                 = 10, ///< @brief Collectible elements sprite sheet (bitmapElement).
        Blupi1_11               = 11, ///< @brief Alternate Blupi sprite sheet variant 1 (bitmapBlupi1).
        Blupi1_12               = 12, ///< @brief Alternate Blupi sprite sheet variant 2 (bitmapBlupi1).
        Blupi1_13               = 13, ///< @brief Alternate Blupi sprite sheet variant 3 (bitmapBlupi1).
        Pad                     = 14, ///< @brief Touch-input pad overlay sprite sheet (bitmapPad).
        SpeedyBlupiBackground   = 15, ///< @brief Speedy Blupi title/background (bitmapSpeedyBlupi).
        BlupiYoupieBackground   = 16, ///< @brief Blupi Youpie background (bitmapBlupiYoupie).
        GearBackground          = 17  ///< @brief Gear/settings background (bitmapGear).

    };

    /**
     * @brief Returns the raw underlying byte value of a PixmapChannel.
     * @param[in] type The channel to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(PixmapChannel type) -> PixmapChannelUnderlying
    {
        return static_cast<PixmapChannelUnderlying>(type);
    }

    /**
     * @brief Converts an integer to a PixmapChannel enum value.
     *
     * Used when reading channel indices from data tables or original game code.
     * The caller is responsible for ensuring @p value maps to a valid channel.
     *
     * @param[in] value Raw integer channel index (valid values: 0-6, 9-17; 7 and 8 are unused).
     * @return Corresponding PixmapChannel enum value.
     */
    static constexpr auto ToPixmapChannel(const int value) -> PixmapChannel
    {
        return static_cast<PixmapChannel>(
            static_cast<PixmapChannelUnderlying>(value)
        );
    }
    //
    // constexpr bool operator<(const PixmapChannel lhs, const PixmapChannel rhs) noexcept
    // {
    //     return ToRaw(lhs) < ToRaw(rhs);
    // }
    //
    // constexpr bool operator>(const PixmapChannel lhs, const PixmapChannel rhs) noexcept
    // {
    //     return ToRaw(lhs) > ToRaw(rhs);
    // }
    //
    // constexpr bool operator<=(const PixmapChannel lhs, const PixmapChannel rhs) noexcept
    // {
    //     return ToRaw(lhs) <= ToRaw(rhs);
    // }
    //
    // constexpr bool operator>=(const PixmapChannel lhs, const PixmapChannel rhs) noexcept
    // {
    //     return ToRaw(lhs) >= ToRaw(rhs);
    // }
}
