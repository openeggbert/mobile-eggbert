//
// Created by robertvokac on 5/25/25.
//

#include "WindowsPhoneSpeedyBlupi/InputPad.h"

#include "CNA/Platform.h"
#include "Microsoft/Devices/Sensors/AccelerometerFailedException.h"
#include "Microsoft/Xna/Framework/Input/Keyboard.h"
#include "Microsoft/Xna/Framework/Input/KeyboardState.h"
#include "Microsoft/Xna/Framework/Input/Mouse.h"
#include "Microsoft/Xna/Framework/Input/MouseState.h"
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.h"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.h"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.h"
#include "System/UnauthorizedAccessException.h"
#include "WindowsPhoneSpeedyBlupi/DDebug.h"
#include "WindowsPhoneSpeedyBlupi/Game1I.h"
#include "WindowsPhoneSpeedyBlupi/Misc.h"


// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.InputPad
// using System;
// using System.Collections.Generic;
// using System.Diagnostics;
// using System.Linq;
// using Microsoft.Devices.Sensors;
// using Microsoft.Xna.Framework.Input;
// using Microsoft.Xna.Framework.Input.Touch;
// using static WindowsPhoneSpeedyBlupi.EnvClasses;

namespace WindowsPhoneSpeedyBlupi {
    /** Properties : Start */
    idata(Def::Phase, Phase, InputPad)
    idata(int, SelectedGamer, InputPad)
    idata(TinyPoint, PixmapOrigin, InputPad)
    int InputPad::getTotalTouch() const { return touchOrClickCount; }

    Def::ButtonGlyph InputPad::getButtonPressed() const {
        Def::ButtonGlyph result = buttonPressed;
        buttonPressed = Def::ButtonGlyph::NoneButtonGlyph;
        return result;
    }

    idata(bool, ShowCheatMenu, InputPad)
    std::vector<Def::ButtonGlyph> InputPad::getButtonGlyphs() const {
            std::vector<Def::ButtonGlyph> glyphs;
            switch (getPhase())
                {
                    case Def::Phase::Init:
                        glyphs.push_back(Def::ButtonGlyph::InitGamerA);
                        glyphs.push_back(Def::ButtonGlyph::InitGamerB);
                        glyphs.push_back(Def::ButtonGlyph::InitGamerC);
                        glyphs.push_back(Def::ButtonGlyph::InitSetup);
                        glyphs.push_back(Def::ButtonGlyph::InitPlay);
                        if (game1->getIsTrialMode())
                        {
                            glyphs.push_back(Def::ButtonGlyph::InitBuy);
                        }
                        if (game1->getIsRankingMode())
                        {
                            glyphs.push_back(Def::ButtonGlyph::InitRanking);
                        }
                        break;
                    case Def::Phase::Play:
                        glyphs.push_back(Def::ButtonGlyph::PlayPause);
                        glyphs.push_back(Def::ButtonGlyph::PlayAction);
                        glyphs.push_back(Def::ButtonGlyph::PlayJump);
                        if (accelStarted)
                        {
                            glyphs.push_back(Def::ButtonGlyph::PlayDown);
                        }
                        glyphs.push_back(Def::ButtonGlyph::Cheat11);
                        glyphs.push_back(Def::ButtonGlyph::Cheat12);
                        glyphs.push_back(Def::ButtonGlyph::Cheat21);
                        glyphs.push_back(Def::ButtonGlyph::Cheat22);
                        glyphs.push_back(Def::ButtonGlyph::Cheat31);
                        glyphs.push_back(Def::ButtonGlyph::Cheat32);
                        break;
                    case Def::Phase::Pause:
                        glyphs.push_back(Def::ButtonGlyph::PauseMenu);
                        if (mission != 1)
                        {
                            glyphs.push_back(Def::ButtonGlyph::PauseBack);
                        }
                        glyphs.push_back(Def::ButtonGlyph::PauseSetup);
                        if (mission != 1 && mission % 10 != 0)
                        {
                            glyphs.push_back(Def::ButtonGlyph::PauseRestart);
                        }
                        glyphs.push_back(Def::ButtonGlyph::PauseContinue);
                        break;
                    case Def::Phase::Resume:
                        glyphs.push_back(Def::ButtonGlyph::ResumeMenu);
                        glyphs.push_back(Def::ButtonGlyph::ResumeContinue);
                        break;
                    case Def::Phase::Lost:
                    case Def::Phase::Win:
                        glyphs.push_back(Def::ButtonGlyph::WinLostReturn);
                        break;
                    case Def::Phase::Trial:
                        glyphs.push_back(Def::ButtonGlyph::TrialBuy);
                        glyphs.push_back(Def::ButtonGlyph::TrialCancel);
                        break;
                    case Def::Phase::MainSetup:
                        glyphs.push_back(Def::ButtonGlyph::SetupSounds);
                        glyphs.push_back(Def::ButtonGlyph::SetupJump);
                        glyphs.push_back(Def::ButtonGlyph::SetupZoom);
                        glyphs.push_back(Def::ButtonGlyph::SetupAccel);
                        glyphs.push_back(Def::ButtonGlyph::SetupReset);
                        glyphs.push_back(Def::ButtonGlyph::SetupReturn);
                        break;
                    case Def::Phase::PlaySetup:
                        glyphs.push_back(Def::ButtonGlyph::SetupSounds);
                        glyphs.push_back(Def::ButtonGlyph::SetupJump);
                        glyphs.push_back(Def::ButtonGlyph::SetupZoom);
                        glyphs.push_back(Def::ButtonGlyph::SetupAccel);
                        glyphs.push_back(Def::ButtonGlyph::SetupReturn);
                        break;
                    case Def::Phase::Ranking:
                        glyphs.push_back(Def::ButtonGlyph::RankingContinue);
                        break;
                }
                if (showCheatMenu)
                {
                    glyphs.push_back(Def::ButtonGlyph::Cheat1);
                    glyphs.push_back(Def::ButtonGlyph::Cheat2);
                    glyphs.push_back(Def::ButtonGlyph::Cheat3);
                    glyphs.push_back(Def::ButtonGlyph::Cheat4);
                    glyphs.push_back(Def::ButtonGlyph::Cheat5);
                    glyphs.push_back(Def::ButtonGlyph::Cheat6);
                    glyphs.push_back(Def::ButtonGlyph::Cheat7);
                    glyphs.push_back(Def::ButtonGlyph::Cheat8);
                    glyphs.push_back(Def::ButtonGlyph::Cheat9);
                }
            return glyphs;
            }
    TinyPoint InputPad::getPadCenter() const
    {
        TinyRect drawBounds = pixmap->getDrawBounds();
        int x = gameData.getJumpRight() ? 100 : drawBounds.getWidth() - 100;
        return TinyPoint(x, drawBounds.getHeight() - 100);
    }
    /** Properties : End */

