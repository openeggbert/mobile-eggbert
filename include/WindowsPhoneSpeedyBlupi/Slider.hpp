#pragma once

#include "SharpRuntime/Prop.hpp"
#include "WindowsPhoneSpeedyBlupi/IPixmap.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief A horizontal slider UI widget used for adjusting continuous settings.
     *
     * Slider renders a draggable thumb on a horizontal track and maps touch/click
     * position to a normalized value in [0,1]. Used in the setup screen for the
     * accelerometer sensitivity control.
     *
     * @note All positions are in HUD-space coordinates.
     * @note This is UI code — it does not affect gameplay state directly.
     */
    class Slider
    {
    public:
        /**
         * @brief Constructs a slider at the given position with an initial value.
         * @param topLeftCorner Top-left corner of the slider track in HUD-space.
         * @param value Initial value in [0,1].
         */
        Slider(TinyPoint topLeftCorner, double value);

        /** Top-left corner of the slider track in HUD-space. */
        TinyPoint topLeftCorner;

        /** Current slider value in [0,1]. */
        double value;

        [[nodiscard]] TinyPoint getTopLeftCornerProperty() const;
        void setTopLeftCornerProperty(TinyPoint point);
        DDATA(double, Value)

    private:
        /** X coordinate of the left end of the slider track in HUD-space. */
        [[nodiscard]] intcs getPosLeftProperty() const;
        /** X coordinate of the right end of the slider track in HUD-space. */
        [[nodiscard]] intcs getPosRightProperty() const;

        /*
                NeoSdk::Property<byte> SelectedGamer;
                SelectedGamer( [this]() { return data[2]; } , [this](byte value) {data[2] = value; }),
        */

    public:
        void Draw(IPixmap& pixmap);

        bool Move(TinyPoint pos);
    };
}
