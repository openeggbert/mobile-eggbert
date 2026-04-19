#pragma once

#include <algorithm>
#include <vector>

#include "CppDotNet/CppDotNetHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{

    using CppDotNet::intcs;

//static class
    class Def
    {

    public:

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
         * 
         * @param buttonGlyphToBeChecked 
         * @param buttonGlyphs 
         * @return
         * @note Additional
         */
        static bool notAnyOf(const ButtonGlyph& buttonGlyphToBeChecked, const std::vector<ButtonGlyph>& buttonGlyphs)
        {
            return std::ranges::none_of(buttonGlyphs, [&buttonGlyphToBeChecked](const auto& buttonGlyph)
            {
                return buttonGlyph == buttonGlyphToBeChecked;
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

        static constexpr intcs ACTION_STOP = 1;

        static constexpr intcs ACTION_MARCH = 2;

        static constexpr intcs ACTION_TURN = 3;

        static constexpr intcs ACTION_JUMP = 4;

        static constexpr intcs ACTION_AIR = 5;

        static constexpr intcs ACTION_DOWN = 6;

        static constexpr intcs ACTION_UP = 7;

        static constexpr intcs ACTION_VERTIGO = 8;

        static constexpr intcs ACTION_RECEDE = 9;

        static constexpr intcs ACTION_ADVANCE = 10;

        static constexpr intcs ACTION_CLEAR1 = 11;

        static constexpr intcs ACTION_SET = 12;

        static constexpr intcs ACTION_WIN = 13;

        static constexpr intcs ACTION_PUSH = 14;

        static constexpr intcs ACTION_STOPHELICO = 15;

        static constexpr intcs ACTION_MARCHHELICO = 16;

        static constexpr intcs ACTION_TURNHELICO = 17;

        static constexpr intcs ACTION_STOPNAGE = 18;

        static constexpr intcs ACTION_MARCHNAGE = 19;

        static constexpr intcs ACTION_TURNNAGE = 20;

        static constexpr intcs ACTION_STOPSURF = 21;

        static constexpr intcs ACTION_MARCHSURF = 22;

        static constexpr intcs ACTION_TURNSURF = 23;

        static constexpr intcs ACTION_DROWN = 24;

        static constexpr intcs ACTION_STOPJEEP = 25;

        static constexpr intcs ACTION_MARCHJEEP = 26;

        static constexpr intcs ACTION_TURNJEEP = 27;

        static constexpr intcs ACTION_STOPPOP = 28;

        static constexpr intcs ACTION_POP = 29;

        static constexpr intcs ACTION_BYE = 30;

        static constexpr intcs ACTION_STOPSUSPEND = 31;

        static constexpr intcs ACTION_MARCHSUSPEND = 32;

        static constexpr intcs ACTION_TURNSUSPEND = 33;

        static constexpr intcs ACTION_JUMPSUSPEND = 34;

        static constexpr intcs ACTION_HIDE = 35;

        static constexpr intcs ACTION_JUMPAIE = 36;

        static constexpr intcs ACTION_STOPSKATE = 37;

        static constexpr intcs ACTION_MARCHSKATE = 38;

        static constexpr intcs ACTION_TURNSKATE = 39;

        static constexpr intcs ACTION_JUMPSKATE = 40;

        static constexpr intcs ACTION_AIRSKATE = 41;

        static constexpr intcs ACTION_TAKESKATE = 42;

        static constexpr intcs ACTION_DEPOSESKATE = 43;

        static constexpr intcs ACTION_OUF1a = 44;

        static constexpr intcs ACTION_OUF1b = 45;

        static constexpr intcs ACTION_OUF2 = 46;

        static constexpr intcs ACTION_OUF3 = 47;

        static constexpr intcs ACTION_OUF4 = 48;

        static constexpr intcs ACTION_SUCETTE = 49;

        static constexpr intcs ACTION_STOPTANK = 50;

        static constexpr intcs ACTION_MARCHTANK = 51;

        static constexpr intcs ACTION_TURNTANK = 52;

        static constexpr intcs ACTION_FIRETANK = 53;

        static constexpr intcs ACTION_GLU = 54;

        static constexpr intcs ACTION_DRINK = 55;

        static constexpr intcs ACTION_CHARGE = 56;

        static constexpr intcs ACTION_ELECTRO = 57;

        static constexpr intcs ACTION_HELICOGLU = 58;

        static constexpr intcs ACTION_TURNAIR = 59;

        static constexpr intcs ACTION_STOPMARCH = 60;

        static constexpr intcs ACTION_STOPJUMP = 61;

        static constexpr intcs ACTION_STOPJUMPh = 62;

        static constexpr intcs ACTION_MOCKERY = 63;

        static constexpr intcs ACTION_MOCKERYi = 64;

        static constexpr intcs ACTION_OUF5 = 65;

        static constexpr intcs ACTION_BALLOON = 66;

        static constexpr intcs ACTION_STOPOVER = 67;

        static constexpr intcs ACTION_MARCHOVER = 68;

        static constexpr intcs ACTION_TURNOVER = 69;

        static constexpr intcs ACTION_RECEDEq = 70;

        static constexpr intcs ACTION_ADVANCEq = 71;

        static constexpr intcs ACTION_STOPECRASE = 72;

        static constexpr intcs ACTION_MARCHECRASE = 73;

        static constexpr intcs ACTION_TELEPORTE = 74;

        static constexpr intcs ACTION_CLEAR2 = 75;

        static constexpr intcs ACTION_CLEAR3 = 76;

        static constexpr intcs ACTION_CLEAR4 = 77;

        static constexpr intcs ACTION_CLEAR5 = 78;

        static constexpr intcs ACTION_CLEAR6 = 79;

        static constexpr intcs ACTION_CLEAR7 = 80;

        static constexpr intcs ACTION_CLEAR8 = 81;

        static constexpr intcs ACTION_SWITCH = 82;

        static constexpr intcs ACTION_MOCKERYp = 83;

        static constexpr intcs ACTION_NON = 84;

        static constexpr intcs ACTION_SLOWDOWNSKATE = 85;

        static constexpr intcs ACTION_TAKEDYNAMITE = 86;

        static constexpr intcs ACTION_PUTDYNAMITE = 87;

        static constexpr intcs DIR_LEFT = 1;

        static constexpr intcs DIR_RIGHT = 2;

        static constexpr intcs SEC_SHIELD = 1;

        static constexpr intcs SEC_POWER = 2;

        static constexpr intcs SEC_CLOUD = 3;

        static constexpr intcs SEC_HIDE = 4;

        static constexpr intcs TYPE_ASCENSEUR = 1;

        static constexpr intcs TYPE_BOMBEDOWN = 2;

        static constexpr intcs TYPE_BOMBEUP = 3;

        static constexpr intcs TYPE_BULLDOZER = 4;

        static constexpr intcs TYPE_TRESOR = 5;

        static constexpr intcs TYPE_EGG = 6;

        static constexpr intcs TYPE_GOAL = 7;

        static constexpr intcs TYPE_EXPLO1 = 8;

        static constexpr intcs TYPE_EXPLO2 = 9;

        static constexpr intcs TYPE_EXPLO3 = 10;

        static constexpr intcs TYPE_EXPLO4 = 11;

        static constexpr intcs TYPE_CAISSE = 12;

        static constexpr intcs TYPE_HELICO = 13;

        static constexpr intcs TYPE_PLOUF = 14;

        static constexpr intcs TYPE_BLUP = 15;

        static constexpr intcs TYPE_BOMBEMOVE = 16;

        static constexpr intcs TYPE_POISSON = 17;

        static constexpr intcs TYPE_TOMATES = 18;

        static constexpr intcs TYPE_JEEP = 19;

        static constexpr intcs TYPE_OISEAU = 20;

        static constexpr intcs TYPE_CLE = 21;

        static constexpr intcs TYPE_DOOR = 22;

        static constexpr intcs TYPE_BALLE = 23;

        static constexpr intcs TYPE_SKATE = 24;

        static constexpr intcs TYPE_SHIELD = 25;

        static constexpr intcs TYPE_POWER = 26;

        static constexpr intcs TYPE_MAGICTRACK = 27;

        static constexpr intcs TYPE_TANK = 28;

        static constexpr intcs TYPE_BULLET = 29;

        static constexpr intcs TYPE_DRINK = 30;

        static constexpr intcs TYPE_CHARGE = 31;

        static constexpr intcs TYPE_BLUPIHELICO = 32;

        static constexpr intcs TYPE_BLUPITANK = 33;

        static constexpr intcs TYPE_GLU = 34;

        static constexpr intcs TYPE_TIPLOUF = 35;

        static constexpr intcs TYPE_POLLUTION = 36;

        static constexpr intcs TYPE_CLEAR = 37;

        static constexpr intcs TYPE_ELECTRO = 38;

        static constexpr intcs TYPE_TRESORTRACK = 39;

        static constexpr intcs TYPE_INVERT = 40;

        static constexpr intcs TYPE_INVERTSTART = 41;

        static constexpr intcs TYPE_INVERTSTOP = 42;

        static constexpr intcs TYPE_GUEPE = 44;

        static constexpr intcs TYPE_OVER = 46;

        static constexpr intcs TYPE_ASCENSEURs = 47;

        static constexpr intcs TYPE_ASCENSEURsi = 48;

        static constexpr intcs TYPE_CLE1 = 49;

        static constexpr intcs TYPE_CLE2 = 50;

        static constexpr intcs TYPE_CLE3 = 51;

        static constexpr intcs TYPE_BRIDGE = 52;

        static constexpr intcs TYPE_TENTACULE = 53;

        static constexpr intcs TYPE_CREATURE = 54;

        static constexpr intcs TYPE_DYNAMITE = 55;

        static constexpr intcs TYPE_DYNAMITEf = 56;

        static constexpr intcs TYPE_SHIELDTRACK = 57;

        static constexpr intcs TYPE_HIDETRACK = 58;

        static constexpr intcs TYPE_EXPLO5 = 90;

        static constexpr intcs TYPE_EXPLO6 = 91;

        static constexpr intcs TYPE_EXPLO7 = 92;

        static constexpr intcs TYPE_EXPLO8 = 93;

        static constexpr intcs TYPE_EXPLO9 = 94;

        static constexpr intcs TYPE_EXPLO10 = 95;

        static constexpr intcs TYPE_BOMBEFOLLOW1 = 96;

        static constexpr intcs TYPE_BOMBEFOLLOW2 = 97;

        static constexpr intcs TYPE_SPLOUTCH1 = 98;

        static constexpr intcs TYPE_SPLOUTCH2 = 99;

        static constexpr intcs TYPE_SPLOUTCH3 = 100;

        static constexpr intcs TYPE_BOMBEPERSO1 = 200;

        static constexpr intcs TYPE_BOMBEPERSO2 = 201;

        static constexpr intcs TYPE_BOMBEPERSO3 = 202;

        static constexpr intcs TYPE_BOMBEPERSO4 = 203;

        static constexpr intcs STEP_STOPSTART = 1;

        static constexpr intcs STEP_ADVANCE = 2;

        static constexpr intcs STEP_STOPEND = 3;

        static constexpr intcs STEP_RECEDE = 4;

        static constexpr intcs DECOR_EXPLO1 = 1;

        static constexpr intcs DECOR_EXPLO2 = 2;

        static constexpr intcs DECOR_EXPLO3 = 3;

        static constexpr intcs DECOR_EXPLO4 = 4;

        static constexpr intcs DECOR_BALLOON = 5;

        static constexpr intcs SOUND_CLICK = 0;

        static constexpr intcs SOUND_JUMP1 = 1;

        static constexpr intcs SOUND_JUMP2 = 2;

        static constexpr intcs SOUND_JUMPEND = 3;

        static constexpr intcs SOUND_JUMPTOC = 4;

        static constexpr intcs SOUND_TURN = 5;

        static constexpr intcs SOUND_VERTIGO = 6;

        static constexpr intcs SOUND_DOWN = 7;

        static constexpr intcs SOUND_FALL = 8;

        static constexpr intcs SOUND_NEW = 9;

        static constexpr intcs SOUND_BOUM = 10;

        static constexpr intcs SOUND_TRESOR = 11;

        static constexpr intcs SOUND_EGG = 12;

        static constexpr intcs SOUND_ENDKO = 13;

        static constexpr intcs SOUND_ENDOK = 14;

        static constexpr intcs SOUND_HELICOSTART = 15;

        static constexpr intcs SOUND_HELICOHIGH = 16;

        static constexpr intcs SOUND_HELICOSTOP = 17;

        static constexpr intcs SOUND_HELICOLOW = 18;

        static constexpr intcs SOUND_LASTTRESOR = 19;

        static constexpr intcs SOUND_UP = 20;

        static constexpr intcs SOUND_LOOKUP = 21;

        static constexpr intcs SOUND_JUMP0 = 22;

        static constexpr intcs SOUND_PLOUF = 23;

        static constexpr intcs SOUND_BLUP = 24;

        static constexpr intcs SOUND_SURF = 25;

        static constexpr intcs SOUND_DROWN = 26;

        static constexpr intcs SOUND_ERROR = 27;

        static constexpr intcs SOUND_JEEPSTART = 28;

        static constexpr intcs SOUND_JEEPHIGH = 29;

        static constexpr intcs SOUND_JEEPSTOP = 30;

        static constexpr intcs SOUND_JEEPLOW = 31;

        static constexpr intcs SOUND_BYE = 32;

        static constexpr intcs SOUND_DOOR = 33;

        static constexpr intcs SOUND_SUSPENDTOC = 34;

        static constexpr intcs SOUND_SUSPENDJUMP = 35;

        static constexpr intcs SOUND_SINGE = 36;

        static constexpr intcs SOUND_PATIENT = 37;

        static constexpr intcs SOUND_PUSH = 38;

        static constexpr intcs SOUND_POP = 39;

        static constexpr intcs SOUND_JUMPAIE = 40;

        static constexpr intcs SOUND_RESSORT = 41;

        static constexpr intcs SOUND_STARTSHIELD = 42;

        static constexpr intcs SOUND_STOPSHIELD = 43;

        static constexpr intcs SOUND_STARTPOWER = 44;

        static constexpr intcs SOUND_STOPPOWER = 45;

        static constexpr intcs SOUND_OUF1 = 46;

        static constexpr intcs SOUND_OUF2 = 47;

        static constexpr intcs SOUND_OUF3 = 48;

        static constexpr intcs SOUND_OUF4 = 49;

        static constexpr intcs SOUND_SUCETTE = 50;

        static constexpr intcs SOUND_GLU = 51;

        static constexpr intcs SOUND_FIREOK = 52;

        static constexpr intcs SOUND_FIREKO = 53;

        static constexpr intcs SOUND_TAKEGLU = 54;

        static constexpr intcs SOUND_STARTCLOUD = 55;

        static constexpr intcs SOUND_STOPCLOUD = 56;

        static constexpr intcs SOUND_DRINK = 57;

        static constexpr intcs SOUND_CHARGE = 58;

        static constexpr intcs SOUND_ELECTRO = 59;

        static constexpr intcs SOUND_PERSOTAKE = 60;

        static constexpr intcs SOUND_PERSOPOSE = 61;

        static constexpr intcs SOUND_STARTHIDE = 62;

        static constexpr intcs SOUND_STOPHIDE = 63;

        static constexpr intcs SOUND_TIPLOUF = 64;

        static constexpr intcs SOUND_MOCKERY = 65;

        static constexpr intcs SOUND_INVERTSTART = 66;

        static constexpr intcs SOUND_INVERTSTOP = 67;

        static constexpr intcs SOUND_OVERSTOP = 68;

        static constexpr intcs SOUND_BLITZ = 69;

        static constexpr intcs SOUND_ECRASE = 70;

        static constexpr intcs SOUND_TELEPORTE = 71;

        static constexpr intcs SOUND_BRIDGE1 = 72;

        static constexpr intcs SOUND_BRIDGE2 = 73;

        static constexpr intcs SOUND_ANGEL = 74;

        static constexpr intcs SOUND_SCIE = 75;

        static constexpr intcs SOUND_SWITCHOFF = 76;

        static constexpr intcs SOUND_SWITCHON = 77;

        static constexpr intcs SOUND_JUMPENDb = 78;

        static constexpr intcs SOUND_JUMPTOCb = 79;

        static constexpr intcs SOUND_JUMPENDm = 80;

        static constexpr intcs SOUND_JUMPTOCm = 81;

        static constexpr intcs SOUND_JUMPENDg = 82;

        static constexpr intcs SOUND_JUMPTOCg = 83;

        static constexpr intcs SOUND_JUMPENDo = 84;

        static constexpr intcs SOUND_JUMPTOCo = 85;

        static constexpr intcs SOUND_JUMPENDk = 86;

        static constexpr intcs SOUND_JUMPTOCk = 87;

        static constexpr intcs SOUND_JUMPENDf = 88;

        static constexpr intcs SOUND_JUMPTOCf = 89;

        static constexpr intcs SOUND_JUMPENDh = 90;

        static constexpr intcs SOUND_JUMPTOCh = 91;

        static constexpr intcs SOUND_FOLLOW = 92;

        static constexpr intcs KEY_JUMP = 1;

        static constexpr intcs KEY_FIRE = 2;

        static constexpr intcs KEY_DOWN = 4;

        static constexpr bool getHasSoundProperty(){return true;}

        static constexpr bool getEasyMoveProperty(){ return true;}

    };

}

