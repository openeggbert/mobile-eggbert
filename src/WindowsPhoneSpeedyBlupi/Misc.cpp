//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Misc.h"
#include <cmath>

// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Misc
// using System;
// using System.Diagnostics;
// using Microsoft.Xna.Framework;
// using WindowsPhoneSpeedyBlupi;
// using static WindowsPhoneSpeedyBlupi.Def;


namespace WindowsPhoneSpeedyBlupi
{

    Microsoft::Xna::Framework::Rectangle Misc::RotateAdjust(const Microsoft::Xna::Framework::Rectangle& rect, const double &angle)
    {
        TinyPoint tinyPoint;
        tinyPoint.X = rect.Width / 2;
        tinyPoint.Y = rect.Height / 2;
        TinyPoint p = tinyPoint;
        TinyPoint tinyPoint2 = RotatePointRad(angle, p);
        int num = tinyPoint2.X - p.X;
        int num2 = tinyPoint2.Y - p.Y;
        return {rect.Left - num, rect.Top - num2, rect.Width, rect.Height};
    }
    double Misc::DegToRad(const double &angle)
    {
        return angle * M_PI / 180.0;
    }

    TinyPoint Misc::RotatePointRad(const TinyPoint& center, const double& angle, const TinyPoint& point)
    {
        TinyPoint tinyPoint;
        TinyPoint result;
        tinyPoint.X = point.X - center.X;
        tinyPoint.Y = point.Y - center.Y;
        double rad = DegToRad(angle);
        double num = sin(rad);
        double num2 = cos(rad);
        result.X = (int)((double)tinyPoint.X * num2 - (double)tinyPoint.Y * num);
        result.Y = (int)((double)tinyPoint.X * num + (double)tinyPoint.Y * num2);
        result.X += center.X;
        result.Y += center.Y;
        return result;
    }
    TinyPoint Misc::RotatePointRad(const double& angle, const TinyPoint& p)
    {
        return RotatePointRad(TinyPoint(), angle, p);
    }

    int Misc::Approach(int actual, const int& final, const int& step)
    {
        if (actual < final)
        {
            actual = std::min(actual + step, final);
        }
        else if (actual > final)
        {
            actual = std::max(actual - step, final);
        }
        return actual;

    }

    int Misc::Speed(const double& speed, const int& max)
    {
        if (speed > 0.0)
        {
            return std::max((int)(speed * (double)max), 1);
        }
        if (speed < 0.0)
        {
            return std::min((int)(speed * (double)max), -1);
        }
        return 0;
    }

    TinyRect Misc::Inflate(const TinyRect& rect, const int& value)
    {
        TinyRect result;
        result.LeftX = rect.LeftX - value;
        result.RightX = rect.RightX + value;
        result.TopY = rect.TopY - value;
        result.BottomY = rect.BottomY + value;
        return result;
    }

    bool Misc::IsInside(const TinyRect &rect, const TinyPoint &p) {
        return p.X >= rect.LeftX && p.X <= rect.RightX && p.Y >= rect.TopY && p.Y <= rect.BottomY;
    }

    bool Misc::IntersectRect(TinyRect& dst, const TinyRect& src1, const TinyRect& src2)
    {
        dst = TinyRect();
        dst.LeftX = std::max(src1.LeftX, src2.LeftX);
        dst.RightX = std::min(src1.RightX, src2.RightX);
        dst.TopY = std::max(src1.TopY, src2.TopY);
        dst.BottomY = std::min(src1.BottomY, src2.BottomY);
        return !IsRectEmpty(dst);
    }

    bool Misc::UnionRect(TinyRect& dst, const TinyRect& src1, const TinyRect& src2)
    {
        dst = TinyRect();
        dst.LeftX = std::min(src1.LeftX, src2.LeftX);
        dst.RightX = std::max(src1.RightX, src2.RightX);
        dst.TopY = std::min(src1.TopY, src2.TopY);
        dst.BottomY = std::max(src1.BottomY, src2.BottomY);
        return !IsRectEmpty(dst);
    }

    bool Misc::IsRectEmpty(const TinyRect& rect)
    {
        return rect.Width <= 0 || rect.Height <= 0;
    }

}
