/**
 * @file Slider.cpp
 * @brief Implementations of the Slider horizontal UI widget.
 * @details See Slider.hpp for the full API contract.
 *
 * ### Drag-to-value mapping (Move)
 * The usable track runs from @c getPosLeftProperty() to @c getPosRightProperty()
 * (a span of 248 - 44 = 204 pixels).  Move() converts an input X coordinate
 * to a normalised value with the formula:
 * @code
 *   val = (pos.X - posLeft) / (posRight - posLeft)
 * @endcode
 * The result is then clamped to [0.0, 1.0] via Math::Max / Math::Min, so
 * touches outside the track extent are still accepted (within the 50-pixel
 * margin of the hit rectangle) but map to the nearest extreme (0 or 1).
 * The method returns @c false without updating the value when the touch falls
 * entirely outside the hit rectangle, or when the computed value is identical
 * to the current value (floating-point equality).
 */

#include "WindowsPhoneSpeedyBlupi/Slider.hpp"

#include "System/Math.hpp"
#include "WindowsPhoneSpeedyBlupi/Misc.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    Slider::Slider(TinyPoint topLeftCorner, double value)
    {
        this->setTopLeftCornerProperty(topLeftCorner);
        // Decompiled C# shows "value = Value", which is likely reversed by decompilation.
        // Intentionally initializing Value from constructor parameter.
        // value = Value;
        this->setValueProperty(value);
    }

    TinyPoint Slider::getTopLeftCornerProperty() const { return topLeftCorner; }

    void Slider::setTopLeftCornerProperty(TinyPoint point)
    {
        this->topLeftCorner = point;
    }

    IDATA(double, Value, Slider)
    intcs Slider::getPosLeftProperty() const { return getTopLeftCornerProperty().X + 22; }

    intcs Slider::getPosRightProperty() const
    {
        return getTopLeftCornerProperty().X + 248 - 22;;
    }


    void Slider::Draw(IPixmap& pixmap)
    {
        TinyPoint tinyPoint{};
        tinyPoint.X = getTopLeftCornerProperty().X - pixmap.getOriginProperty().X;
        tinyPoint.Y = getTopLeftCornerProperty().Y - pixmap.getOriginProperty().Y;
        TinyPoint dest = tinyPoint;
        TinyRect tinyRect;
        tinyRect.Left = 0;
        tinyRect.Right = 124;
        tinyRect.Top = 0;
        tinyRect.Bottom = 22;
        TinyRect rect = tinyRect;
        pixmap.DrawPart(PixmapChannel::Jauge, dest, rect, 2.0);
        intcs num = (intcs)((double)(getPosRightProperty() - getPosLeftProperty()) * getValueProperty());
        intcs num2 = getTopLeftCornerProperty().Y + 22;
        intcs num3 = 94;
        TinyRect tinyRect2{};
        tinyRect2.Left = getPosLeftProperty() + num - num3 / 2;
        tinyRect2.Right = getPosLeftProperty() + num + num3 / 2;
        tinyRect2.Top = num2 - num3 / 2;
        tinyRect2.Bottom = num2 + num3 / 2;
        rect = tinyRect2;
        pixmap.DrawIcon(PixmapChannel::Pad, 1, rect, 1.0, false);
        TinyRect tinyRect3{};
        tinyRect3.Left = getTopLeftCornerProperty().X - 65;
        tinyRect3.Right = getTopLeftCornerProperty().X - 65 + 60;
        tinyRect3.Top = getTopLeftCornerProperty().Y - 10;
        tinyRect3.Bottom = getTopLeftCornerProperty().Y - 10 + 60;
        rect = tinyRect3;
        pixmap.DrawIcon(PixmapChannel::Element, 37, rect, 1.0, false);
        TinyRect tinyRect4{};
        tinyRect4.Left = getTopLeftCornerProperty().X + 248 + 5;
        tinyRect4.Right = getTopLeftCornerProperty().X + 248 + 5 + 60;
        tinyRect4.Top = getTopLeftCornerProperty().Y - 10;
        tinyRect4.Bottom = getTopLeftCornerProperty().Y - 10 + 60;
        rect = tinyRect4;
        pixmap.DrawIcon(PixmapChannel::Element, 38, rect, 1.0, false);
    }

    bool Slider::Move(TinyPoint pos)
    {
        TinyRect tinyRect{};
        tinyRect.Left = getTopLeftCornerProperty().X - 50;
        tinyRect.Right = getTopLeftCornerProperty().X + 248 + 50;
        tinyRect.Top = getTopLeftCornerProperty().Y - 50;
        tinyRect.Bottom = getTopLeftCornerProperty().Y + 44 + 50;
        TinyRect rect = tinyRect;
        if (Misc::IsInside(rect, pos))
        {
            double val = ((double)pos.X - (double)getPosLeftProperty()) / (double)(getPosRightProperty() -
                getPosLeftProperty());
            val = System::Math::Max(val, 0.0);
            val = System::Math::Min(val, 1.0);
            if (getValueProperty() != val)
            {
                setValueProperty(val);
                return true;
            }
        }
        return false;
    }
}