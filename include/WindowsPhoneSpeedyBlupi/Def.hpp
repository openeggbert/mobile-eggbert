/**
 * @file Def.hpp
 * @brief Global game constants, enumerations, and static helpers for the Speedy Blupi port.
 *
 * @details Contains the Def class, which aggregates compile-time integer constants for
 * sprite-sheet cell dimensions, viewport size, legacy channel identifiers, and the
 * Phase and ButtonGlyph enumerations that drive the game's state machine and UI overlay.
 */

#pragma once

#include <algorithm>
#include <ranges>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "WindowsPhoneSpeedyBlupi/def/BlupiAction.hpp"
#include "WindowsPhoneSpeedyBlupi/def/Direction.hpp"
#include "WindowsPhoneSpeedyBlupi/def/SecretPower.hpp"
#include "WindowsPhoneSpeedyBlupi/def/KeyPressFlags.hpp"
#include "WindowsPhoneSpeedyBlupi/def/GameSpeed.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::intcs;

    /**
     * @class Def
     * @brief Provides global game constants, enums, and static helper members.
     *
     * @details This class is a C++ port of the original C# static class Def from
     * WindowsPhoneSpeedyBlupi. All members are static; the class cannot be
     * instantiated or destroyed by client code.
     *
     * @note Status: Ported
     */
    class Def
    {
    public:
        Def() = delete;
        ~Def() = delete;

        /**
         * @brief Represents the current high-level game phase (screen/mode).
         *
         * @details The game is always in exactly one Phase. Transitions are performed
         * by Game1::SetPhase(). The phase controls which UI buttons are drawn and
         * which update path is executed each frame.
         *
         * @note Status: Ported
         */
        enum class Phase
        {
            None,       ///< @brief No phase active (initial / uninitialised state).
            First,      ///< @brief Very first frame after startup.
            Wait,       ///< @brief Waiting for an asynchronous operation to complete.
            Init,       ///< @brief Main-menu / gamer-select screen.
            Play,       ///< @brief Active gameplay.
            Pause,      ///< @brief Game paused (pause overlay displayed).
            Lost,       ///< @brief Player lost the current level.
            Win,        ///< @brief Player completed the current level.
            Trial,      ///< @brief Trial / demo mode — purchase prompt screen.
            MainSetup,  ///< @brief Settings screen accessed from the main menu.
            PlaySetup,  ///< @brief Settings screen accessed during gameplay.
            Resume,     ///< @brief Resume-from-checkpoint confirmation screen.
            Ranking     ///< @brief High-score / ranking screen.
        };

        /**
         * @brief Identifies a specific on-screen button by its logical role.
         *
         * @details Each ButtonGlyph value corresponds to one interactive button that
         * can appear during a given Phase. The mapping from ButtonGlyph to on-screen
         * position and sprite is maintained in Game1's draw methods.
         *
         * @note Status: Ported
         */
        enum class ButtonGlyph
        {
            None,              ///< @brief No button / invalid sentinel.
            InitGamerA,        ///< @brief Gamer-slot A selector on the init screen.
            InitGamerB,        ///< @brief Gamer-slot B selector on the init screen.
            InitGamerC,        ///< @brief Gamer-slot C selector on the init screen.
            InitSetup,         ///< @brief Settings button on the init screen.
            InitPlay,          ///< @brief Play button on the init screen.
            InitBuy,           ///< @brief Buy / unlock button on the init screen.
            InitRanking,       ///< @brief Ranking button on the init screen.
            WinLostReturn,     ///< @brief Return-to-menu button on the win/lost screen.
            TrialBuy,          ///< @brief Purchase button on the trial screen.
            TrialCancel,       ///< @brief Cancel button on the trial screen.
            SetupSounds,       ///< @brief Toggle sounds button on the setup screen.
            SetupJump,         ///< @brief Jump-mode toggle on the setup screen.
            SetupZoom,         ///< @brief Zoom-mode toggle on the setup screen.
            SetupAccel,        ///< @brief Accelerometer toggle on the setup screen.
            SetupReset,        ///< @brief Reset-progress button on the setup screen.
            SetupReturn,       ///< @brief Return button on the setup screen.
            PauseMenu,         ///< @brief Main-menu button on the pause screen.
            PauseBack,         ///< @brief Back button on the pause screen.
            PauseSetup,        ///< @brief Settings button on the pause screen.
            PauseRestart,      ///< @brief Restart-level button on the pause screen.
            PauseContinue,     ///< @brief Continue button on the pause screen.
            PlayPause,         ///< @brief Pause button visible during active play.
            PlayJump,          ///< @brief On-screen jump button during active play.
            PlayAction,        ///< @brief On-screen action/fire button during active play.
            PlayDown,          ///< @brief On-screen down button during active play.
            ResumeMenu,        ///< @brief Main-menu button on the resume screen.
            ResumeContinue,    ///< @brief Continue button on the resume screen.
            RankingContinue,   ///< @brief Continue button on the ranking screen.
            Cheat11,           ///< @brief Cheat code button 1-1.
            Cheat12,           ///< @brief Cheat code button 1-2.
            Cheat21,           ///< @brief Cheat code button 2-1.
            Cheat22,           ///< @brief Cheat code button 2-2.
            Cheat31,           ///< @brief Cheat code button 3-1.
            Cheat32,           ///< @brief Cheat code button 3-2.
            Cheat1,            ///< @brief Cheat action button 1.
            Cheat2,            ///< @brief Cheat action button 2.
            Cheat3,            ///< @brief Cheat action button 3.
            Cheat4,            ///< @brief Cheat action button 4.
            Cheat5,            ///< @brief Cheat action button 5.
            Cheat6,            ///< @brief Cheat action button 6.
            Cheat7,            ///< @brief Cheat action button 7.
            Cheat8,            ///< @brief Cheat action button 8.
            Cheat9             ///< @brief Cheat action button 9.
        };

        /**
         * @brief Returns true if the specified glyph is not present in the given list.
         *
         * @details This is an additional helper not present in the original C# code.
         * Uses std::ranges::none_of for a clean O(n) membership test.
         *
         * @param[in] buttonGlyphToBeChecked Glyph to search for.
         * @param[in] buttonGlyphs Collection of glyphs to search within.
         * @return True if @p buttonGlyphToBeChecked is absent from @p buttonGlyphs; false otherwise.
         *
         * @note Additional
         */
        static bool isNotOneOf(const ButtonGlyph& buttonGlyphToBeChecked,
                               std::initializer_list<ButtonGlyph> buttonGlyphs)
        {
            return std::ranges::none_of(buttonGlyphs,
                                        [&buttonGlyphToBeChecked](const ButtonGlyph candidate)
                                        {
                                            return candidate == buttonGlyphToBeChecked;
                                        });
        }

        static constexpr intcs LXIMAGE = 640;     ///< @brief Logical game viewport width in pixels.
        static constexpr intcs LYIMAGE = 480;     ///< @brief Logical game viewport height in pixels.
        static constexpr intcs MAXCELX = 100;     ///< @brief Maximum number of tile columns in a level.
        static constexpr intcs MAXCELY = 100;     ///< @brief Maximum number of tile rows in a level.
        static constexpr intcs DIMOBJX = 64;      ///< @brief Width of a moving-object sprite cell in pixels.
        static constexpr intcs DIMOBJY = 64;      ///< @brief Height of a moving-object sprite cell in pixels.
        static constexpr intcs DIMBLUPIX = 60;    ///< @brief Width of the Blupi character sprite cell in pixels.
        static constexpr intcs DIMBLUPIY = 60;    ///< @brief Height of the Blupi character sprite cell in pixels.
        static constexpr intcs DIMEXPLOX = 128;   ///< @brief Width of an explosion sprite cell in pixels.
        static constexpr intcs DIMEXPLOY = 128;   ///< @brief Height of an explosion sprite cell in pixels.
        static constexpr intcs DIMBUTTONX = 40;   ///< @brief Width of a UI button sprite cell in pixels.
        static constexpr intcs DIMBUTTONY = 40;   ///< @brief Height of a UI button sprite cell in pixels.
        static constexpr intcs DIMJAUGEX = 124;   ///< @brief Width of the HUD gauge sprite cell in pixels.
        static constexpr intcs DIMJAUGEY = 22;    ///< @brief Height of the HUD gauge sprite cell in pixels.
        static constexpr intcs POSSTATX = 12;     ///< @brief X position of the status display in HUD-space pixels.
        static constexpr intcs POSSTATY = 220;    ///< @brief Y position of the status display in HUD-space pixels.
        static constexpr intcs DIMSTATX = 60;     ///< @brief Width of the status display area in pixels.
        static constexpr intcs DIMSTATY = 30;     ///< @brief Height of the status display area in pixels.
        static constexpr intcs DIMTEXTX = 32;     ///< @brief Width of a single font glyph cell in pixels.
        static constexpr intcs DIMTEXTY = 32;     ///< @brief Height of a single font glyph cell in pixels.

        //TODO: These values exist in the enum class PixmapChannel
        static constexpr intcs CHOBJECT = 1;         ///< @brief Channel index for the moving-objects sprite sheet. @see PixmapChannel::Object
        static constexpr intcs CHBLUPI = 2;          ///< @brief Channel index for the main Blupi sprite sheet. @see PixmapChannel::Blupi
        static constexpr intcs CHDECOR = 3;          ///< @brief Channel index for the background/decor sprite sheet. @see PixmapChannel::Background
        static constexpr intcs CHBUTTON = 4;         ///< @brief Channel index for the UI buttons sprite sheet. @see PixmapChannel::Button
        static constexpr intcs CHJAUGE = 5;          ///< @brief Channel index for the HUD gauge sprite sheet. @see PixmapChannel::Jauge
        static constexpr intcs CHTEXT = 6;           ///< @brief Channel index for the font/text glyph sprite sheet. @see PixmapChannel::Text
        static constexpr intcs CHEXPLO = 9;          ///< @brief Channel index for the explosion sprite sheet. @see PixmapChannel::Explosion
        static constexpr intcs CHELEMENT = 10;       ///< @brief Channel index for the collectible elements sprite sheet. @see PixmapChannel::Element
        static constexpr intcs CHBLUPI1 = 11;        ///< @brief Channel index for alternate Blupi variant 1. @see PixmapChannel::Blupi1_11
        static constexpr intcs CHBLUPI2 = 12;        ///< @brief Channel index for alternate Blupi variant 2. @see PixmapChannel::Blupi1_12
        static constexpr intcs CHBLUPI3 = 13;        ///< @brief Channel index for alternate Blupi variant 3. @see PixmapChannel::Blupi1_13
        static constexpr intcs CHPAD = 14;           ///< @brief Channel index for the touch-input pad overlay. @see PixmapChannel::Pad
        static constexpr intcs CHSPEEDYBLUPI = 15;   ///< @brief Channel index for the Speedy Blupi title background. @see PixmapChannel::SpeedyBlupiBackground
        static constexpr intcs CHBLUPIYOUPIE = 16;   ///< @brief Channel index for the Blupi Youpie background. @see PixmapChannel::BlupiYoupieBackground
        static constexpr intcs CHGEAR = 17;          ///< @brief Channel index for the gear/settings background. @see PixmapChannel::GearBackground

        /**
         * @brief Gets the HasSound property value.
         *
         * @details Corresponds to the original C# static read-only property HasSound.
         * Always returns true; the port assumes audio hardware is present.
         *
         * @return Always true.
         *
         * @note Status: Ported
         */
        static constexpr bool getHasSoundProperty()
        {
            return true;
        }

        /**
         * @brief Gets the EasyMove property value.
         *
         * @details Corresponds to the original C# static read-only property EasyMove.
         * Always returns true; the port uses the simplified movement model.
         *
         * @return Always true.
         *
         * @note Status: Ported
         */
        static constexpr bool getEasyMoveProperty()
        {
            return true;
        }
    };
}
