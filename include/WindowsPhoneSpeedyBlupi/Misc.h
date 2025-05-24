//
// Created by robertvokac on 5/24/25.
//

#ifndef MISC_H
#define MISC_H
#include "TinyPoint.h"
#include "TinyRect.h"
#include "Microsoft/Xna/Framework/Rectangle.h"


namespace WindowsPhoneSpeedyBlupi {

using WindowsPhoneSpeedyBlupi::TinyPoint;
//static class
    class Misc
    {
    public:
        static Microsoft::Xna::Framework::Rectangle RotateAdjust(const Microsoft::Xna::Framework::Rectangle& rect, const double &angle);

        static double DegToRad(const double &angle);

        static TinyPoint RotatePointRad(const TinyPoint& center, const double& angle, const TinyPoint& point);

         static TinyPoint RotatePointRad(const double& angle, const TinyPoint& p);





         static int Approach(int actual, int& final, int& step);

         static int Speed(const double& speed, const int& max);

         static TinyRect Inflate(const TinyRect& rect, const int& value);

         static bool IsInside(const TinyRect& rect, const TinyPoint& p);

        static bool IntersectRect(TinyRect& dst, const TinyRect& src1, const TinyRect& src2);

        static bool UnionRect(TinyRect& dst, const TinyRect& src1, const TinyRect& src2);

    private:
        static bool IsRectEmpty(const TinyRect& rect);

    };

}

#endif //MISC_H
