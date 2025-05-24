//
// Created by robertvokac on 5/24/25.
//

#include "../../include/WindowsPhoneSpeedyBlupi/Slider.h"

#include "../../include/WindowsPhoneSpeedyBlupi/Pixmap.h"
#include "WindowsPhoneSpeedyBlupi/Misc.h"


// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Slider


namespace WindowsPhoneSpeedyBlupi
{

    WindowsPhoneSpeedyBlupi::TinyPoint topLeftCorner;
    double value;

    Slider::Slider(WindowsPhoneSpeedyBlupi::TinyPoint topLeftCorner, double value) :
    TopLeftCorner( [&topLeftCorner]() { return topLeftCorner; } , [&topLeftCorner](TinyPoint value) {topLeftCorner = value; }),
    Value( [&value]() { return value; } , [&value](double value_) {value = value_; }),
    PosLeft( [this]() { return TopLeftCorner.get().X + 22; }),
    PosRight( [this]() { return TopLeftCorner.get().X + 248 - 22;; })
    {
        this->TopLeftCorner = topLeftCorner;
        this->Value = value;//to be checked

    }


    void Slider::Draw(Pixmap& pixmap) {
        TinyPoint tinyPoint;
        tinyPoint.X = TopLeftCorner.get().X - pixmap.Origin.X;
        tinyPoint.Y = TopLeftCorner.get().Y - pixmap.Origin.Y;
        TinyPoint dest = tinyPoint;
        TinyRect tinyRect;
        tinyRect.LeftX = 0;
        tinyRect.RightX = 124;
        tinyRect.TopY = 0;
        tinyRect.BottomY = 22;
        TinyRect rect = tinyRect;
        pixmap.DrawPart(5, dest, rect, 2.0);
        int num = (int)((double)(PosRight - PosLeft) * Value);
        int num2 = TopLeftCorner.get().Y + 22;
        int num3 = 94;
        TinyRect tinyRect2;
        tinyRect2.LeftX = PosLeft + num - num3 / 2;
        tinyRect2.RightX = PosLeft + num + num3 / 2;
        tinyRect2.TopY = num2 - num3 / 2;
        tinyRect2.BottomY = num2 + num3 / 2;
        rect = tinyRect2;
        pixmap.DrawIcon(14, 1, rect, 1.0, false);
        TinyRect tinyRect3;
        tinyRect3.LeftX = TopLeftCorner.get().X - 65;
        tinyRect3.RightX = TopLeftCorner.get().X - 65 + 60;
        tinyRect3.TopY = TopLeftCorner.get().Y - 10;
        tinyRect3.BottomY = TopLeftCorner.get().Y - 10 + 60;
        rect = tinyRect3;
        pixmap.DrawIcon(10, 37, rect, 1.0, false);
        TinyRect tinyRect4;
        tinyRect4.LeftX = TopLeftCorner.get().X + 248 + 5;
        tinyRect4.RightX = TopLeftCorner.get().X + 248 + 5 + 60;
        tinyRect4.TopY = TopLeftCorner.get().Y - 10;
        tinyRect4.BottomY = TopLeftCorner.get().Y - 10 + 60;
        rect = tinyRect4;
        pixmap.DrawIcon(10, 38, rect, 1.0, false);
    }

        bool Slider::Move(TinyPoint& pos)
        {
            TinyRect tinyRect;
            tinyRect.LeftX = TopLeftCorner.get().X - 50;
            tinyRect.RightX = TopLeftCorner.get().X + 248 + 50;
            tinyRect.TopY = TopLeftCorner.get().Y - 50;
            tinyRect.BottomY = TopLeftCorner.get().Y + 44 + 50;
            TinyRect rect = tinyRect;
            if (Misc::IsInside(rect, pos))
            {
                double val = ((double)pos.X - (double)PosLeft) / (double)(PosRight - PosLeft);
                val = std::max(val, 0.0);
                val = std::min(val, 1.0);
                if (Value != val)
                {
                    Value = val;
                    return true;
                }
            }
            return false;
        }


}