    InputPad::InputPad(Game1I* game1, Decor& decor, PixmapI* pixmap, SoundI* sound, GameData& gameData):
        game1(game1),
        decor(decor),
        pixmap(pixmap),
        sound(sound),
        gameData(gameData),
        accelSensor(Microsoft::Devices::Sensors::Accelerometer()),
        accelSlider(Slider(TinyPoint(320, 400), this->gameData.getAccelSensitivity()))
        {
            //IL_0037: Unknown result type (might be due to invalid IL or missing references)
            //IL_0041: Expected O, but got Unknown

            using Microsoft::Devices::Sensors::AccelerometerReading;
            using Microsoft::Devices::Sensors::SensorBase;
            accelSensor.CurrentValueChanged +=
                    [this](
                const Microsoft::Devices::Sensors::SensorReadingEventArgs<AccelerometerReading> &
                sensor_reading_event_args) {
                        HandleAccelSensorCurrentValueChanged(sensor_reading_event_args);
                    };

            lastButtonDown = Def::ButtonGlyph::NoneButtonGlyph;
            buttonPressed = Def::ButtonGlyph::NoneButtonGlyph;
        }

        void InputPad::StartMission(int mission)
        {
            this->mission = mission;
            accelWaitZero = true;
        }

        void InputPad::Update()
        {
            pressedGlyphs.clear();
            if (accelActive != gameData.getAccelActive())
            {
                accelActive = gameData.getAccelActive();
                if (accelActive)
                {
                    StartAccel();
                }
                else
                {
                    StopAccel();
                }
            }
            double horizontalChange = 0.0;
            double verticalChange = 0.0;
            int keyPress = 0;
            padPressed = false;
            Def::ButtonGlyph buttonGlyph = Def::ButtonGlyph::NoneButtonGlyph;

            Microsoft::Xna::Framework::Input::Touch::TouchCollection touches{};
            bool touchScreenIsSupported = true;
            if (touchScreenIsSupported)
            {
                using Microsoft::Xna::Framework::Input::Touch::TouchPanel;
                touches = TouchPanel::GetState();
                touchOrClickCount = touches.getCount();
            }

            std::vector<TinyPoint> touchesOrClicks;

            using Microsoft::Xna::Framework::Input::Touch::TouchLocation;
            using Microsoft::Xna::Framework::Input::Touch::TouchLocationState;
            if(touchScreenIsSupported) for (TouchLocation item : touches)
            {
                if (item.getState() == TouchLocationState::Pressed || item.getState() == TouchLocationState::Moved)
                {
                    TinyPoint touchPress = TinyPoint((int)item.getPosition().X, (int)item.getPosition().Y);
                    touchesOrClicks.push_back(touchPress);
                }
            }

            using Microsoft::Xna::Framework::Input::MouseState;
            using Microsoft::Xna::Framework::Input::Mouse;
            using Microsoft::Xna::Framework::Input::ButtonState;
            MouseState mouseState = Mouse::GetState();
            if (mouseState.getLeftButton() == ButtonState::Pressed)
            {
                touchOrClickCount++;
                TinyPoint mouseClick(mouseState.getX(), mouseState.getY());
                touchesOrClicks.push_back(mouseClick);
            }

            float screenWidth = game1->getGraphics().getGraphicsDevice().getViewport().getWidth();
            float screenHeight = game1->getGraphics().getGraphicsDevice().getViewport().getHeight();
            float screenRatio = screenWidth / screenHeight;

            if ((CNA::getCurrentPlatform() == CNA::Platform::Android && screenRatio > 1.3333333333333333) /*|| Env.IMPL.isKNI()*/)
            {
                for (int i = 0; i < touchesOrClicks.size(); i++)
                {

                    auto touchOrClick = touchesOrClicks[i];
                    if (touchOrClick.X == -1) continue;

                    float originalX = touchOrClick.X;
                    float originalY = touchOrClick.Y;

                    float widthHeightRatio = screenWidth / screenHeight;
                    float heightRatio = 480 / screenHeight;
                    float widthRatio = 640 / screenWidth;

                    {
                    WindowsPhoneSpeedyBlupi::DDebug::WriteLine("-----");
                    DDebug::WriteLine("originalX=" + std::to_string(originalX));
                    DDebug::WriteLine("originalY=" + std::to_string(originalY));
                    DDebug::WriteLine("heightRatio=" + std::to_string(heightRatio));
                    DDebug::WriteLine("widthRatio=" + std::to_string(widthRatio));
                    DDebug::WriteLine("widthHeightRatio=" + std::to_string(widthHeightRatio));
                    }
                    if (screenHeight> 480) {
                    touchOrClick.X = (int)(originalX * heightRatio);
                    touchOrClick.Y = (int)(originalY * heightRatio);
                    touchesOrClicks[i] = touchOrClick;
                    }

                    DDebug::WriteLine("new X" + touchOrClick.X);
                    DDebug::WriteLine("new Y" + touchOrClick.Y);
                }
            }
            using Microsoft::Xna::Framework::Input::KeyboardState;
            using namespace Microsoft::Xna::Framework::Input;
            KeyboardState newKeyboardState = Keyboard::GetState();


            Keys keysToBeChecked[] = { Keys::LeftControl, Keys::Up, Keys::Right, Keys::Down, Keys::Left, Keys::Space, Keys::Escape,};
            for(Keys keys : keysToBeChecked) {
                if (newKeyboardState.IsKeyDown(keys)) touchesOrClicks.push_back(TinyPoint(-1, (int)keys));
            }
            if (newKeyboardState.IsKeyDown(Keys::F11))
            {
                game1->ToggleFullScreen ();
                DDebug::WriteLine("F11 was pressed.");
            }

            bool keyPressedUp = false;
            bool keyPressedDown = false;
            bool keyPressedLeft = false;
            bool keyPressedRight = false;
            for (TinyPoint touchOrClickItem : touchesOrClicks)
            {
                bool keyboardPressed = false;
                if (touchOrClickItem.X == -1)
                {
                    keyboardPressed = true;
                }
                Keys keyPressed = keyboardPressed ? (Keys)touchOrClickItem.Y : Keys::None;
                keyPressedUp = keyPressed == Keys::Up ? true : keyPressedUp;
                keyPressedDown = keyPressed == Keys::Down ? true : keyPressedDown;
                keyPressedLeft = keyPressed == Keys::Left ? true : keyPressedLeft;
                keyPressedRight = keyPressed == Keys::Right ? true : keyPressedRight;

                {
                    TinyPoint touchOrClick = keyboardPressed ? TinyPoint(1, 1) : touchOrClickItem;
                    if (!accelStarted && Misc::IsInside(GetPadBounds(getPadCenter(), padRadius), touchOrClick))
                    {
                        padPressed = true;
                        padTouchPos = touchOrClick;
                    }
                    if (keyPressedUp || keyPressedDown || keyPressedLeft || keyPressedRight)
                    {
                        padPressed = true;
                    }
                    DDebug::WriteLine("padPressed=" + padPressed);
                    Def::ButtonGlyph pressedGlyph = ButtonDetect(touchOrClick);
                    DDebug::WriteLine("buttonGlyph2 =" + pressedGlyph);
                    if (pressedGlyph != 0)
                    {
                        pressedGlyphs.push_back(pressedGlyph);
                    }
                    if (keyboardPressed)
                    {
                        switch (keyPressed)
                        {
                            case Keys::LeftControl: pressedGlyph = Def::ButtonGlyph::PlayJump; pressedGlyphs.push_back(pressedGlyph); break;
                            case Keys::Space: pressedGlyph = Def::ButtonGlyph::PlayAction; pressedGlyphs.push_back(pressedGlyph); break;
                            case Keys::Escape: pressedGlyph = Def::ButtonGlyph::PlayPause; pressedGlyphs.push_back(pressedGlyph); break;
                        }
                    }

                    if ((getPhase() == Def::Phase::MainSetup || getPhase() == Def::Phase::PlaySetup) && accelSlider.Move(touchOrClick))
                    {
                        gameData.setAccelSensitivity(accelSlider.getValue());
                    }
                    switch (pressedGlyph)
                    {
                        case Def::ButtonGlyph::PlayJump:
                            DDebug::WriteLine("Jumping detected");
                            accelWaitZero = false;
                            keyPress |= 1;
                            break;
                        case Def::ButtonGlyph::PlayDown:
                            accelWaitZero = false;
                            keyPress |= 4;
                            break;
                        case Def::ButtonGlyph::InitGamerA:
                        case Def::ButtonGlyph::InitGamerB:
                        case Def::ButtonGlyph::InitGamerC:
                        case Def::ButtonGlyph::InitSetup:
                        case Def::ButtonGlyph::InitPlay:
                        case Def::ButtonGlyph::InitBuy:
                        case Def::ButtonGlyph::InitRanking:
                        case Def::ButtonGlyph::WinLostReturn:
                        case Def::ButtonGlyph::TrialBuy:
                        case Def::ButtonGlyph::TrialCancel:
                        case Def::ButtonGlyph::SetupSounds:
                        case Def::ButtonGlyph::SetupJump:
                        case Def::ButtonGlyph::SetupZoom:
                        case Def::ButtonGlyph::SetupAccel:
                        case Def::ButtonGlyph::SetupReset:
                        case Def::ButtonGlyph::SetupReturn:
                        case Def::ButtonGlyph::PauseMenu:
                        case Def::ButtonGlyph::PauseBack:
                        case Def::ButtonGlyph::PauseSetup:
                        case Def::ButtonGlyph::PauseRestart:
                        case Def::ButtonGlyph::PauseContinue:
                        case Def::ButtonGlyph::PlayPause:
                        case Def::ButtonGlyph::PlayAction:
                        case Def::ButtonGlyph::ResumeMenu:
                        case Def::ButtonGlyph::ResumeContinue:
                        case Def::ButtonGlyph::RankingContinue:
                        case Def::ButtonGlyph::Cheat11:
                        case Def::ButtonGlyph::Cheat12:
                        case Def::ButtonGlyph::Cheat21:
                        case Def::ButtonGlyph::Cheat22:
                        case Def::ButtonGlyph::Cheat31:
                        case Def::ButtonGlyph::Cheat32:
                        case Def::ButtonGlyph::Cheat1:
                        case Def::ButtonGlyph::Cheat2:
                        case Def::ButtonGlyph::Cheat3:
                        case Def::ButtonGlyph::Cheat4:
                        case Def::ButtonGlyph::Cheat5:
                        case Def::ButtonGlyph::Cheat6:
                        case Def::ButtonGlyph::Cheat7:
                        case Def::ButtonGlyph::Cheat8:
                        case Def::ButtonGlyph::Cheat9:
                            accelWaitZero = false;
                            buttonGlyph = pressedGlyph;
                            showCheatMenu = false;
                            break;
                    }
                }
            }

            if (
                Def::notAnyOf(
                    buttonGlyph,
                    std::vector{
                        Def::ButtonGlyph(0),
                        Def::ButtonGlyph::PlayAction,
                        Def::ButtonGlyph::Cheat11,
                        Def::ButtonGlyph::Cheat12,
                        Def::ButtonGlyph::Cheat21,
                        Def::ButtonGlyph::Cheat22,
                        Def::ButtonGlyph::Cheat31,
                        Def::ButtonGlyph::Cheat32}
                )
                &&
                lastButtonDown == Def::ButtonGlyph::NoneButtonGlyph)
            {
                TinyPoint pos(320, 240);
                sound->PlayImage(0, pos);
            }
            if (buttonGlyph == Def::ButtonGlyph::NoneButtonGlyph && lastButtonDown != 0)
            {
                buttonPressed = lastButtonDown;
            }
            lastButtonDown = buttonGlyph;
            if (padPressed)
            {
                DDebug::WriteLine("getPadCenter().X=" + getPadCenter().X);
                DDebug::WriteLine("getPadCenter().Y=" + getPadCenter().Y);
                DDebug::WriteLine("padTouchPos.X=" + padTouchPos.X);
                DDebug::WriteLine("padTouchPos.Y=" + padTouchPos.Y);
                DDebug::WriteLine("keyPressedUp=" + keyPressedUp);
                DDebug::WriteLine("keyPressedDown=" + keyPressedDown);
                DDebug::WriteLine("keyPressedLeft=" + keyPressedLeft);
                DDebug::WriteLine("keyPressedRight=" + keyPressedRight);
                {
                    if (keyPressedUp)
                    {
                        padTouchPos.Y = getPadCenter().Y - 30;
                        padTouchPos.X = getPadCenter().X;
                        if (keyPressedLeft) padTouchPos.X = getPadCenter().X - 30;
                        if (keyPressedRight) padTouchPos.X = getPadCenter().X + 30;
                    }
                    if (keyPressedDown) {
                        padTouchPos.Y = getPadCenter().Y + 30;
                        padTouchPos.X = getPadCenter().X;
                        if (keyPressedLeft) padTouchPos.X = getPadCenter().X - 30;
                        if (keyPressedRight) padTouchPos.X = getPadCenter().X + 30;
                    }
                    if (keyPressedLeft) {
                        padTouchPos.X = getPadCenter().X - 30;
                        padTouchPos.Y = getPadCenter().Y;
                        if (keyPressedUp) padTouchPos.Y = getPadCenter().Y - 30;
                        if (keyPressedDown) padTouchPos.Y = getPadCenter().Y + 30;
                    }
                    if (keyPressedRight) {
                        padTouchPos.X = getPadCenter().X + 30;
                        padTouchPos.Y = getPadCenter().Y;
                        if (keyPressedUp) padTouchPos.Y = getPadCenter().Y - 30;
                        if (keyPressedDown) padTouchPos.Y = getPadCenter().Y + 30;
                    }
                }
                double horizontalPosition = padTouchPos.X - getPadCenter().X;
                double verticalPosition = padTouchPos.Y - getPadCenter().Y;

                if (horizontalPosition > 20.0)
                {
                    horizontalChange += 1.0;
                    DDebug::WriteLine(" horizontalChange += 1.0;");
                }
                if (horizontalPosition < -20.0)
                {
                    horizontalChange -= 1.0;
                    DDebug::WriteLine(" horizontalChange -= 1.0;");

                }
                if (verticalPosition > 20.0)
                {
                    verticalChange += 1.0;
                    DDebug::WriteLine(" verticalPosition += 1.0;");

                }
                if (verticalPosition < -20.0)
                {
                    verticalChange -= 1.0;
                    DDebug::WriteLine(" verticalPosition -= 1.0;");
                }

            }
            if (accelStarted)
            {
                horizontalChange = accelSpeedX;
                verticalChange = 0.0;
                if (((uint)keyPress & 4u) != 0)
                {
                    verticalChange = 1.0;
                }
            }
            decor.SetSpeedX(horizontalChange);
            decor.SetSpeedY(verticalChange);
            decor.KeyChange(keyPress);
        }

