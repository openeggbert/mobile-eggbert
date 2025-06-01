//
// Created by robertvokac on 5/25/25.
//

#ifndef INPUTPAD_H
#define INPUTPAD_H

#include "Microsoft/Devices/Sensors/Accelerometer.h"
#include "Microsoft/Devices/Sensors/AccelerometerReading.h"
#include "WindowsPhoneSpeedyBlupi/Decor.h"
#include "WindowsPhoneSpeedyBlupi/PixmapI.h"
#include "WindowsPhoneSpeedyBlupi/Slider.h"
#include "WindowsPhoneSpeedyBlupi/SoundI.h"

#define VECTOR_CONTAINS(vector, element) count( vector .begin(), vector. end(), element );

namespace WindowsPhoneSpeedyBlupi {
    class Game1I;

    class InputPad
    {
    private:
        static const int padRadius = 140;

         mutable Game1I* game1;

         mutable Decor decor;

         mutable PixmapI* pixmap;

         mutable SoundI* sound;

         mutable GameData gameData;

         mutable std::vector<Def::ButtonGlyph> pressedGlyphs;

         mutable Microsoft::Devices::Sensors::Accelerometer accelSensor;

         mutable Slider accelSlider;

         bool padPressed = false;

         bool showCheatMenu = false;

         TinyPoint padTouchPos;

         Def::ButtonGlyph lastButtonDown;

        mutable Def::ButtonGlyph buttonPressed;

         int touchOrClickCount = 0;

         bool accelStarted = false;;

         bool accelActive = false;

         double accelSpeedX = 0.0f;

         bool accelLastState = false;

         bool accelWaitZero = false;

         int mission = 0;

    public:

        ddata(Def::Phase, Phase)
        ddata(int, SelectedGamer)
        ddata(TinyPoint, PixmapOrigin)
        public: [[nodiscard]] int getTotalTouch() const;
        public: [[nodiscard]] Def::ButtonGlyph getButtonPressed() const;
        ddata(bool , ShowCheatMenu)
        public: [[nodiscard]] std::vector<Def::ButtonGlyph> getButtonGlyphs() const;
        // Returns the point of the center of the pad on the screen.
        public: [[nodiscard]] TinyPoint getPadCenter() const;

    public:
        InputPad(Game1I* game1, Decor& decor, PixmapI* pixmap, SoundI* sound, GameData& gameData);

        void StartMission(int mission);

        void Update();


    private: Def::ButtonGlyph ButtonDetect(TinyPoint touchOrClick);

    public: void Draw();

    private: TinyRect GetPadBounds(TinyPoint center, int radius);

    public: TinyRect GetButtonRect(Def::ButtonGlyph glyph);

        private: void StartAccel();

        private: void StopAccel();


        private: void HandleAccelSensorCurrentValueChanged(Microsoft::Devices::Sensors::SensorReadingEventArgs<Microsoft::Devices::Sensors::AccelerometerReading> e);
    };

}


#endif //INPUTPAD_H
