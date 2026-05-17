#include "WindowsPhoneSpeedyBlupi/InputPad.hpp"

#ifndef LEGACY
#include <algorithm>
#include <string>
#endif

#include "CNA/Platform.hpp"
#include "Microsoft/Devices/Sensors/AccelerometerFailedException.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "System/UnauthorizedAccessException.hpp"
#include "WindowsPhoneSpeedyBlupi/Config.hpp"
#include "WindowsPhoneSpeedyBlupi/IGame1.hpp"
#include "WindowsPhoneSpeedyBlupi/Misc.hpp"
#ifndef LEGACY
#include "WindowsPhoneSpeedyBlupi/Text.hpp"
#endif

#include "WindowsPhoneSpeedyBlupi/def/SoundChannel.hpp"

#define INPUT_DEBUG(msg) CNA::Logger::DebugIf(msg, Config::INPUT_DETAILED_DEBUGGING_ENABLED);
#define INPUT_ENABLED

#ifndef INPUT_ENABLED
#define INPUT_DISABLED
#endif

namespace WindowsPhoneSpeedyBlupi
{
    IDATA(Def::Phase, Phase, InputPad)
    IDATA(int, SelectedGamer, InputPad)
    IDATA(TinyPoint, PixmapOrigin, InputPad)
    int InputPad::getTotalTouchOrClickProperty() const { return touchOrClickCount; }

    Def::ButtonGlyph InputPad::getButtonPressedProperty() const
    {
#ifdef INPUT_DISABLED
        return Def::ButtonGlyph::None;
#endif
        Def::ButtonGlyph result = buttonPressed;
        buttonPressed = Def::ButtonGlyph::None;
        return result;
    }

    IDATA(bool, ShowCheatMenu, InputPad)

