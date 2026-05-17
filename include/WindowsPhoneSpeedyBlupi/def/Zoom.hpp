#pragma once

#ifdef MODERN
namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Zoom levels for the MODERN-only zoom cheat.
     *
     * The value represents the render scale of the world:
     * Zoom100 = 100% scale (normal view)
     * Zoom50  = 50% scale  (visible width ~2x)
     * Zoom25  = 25% scale  (visible width ~4x)
     * Zoom12  = 12.5% scale (visible width ~8x)
     */
    enum class ZoomCheat
    {
        Zoom100,
        Zoom50,
        Zoom25,
        Zoom12
    };
}
#endif