    Def::ButtonGlyph InputPad::ButtonDetect(TinyPoint touchOrClick)
        {
            std::vector<Def::ButtonGlyph> buttonGlyphsVector = getButtonGlyphs();
            for (auto i = getButtonGlyphs().rbegin(); i != getButtonGlyphs().rend(); i++)
            {
                Def::ButtonGlyph buttonGlyph = *i;
                TinyRect buttonRect = GetButtonRect(buttonGlyph);

                if (buttonGlyph == Def::ButtonGlyph::PlayJump || buttonGlyph == Def::ButtonGlyph::PlayAction || buttonGlyph == Def::ButtonGlyph::PlayDown || buttonGlyph == Def::ButtonGlyph::PlayPause)
                {
                    buttonRect = Misc::Inflate(buttonRect, 20);
                }

                if (Misc::IsInside(buttonRect, touchOrClick))
                {
                    return buttonGlyph;
                }
            }
            return Def::ButtonGlyph::NoneButtonGlyph;
        }

    void InputPad::Draw()
        {
            if (!accelStarted && getPhase() == Def::Phase::Play)
            {
                pixmap->DrawIcon(14, 0, GetPadBounds(getPadCenter(), padRadius / 2), 1.0, false);
                TinyPoint center = (padPressed ? padTouchPos : getPadCenter());
                pixmap->DrawIcon(14, 1, GetPadBounds(center, padRadius / 2), 1.0, false);
            }
            for (Def::ButtonGlyph buttonGlyph : getButtonGlyphs())
            {
                bool pressed = VECTOR_CONTAINS(pressedGlyphs, buttonGlyph);
                bool selected = false;
                if (buttonGlyph >= Def::ButtonGlyph::InitGamerA && buttonGlyph <= Def::ButtonGlyph::InitGamerC)
                {
                    int selectedGamer = (int)(buttonGlyph - 1);
                    selected = selectedGamer == gameData.getSelectedGamer();
                }
                if (buttonGlyph == Def::ButtonGlyph::SetupSounds)
                {
                    selected = gameData.getSounds();
                }
                if (buttonGlyph == Def::ButtonGlyph::SetupJump)
                {
                    selected = gameData.getJumpRight();
                }
                if (buttonGlyph == Def::ButtonGlyph::SetupZoom)
                {
                    selected = gameData.getAutoZoom();
                }
                if (buttonGlyph == Def::ButtonGlyph::SetupAccel)
                {
                    selected = gameData.getAccelActive();
                }
                pixmap->DrawInputButton(GetButtonRect(buttonGlyph), buttonGlyph, pressed, selected);
            }
            if ((getPhase() == Def::Phase::MainSetup || getPhase() == Def::Phase::PlaySetup) && gameData.getAccelActive())
            {
                accelSlider.Draw(pixmap);
            }
        }

