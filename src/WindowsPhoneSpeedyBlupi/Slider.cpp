//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Slider.h"

#include "WindowsPhoneSpeedyBlupi/Pixmap.h"
#include "WindowsPhoneSpeedyBlupi/Misc.h"


// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Slider


namespace WindowsPhoneSpeedyBlupi
{

    TinyPoint Slider::getTopLeftCorner() const { return topLeftCorner ; }
    idata(double, Value, Slider)
    int Slider::getPosLeft() const { return getTopLeftCorner().X + 22; }
    int Slider::getPosRight() const { return getTopLeftCorner().X + 248 - 22;; }

    Slider::Slider(WindowsPhoneSpeedyBlupi::TinyPoint topLeftCorner, double value)
    {
        this->topLeftCorner = topLeftCorner;
        this->setValue(value);//to be checked
    }

    void Slider::Draw(Pixmap& pixmap) {
        TinyPoint tinyPoint;
        tinyPoint.X = getTopLeftCorner().X - pixmap.getOrigin().X;
        tinyPoint.Y = getTopLeftCorner().Y - pixmap.getOrigin().Y;
        TinyPoint dest = tinyPoint;
        TinyRect tinyRect;
        tinyRect.LeftX = 0;
        tinyRect.RightX = 124;
        tinyRect.TopY = 0;
        tinyRect.BottomY = 22;
        TinyRect rect = tinyRect;
        pixmap.DrawPart(5, dest, rect, 2.0);
        int num = (int)((double)(getPosRight() - getPosLeft()) * getValue());
        int num2 = getTopLeftCorner().Y + 22;
        int num3 = 94;
        TinyRect tinyRect2;
        tinyRect2.LeftX = getPosLeft() + num - num3 / 2;
        tinyRect2.RightX = getPosLeft() + num + num3 / 2;
        tinyRect2.TopY = num2 - num3 / 2;
        tinyRect2.BottomY = num2 + num3 / 2;
        rect = tinyRect2;
        pixmap.DrawIcon(14, 1, rect, 1.0, false);
        TinyRect tinyRect3;
        tinyRect3.LeftX = getTopLeftCorner().X - 65;
        tinyRect3.RightX = getTopLeftCorner().X - 65 + 60;
        tinyRect3.TopY = getTopLeftCorner().Y - 10;
        tinyRect3.BottomY = getTopLeftCorner().Y - 10 + 60;
        rect = tinyRect3;
        pixmap.DrawIcon(10, 37, rect, 1.0, false);
        TinyRect tinyRect4;
        tinyRect4.LeftX = getTopLeftCorner().X + 248 + 5;
        tinyRect4.RightX = getTopLeftCorner().X + 248 + 5 + 60;
        tinyRect4.TopY = getTopLeftCorner().Y - 10;
        tinyRect4.BottomY = getTopLeftCorner().Y - 10 + 60;
        rect = tinyRect4;
        pixmap.DrawIcon(10, 38, rect, 1.0, false);
    }

        bool Slider::Move(TinyPoint& pos)
        {
            TinyRect tinyRect;
            tinyRect.LeftX = getTopLeftCorner().X - 50;
            tinyRect.RightX = getTopLeftCorner().X + 248 + 50;
            tinyRect.TopY = getTopLeftCorner().Y - 50;
            tinyRect.BottomY = getTopLeftCorner().Y + 44 + 50;
            TinyRect rect = tinyRect;
            if (Misc::IsInside(rect, pos))
            {
                double val = ((double)pos.X - (double)getPosLeft()) / (double)(getPosRight() - getPosLeft());
                val = std::max(val, 0.0);
                val = std::min(val, 1.0);
                if (getValue() != val)
                {
                    setValue(val);
                    return true;
                }
            }
            return false;
        }


}