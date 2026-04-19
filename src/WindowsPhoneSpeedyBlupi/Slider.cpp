//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Slider.hpp"
#include "WindowsPhoneSpeedyBlupi/Misc.hpp"


// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Slider


namespace WindowsPhoneSpeedyBlupi
{

    TinyPoint Slider::getTopLeftCorner() const { return topLeftCorner ; }
    IDATA(double, Value, Slider)
    int Slider::getPosLeft() const { return getTopLeftCorner().X + 22; }
    int Slider::getPosRight() const { return getTopLeftCorner().X + 248 - 22;; }

    Slider::Slider(WindowsPhoneSpeedyBlupi::TinyPoint topLeftCorner, double value)
    {
        this->topLeftCorner = topLeftCorner;
        this->setValueProperty(value);//to be checked
    }

    void Slider::Draw(PixmapI* pixmap) {
        TinyPoint tinyPoint;
        tinyPoint.X = getTopLeftCorner().X - pixmap->getOriginProperty().X;
        tinyPoint.Y = getTopLeftCorner().Y - pixmap->getOriginProperty().Y;
        TinyPoint dest = tinyPoint;
        TinyRect tinyRect;
        tinyRect.Left = 0;
        tinyRect.Right = 124;
        tinyRect.Top = 0;
        tinyRect.Bottom = 22;
        TinyRect rect = tinyRect;
        pixmap->DrawPart(5, dest, rect, 2.0);
        int num = (int)((double)(getPosRight() - getPosLeft()) * getValueProperty());
        int num2 = getTopLeftCorner().Y + 22;
        int num3 = 94;
        TinyRect tinyRect2;
        tinyRect2.Left = getPosLeft() + num - num3 / 2;
        tinyRect2.Right = getPosLeft() + num + num3 / 2;
        tinyRect2.Top = num2 - num3 / 2;
        tinyRect2.Bottom = num2 + num3 / 2;
        rect = tinyRect2;
        pixmap->DrawIcon(14, 1, rect, 1.0, false);
        TinyRect tinyRect3;
        tinyRect3.Left = getTopLeftCorner().X - 65;
        tinyRect3.Right = getTopLeftCorner().X - 65 + 60;
        tinyRect3.Top = getTopLeftCorner().Y - 10;
        tinyRect3.Bottom = getTopLeftCorner().Y - 10 + 60;
        rect = tinyRect3;
        pixmap->DrawIcon(10, 37, rect, 1.0, false);
        TinyRect tinyRect4;
        tinyRect4.Left = getTopLeftCorner().X + 248 + 5;
        tinyRect4.Right = getTopLeftCorner().X + 248 + 5 + 60;
        tinyRect4.Top = getTopLeftCorner().Y - 10;
        tinyRect4.Bottom = getTopLeftCorner().Y - 10 + 60;
        rect = tinyRect4;
        pixmap->DrawIcon(10, 38, rect, 1.0, false);
    }

        bool Slider::Move(TinyPoint& pos)
        {
            TinyRect tinyRect;
            tinyRect.Left = getTopLeftCorner().X - 50;
            tinyRect.Right = getTopLeftCorner().X + 248 + 50;
            tinyRect.Top = getTopLeftCorner().Y - 50;
            tinyRect.Bottom = getTopLeftCorner().Y + 44 + 50;
            TinyRect rect = tinyRect;
            if (Misc::IsInside(rect, pos))
            {
                double val = ((double)pos.X - (double)getPosLeft()) / (double)(getPosRight() - getPosLeft());
                val = std::max(val, 0.0);
                val = std::min(val, 1.0);
                if (getValueProperty() != val)
                {
                    setValueProperty(val);
                    return true;
                }
            }
            return false;
        }


}