     TinyRect InputPad::GetPadBounds(TinyPoint center, int radius)
        {
            return TinyRect(center.X - radius, center.X + radius, center.Y - radius, center.Y + radius);
        }

    TinyRect InputPad::GetButtonRect(Def::ButtonGlyph glyph)
        {
            TinyRect drawBounds = pixmap->getDrawBounds();
            double drawBoundsWidth = drawBounds.getWidth();
            double drawBoundsHeight = drawBounds.getHeight();
            double buttonSizeFactor1 = drawBoundsHeight / 5.0;
            double buttonSizeFactor2 = drawBoundsHeight * 140.0 / 480.0;
            double cheatButtonSizeFactor = drawBoundsHeight / 3.5;
            if (glyph >= Def::ButtonGlyph::Cheat1 && glyph <= Def::ButtonGlyph::Cheat9)
            {
                int cheatNumber = (int)(glyph - 35);
                TinyRect result = TinyRect();
                result.LeftX = 80 * cheatNumber;
                result.RightX = 80 * (cheatNumber + 1);
                result.TopY = 0;
                result.BottomY = 80;
                return result;
            }
            int leftXForButtonsInLeftColumn = (int)(20.0 + buttonSizeFactor2 * 0.0);
            int rightXForButtonsInLeftColumn = (int)(20.0 + buttonSizeFactor2 * 0.5);
            switch (glyph)
            {
                case Def::ButtonGlyph::InitGamerA:
                    {
                        TinyRect result19 = TinyRect();
                        result19.LeftX = leftXForButtonsInLeftColumn;
                        result19.RightX = rightXForButtonsInLeftColumn;
                        result19.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.1);
                        result19.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.6);
                        return result19;
                    }
                case Def::ButtonGlyph::InitGamerB:
                    {
                        TinyRect result18 = TinyRect();
                        result18.LeftX = leftXForButtonsInLeftColumn;
                        result18.RightX = rightXForButtonsInLeftColumn;
                        result18.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.6);
                        result18.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.1);
                        return result18;
                    }
                case Def::ButtonGlyph::InitGamerC:
                    {
                        TinyRect result15 = TinyRect();
                        result15.LeftX = leftXForButtonsInLeftColumn;
                        result15.RightX = rightXForButtonsInLeftColumn;
                        result15.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.1);
                        result15.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.6);
                        return result15;
                    }
                case Def::ButtonGlyph::InitSetup:
                    {
                        TinyRect result14 = TinyRect();
                        result14.LeftX = leftXForButtonsInLeftColumn;
                        result14.RightX = rightXForButtonsInLeftColumn;
                        result14.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.5);
                        result14.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.0);
                        return result14;
                    }
                case Def::ButtonGlyph::InitPlay:
                    {
                        TinyRect result11 = TinyRect();
                        result11.LeftX = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 1.0);
                        result11.RightX = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.0);
                        result11.TopY = (int)(drawBoundsHeight - 40.0 - buttonSizeFactor2 * 1.0);
                        result11.BottomY = (int)(drawBoundsHeight - 40.0 - buttonSizeFactor2 * 0.0);
                        return result11;
                    }
                case Def::ButtonGlyph::InitBuy:
                case Def::ButtonGlyph::InitRanking:
                    {
                        TinyRect result10 = TinyRect();
                        result10.LeftX = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.75);
                        result10.RightX = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.25);
                        result10.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.1);
                        result10.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.6);
                        return result10;
                    }
                case Def::ButtonGlyph::PauseMenu:
                    {
                        TinyRect result37 = TinyRect();
                        result37.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * -0.21);
                        result37.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 0.79);
                        result37.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.2);
                        result37.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.2);
                        return result37;
                    }
                case Def::ButtonGlyph::PauseBack:
                    {
                        TinyRect result36 = TinyRect();
                        result36.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 0.79);
                        result36.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 1.79);
                        result36.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.2);
                        result36.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.2);
                        return result36;
                    }
                case Def::ButtonGlyph::PauseSetup:
                    {
                        TinyRect result35 = TinyRect();
                        result35.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 1.79);
                        result35.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 2.79);
                        result35.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.2);
                        result35.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.2);
                        return result35;
                    }
                case Def::ButtonGlyph::PauseRestart:
                    {
                        TinyRect result34 = TinyRect();
                        result34.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 2.79);
                        result34.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 3.79);
                        result34.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.2);
                        result34.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.2);
                        return result34;
                    }
                case Def::ButtonGlyph::PauseContinue:
                    {
                        TinyRect result33 = TinyRect();
                        result33.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 3.79);
                        result33.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 4.79);
                        result33.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.2);
                        result33.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.2);
                        return result33;
                    }
                case Def::ButtonGlyph::ResumeMenu:
                    {
                        TinyRect result32 = TinyRect();
                        result32.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 1.29);
                        result32.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 2.29);
                        result32.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.2);
                        result32.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.2);
                        return result32;
                    }
                case Def::ButtonGlyph::ResumeContinue:
                    {
                        TinyRect result31 = TinyRect();
                        result31.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 2.29);
                        result31.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 3.29);
                        result31.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.2);
                        result31.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.2);
                        return result31;
                    }
                case Def::ButtonGlyph::WinLostReturn:
                    {
                        TinyRect result30 = TinyRect();
                        result30.LeftX = (int)((double)getPixmapOrigin().X + drawBoundsWidth - buttonSizeFactor1 * 2.2);
                        result30.RightX = (int)((double)getPixmapOrigin().X + drawBoundsWidth - buttonSizeFactor1 * 1.2);
                        result30.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor1 * 0.2);
                        result30.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor1 * 1.2);
                        return result30;
                    }
                case Def::ButtonGlyph::TrialBuy:
                    {
                        TinyRect result29 = TinyRect();
                        result29.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 2.5);
                        result29.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 3.5);
                        result29.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.1);
                        result29.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.1);
                        return result29;
                    }
                case Def::ButtonGlyph::TrialCancel:
                    {
                        TinyRect result28 = TinyRect();
                        result28.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 3.5);
                        result28.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 4.5);
                        result28.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.1);
                        result28.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.1);
                        return result28;
                    }
                case Def::ButtonGlyph::RankingContinue:
                    {
                        TinyRect result27 = TinyRect();
                        result27.LeftX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 3.5);
                        result27.RightX = (int)((double)getPixmapOrigin().X + buttonSizeFactor2 * 4.5);
                        result27.TopY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 2.1);
                        result27.BottomY = (int)((double)getPixmapOrigin().Y + buttonSizeFactor2 * 3.1);
                        return result27;
                    }
                case Def::ButtonGlyph::SetupSounds:
                    {
                        TinyRect result26 = TinyRect();
                        result26.LeftX = leftXForButtonsInLeftColumn;
                        result26.RightX = rightXForButtonsInLeftColumn;
                        result26.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.0);
                        result26.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.5);
                        return result26;
                    }
                case Def::ButtonGlyph::SetupJump:
                    {
                        TinyRect result25 = TinyRect();
                        result25.LeftX = leftXForButtonsInLeftColumn;
                        result25.RightX = rightXForButtonsInLeftColumn;
                        result25.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.5);
                        result25.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.0);
                        return result25;
                    }
                case Def::ButtonGlyph::SetupZoom:
                    {
                        TinyRect result24 = TinyRect();
                        result24.LeftX = leftXForButtonsInLeftColumn;
                        result24.RightX = rightXForButtonsInLeftColumn;
                        result24.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.0);
                        result24.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.5);
                        return result24;
                    }
                case Def::ButtonGlyph::SetupAccel:
                    {
                        TinyRect result23 = TinyRect();
                        result23.LeftX = leftXForButtonsInLeftColumn;
                        result23.RightX = rightXForButtonsInLeftColumn;
                        result23.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.5);
                        result23.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.0);
                        return result23;
                    }
                case Def::ButtonGlyph::SetupReset:
                    {
                        TinyRect result22 = TinyRect();
                        result22.LeftX = (int)(450.0 + buttonSizeFactor2 * 0.0);
                        result22.RightX = (int)(450.0 + buttonSizeFactor2 * 0.5);
                        result22.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.0);
                        result22.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.5);
                        return result22;
                    }
                case Def::ButtonGlyph::SetupReturn:
                    {
                        TinyRect result21 = TinyRect();
                        result21.LeftX = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.8);
                        result21.RightX = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.0);
                        result21.TopY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.8);
                        result21.BottomY = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.0);
                        return result21;
                    }
                case Def::ButtonGlyph::PlayPause:
                    {
                        TinyRect result20 = TinyRect();
                        result20.LeftX = (int)(drawBoundsWidth - buttonSizeFactor1 * 0.7);
                        result20.RightX = (int)(drawBoundsWidth - buttonSizeFactor1 * 0.2);
                        result20.TopY = (int)(buttonSizeFactor1 * 0.2);
                        result20.BottomY = (int)(buttonSizeFactor1 * 0.7);
                        return result20;
                    }
                case Def::ButtonGlyph::PlayAction:
                    {
                        if (gameData.getJumpRight())
                        {
                            TinyRect result16 = TinyRect();
                            result16.LeftX = (int)((double)drawBounds.getWidth() - buttonSizeFactor1 * 1.2);
                            result16.RightX = (int)((double)drawBounds.getWidth() - buttonSizeFactor1 * 0.2);
                            result16.TopY = (int)(drawBoundsHeight - buttonSizeFactor1 * 2.6);
                            result16.BottomY = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.6);
                            return result16;
                        }
                        TinyRect result17 = TinyRect();
                        result17.LeftX = (int)(buttonSizeFactor1 * 0.2);
                        result17.RightX = (int)(buttonSizeFactor1 * 1.2);
                        result17.TopY = (int)(drawBoundsHeight - buttonSizeFactor1 * 2.6);
                        result17.BottomY = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.6);
                        return result17;
                    }
                case Def::ButtonGlyph::PlayJump:
                    {
                        if (gameData.getJumpRight())
                        {
                            TinyRect result12 = TinyRect();
                            result12.LeftX = (int)((double)drawBounds.getWidth() - buttonSizeFactor1 * 1.2);
                            result12.RightX = (int)((double)drawBounds.getWidth() - buttonSizeFactor1 * 0.2);
                            result12.TopY = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                            result12.BottomY = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                            return result12;
                        }
                        TinyRect result13 = TinyRect();
                        result13.LeftX = (int)(buttonSizeFactor1 * 0.2);
                        result13.RightX = (int)(buttonSizeFactor1 * 1.2);
                        result13.TopY = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                        result13.BottomY = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                        return result13;
                    }
                case Def::ButtonGlyph::PlayDown:
                    {
                        if (gameData.getJumpRight())
                        {
                            TinyRect result8 = TinyRect();
                            result8.LeftX = (int)(buttonSizeFactor1 * 0.2);
                            result8.RightX = (int)(buttonSizeFactor1 * 1.2);
                            result8.TopY = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                            result8.BottomY = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                            return result8;
                        }
                        TinyRect result9 = TinyRect();
                        result9.LeftX = (int)((double)drawBounds.getWidth() - buttonSizeFactor1 * 1.2);
                        result9.RightX = (int)((double)drawBounds.getWidth() - buttonSizeFactor1 * 0.2);
                        result9.TopY = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                        result9.BottomY = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                        return result9;
                    }
                case Def::ButtonGlyph::Cheat11:
                    {
                        TinyRect result7 = TinyRect();
                        result7.LeftX = (int)(cheatButtonSizeFactor * 0.0);
                        result7.RightX = (int)(cheatButtonSizeFactor * 1.0);
                        result7.TopY = (int)(cheatButtonSizeFactor * 0.0);
                        result7.BottomY = (int)(cheatButtonSizeFactor * 1.0);
                        return result7;
                    }
                case Def::ButtonGlyph::Cheat12:
                    {
                        TinyRect result6 = TinyRect();
                        result6.LeftX = (int)(cheatButtonSizeFactor * 0.0);
                        result6.RightX = (int)(cheatButtonSizeFactor * 1.0);
                        result6.TopY = (int)(cheatButtonSizeFactor * 1.0);
                        result6.BottomY = (int)(cheatButtonSizeFactor * 2.0);
                        return result6;
                    }
                case Def::ButtonGlyph::Cheat21:
                    {
                        TinyRect result5 = TinyRect();
                        result5.LeftX = (int)(cheatButtonSizeFactor * 1.0);
                        result5.RightX = (int)(cheatButtonSizeFactor * 2.0);
                        result5.TopY = (int)(cheatButtonSizeFactor * 0.0);
                        result5.BottomY = (int)(cheatButtonSizeFactor * 1.0);
                        return result5;
                    }
                case Def::ButtonGlyph::Cheat22:
                    {
                        TinyRect result4 = TinyRect();
                        result4.LeftX = (int)(cheatButtonSizeFactor * 1.0);
                        result4.RightX = (int)(cheatButtonSizeFactor * 2.0);
                        result4.TopY = (int)(cheatButtonSizeFactor * 1.0);
                        result4.BottomY = (int)(cheatButtonSizeFactor * 2.0);
                        return result4;
                    }
                case Def::ButtonGlyph::Cheat31:
                    {
                        TinyRect result3 = TinyRect();
                        result3.LeftX = (int)(cheatButtonSizeFactor * 2.0);
                        result3.RightX = (int)(cheatButtonSizeFactor * 3.0);
                        result3.TopY = (int)(cheatButtonSizeFactor * 0.0);
                        result3.BottomY = (int)(cheatButtonSizeFactor * 1.0);
                        return result3;
                    }
                case Def::ButtonGlyph::Cheat32:
                    {
                        TinyRect result2 = TinyRect();
                        result2.LeftX = (int)(cheatButtonSizeFactor * 2.0);
                        result2.RightX = (int)(cheatButtonSizeFactor * 3.0);
                        result2.TopY = (int)(cheatButtonSizeFactor * 1.0);
                        result2.BottomY = (int)(cheatButtonSizeFactor * 2.0);
                        return result2;
                    }
                default:
                    return TinyRect();
            }
        }

        void InputPad::StartAccel()
        {
            try
            {
                accelSensor.Start();
                accelStarted = true;
            }
            catch (Microsoft::Devices::Sensors::AccelerometerFailedException)
            {
                accelStarted = false;
            }
            catch (System::UnauthorizedAccessException)
            {
                accelStarted = false;
            }
        }

        void InputPad::StopAccel()
        {
            if (accelStarted)
            {
                try
                {
                    accelSensor.Stop();
                }
                catch (Microsoft::Devices::Sensors::AccelerometerFailedException)
                {
                }
                accelStarted = false;
            }
        }


        void InputPad::HandleAccelSensorCurrentValueChanged(Microsoft::Devices::Sensors::SensorReadingEventArgs<Microsoft::Devices::Sensors::AccelerometerReading> e)
        {
            //IL_0001: Unknown result type (might be due to invalid IL or missing references)
            //IL_0006: Unknown result type (might be due to invalid IL or missing references)

            Microsoft::Devices::Sensors::AccelerometerReading sensorReading = e.getSensorReading();
            float y = ((Microsoft::Devices::Sensors::AccelerometerReading)(sensorReading)).getAcceleration().Y;
            float sensitivityThreshold = (1.0f - (float)gameData.getAccelSensitivity()) * 0.06f + 0.04f;
            float adjustedThreshold = (accelLastState ? (sensitivityThreshold * 0.6f) : sensitivityThreshold);
            if (y > adjustedThreshold)
            {
                accelSpeedX = 0.0 - std::min((double)y * 0.25 / (double)sensitivityThreshold + 0.25, 1.0);
            }
            else if (y < 0.0f - adjustedThreshold)
            {
                accelSpeedX = std::min((double)(0.0f - y) * 0.25 / (double)sensitivityThreshold + 0.25, 1.0);
            }
            else
            {
                accelSpeedX = 0.0;
            }
            accelLastState = accelSpeedX != 0.0;
            if (accelWaitZero)
            {
                if (accelSpeedX == 0.0)
                {
                    accelWaitZero = false;
                }
                else
                {
                    accelSpeedX = 0.0;
                }
            }
        }

}
