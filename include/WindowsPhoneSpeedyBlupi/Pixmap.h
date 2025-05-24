//
// Created by robertvokac on 5/24/25.
//

#ifndef PIXMAP_H
#define PIXMAP_H
#include "WindowsPhoneSpeedyBlupi/TinyPoint.h"
#include "WindowsPhoneSpeedyBlupi/TinyRect.h"


class Pixmap {
    //todo
public:
    WindowsPhoneSpeedyBlupi::TinyPoint Origin;

    void DrawPart(int i, WindowsPhoneSpeedyBlupi::TinyPoint dest, const WindowsPhoneSpeedyBlupi::TinyRect & rect, double x);

    void DrawIcon(int i, int i1, const WindowsPhoneSpeedyBlupi::TinyRect & rect, double x, bool cond);
};



#endif //PIXMAP_H
