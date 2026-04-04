//
// Created by robertvokac on 5/24/25.
//

#ifndef SLIDER_H
#define SLIDER_H
#include "WindowsPhoneSpeedyBlupi/PixmapI.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"

namespace WindowsPhoneSpeedyBlupi {

    class Slider
    {
    private: WindowsPhoneSpeedyBlupi::TinyPoint topLeftCorner;
    private: double value;

    public: [[nodiscard]] WindowsPhoneSpeedyBlupi::TinyPoint getTopLeftCorner() const;
    DDATA(double, Value)
    public: [[nodiscard]] int getPosLeft() const;
    public: [[nodiscard]] int getPosRight() const;

    public: Slider(WindowsPhoneSpeedyBlupi::TinyPoint topLeftCorner, double value) ;

/*
        NeoSdk::Property<byte> SelectedGamer;
        SelectedGamer( [this]() { return data[2]; } , [this](byte value) {data[2] = value; }),
*/


    public: void Draw(PixmapI* pixmap);

    public: bool Move(TinyPoint &pos);
    };
}

#endif //SLIDER_H