    std::vector<Def::ButtonGlyph> InputPad::getButtonGlyphsProperty() const
    {
        std::vector<Def::ButtonGlyph> glyphs;
        glyphs.reserve(16);
#ifdef INPUT_DISABLED
        return glyphs;
#endif
        switch (getPhaseProperty())
        {
        case Def::Phase::Init:
            glyphs.push_back(Def::ButtonGlyph::InitGamerA);
            glyphs.push_back(Def::ButtonGlyph::InitGamerB);
            glyphs.push_back(Def::ButtonGlyph::InitGamerC);
            glyphs.push_back(Def::ButtonGlyph::InitSetup);
            glyphs.push_back(Def::ButtonGlyph::InitPlay);
            if (game1->getIsTrialModeProperty())
            {
                glyphs.push_back(Def::ButtonGlyph::InitBuy);
            }
            if (game1->getIsRankingModeProperty())
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

    TinyPoint InputPad::getPadCenterProperty() const
    {
#ifdef INPUT_DISABLED
        return {};
#endif
        TinyRect drawBounds = pixmap->getDrawBoundsProperty();
        int x = gameData->getJumpRightProperty() ? 100 : drawBounds.getWidthProperty() - 100;
        return {x, drawBounds.getHeightProperty() - 100};
    }

    InputPad::InputPad(IGame1* game1, Decor* decor, IPixmap* pixmap, ISound* sound, GameData* gameData) :
        game1(game1),
        decor(decor),
        pixmap(pixmap),
        sound(sound),
        gameData(gameData),
        accelSensor(Microsoft::Devices::Sensors::Accelerometer()),
        accelSlider(Slider(TinyPoint(320, 400), this->gameData->getAccelSensitivityProperty()))
    {
        using Microsoft::Devices::Sensors::AccelerometerReading;
        using Microsoft::Devices::Sensors::SensorBase;
        accelSensor.CurrentValueChanged +=
            [this](
            System::Object* /*sender*/,
            const Microsoft::Devices::Sensors::SensorReadingEventArgs<AccelerometerReading>&
            sensor_reading_event_args)
            {
                HandleAccelSensorCurrentValueChanged(sensor_reading_event_args);
            };

        lastButtonDown = Def::ButtonGlyph::None;
        buttonPressed = Def::ButtonGlyph::None;
    }

    void InputPad::StartMission(int mission)
    {
        this->mission = mission;
        accelWaitZero = true;
    }

#ifdef MODERN
    InputPad::VirtualKeyboardLayout InputPad::GetVirtualKeyboardLayout() const
    {
        VirtualKeyboardLayout L;
        TinyRect drawBounds = pixmap->getDrawBoundsProperty();
        const int width = drawBounds.getWidthProperty();

        L.keyGap  = std::max(2, width / 130);
        L.rowGap  = L.keyGap;
        L.panelX  = std::max(4, width / 80);
        L.panelY  = 10;
        // 10 QWERTY-wide slots fill the width; the close button is anchored to panelRect right edge.
        L.keyW    = (width - 2 * L.panelX - 10 * L.keyGap) / 11;
        L.keyH    = L.keyW;
        L.panelW  = 10 * (L.keyW + L.keyGap) - L.keyGap;  // 10 slots wide
        L.panelH  = 4 * (L.keyH + L.rowGap) + L.rowGap + L.keyH;
        L.origin = pixmap->getOriginProperty();

        // Panel rectangle
        L.panelRect.Left   = L.panelX - 4             + L.origin.X;
        L.panelRect.Right  = L.panelX + L.panelW + 4  + L.origin.X;
        L.panelRect.Top    = L.panelY - 4             + L.origin.Y;
        L.panelRect.Bottom = L.panelY + L.panelH + 4  + L.origin.Y;

        // Close button: one key-width inset from the right edge of the panel
        L.closeRect.Right  = L.panelRect.Right  - 4 - (L.keyW + L.keyGap);
        L.closeRect.Left   = L.closeRect.Right  - L.keyW;
        L.closeRect.Top    = L.panelRect.Top    + 4;
        L.closeRect.Bottom = L.closeRect.Top    + L.keyH;

        // F12: one slot to the left of the close button
        L.f12OffsetX = (L.closeRect.Left - 4 - L.origin.X) - L.panelX - (L.keyW + L.keyGap);

        INPUT_DEBUG("VK panel: L=" + std::to_string(L.panelRect.Left) +
                    " R=" + std::to_string(L.panelRect.Right) +
                    " T=" + std::to_string(L.panelRect.Top) +
                    " B=" + std::to_string(L.panelRect.Bottom));
        INPUT_DEBUG("VK close: L=" + std::to_string(L.closeRect.Left) +
                    " R=" + std::to_string(L.closeRect.Right) +
                    " T=" + std::to_string(L.closeRect.Top) +
                    " B=" + std::to_string(L.closeRect.Bottom));

        // Defensive: clamp closeRect inside panelRect
        if (L.closeRect.Left   < L.panelRect.Left)   L.closeRect.Left   = L.panelRect.Left;
        if (L.closeRect.Right  > L.panelRect.Right)  L.closeRect.Right  = L.panelRect.Right;
        if (L.closeRect.Top    < L.panelRect.Top)    L.closeRect.Top    = L.panelRect.Top;
        if (L.closeRect.Bottom > L.panelRect.Bottom) L.closeRect.Bottom = L.panelRect.Bottom;

        return L;
    }
#endif

    void InputPad::Update()
    {
#ifdef INPUT_DISABLED
        return;
#endif
        pressedGlyphs.clear();
        if (accelActive != gameData->getAccelActiveProperty())
        {
            accelActive = gameData->getAccelActiveProperty();
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
        Def::ButtonGlyph buttonGlyph = Def::ButtonGlyph::None;

        Microsoft::Xna::Framework::Input::Touch::TouchCollection touches{};
        bool touchScreenIsSupported = true;
        if (touchScreenIsSupported)
        {
            using Microsoft::Xna::Framework::Input::Touch::TouchPanel;
            touches = TouchPanel::GetState();
            touchOrClickCount = touches.getCountProperty();
        }

        std::vector<TinyPoint> touchesOrClicks;

        using Microsoft::Xna::Framework::Input::Touch::TouchLocation;
        using Microsoft::Xna::Framework::Input::Touch::TouchLocationState;
        if (touchScreenIsSupported)
            for (const TouchLocation& item : touches)
            {
                if (item.getStateProperty() == TouchLocationState::Pressed || item.getStateProperty() ==
                    TouchLocationState::Moved)
                {
                    TinyPoint touchPress{
                        static_cast<int>(item.getPositionProperty().X), static_cast<int>(item.getPositionProperty().Y)
                    };
                    touchesOrClicks.push_back(touchPress);
                }
            }

        using Microsoft::Xna::Framework::Input::MouseState;
        using Microsoft::Xna::Framework::Input::Mouse;
        using Microsoft::Xna::Framework::Input::ButtonState;
        MouseState mouseState = Mouse::GetState();
        if (mouseState.getLeftButtonProperty() == ButtonState::Pressed)
        {
            touchOrClickCount++;
            TinyPoint mouseClick(mouseState.getXProperty(), mouseState.getYProperty());
            touchesOrClicks.push_back(mouseClick);
        }

        float screenWidth = game1->getGraphics().getGraphicsDeviceProperty()->getViewportProperty().getWidthProperty();
        float screenHeight = game1->getGraphics().getGraphicsDeviceProperty()->getViewportProperty().
                                    getHeightProperty();
        float screenRatio = screenWidth / screenHeight;

        if ((CNA::getCurrentPlatform() == CNA::Platform::Android && screenRatio > 1.3333333333333333)
            /*|| Env.IMPL.isKNI()*/)
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
                    INPUT_DEBUG("-----");
                    INPUT_DEBUG("originalX=" + std::to_string(originalX));
                    INPUT_DEBUG("originalY=" + std::to_string(originalY));
                    INPUT_DEBUG("heightRatio=" + std::to_string(heightRatio));
                    INPUT_DEBUG("widthRatio=" + std::to_string(widthRatio));
                    INPUT_DEBUG("widthHeightRatio=" + std::to_string(widthHeightRatio));
                }
                if (screenHeight > 480)
                {
                    touchOrClick.X = (int)(originalX * heightRatio);
                    touchOrClick.Y = (int)(originalY * heightRatio);
                    touchesOrClicks[i] = touchOrClick;
                }

                using namespace std::string_literals;
                INPUT_DEBUG("new X"s + std::to_string(touchOrClick.X));
                INPUT_DEBUG("new Y"s + std::to_string(touchOrClick.Y));
            }
        }
        using Microsoft::Xna::Framework::Input::KeyboardState;
        using namespace Microsoft::Xna::Framework::Input;
        KeyboardState newKeyboardState = Keyboard::GetState();

        Keys keysToBeChecked[] = {
            Keys::LeftControl, Keys::Up, Keys::Right, Keys::Down, Keys::Left, Keys::Space, Keys::Escape,
        };
        for (Keys keys : keysToBeChecked)
        {
            if (newKeyboardState.IsKeyDown(keys)) touchesOrClicks.push_back(TinyPoint(-1, static_cast<int>(keys)));
        }
        static bool F11_pressed_previously = false;
        static int fullscreen_timeout = 0;
        if (CNA::getCurrentPlatform() != CNA::Platform::Android && CNA::getCurrentPlatform() != CNA::Platform::Web &&
            newKeyboardState.IsKeyDown(Keys::F11) && !F11_pressed_previously && fullscreen_timeout == 0)
        {
            F11_pressed_previously = true;
            game1->ToggleFullScreen();
            INPUT_DEBUG("F11 was pressed.");
            static const constexpr int MAX_FULLSCREEN_TIMEOUT = static_cast<int>(Config::FPS) / 4; // 250 ms
            fullscreen_timeout = MAX_FULLSCREEN_TIMEOUT;
        }
        else
        {
            F11_pressed_previously = false;
        }
        if (fullscreen_timeout > 0)
        {
            fullscreen_timeout--;
        }

#ifdef MODERN
        // --- Virtual on-screen keyboard activation ---
        // Clear virtual keys from the previous frame.
        virtualKeysPressedThisFrame.clear();

        // Helper: returns true if a key is pressed physically OR via the virtual keyboard this frame.
        auto IsKeyDownOrVirtual = [&](Keys key) -> bool
        {
            if (newKeyboardState.IsKeyDown(key)) return true;
            return std::find(virtualKeysPressedThisFrame.begin(), virtualKeysPressedThisFrame.end(), key)
                   != virtualKeysPressedThisFrame.end();
        };

        {
            // Activation area: top-left 10% of screen width and height (screen-space pixels).
            const int kLargerDim   = std::max(screenWidth, screenHeight);
            const int kActivationW = static_cast<int>(kLargerDim * 0.20f);
            const int kActivationH = static_cast<int>(kLargerDim * 0.10f);
            constexpr int kHoldFrames = 1 * Config::CURRENT_FPS;  // 1 second

            bool inActivationArea = false;
            for (const TinyPoint& tp : touchesOrClicks)
            {
                if (tp.X == -1) continue;  // keyboard event, not a touch
                if (tp.X >= 0 && tp.X <= kActivationW && tp.Y >= 0 && tp.Y <= kActivationH)
                {
                    inActivationArea = true;
                    break;
                }
            }

            if (inActivationArea && !virtualKeyboardActivationConsumed)
            {
                virtualKeyboardHoldFrames++;
                if (virtualKeyboardHoldFrames >= kHoldFrames)
                {
                    virtualKeyboardVisible = true;
                    virtualKeyboardActivationConsumed = true;
                    INPUT_DEBUG("Virtual keyboard activated.");
                }
            }
            else if (!inActivationArea)
            {
                virtualKeyboardHoldFrames = 0;
                virtualKeyboardActivationConsumed = false;
            }
        }

        // When the virtual keyboard is visible, detect taps on its key rectangles.
        if (virtualKeyboardVisible)
        {
            const VirtualKeyboardLayout layout = GetVirtualKeyboardLayout();
            const int kKeyW   = layout.keyW;
            const int kKeyH   = layout.keyH;
            const int kKeyGap = layout.keyGap;
            const int kRowGap = layout.rowGap;
            const int kPanelX = layout.panelX;
            const int kPanelY = layout.panelY;
            const TinyPoint& origin = layout.origin;

            // Key rows: F* on top (F5-F8 left, F12 right), then QWERTY
            static const Keys fnLeftRow[] = { Keys::F5, Keys::F6, Keys::F7, Keys::F8 };
            static const Keys fnRightRow[] = { Keys::F12 };
            static const Keys row0[] = { Keys::Q, Keys::W, Keys::E, Keys::R, Keys::T, Keys::Y, Keys::U, Keys::I, Keys::O, Keys::P };
            static const Keys row1[] = { Keys::A, Keys::S, Keys::D, Keys::F, Keys::G, Keys::H, Keys::J, Keys::K, Keys::L };
            static const Keys row2[] = { Keys::Z, Keys::X, Keys::C, Keys::V, Keys::B, Keys::N, Keys::M };

            struct KbRow { const Keys* keys; int count; int offsetX; int rowY; };
            const KbRow rows[] = {
                { fnLeftRow,  4, 0,                        0 },
                { fnRightRow, 1, layout.f12OffsetX,        0 },
                { row0, 10, 0,                             kKeyH + kRowGap },
                { row1,  9, (kKeyW + kKeyGap) / 2,        2 * (kKeyH + kRowGap) },
                { row2,  7, (kKeyW + kKeyGap),             3 * (kKeyH + kRowGap) },
            };

            bool keyboardConsumedTouch = false;
            for (const TinyPoint& tp : touchesOrClicks)
            {
                if (tp.X == -1) continue;

                // Close button — uses the same closeRect as Draw()
                if (Misc::IsInside(layout.closeRect, tp))
                {
                    virtualKeyboardVisible = false;
                    keyboardConsumedTouch = true;
                    INPUT_DEBUG("Virtual keyboard closed.");
                    break;
                }

                // Key rows (screen-space, with origin offset)
                for (const KbRow& row : rows)
                {
                    for (int ki = 0; ki < row.count; ki++)
                    {
                        TinyRect keyRect;
                        keyRect.Left   = kPanelX + row.offsetX + ki * (kKeyW + kKeyGap) + origin.X;
                        keyRect.Right  = keyRect.Left + kKeyW;
                        keyRect.Top    = kPanelY + row.rowY + origin.Y;
                        keyRect.Bottom = keyRect.Top + kKeyH;
                        if (Misc::IsInside(keyRect, tp))
                        {
                            virtualKeysPressedThisFrame.push_back(row.keys[ki]);
                            keyboardConsumedTouch = true;
                            INPUT_DEBUG(std::string("Virtual key pressed: ") + std::to_string(static_cast<int>(row.keys[ki])));
                        }
                    }
                }
            }

            // Consume all touches so they don't also trigger game buttons.
            if (keyboardConsumedTouch || virtualKeyboardVisible)
            {
                touchesOrClicks.clear();
            }
        }

        // quick_cheat_enabled, ghost_cheat_enabled, debug_cheat_enabled are member fields.

        if (ghost_cheat_enabled != decor->IsGhost())
        {
            ghost_cheat_enabled = decor->IsGhost();
        }
        static GameSpeed game_speed_before_quick_cheat{GameSpeed::Normal};

        if (!quick_cheat_enabled && !ghost_cheat_enabled && game1->getGameSpeed() > GameSpeed::Fast)
        {
            game1->SetGameSpeed(GameSpeed::Normal);
        }
        static bool F5_pressed_previously = false;
        static bool F6_pressed_previously = false;
        static bool F7_pressed_previously = false;
        static bool F8_pressed_previously = false;

        if (!ghost_cheat_enabled)
        {
            if (IsKeyDownOrVirtual(Keys::F5) && !F5_pressed_previously)
            {
                game1->SetGameSpeed(ToGameSpeed(Keys::F5));
                INPUT_DEBUG("F5 was pressed: game speed set to 1x.");
            }
            F5_pressed_previously = IsKeyDownOrVirtual(Keys::F5);
            if (IsKeyDownOrVirtual(Keys::F6) && !F6_pressed_previously)
            {
                game1->SetGameSpeed(ToGameSpeed(Keys::F6));
                INPUT_DEBUG("F6 was pressed: game speed set to 2x.");
            }
            F6_pressed_previously = IsKeyDownOrVirtual(Keys::F6);

            if (quick_cheat_enabled)
            {
                if (IsKeyDownOrVirtual(Keys::F7) && !F7_pressed_previously)
                {
                    game1->SetGameSpeed(ToGameSpeed(Keys::F7));
                    INPUT_DEBUG("F7 was pressed: game speed set to 4x.");
                }
                F7_pressed_previously = IsKeyDownOrVirtual(Keys::F7);
                if (IsKeyDownOrVirtual(Keys::F8) && !F8_pressed_previously)
                {
                    game1->SetGameSpeed(ToGameSpeed(Keys::F8));
                    INPUT_DEBUG("F8 was pressed: game speed set to 8x.");
                }
                F8_pressed_previously = IsKeyDownOrVirtual(Keys::F8);
            }
        }

        static bool F12_pressed_previously = false;
        if (IsKeyDownOrVirtual(Keys::F12) && !F12_pressed_previously)
        {
            showCheatMenu = !showCheatMenu;
            INPUT_DEBUG(std::string("F12 was pressed: cheat menu toggled to ") + (showCheatMenu ? "visible" : "hidden") + ".");
        }
        F12_pressed_previously = IsKeyDownOrVirtual(Keys::F12);

        // Typed cheat code detection: accumulate letters typed during Play phase.
        // When the accumulated string matches a known cheat code name, activate it.
        if (getPhaseProperty() == Def::Phase::Play)
        {
            static const Keys letterKeys[26] = {
                Keys::A, Keys::B, Keys::C, Keys::D, Keys::E, Keys::F, Keys::G, Keys::H,
                Keys::I, Keys::J, Keys::K, Keys::L, Keys::M, Keys::N, Keys::O, Keys::P,
                Keys::Q, Keys::R, Keys::S, Keys::T, Keys::U, Keys::V, Keys::W, Keys::X,
                Keys::Y, Keys::Z
            };
            for (int li = 0; li < 26; li++)
            {
                bool down = IsKeyDownOrVirtual(letterKeys[li]);
                if (down && !letterPrev[li])
                {
                    typedCheatBuffer += static_cast<char>('a' + li);
                    if (typedCheatBuffer.size() > 32)
                    {
                        typedCheatBuffer = typedCheatBuffer.substr(typedCheatBuffer.size() - 32);
                    }

                    // Map cheat code names to CheatCodes enum values.
                    // persistent = true means the cheat toggles a lasting state.
                    struct CheatEntry
                    {
                        const char* name;
                        Tables::CheatCodes code;
                        bool persistent;
                    };
                    static const CheatEntry cheatEntries[] = {
                        { "buildofficialmissions", Tables::CheatCodes::BuildOfficialMissions, true  },
                        { "opendoors",             Tables::CheatCodes::OpenDoors,             true },
                        { "cleanall",              Tables::CheatCodes::CleanAll,              false },
                        { "megablupi",            Tables::CheatCodes::SuperBlupi,            true  },
                        { "layegg",                Tables::CheatCodes::LayEgg,                false },
                        { "killegg",               Tables::CheatCodes::KillEgg,               false },
                        { "funskate",                 Tables::CheatCodes::Skate,                 false  },
                        { "givecopter",                Tables::CheatCodes::Copter,                false  },
                        { "jeepdrive",                  Tables::CheatCodes::Jeep,                  false  },
                        { "alltreasure",           Tables::CheatCodes::AllTreasure,           false },
                        { "endgoal",               Tables::CheatCodes::EndGoal,               false },
                        { "showsecret",            Tables::CheatCodes::ShowSecret,            true },
                        { "roundshield",           Tables::CheatCodes::RoundShield,           false  },
                        { "quicklollypop",              Tables::CheatCodes::Lollipop,              false  },
                        { "tenbombs",                 Tables::CheatCodes::Bombs,                 false },
                        { "birdlime",              Tables::CheatCodes::BirdLime,              false  },
                        { "drivetank",                  Tables::CheatCodes::Tank,                  false  },
                        { "powercharge",           Tables::CheatCodes::PowerCharge,           false },
                        { "hidedrink",                 Tables::CheatCodes::Drink,                 false },
                        { "iovercraft",             Tables::CheatCodes::Overcraft,             false  },
                        { "udynamite",              Tables::CheatCodes::Dynamite,              false },
                        { "weelkeys",              Tables::CheatCodes::WeelKeys,              false },
#ifdef MODERN
                        { "quick",              Tables::CheatCodes::Quick,              true },
                        { "ghost",              Tables::CheatCodes::Ghost,              false },
                        { "debug",              Tables::CheatCodes::Debug,              true },
                        { "zoom",               Tables::CheatCodes::Zoom,               false },
                        { "cheats",             Tables::CheatCodes::Cheats,             false },
#endif
                    };
                    static const int cheatEntriesCount = static_cast<int>(sizeof(cheatEntries) / sizeof(cheatEntries[0]));

                    for (int ci = 0; ci < cheatEntriesCount; ci++)
                    {
                        const std::string name = cheatEntries[ci].name;
                        if (typedCheatBuffer.size() >= name.size() &&
                            typedCheatBuffer.substr(typedCheatBuffer.size() - name.size()) == name)
                        {
                            if (cheatEntries[ci].code == Tables::CheatCodes::Quick)
                            {
                                quick_cheat_enabled = ! quick_cheat_enabled;
                                if (!quick_cheat_enabled && game1->getGameSpeed() > GameSpeed::Fast)
                                {
                                    game1->SetGameSpeed(GameSpeed::Fast);
                                }
                            }
#ifdef MODERN
                            else if (cheatEntries[ci].code == Tables::CheatCodes::Debug)
                            {
                                debug_cheat_enabled = !debug_cheat_enabled;
                            }
                            else if (cheatEntries[ci].code == Tables::CheatCodes::Zoom)
                            {
                                // Zoom is handled below in the post-action block.
                            }
                            else if (cheatEntries[ci].code == Tables::CheatCodes::Cheats)
                            {
                                // Show cheats overlay for 5 seconds (100 frames at 20fps).
                                cheats_display_timer = 5 * Config::CURRENT_FPS;
                            }
#endif
                            else
                            {
                                decor->CheatAction(cheatEntries[ci].code);
                            }
#ifdef MODERN
                            if (cheatEntries[ci].code == Tables::CheatCodes::Zoom)
                            {
                                // Cycle: Zoom100 -> Zoom50 -> Zoom25 -> Zoom12 -> Zoom100
                                // Remove old zoom labels from activePersistentCheats.
                                activePersistentCheats.erase(
                                    std::remove_if(activePersistentCheats.begin(), activePersistentCheats.end(),
                                        [](const std::string& s){ return s == "zoom50" || s == "zoom25" || s == "zoom12"; }),
                                    activePersistentCheats.end());
                                if (zoom_cheat_state == ZoomCheat::Zoom100)
                                {
                                    zoom_cheat_state = ZoomCheat::Zoom50;
                                    decor->SetCheatZoom(0.5);
                                    activePersistentCheats.push_back("zoom50");
                                }
                                else if (zoom_cheat_state == ZoomCheat::Zoom50)
                                {
                                    zoom_cheat_state = ZoomCheat::Zoom25;
                                    decor->SetCheatZoom(0.25);
                                    activePersistentCheats.push_back("zoom25");
                                }
                                else if (zoom_cheat_state == ZoomCheat::Zoom25)
                                {
                                    zoom_cheat_state = ZoomCheat::Zoom12;
                                    decor->SetCheatZoom(0.125);
                                    activePersistentCheats.push_back("zoom12");
                                }
                                else
                                {
                                    zoom_cheat_state = ZoomCheat::Zoom100;
                                    decor->SetCheatZoom(1.0);
                                    // No label added for Zoom100 (normal view).
                                }
                            }
                            if (cheatEntries[ci].code == Tables::CheatCodes::Debug)
                            {
                                // handled above; no-op here
                            }
                            if (cheatEntries[ci].code == Tables::CheatCodes::Ghost)
                            {
                                ghost_cheat_enabled = decor->IsGhost();
                                // Sync activePersistentCheats based on the real runtime ghost state.
                                activePersistentCheats.erase(
                                    std::remove(activePersistentCheats.begin(), activePersistentCheats.end(), std::string("ghost")),
                                    activePersistentCheats.end());
                                if (ghost_cheat_enabled)
                                {
                                    activePersistentCheats.push_back("ghost");
                                    game_speed_before_quick_cheat = game1->getGameSpeed();
                                    game1->SetGameSpeed(GameSpeed::Faster);
                                }
                                else
                                {
                                    game1->SetGameSpeed(game_speed_before_quick_cheat);
                                }
                            }
#endif
                            if (cheatEntries[ci].code == Tables::CheatCodes::OpenDoors)
                            {
                                decor->CheatAction(Tables::CheatCodes::WeelKeys);
                            }
                            if (cheatEntries[ci].persistent)
                            {
                                const std::string& n = name;
                                auto it = std::find(activePersistentCheats.begin(), activePersistentCheats.end(), n);
                                if (it != activePersistentCheats.end())
                                {
                                    activePersistentCheats.erase(it);
                                }
                                else
                                {
                                    activePersistentCheats.push_back(n);
                                }
                            }
                            typedCheatBuffer.clear();
                            INPUT_DEBUG(std::string("Typed cheat activated: ") + name);
                            break;
                        }
                    }
                }
                letterPrev[li] = down;
            }
        }
        else
        {
            // Update debounce state even outside Play so we don't get spurious triggers on entry.
            static const Keys letterKeysOuter[26] = {
                Keys::A, Keys::B, Keys::C, Keys::D, Keys::E, Keys::F, Keys::G, Keys::H,
                Keys::I, Keys::J, Keys::K, Keys::L, Keys::M, Keys::N, Keys::O, Keys::P,
                Keys::Q, Keys::R, Keys::S, Keys::T, Keys::U, Keys::V, Keys::W, Keys::X,
                Keys::Y, Keys::Z
            };
            for (int li = 0; li < 26; li++)
            {
                letterPrev[li] = IsKeyDownOrVirtual(letterKeysOuter[li]);
            }
        }
#endif

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
                if (!accelStarted && Misc::IsInside(GetPadBounds(getPadCenterProperty(), padRadius), touchOrClick))
                {
                    padPressed = true;
                    padTouchPos = touchOrClick;
                }
                if (keyPressedUp || keyPressedDown || keyPressedLeft || keyPressedRight)
                {
                    padPressed = true;
                }
                INPUT_DEBUG(std::string("padPressed=") + (padPressed ? "true" : "false"));
                Def::ButtonGlyph pressedGlyph = ButtonDetect(touchOrClick);
                INPUT_DEBUG(std::string("pressedGlyph =") + std::to_string(static_cast<intcs>(pressedGlyph)));
                if (pressedGlyph != Def::ButtonGlyph::None)
                {
                    pressedGlyphs.push_back(pressedGlyph);
                }
                if (keyboardPressed)
                {
                    switch (keyPressed)
                    {
                    case Keys::LeftControl: pressedGlyph = Def::ButtonGlyph::PlayJump;
                        pressedGlyphs.push_back(pressedGlyph);
                        break;
                    case Keys::Space: pressedGlyph = Def::ButtonGlyph::PlayAction;
                        pressedGlyphs.push_back(pressedGlyph);
                        break;
                    case Keys::Escape: pressedGlyph = Def::ButtonGlyph::PlayPause;
                        pressedGlyphs.push_back(pressedGlyph);
                        break;
                    }
                }

                if ((getPhaseProperty() == Def::Phase::MainSetup || getPhaseProperty() == Def::Phase::PlaySetup) &&
                    accelSlider.Move(touchOrClick))
                {
                    gameData->setAccelSensitivityProperty(accelSlider.getValueProperty());
                }
                switch (pressedGlyph)
                {
                case Def::ButtonGlyph::PlayJump:
                    INPUT_DEBUG("Jumping detected");
                    accelWaitZero = false;
                    keyPress |= ToRaw(KeyPressFlags::Jump);
                    break;
                case Def::ButtonGlyph::PlayDown:
                    accelWaitZero = false;
                    keyPress |= ToRaw(KeyPressFlags::Down);
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
            Def::isNotOneOf(
                buttonGlyph, {
                    Def::ButtonGlyph::None,
                    Def::ButtonGlyph::PlayAction,
                    Def::ButtonGlyph::Cheat11,
                    Def::ButtonGlyph::Cheat12,
                    Def::ButtonGlyph::Cheat21,
                    Def::ButtonGlyph::Cheat22,
                    Def::ButtonGlyph::Cheat31,
                    Def::ButtonGlyph::Cheat32
                }
            )
            &&
            lastButtonDown == Def::ButtonGlyph::None)
        {
            TinyPoint pos(320, 240);
            sound->PlayImage(SoundChannel::SoundChannel0, pos);
        }
        if (buttonGlyph == Def::ButtonGlyph::None && lastButtonDown != Def::ButtonGlyph::None)
        {
            buttonPressed = lastButtonDown;
        }
        lastButtonDown = buttonGlyph;
        if (padPressed)
        {
            using namespace std::string_literals;
            using std::to_string;

            INPUT_DEBUG("getPadCenter().X="s + to_string(getPadCenterProperty().X));
            INPUT_DEBUG("getPadCenter().Y="s + to_string(getPadCenterProperty().Y));
            INPUT_DEBUG("padTouchPos.X="s + to_string(padTouchPos.X));
            INPUT_DEBUG("padTouchPos.Y="s + to_string(padTouchPos.Y));
            INPUT_DEBUG("keyPressedUp="s + to_string(keyPressedUp));
            INPUT_DEBUG("keyPressedDown="s + to_string(keyPressedDown));
            INPUT_DEBUG("keyPressedLeft="s + to_string(keyPressedLeft));
            INPUT_DEBUG("keyPressedRight="s + to_string(keyPressedRight));
            {
                if (keyPressedUp)
                {
                    padTouchPos.Y = getPadCenterProperty().Y - 30;
                    padTouchPos.X = getPadCenterProperty().X;
                    if (keyPressedLeft) padTouchPos.X = getPadCenterProperty().X - 30;
                    if (keyPressedRight) padTouchPos.X = getPadCenterProperty().X + 30;
                }
                if (keyPressedDown)
                {
                    padTouchPos.Y = getPadCenterProperty().Y + 30;
                    padTouchPos.X = getPadCenterProperty().X;
                    if (keyPressedLeft) padTouchPos.X = getPadCenterProperty().X - 30;
                    if (keyPressedRight) padTouchPos.X = getPadCenterProperty().X + 30;
                }
                if (keyPressedLeft)
                {
                    padTouchPos.X = getPadCenterProperty().X - 30;
                    padTouchPos.Y = getPadCenterProperty().Y;
                    if (keyPressedUp) padTouchPos.Y = getPadCenterProperty().Y - 30;
                    if (keyPressedDown) padTouchPos.Y = getPadCenterProperty().Y + 30;
                }
                if (keyPressedRight)
                {
                    padTouchPos.X = getPadCenterProperty().X + 30;
                    padTouchPos.Y = getPadCenterProperty().Y;
                    if (keyPressedUp) padTouchPos.Y = getPadCenterProperty().Y - 30;
                    if (keyPressedDown) padTouchPos.Y = getPadCenterProperty().Y + 30;
                }
            }
            double horizontalPosition = padTouchPos.X - getPadCenterProperty().X;
            double verticalPosition = padTouchPos.Y - getPadCenterProperty().Y;

            if (horizontalPosition > 20.0)
            {
                horizontalChange += 1.0;
                INPUT_DEBUG(" horizontalChange += 1.0;");
            }
            if (horizontalPosition < -20.0)
            {
                horizontalChange -= 1.0;
                INPUT_DEBUG(" horizontalChange -= 1.0;");
            }
            if (verticalPosition > 20.0)
            {
                verticalChange += 1.0;
                INPUT_DEBUG(" verticalPosition += 1.0;");
            }
            if (verticalPosition < -20.0)
            {
                verticalChange -= 1.0;
                INPUT_DEBUG(" verticalPosition -= 1.0;");
            }
        }
        if (accelStarted)
        {
            horizontalChange = accelSpeedX;
            verticalChange = 0.0;
            if (((unsigned int)keyPress & ToRaw(KeyPressFlags::Down)) != 0)
            {
                verticalChange = 1.0;
            }
#ifdef MODERN
            // In ghost mode, the Jump button moves Blupi upward (verticalChange = -1).
            if (decor != nullptr && decor->IsGhost()
                && ((unsigned int)keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
            {
                verticalChange = -1.0;
            }
#endif
        }
        decor->SetSpeedX(horizontalChange);
        decor->SetSpeedY(verticalChange);
        decor->KeyChange(keyPress);
    }

    Def::ButtonGlyph InputPad::ButtonDetect(TinyPoint touchOrClick)
    {
#ifdef INPUT_DISABLED
        return Def::ButtonGlyph::None;
#endif
        const auto buttonGlyphs = getButtonGlyphsProperty();
        for (auto i = buttonGlyphs.rbegin(); i != buttonGlyphs.rend(); ++i)
        {
            Def::ButtonGlyph buttonGlyph = *i;
            TinyRect buttonRect = GetButtonRect(buttonGlyph);

            if (buttonGlyph == Def::ButtonGlyph::PlayJump || buttonGlyph == Def::ButtonGlyph::PlayAction || buttonGlyph
                == Def::ButtonGlyph::PlayDown || buttonGlyph == Def::ButtonGlyph::PlayPause)
            {
                buttonRect = Misc::Inflate(buttonRect, 20);
            }

            if (Misc::IsInside(buttonRect, touchOrClick))
            {
                return buttonGlyph;
            }
        }
        return Def::ButtonGlyph::None;
    }

    void InputPad::Draw()
    {
#ifdef INPUT_DISABLED
        return;
#endif
        if (!accelStarted && getPhaseProperty() == Def::Phase::Play)
        {
            pixmap->DrawIcon(PixmapChannel::Pad, 0, GetPadBounds(getPadCenterProperty(), padRadius / 2), 1.0, false);
            TinyPoint center = (padPressed ? padTouchPos : getPadCenterProperty());
            pixmap->DrawIcon(PixmapChannel::Pad, 1, GetPadBounds(center, padRadius / 2), 1.0, false);
        }
        for (Def::ButtonGlyph buttonGlyph : getButtonGlyphsProperty())
        {
            bool pressed = VECTOR_CONTAINS(pressedGlyphs, buttonGlyph);
            bool selected = false;
            if (buttonGlyph >= Def::ButtonGlyph::InitGamerA && buttonGlyph <= Def::ButtonGlyph::InitGamerC)
            {
                int selectedGamer = (int)(static_cast<intcs>(buttonGlyph) - 1);
                selected = selectedGamer == gameData->getSelectedGamerProperty();
            }
            if (buttonGlyph == Def::ButtonGlyph::SetupSounds)
            {
                selected = gameData->getSoundsProperty();
            }
            if (buttonGlyph == Def::ButtonGlyph::SetupJump)
            {
                selected = gameData->getJumpRightProperty();
            }
            if (buttonGlyph == Def::ButtonGlyph::SetupZoom)
            {
                selected = gameData->getAutoZoomProperty();
            }
            if (buttonGlyph == Def::ButtonGlyph::SetupAccel)
            {
                selected = gameData->getAccelActiveProperty();
            }
            pixmap->DrawInputButton(GetButtonRect(buttonGlyph), buttonGlyph, pressed, selected);
        }
        if ((getPhaseProperty() == Def::Phase::MainSetup || getPhaseProperty() == Def::Phase::PlaySetup) && gameData->
            getAccelActiveProperty())
        {
            accelSlider.Draw(*pixmap);
        }
#ifdef MODERN
        // Sync ghost entry in activePersistentCheats with the actual runtime ghost state,
        // so the UI label is never shown after Back/level-exit when ghost is no longer active.
        activePersistentCheats.erase(
            std::remove_if(
                activePersistentCheats.begin(),
                activePersistentCheats.end(),
                [this](const std::string& cheatName)
                {
                    return cheatName == "ghost" && (decor == nullptr || !decor->IsGhost());
                }
            ),
            activePersistentCheats.end()
        );
        // When leaving a level (not in Play phase), reset zoom cheat to normal.
        if (getPhaseProperty() != Def::Phase::Play && zoom_cheat_state != ZoomCheat::Zoom100)
        {
            zoom_cheat_state = ZoomCheat::Zoom100;
            if (decor != nullptr)
            {
                decor->SetCheatZoom(1.0);
            }
            activePersistentCheats.erase(
                std::remove_if(activePersistentCheats.begin(), activePersistentCheats.end(),
                    [](const std::string& s){ return s == "zoom50" || s == "zoom25" || s == "zoom12"; }),
                activePersistentCheats.end());
        }
#endif
#ifdef MODERN
        if (getPhaseProperty() == Def::Phase::Play && debug_cheat_enabled)
        {
            constexpr double dbgScale = 0.50;
            constexpr int dbgLineH = 16;
            constexpr int dbgPadding = 3;
            TinyPoint origin = pixmap->getOriginProperty();
            std::vector<std::string> dbgLines;

            // --- Gameplay state from Decor ---
            if (decor != nullptr)
            {
                // Game time: convert frame ticks to seconds (50 fps assumed)
                int ticks = decor->GetTime();
                int secs  = ticks / 50;
                int mins  = secs / 60;
                secs      = secs % 60;
                char timeBuf[32];
                std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d (%d)", mins, secs, ticks);
                dbgLines.push_back("time: " + std::string(timeBuf));

                dbgLines.push_back("mission: " + std::to_string(decor->GetMission()));
                dbgLines.push_back("region:  " + std::to_string(decor->GetRegionDebug()));
                dbgLines.push_back("lives:   " + std::to_string(decor->GetNbVies()));

                TinyPoint bp = decor->GetBlupiPos();
                dbgLines.push_back("pos: " + std::to_string(bp.X) + "," + std::to_string(bp.Y));

                // Cell coordinates (each tile is 64px)
                dbgLines.push_back("cel: " + std::to_string(bp.X / 64) + "," + std::to_string(bp.Y / 64));

                // Velocities — format with one decimal
                char vxBuf[24], vyBuf[24];
                std::snprintf(vxBuf, sizeof(vxBuf), "%.1f", decor->GetBlupiVX());
                std::snprintf(vyBuf, sizeof(vyBuf), "%.1f", decor->GetBlupiVY());
                dbgLines.push_back("vx: " + std::string(vxBuf) + "  vy: " + std::string(vyBuf));

                // Mode flags — show only active ones
                std::string flags;
                if (decor->GetBlupiAir())    flags += "air ";
                if (decor->GetBlupiHelico()) flags += "heli ";
                if (decor->GetBlupiSkate())  flags += "skate ";
                if (decor->GetBlupiNage())   flags += "swim ";
                if (decor->IsGhost())        flags += "ghost ";
                if (flags.empty()) flags = "-";
                dbgLines.push_back("mode: " + flags);
            }

            // --- Input / system state ---
            if (game1 != nullptr)
            {
                dbgLines.push_back("speed: " + std::to_string(ToRaw(game1->getGameSpeed())) + "x");
            }
            dbgLines.push_back("touches: " + std::to_string(touchOrClickCount));
            dbgLines.push_back("pad: " + std::string(padPressed ? "on" : "off")
                               + "  accel: " + std::string(accelStarted ? "on" : "off"));

            int maxW = 0;
            for (const auto& l : dbgLines)
            {
                int w = Text::GetTextWidth(l, dbgScale);
                if (w > maxW) maxW = w;
            }
            int totalH = static_cast<int>(dbgLines.size()) * dbgLineH;
            // Use fixed logical game width (640) so the overlay stays in the
            // visible area regardless of screen aspect ratio.
            constexpr int kLogicalWidth = 640;
            int rightEdge = kLogicalWidth - 5;
            int leftEdge  = rightEdge - maxW;
            TinyRect bgRect;
            bgRect.Left   = leftEdge - dbgPadding + origin.X;
            bgRect.Right  = rightEdge + dbgPadding + origin.X;
            bgRect.Top    = 5 - dbgPadding + origin.Y;
            bgRect.Bottom = 5 + totalH + dbgPadding + origin.Y;
            pixmap->DrawIcon(PixmapChannel::Pad, 15, bgRect, 0.6, false);
            int posY = 5;
            for (const auto& l : dbgLines)
            {
                TinyPoint pos{leftEdge, posY};
                Text::DrawTextLeft(*pixmap, pos, l, dbgScale);
                posY += dbgLineH;
            }
        }
#endif
#ifndef LEGACY
        if (getPhaseProperty() == Def::Phase::Play && !activePersistentCheats.empty())
        {
            constexpr int padding = 3;
            constexpr double cheatTextScale = 0.55;
            constexpr int lineHeight = 18;
            int maxWidth = 0;
            for (const std::string& cheatName : activePersistentCheats)
            {
                int w = Text::GetTextWidth(cheatName, cheatTextScale);
                if (w > maxWidth) maxWidth = w;
            }
            int totalHeight = static_cast<int>(activePersistentCheats.size()) * lineHeight;
            TinyPoint origin = pixmap->getOriginProperty();
            TinyRect bgRect;
            bgRect.Left   = 5 - padding + origin.X;
            bgRect.Right  = 5 + maxWidth + padding + origin.X;
            bgRect.Top    = 5 - padding + origin.Y;
            bgRect.Bottom = 5 + totalHeight + padding + origin.Y;
            pixmap->DrawIcon(PixmapChannel::Pad, 15, bgRect, 0.6, false);
            TinyPoint pos{5, 5};
            for (const std::string& cheatName : activePersistentCheats)
            {
                Text::DrawTextLeft(*pixmap, pos, cheatName, cheatTextScale);
                pos.Y += lineHeight;
            }
        }
#endif
#ifdef MODERN
        // Cheats overlay: show all cheat names centred on screen for 5 seconds.
        if (cheats_display_timer > 0)
        {
            cheats_display_timer--;
            // Collect all cheat names from the static cheatEntries table.
            static const char* allCheatNames[] = {
                "buildofficialmissions", "opendoors", "cleanall", "megablupi",
                "layegg", "killegg", "funskate", "givecopter", "jeepdrive",
                "alltreasure", "endgoal", "showsecret", "roundshield", "quicklollypop",
                "tenbombs", "birdlime", "drivetank", "powercharge", "hidedrink",
                "iovercraft", "udynamite", "weelkeys",
                "quick", "ghost", "debug", "zoom", "cheats"
            };
            constexpr int allCheatNamesCount = static_cast<int>(sizeof(allCheatNames) / sizeof(allCheatNames[0]));
            constexpr double chScale = 0.55;
            constexpr int chLineH = 18;
            constexpr int chPadding = 6;
            constexpr int chColGap = 12;  // gap between the two columns
            constexpr int kGameW = 480;
            constexpr int kGameH = 480;
            // Split cheats into two columns.
            int col0Count = (allCheatNamesCount + 1) / 2;   // left column (ceiling half)
            int col1Count = allCheatNamesCount - col0Count;  // right column
            int maxW0 = 0, maxW1 = 0;
            for (int i = 0; i < col0Count; i++)
            {
                int w = Text::GetTextWidth(allCheatNames[i], chScale);
                if (w > maxW0) maxW0 = w;
            }
            for (int i = col0Count; i < allCheatNamesCount; i++)
            {
                int w = Text::GetTextWidth(allCheatNames[i], chScale);
                if (w > maxW1) maxW1 = w;
            }
            int totalW = maxW0 + chColGap + maxW1;
            int rowCount = col0Count;  // left column has more or equal rows
            int totalH = rowCount * chLineH;
            // Centre the overlay in the logical game area (640×480).
            int bgLeft  = (kGameW - totalW)  / 2 - chPadding;
            int bgTop   = (kGameH - totalH) / 2 - chPadding;
            int bgRight  = bgLeft + totalW  + chPadding * 2;
            int bgBottom = bgTop  + totalH + chPadding * 2;
            TinyPoint chOrigin = pixmap->getOriginProperty();
            TinyRect bgRect;
            bgRect.Left   = bgLeft  + chOrigin.X;
            bgRect.Right  = bgRight + chOrigin.X;
            bgRect.Top    = bgTop   + chOrigin.Y;
            bgRect.Bottom = bgBottom + chOrigin.Y;
            pixmap->DrawIcon(PixmapChannel::Pad, 15, bgRect, 0.7, false);
            int startX0 = bgLeft + chPadding;
            int startX1 = startX0 + maxW0 + chColGap;
            int startY  = bgTop + chPadding;
            // Draw left column.
            for (int i = 0; i < col0Count; i++)
            {
                TinyPoint pos{startX0, startY + i * chLineH};
                Text::DrawTextLeft(*pixmap, pos, allCheatNames[i], chScale);
            }
            // Draw right column.
            for (int i = 0; i < col1Count; i++)
            {
                TinyPoint pos{startX1, startY + i * chLineH};
                Text::DrawTextLeft(*pixmap, pos, allCheatNames[col0Count + i], chScale);
            }
        }
        if (getPhaseProperty() == Def::Phase::Play && game1 != nullptr)
        {
            GameSpeed spd = game1->getGameSpeed();
            if (spd > GameSpeed::Normal)
            {
                TinyRect drawBounds = pixmap->getDrawBoundsProperty();
                std::string speedText = std::to_string(ToRaw(spd)) + "x";
                constexpr double speedTextScale = 0.55;
                constexpr int padding = 3;
                int textW = Text::GetTextWidth(speedText, speedTextScale);
                constexpr int textH = 14;
                int baseY = drawBounds.getHeightProperty() - 22;
                TinyPoint speedOrigin = pixmap->getOriginProperty();
                TinyRect bgRect;
                bgRect.Left   = 5 - padding + speedOrigin.X;
                bgRect.Right  = 5 + textW + padding + speedOrigin.X;
                bgRect.Top    = baseY - padding + speedOrigin.Y;
                bgRect.Bottom = baseY + textH + padding + speedOrigin.Y;
                pixmap->DrawIcon(PixmapChannel::Pad, 15, bgRect, 0.6, false);
                TinyPoint pos{5, baseY};
                Text::DrawTextLeft(*pixmap, pos, speedText, speedTextScale);
            }
        }

        // --- Virtual on-screen keyboard drawing ---
        if (virtualKeyboardVisible)
        {
            const VirtualKeyboardLayout layout = GetVirtualKeyboardLayout();
            const int kKeyW   = layout.keyW;
            const int kKeyH   = layout.keyH;
            const int kKeyGap = layout.keyGap;
            const int kRowGap = layout.rowGap;
            const int kPanelX = layout.panelX;
            const int kPanelY = layout.panelY;
            const TinyPoint& origin = layout.origin;
            constexpr double kKeyOpacity = 0.75;
            const double kKeyTextScale = kKeyW / 84.0;

            // Key label text: F5-F8 left, F12 right-aligned, then QWERTY rows.
            static const char* fnLeftLabels[]  = { "F5","F6","F7","F8" };
            static const char* fnRightLabels[] = { "F12" };
            static const char* row0Labels[] = { "Q","W","E","R","T","Y","U","I","O","P" };
            static const char* row1Labels[] = { "A","S","D","F","G","H","J","K","L" };
            static const char* row2Labels[] = { "Z","X","C","V","B","N","M" };

            struct KbRow { const char* const* labels; int count; int offsetX; int rowY; };
            const KbRow rows[] = {
                { fnLeftLabels,  4, 0,                        0 },
                { fnRightLabels, 1, layout.f12OffsetX,        0 },
                { row0Labels, 10, 0,                          kKeyH + kRowGap },
                { row1Labels,  9, (kKeyW + kKeyGap) / 2,     2 * (kKeyH + kRowGap) },
                { row2Labels,  7, (kKeyW + kKeyGap),          3 * (kKeyH + kRowGap) },
            };

            // 1. Draw keyboard panel background.
            pixmap->DrawIcon(PixmapChannel::Pad, 15, layout.panelRect, 0.85, false);

            // 2. Draw key rows.
            for (const KbRow& row : rows)
            {
                for (int ki = 0; ki < row.count; ki++)
                {
                    TinyRect keyRect;
                    keyRect.Left   = kPanelX + row.offsetX + ki * (kKeyW + kKeyGap) + origin.X;
                    keyRect.Right  = keyRect.Left + kKeyW;
                    keyRect.Top    = kPanelY + row.rowY + origin.Y;
                    keyRect.Bottom = keyRect.Top + kKeyH;
                    pixmap->DrawIcon(PixmapChannel::Pad, 15, keyRect, kKeyOpacity, false);
                    const std::string label = row.labels[ki];
                    int tw = Text::GetTextWidth(label, kKeyTextScale);
                    int tx = kPanelX + row.offsetX + ki * (kKeyW + kKeyGap) + (kKeyW - tw) / 2;
                    int ty = kPanelY + row.rowY + (kKeyH - 12) / 2;
                    Text::DrawTextLeft(*pixmap, TinyPoint{tx, ty}, label, kKeyTextScale);
                }
            }

            // 3. Draw close button (X) last — anchored to panelRect top-right, always on top.
            pixmap->DrawIcon(PixmapChannel::Pad, 15, layout.closeRect, 1.0, false);
            {
                int tw = Text::GetTextWidth("X", kKeyTextScale);
                int tx = layout.closeRect.Left + (kKeyW - tw) / 2 - layout.origin.X;
                int ty = layout.closeRect.Top  + (kKeyH - 12) / 2 - layout.origin.Y;
                Text::DrawTextLeft(*pixmap, TinyPoint{tx, ty}, "X", kKeyTextScale);
            }
        }
#endif
    }

    TinyRect InputPad::GetPadBounds(TinyPoint center, int radius)
    {
        return TinyRect(center.X - radius, center.X + radius, center.Y - radius, center.Y + radius);
    }

    TinyRect InputPad::GetButtonRect(Def::ButtonGlyph glyph)
    {
#ifdef INPUT_DISABLED
        return {};
#endif
        TinyRect drawBounds = pixmap->getDrawBoundsProperty();
        double drawBoundsWidth = drawBounds.getWidthProperty();
        double drawBoundsHeight = drawBounds.getHeightProperty();
        double buttonSizeFactor1 = drawBoundsHeight / 5.0;
        double buttonSizeFactor2 = drawBoundsHeight * 140.0 / 480.0;
        double cheatButtonSizeFactor = drawBoundsHeight / 3.5;
        if (glyph >= Def::ButtonGlyph::Cheat1 && glyph <= Def::ButtonGlyph::Cheat9)
        {
            int cheatNumber = (int)(static_cast<intcs>(glyph) - static_cast<intcs>(Def::ButtonGlyph::Cheat1));
            TinyRect result = TinyRect();
            result.Left = 80 * cheatNumber;
            result.Right = 80 * (cheatNumber + 1);
            result.Top = 0;
            result.Bottom = 80;
            return result;
        }
        int leftXForButtonsInLeftColumn = (int)(20.0 + buttonSizeFactor2 * 0.0);
        int rightXForButtonsInLeftColumn = (int)(20.0 + buttonSizeFactor2 * 0.5);
        switch (glyph)
        {
        case Def::ButtonGlyph::InitGamerA:
            {
                TinyRect result19 = TinyRect();
                result19.Left = leftXForButtonsInLeftColumn;
                result19.Right = rightXForButtonsInLeftColumn;
                result19.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.1);
                result19.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.6);
                return result19;
            }
        case Def::ButtonGlyph::InitGamerB:
            {
                TinyRect result18 = TinyRect();
                result18.Left = leftXForButtonsInLeftColumn;
                result18.Right = rightXForButtonsInLeftColumn;
                result18.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.6);
                result18.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.1);
                return result18;
            }
        case Def::ButtonGlyph::InitGamerC:
            {
                TinyRect result15 = TinyRect();
                result15.Left = leftXForButtonsInLeftColumn;
                result15.Right = rightXForButtonsInLeftColumn;
                result15.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.1);
                result15.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.6);
                return result15;
            }
        case Def::ButtonGlyph::InitSetup:
            {
                TinyRect result14 = TinyRect();
                result14.Left = leftXForButtonsInLeftColumn;
                result14.Right = rightXForButtonsInLeftColumn;
                result14.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.5);
                result14.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.0);
                return result14;
            }
        case Def::ButtonGlyph::InitPlay:
            {
                TinyRect result11 = TinyRect();
                result11.Left = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 1.0);
                result11.Right = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.0);
                result11.Top = (int)(drawBoundsHeight - 40.0 - buttonSizeFactor2 * 1.0);
                result11.Bottom = (int)(drawBoundsHeight - 40.0 - buttonSizeFactor2 * 0.0);
                return result11;
            }
        case Def::ButtonGlyph::InitBuy:
        case Def::ButtonGlyph::InitRanking:
            {
                TinyRect result10 = TinyRect();
                result10.Left = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.75);
                result10.Right = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.25);
                result10.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.1);
                result10.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.6);
                return result10;
            }
        case Def::ButtonGlyph::PauseMenu:
            {
                TinyRect result37 = TinyRect();
                result37.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * -0.21);
                result37.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 0.79);
                result37.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.2);
                result37.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.2);
                return result37;
            }
        case Def::ButtonGlyph::PauseBack:
            {
                TinyRect result36 = TinyRect();
                result36.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 0.79);
                result36.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 1.79);
                result36.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.2);
                result36.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.2);
                return result36;
            }
        case Def::ButtonGlyph::PauseSetup:
            {
                TinyRect result35 = TinyRect();
                result35.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 1.79);
                result35.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 2.79);
                result35.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.2);
                result35.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.2);
                return result35;
            }
        case Def::ButtonGlyph::PauseRestart:
            {
                TinyRect result34 = TinyRect();
                result34.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 2.79);
                result34.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 3.79);
                result34.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.2);
                result34.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.2);
                return result34;
            }
        case Def::ButtonGlyph::PauseContinue:
            {
                TinyRect result33 = TinyRect();
                result33.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 3.79);
                result33.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 4.79);
                result33.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.2);
                result33.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.2);
                return result33;
            }
        case Def::ButtonGlyph::ResumeMenu:
            {
                TinyRect result32 = TinyRect();
                result32.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 1.29);
                result32.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 2.29);
                result32.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.2);
                result32.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.2);
                return result32;
            }
        case Def::ButtonGlyph::ResumeContinue:
            {
                TinyRect result31 = TinyRect();
                result31.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 2.29);
                result31.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 3.29);
                result31.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.2);
                result31.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.2);
                return result31;
            }
        case Def::ButtonGlyph::WinLostReturn:
            {
                TinyRect result30 = TinyRect();
                result30.Left = (int)((double)getPixmapOriginProperty().X + drawBoundsWidth - buttonSizeFactor1 * 2.2);
                result30.Right = (int)((double)getPixmapOriginProperty().X + drawBoundsWidth - buttonSizeFactor1 * 1.2);
                result30.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor1 * 0.2);
                result30.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor1 * 1.2);
                return result30;
            }
        case Def::ButtonGlyph::TrialBuy:
            {
                TinyRect result29 = TinyRect();
                result29.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 2.5);
                result29.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 3.5);
                result29.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.1);
                result29.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.1);
                return result29;
            }
        case Def::ButtonGlyph::TrialCancel:
            {
                TinyRect result28 = TinyRect();
                result28.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 3.5);
                result28.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 4.5);
                result28.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.1);
                result28.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.1);
                return result28;
            }
        case Def::ButtonGlyph::RankingContinue:
            {
                TinyRect result27 = TinyRect();
                result27.Left = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 3.5);
                result27.Right = (int)((double)getPixmapOriginProperty().X + buttonSizeFactor2 * 4.5);
                result27.Top = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 2.1);
                result27.Bottom = (int)((double)getPixmapOriginProperty().Y + buttonSizeFactor2 * 3.1);
                return result27;
            }
        case Def::ButtonGlyph::SetupSounds:
            {
                TinyRect result26 = TinyRect();
                result26.Left = leftXForButtonsInLeftColumn;
                result26.Right = rightXForButtonsInLeftColumn;
                result26.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.0);
                result26.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.5);
                return result26;
            }
        case Def::ButtonGlyph::SetupJump:
            {
                TinyRect result25 = TinyRect();
                result25.Left = leftXForButtonsInLeftColumn;
                result25.Right = rightXForButtonsInLeftColumn;
                result25.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.5);
                result25.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.0);
                return result25;
            }
        case Def::ButtonGlyph::SetupZoom:
            {
                TinyRect result24 = TinyRect();
                result24.Left = leftXForButtonsInLeftColumn;
                result24.Right = rightXForButtonsInLeftColumn;
                result24.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.0);
                result24.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.5);
                return result24;
            }
        case Def::ButtonGlyph::SetupAccel:
            {
                TinyRect result23 = TinyRect();
                result23.Left = leftXForButtonsInLeftColumn;
                result23.Right = rightXForButtonsInLeftColumn;
                result23.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.5);
                result23.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.0);
                return result23;
            }
        case Def::ButtonGlyph::SetupReset:
            {
                TinyRect result22 = TinyRect();
                result22.Left = (int)(450.0 + buttonSizeFactor2 * 0.0);
                result22.Right = (int)(450.0 + buttonSizeFactor2 * 0.5);
                result22.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 2.0);
                result22.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 1.5);
                return result22;
            }
        case Def::ButtonGlyph::SetupReturn:
            {
                TinyRect result21 = TinyRect();
                result21.Left = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.8);
                result21.Right = (int)(drawBoundsWidth - 20.0 - buttonSizeFactor2 * 0.0);
                result21.Top = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.8);
                result21.Bottom = (int)(drawBoundsHeight - 20.0 - buttonSizeFactor2 * 0.0);
                return result21;
            }
        case Def::ButtonGlyph::PlayPause:
            {
                TinyRect result20 = TinyRect();
                result20.Left = (int)(drawBoundsWidth - buttonSizeFactor1 * 0.7);
                result20.Right = (int)(drawBoundsWidth - buttonSizeFactor1 * 0.2);
                result20.Top = (int)(buttonSizeFactor1 * 0.2);
                result20.Bottom = (int)(buttonSizeFactor1 * 0.7);
                return result20;
            }
        case Def::ButtonGlyph::PlayAction:
            {
                if (gameData->getJumpRightProperty())
                {
                    TinyRect result16 = TinyRect();
                    result16.Left = (int)((double)drawBounds.getWidthProperty() - buttonSizeFactor1 * 1.2);
                    result16.Right = (int)((double)drawBounds.getWidthProperty() - buttonSizeFactor1 * 0.2);
                    result16.Top = (int)(drawBoundsHeight - buttonSizeFactor1 * 2.6);
                    result16.Bottom = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.6);
                    return result16;
                }
                TinyRect result17 = TinyRect();
                result17.Left = (int)(buttonSizeFactor1 * 0.2);
                result17.Right = (int)(buttonSizeFactor1 * 1.2);
                result17.Top = (int)(drawBoundsHeight - buttonSizeFactor1 * 2.6);
                result17.Bottom = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.6);
                return result17;
            }
        case Def::ButtonGlyph::PlayJump:
            {
                if (gameData->getJumpRightProperty())
                {
                    TinyRect result12 = TinyRect();
                    result12.Left = (int)((double)drawBounds.getWidthProperty() - buttonSizeFactor1 * 1.2);
                    result12.Right = (int)((double)drawBounds.getWidthProperty() - buttonSizeFactor1 * 0.2);
                    result12.Top = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                    result12.Bottom = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                    return result12;
                }
                TinyRect result13 = TinyRect();
                result13.Left = (int)(buttonSizeFactor1 * 0.2);
                result13.Right = (int)(buttonSizeFactor1 * 1.2);
                result13.Top = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                result13.Bottom = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                return result13;
            }
        case Def::ButtonGlyph::PlayDown:
            {
                if (gameData->getJumpRightProperty())
                {
                    TinyRect result8 = TinyRect();
                    result8.Left = (int)(buttonSizeFactor1 * 0.2);
                    result8.Right = (int)(buttonSizeFactor1 * 1.2);
                    result8.Top = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                    result8.Bottom = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                    return result8;
                }
                TinyRect result9 = TinyRect();
                result9.Left = (int)((double)drawBounds.getWidthProperty() - buttonSizeFactor1 * 1.2);
                result9.Right = (int)((double)drawBounds.getWidthProperty() - buttonSizeFactor1 * 0.2);
                result9.Top = (int)(drawBoundsHeight - buttonSizeFactor1 * 1.2);
                result9.Bottom = (int)(drawBoundsHeight - buttonSizeFactor1 * 0.2);
                return result9;
            }
        case Def::ButtonGlyph::Cheat11:
            {
                TinyRect result7 = TinyRect();
                result7.Left = (int)(cheatButtonSizeFactor * 0.0);
                result7.Right = (int)(cheatButtonSizeFactor * 1.0);
                result7.Top = (int)(cheatButtonSizeFactor * 0.0);
                result7.Bottom = (int)(cheatButtonSizeFactor * 1.0);
                return result7;
            }
        case Def::ButtonGlyph::Cheat12:
            {
                TinyRect result6 = TinyRect();
                result6.Left = (int)(cheatButtonSizeFactor * 0.0);
                result6.Right = (int)(cheatButtonSizeFactor * 1.0);
                result6.Top = (int)(cheatButtonSizeFactor * 1.0);
                result6.Bottom = (int)(cheatButtonSizeFactor * 2.0);
                return result6;
            }
        case Def::ButtonGlyph::Cheat21:
            {
                TinyRect result5 = TinyRect();
                result5.Left = (int)(cheatButtonSizeFactor * 1.0);
                result5.Right = (int)(cheatButtonSizeFactor * 2.0);
                result5.Top = (int)(cheatButtonSizeFactor * 0.0);
                result5.Bottom = (int)(cheatButtonSizeFactor * 1.0);
                return result5;
            }
        case Def::ButtonGlyph::Cheat22:
            {
                TinyRect result4 = TinyRect();
                result4.Left = (int)(cheatButtonSizeFactor * 1.0);
                result4.Right = (int)(cheatButtonSizeFactor * 2.0);
                result4.Top = (int)(cheatButtonSizeFactor * 1.0);
                result4.Bottom = (int)(cheatButtonSizeFactor * 2.0);
                return result4;
            }
        case Def::ButtonGlyph::Cheat31:
            {
                TinyRect result3 = TinyRect();
                result3.Left = (int)(cheatButtonSizeFactor * 2.0);
                result3.Right = (int)(cheatButtonSizeFactor * 3.0);
                result3.Top = (int)(cheatButtonSizeFactor * 0.0);
                result3.Bottom = (int)(cheatButtonSizeFactor * 1.0);
                return result3;
            }
        case Def::ButtonGlyph::Cheat32:
            {
                TinyRect result2 = TinyRect();
                result2.Left = (int)(cheatButtonSizeFactor * 2.0);
                result2.Right = (int)(cheatButtonSizeFactor * 3.0);
                result2.Top = (int)(cheatButtonSizeFactor * 1.0);
                result2.Bottom = (int)(cheatButtonSizeFactor * 2.0);
                return result2;
            }
        default:
            return TinyRect();
        }
    }

    void InputPad::StartAccel()
    {
#ifdef INPUT_DISABLED
        return;
#endif
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
#ifdef INPUT_DISABLED
        return;
#endif
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


    void InputPad::HandleAccelSensorCurrentValueChanged(
        Microsoft::Devices::Sensors::SensorReadingEventArgs<Microsoft::Devices::Sensors::AccelerometerReading> e)
    {
#ifdef INPUT_DISABLED
        return;
#endif
        //IL_0001: Unknown result type (might be due to invalid IL or missing references)
        //IL_0006: Unknown result type (might be due to invalid IL or missing references)

        Microsoft::Devices::Sensors::AccelerometerReading sensorReading = e.getSensorReadingProperty();
        float y = ((Microsoft::Devices::Sensors::AccelerometerReading)(sensorReading)).getAccelerationProperty().Y;
        float sensitivityThreshold = (1.0f - (float)gameData->getAccelSensitivityProperty()) * 0.06f + 0.04f;
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
