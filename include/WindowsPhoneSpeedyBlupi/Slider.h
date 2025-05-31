//
// Created by robertvokac on 5/24/25.
//

#ifndef SLIDER_H
#define SLIDER_H
#include "Pixmap.h"
#include "CNA/Prop.h"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.h"
#include "WindowsPhoneSpeedyBlupi/TinyRect.h"

namespace WindowsPhoneSpeedyBlupi {

    using byte = unsigned char;
    class Slider
    {

    public:NeoSdk::Property<WindowsPhoneSpeedyBlupi::TinyPoint> TopLeftCorner;
    public:NeoSdk::Property<double> Value;
    public:NeoSdk::Property<int> PosLeft;
    public:NeoSdk::Property<int> PosRight;

    public: Slider(WindowsPhoneSpeedyBlupi::TinyPoint topLeftCorner, double value) ;



/*
        NeoSdk::Property<byte> SelectedGamer;
        SelectedGamer( [this]() { return data[2]; } , [this](byte value) {data[2] = value; }),
*/


    public: void Draw(Pixmap& pixmap);

    public: bool Move(TinyPoint &pos);
    };
}

#endif //SLIDER_H
