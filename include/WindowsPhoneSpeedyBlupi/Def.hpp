#pragma once

#include <algorithm>
#include <ranges>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "WindowsPhoneSpeedyBlupi/def/BlupiAction.hpp"
#include "WindowsPhoneSpeedyBlupi/def/Direction.hpp"
#include "WindowsPhoneSpeedyBlupi/def/SecretPower.hpp"
#include "WindowsPhoneSpeedyBlupi/def/KeyPressFlags.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::intcs;

    /**
     * @brief Provides global game constants, enums, and static helper members.
     *
     * This class is a C++ port of the original C# static class Def from
     * WindowsPhoneSpeedyBlupi.
     *
     * @note Status: Ported
     */
    class Def
    {
    public:
        Def() = delete;
        ~Def() = delete;

        /**
         * @brief Represents the current game phase.
         *
         * @note Status: Ported
         */
        enum class Phase
        {
            None,
            First,
            Wait,
            Init,
            Play,
            Pause,
            Lost,
            Win,
            Trial,
            MainSetup,
            PlaySetup,
            Resume,
            Ranking
        };

        /**
         * @brief Identifies a button glyph.
         *
         * @note Status: Ported
         */
        enum class ButtonGlyph
        {
            None,
            InitGamerA,
            InitGamerB,
            InitGamerC,
            InitSetup,
            InitPlay,
            InitBuy,
            InitRanking,
            WinLostReturn,
            TrialBuy,
            TrialCancel,
            SetupSounds,
            SetupJump,
            SetupZoom,
            SetupAccel,
            SetupReset,
            SetupReturn,
            PauseMenu,
            PauseBack,
            PauseSetup,
            PauseRestart,
            PauseContinue,
            PlayPause,
            PlayJump,
            PlayAction,
            PlayDown,
            ResumeMenu,
            ResumeContinue,
            RankingContinue,
            Cheat11,
            Cheat12,
            Cheat21,
            Cheat22,
            Cheat31,
            Cheat32,
            Cheat1,
            Cheat2,
            Cheat3,
            Cheat4,
            Cheat5,
            Cheat6,
            Cheat7,
            Cheat8,
            Cheat9
        };

        /**
         * @brief Returns true if the specified glyph is not present in the given list.
         *
         * This is an additional helper not present in the original C# code.
         *
         * @param buttonGlyphToBeChecked Glyph to search for.
         * @param buttonGlyphs Collection of glyphs.
         * @return True if the glyph is not present; otherwise false.
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

        static constexpr intcs LXIMAGE = 640;
        static constexpr intcs LYIMAGE = 480;
        static constexpr intcs MAXCELX = 100;
        static constexpr intcs MAXCELY = 100;
        static constexpr intcs DIMOBJX = 64;
        static constexpr intcs DIMOBJY = 64;
        static constexpr intcs DIMBLUPIX = 60;
        static constexpr intcs DIMBLUPIY = 60;
        static constexpr intcs DIMEXPLOX = 128;
        static constexpr intcs DIMEXPLOY = 128;
        static constexpr intcs DIMBUTTONX = 40;
        static constexpr intcs DIMBUTTONY = 40;
        static constexpr intcs DIMJAUGEX = 124;
        static constexpr intcs DIMJAUGEY = 22;
        static constexpr intcs POSSTATX = 12;
        static constexpr intcs POSSTATY = 220;
        static constexpr intcs DIMSTATX = 60;
        static constexpr intcs DIMSTATY = 30;
        static constexpr intcs DIMTEXTX = 32;
        static constexpr intcs DIMTEXTY = 32;

        //TODO: These values exist in the enum class PixmapChannel
        static constexpr intcs CHOBJECT = 1;
        static constexpr intcs CHBLUPI = 2;
        static constexpr intcs CHDECOR = 3;
        static constexpr intcs CHBUTTON = 4;
        static constexpr intcs CHJAUGE = 5;
        static constexpr intcs CHTEXT = 6;
        static constexpr intcs CHEXPLO = 9;
        static constexpr intcs CHELEMENT = 10;
        static constexpr intcs CHBLUPI1 = 11;
        static constexpr intcs CHBLUPI2 = 12;
        static constexpr intcs CHBLUPI3 = 13;
        static constexpr intcs CHPAD = 14;
        static constexpr intcs CHSPEEDYBLUPI = 15;
        static constexpr intcs CHBLUPIYOUPIE = 16;
        static constexpr intcs CHGEAR = 17;

        /**
         * @brief Gets the HasSound property value.
         *
         * Corresponds to the original C# static read-only property HasSound.
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
         * Corresponds to the original C# static read-only property EasyMove.
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
