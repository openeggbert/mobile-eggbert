/**
 * @file Slider.hpp
 * @brief Declarations for the Slider horizontal UI widget.
 * @details The Slider class renders a draggable thumb on a horizontal track
 *          and maps the touch/click position to a normalised value in [0, 1].
 *          It is used on the settings screen to control accelerometer sensitivity.
 */

#pragma once

#include "SharpRuntime/Prop.hpp"
#include "WindowsPhoneSpeedyBlupi/IPixmap.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @class Slider
     * @brief Horizontal UI slider widget for adjusting a continuous setting.
     *
     * @details Renders a draggable thumb (94 × 94 px icon) centred at the
     *          X position corresponding to the current @c value on a 248-pixel-wide
     *          track.  The usable track extent runs from @c topLeftCorner.X + 22
     *          to @c topLeftCorner.X + 226 (248 - 22).  Decorative icons are drawn
     *          to the left and right of the track to indicate the min/max extremes.
     *
     *          Move() maps an input point to a new value using linear interpolation
     *          over the track extent and clamps the result to [0, 1].  The caller
     *          receives @c true when the value actually changes.
     *
     * @note All positions are in HUD-space coordinates.
     * @note This class is UI code and does not directly affect gameplay state.
     */
    class Slider
    {
    public:
        /**
         * @brief Constructs a slider at the given position with an initial value.
         *
         * @param[in] topLeftCorner Top-left corner of the slider track in HUD-space.
         * @param[in] value         Initial normalised value in [0, 1].
         */
        Slider(TinyPoint topLeftCorner, double value);

        TinyPoint topLeftCorner; ///< @brief Top-left corner of the slider track in HUD-space.
        double    value;         ///< @brief Current normalised slider value in [0, 1].

        /**
         * @brief Returns the top-left corner of the slider track.
         * @return Current top-left corner in HUD-space.
         */
        [[nodiscard]] TinyPoint getTopLeftCornerProperty() const;

        /**
         * @brief Sets the top-left corner of the slider track.
         * @param[in] point New top-left corner in HUD-space.
         */
        void setTopLeftCornerProperty(TinyPoint point);

        DDATA(double, Value) ///< @brief Property accessors (getValueProperty / setValueProperty) for the normalised value.

    private:
        /**
         * @brief Returns the X coordinate of the left end of the usable track.
         *
         * @details Computed as @c topLeftCorner.X + 22.
         *
         * @return Left track boundary in HUD-space pixels.
         */
        [[nodiscard]] intcs getPosLeftProperty() const;

        /**
         * @brief Returns the X coordinate of the right end of the usable track.
         *
         * @details Computed as @c topLeftCorner.X + 248 - 22.
         *
         * @return Right track boundary in HUD-space pixels.
         */
        [[nodiscard]] intcs getPosRightProperty() const;

        /*
                NeoSdk::Property<byte> SelectedGamer;
                SelectedGamer( [this]() { return data[2]; } , [this](byte value) {data[2] = value; }),
        */

    public:
        /**
         * @brief Draws the slider track, thumb, and decorative end icons.
         *
         * @details Renders in three parts:
         *          1. The 124 × 22-pixel gauge-bar background at the track position
         *             (PixmapChannel::Jauge, zoom 2.0).
         *          2. A 94 × 94-pixel pad icon (PixmapChannel::Pad, icon 1) centred
         *             on the thumb X = @c getPosLeftProperty() + value * trackWidth.
         *          3. Decorative left (Element icon 37) and right (Element icon 38)
         *             sensitivity indicator icons flanking the track.
         *
         * @param[in,out] pixmap Rendering subsystem.
         */
        void Draw(IPixmap& pixmap);

        /**
         * @brief Handles a drag/click event and updates the slider value.
         *
         * @details Checks whether @p pos falls within a generous hit rectangle
         *          (track bounds expanded by 50 pixels in each direction).  If so,
         *          maps @p pos.X linearly to [0, 1] over the usable track extent
         *          and clamps the result.  Returns @c true only when the value
         *          actually changes.
         *
         * @param[in] pos Touch or cursor position in HUD-space.
         * @return @c true if the value changed; @c false if the point was outside
         *         the hit region or the value was unchanged.
         */
        bool Move(TinyPoint pos);
    };
}
