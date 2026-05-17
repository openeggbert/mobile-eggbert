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
        PixmapChannel0          = 0,  ///< Reserved / unused channel 0.
        Object                  = 1,  ///< Moving objects sprite sheet (bitmapObject).
        Blupi                   = 2,  ///< Main Blupi character sprite sheet (bitmapBlupi).
        Background              = 3,  ///< World/decor background tiles (bitmapBackground via BackgroundCache).
        Button                  = 4,  ///< UI buttons sprite sheet (bitmapButton).
        Jauge                   = 5,  ///< HUD gauge/status bar sprite sheet (bitmapJauge).
        Text                    = 6,  ///< Font/text glyph sprite sheet (bitmapText).
        Explosion               = 9,  ///< Explosion particle sprite sheet (bitmapExplo).
        Element                 = 10, ///< Collectible elements sprite sheet (bitmapElement).
        Blupi1_11               = 11, ///< Alternate Blupi sprite sheet variant 1 (bitmapBlupi1).
        Blupi1_12               = 12, ///< Alternate Blupi sprite sheet variant 2 (bitmapBlupi1).
        Blupi1_13               = 13, ///< Alternate Blupi sprite sheet variant 3 (bitmapBlupi1).
        Pad                     = 14, ///< Touch-input pad overlay sprite sheet (bitmapPad).
        SpeedyBlupiBackground   = 15, ///< Speedy Blupi title/background (bitmapSpeedyBlupi).
        BlupiYoupieBackground   = 16, ///< Blupi Youpie background (bitmapBlupiYoupie).
        GearBackground          = 17  ///< Gear/settings background (bitmapGear).

    };

    /**
     * @brief Returns the raw underlying byte value of a PixmapChannel.
     * @param type The channel to convert.
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
     * @param value Raw integer channel index.
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
