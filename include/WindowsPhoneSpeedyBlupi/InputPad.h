//
// Created by robertvokac on 5/25/25.
//

#ifndef INPUTPAD_H
#define INPUTPAD_H

#include "Microsoft/Devices/Sensors/Accelerometer.h"
#include "Microsoft/Devices/Sensors/AccelerometerFailedException.h"
#include "Microsoft/Devices/Sensors/AccelerometerReading.h"
#include "Microsoft/Xna/Framework/Input/Keys.h"
#include "Microsoft/Xna/Framework/Input/Keyboard.h"
#include "Microsoft/Xna/Framework/Input/KeyboardState.h"
#include "Microsoft/Xna/Framework/Input/Mouse.h"
#include "Microsoft/Xna/Framework/Input/MouseState.h"
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.h"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.h"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.h"
#include "WindowsPhoneSpeedyBlupi/DDebug.h"
#include "WindowsPhoneSpeedyBlupi/Decor.h"
#include "WindowsPhoneSpeedyBlupi/Game1.h"
#include "WindowsPhoneSpeedyBlupi/Pixmap.h"
#include "WindowsPhoneSpeedyBlupi/Slider.h"
#include "WindowsPhoneSpeedyBlupi/Sound.h"
#include "NeoSdk/Property.h"
#include "System/UnauthorizedAccessException.h"
#define VECTOR_CONTAINS(vector, element) count( vector .begin(), vector. end(), element );


namespace WindowsPhoneSpeedyBlupi {
class InputPad
    {
    private:
        static const int padRadius = 140;

         const Game1 game1;

         const Decor decor;

         mutable Pixmap pixmap;

         const Sound sound;

         mutable GameData gameData;

         mutable std::vector<Def::ButtonGlyph> pressedGlyphs;

         mutable Accelerometer accelSensor;

         mutable Slider accelSlider;

         bool padPressed = false;

         bool showCheatMenu = false;

         TinyPoint padTouchPos;

         Def::ButtonGlyph lastButtonDown;

         Def::ButtonGlyph buttonPressed;

         int touchCount = 0;

         bool accelStarted = false;;

         bool accelActive = false;

         double accelSpeedX = 0.0f;

         bool accelLastState = false;

         bool accelWaitZero = false;

         int mission = 0;

    public:

        DEF_PROP_AUTO(Def::Phase, Phase, Def::Phase::NonePhase)
        DEF_PROP_AUTO(int, SelectedGamer, 0)
        DEF_PROP_AUTO(TinyPoint, PixmapOrigin, TinyPoint())
        DEF_PROP_CUSTOM(int, TotalTouch)
        DEF_PROP_CUSTOM(Def::ButtonGlyph , ButtonPressed)
        DEF_PROP_CUSTOM(bool , ShowCheatMenu)
    private:
        DEF_PROP_CUSTOM(std::vector<Def::ButtonGlyph>, ButtonGlyphs)
        // Returns the point of the center of the pad on the screen.
        DEF_PROP_CUSTOM(TinyPoint, PadCenter)

    public:
        InputPad(Game1& game1, Decor& decor, Pixmap& pixmap, Sound& sound, GameData& gameData);

        void StartMission(int mission);

        void Update();


    private: Def::ButtonGlyph ButtonDetect(TinyPoint touchOrClick);

    public: void Draw();

    private: TinyRect GetPadBounds(TinyPoint center, int radius);

    public: TinyRect GetButtonRect(Def::ButtonGlyph glyph);

        private: void StartAccel();

        private: void StopAccel();


        private: void HandleAccelSensorCurrentValueChanged(Microsoft::Devices::Sensors::SensorReadingEventArgs<AccelerometerReading> e);
    };

}


#endif //INPUTPAD_H
