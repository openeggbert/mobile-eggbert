// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Def

//using Microsoft.Xna.Framework.Input;
//using static WindowsPhoneSpeedyBlupi.Def;
#ifndef DEF_H
#define DEF_H
#include <vector>

namespace WindowsPhoneSpeedyBlupi
{
    using ushort = unsigned short;
//static class
    class Def
    {

    public: enum Phase
        {
            NonePhase,
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

    public: enum ButtonGlyph
        {
            NoneButtonGlyph,
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



    public: static bool notAnyOf(const ButtonGlyph& buttonGlyphToBeChecked, const std::vector<ButtonGlyph>& buttonGlyphs) {
        for (auto buttonGlyph : buttonGlyphs) {
            if (buttonGlyph == buttonGlyphToBeChecked) {
                return false;
            }
        }
        return true;
    }

        public: static constexpr ushort LXIMAGE = 640;

        public: static constexpr ushort LYIMAGE = 480;

        public: static constexpr ushort MAXCELX = 100;

        public: static constexpr ushort MAXCELY = 100;

        public: static constexpr ushort DIMOBJX = 64;

        public: static constexpr ushort DIMOBJY = 64;

        public: static constexpr ushort DIMBLUPIX = 60;

        public: static constexpr ushort DIMBLUPIY = 60;

        public: static constexpr ushort DIMEXPLOX = 128;

        public: static constexpr ushort DIMEXPLOY = 128;

        public: static constexpr ushort DIMBUTTONX = 40;

        public: static constexpr ushort DIMBUTTONY = 40;

        public: static constexpr ushort DIMJAUGEX = 124;

        public: static constexpr ushort DIMJAUGEY = 22;

        public: static constexpr ushort POSSTATX = 12;

        public: static constexpr ushort POSSTATY = 220;

        public: static constexpr ushort DIMSTATX = 60;

        public: static constexpr ushort DIMSTATY = 30;

        public: static constexpr ushort DIMTEXTX = 32;

        public: static constexpr ushort DIMTEXTY = 32;

        public: static constexpr ushort CHOBJECT = 1;

        public: static constexpr ushort CHBLUPI = 2;

        public: static constexpr ushort CHDECOR = 3;

        public: static constexpr ushort CHBUTTON = 4;

        public: static constexpr ushort CHJAUGE = 5;

        public: static constexpr ushort CHTEXT = 6;

        public: static constexpr ushort CHEXPLO = 9;

        public: static constexpr ushort CHELEMENT = 10;

        public: static constexpr ushort CHBLUPI1 = 11;

        public: static constexpr ushort CHBLUPI2 = 12;

        public: static constexpr ushort CHBLUPI3 = 13;

        public: static constexpr ushort CHPAD = 14;

        public: static constexpr ushort CHSPEEDYBLUPI = 15;

        public: static constexpr ushort CHBLUPIYOUPIE = 16;

        public: static constexpr ushort CHGEAR = 17;

        public: static constexpr ushort ACTION_STOP = 1;

        public: static constexpr ushort ACTION_MARCH = 2;

        public: static constexpr ushort ACTION_TURN = 3;

        public: static constexpr ushort ACTION_JUMP = 4;

        public: static constexpr ushort ACTION_AIR = 5;

        public: static constexpr ushort ACTION_DOWN = 6;

        public: static constexpr ushort ACTION_UP = 7;

        public: static constexpr ushort ACTION_VERTIGO = 8;

        public: static constexpr ushort ACTION_RECEDE = 9;

        public: static constexpr ushort ACTION_ADVANCE = 10;

        public: static constexpr ushort ACTION_CLEAR1 = 11;

        public: static constexpr ushort ACTION_SET = 12;

        public: static constexpr ushort ACTION_WIN = 13;

        public: static constexpr ushort ACTION_PUSH = 14;

        public: static constexpr ushort ACTION_STOPHELICO = 15;

        public: static constexpr ushort ACTION_MARCHHELICO = 16;

        public: static constexpr ushort ACTION_TURNHELICO = 17;

        public: static constexpr ushort ACTION_STOPNAGE = 18;

        public: static constexpr ushort ACTION_MARCHNAGE = 19;

        public: static constexpr ushort ACTION_TURNNAGE = 20;

        public: static constexpr ushort ACTION_STOPSURF = 21;

        public: static constexpr ushort ACTION_MARCHSURF = 22;

        public: static constexpr ushort ACTION_TURNSURF = 23;

        public: static constexpr ushort ACTION_DROWN = 24;

        public: static constexpr ushort ACTION_STOPJEEP = 25;

        public: static constexpr ushort ACTION_MARCHJEEP = 26;

        public: static constexpr ushort ACTION_TURNJEEP = 27;

        public: static constexpr ushort ACTION_STOPPOP = 28;

        public: static constexpr ushort ACTION_POP = 29;

        public: static constexpr ushort ACTION_BYE = 30;

        public: static constexpr ushort ACTION_STOPSUSPEND = 31;

        public: static constexpr ushort ACTION_MARCHSUSPEND = 32;

        public: static constexpr ushort ACTION_TURNSUSPEND = 33;

        public: static constexpr ushort ACTION_JUMPSUSPEND = 34;

        public: static constexpr ushort ACTION_HIDE = 35;

        public: static constexpr ushort ACTION_JUMPAIE = 36;

        public: static constexpr ushort ACTION_STOPSKATE = 37;

        public: static constexpr ushort ACTION_MARCHSKATE = 38;

        public: static constexpr ushort ACTION_TURNSKATE = 39;

        public: static constexpr ushort ACTION_JUMPSKATE = 40;

        public: static constexpr ushort ACTION_AIRSKATE = 41;

        public: static constexpr ushort ACTION_TAKESKATE = 42;

        public: static constexpr ushort ACTION_DEPOSESKATE = 43;

        public: static constexpr ushort ACTION_OUF1a = 44;

        public: static constexpr ushort ACTION_OUF1b = 45;

        public: static constexpr ushort ACTION_OUF2 = 46;

        public: static constexpr ushort ACTION_OUF3 = 47;

        public: static constexpr ushort ACTION_OUF4 = 48;

        public: static constexpr ushort ACTION_SUCETTE = 49;

        public: static constexpr ushort ACTION_STOPTANK = 50;

        public: static constexpr ushort ACTION_MARCHTANK = 51;

        public: static constexpr ushort ACTION_TURNTANK = 52;

        public: static constexpr ushort ACTION_FIRETANK = 53;

        public: static constexpr ushort ACTION_GLU = 54;

        public: static constexpr ushort ACTION_DRINK = 55;

        public: static constexpr ushort ACTION_CHARGE = 56;

        public: static constexpr ushort ACTION_ELECTRO = 57;

        public: static constexpr ushort ACTION_HELICOGLU = 58;

        public: static constexpr ushort ACTION_TURNAIR = 59;

        public: static constexpr ushort ACTION_STOPMARCH = 60;

        public: static constexpr ushort ACTION_STOPJUMP = 61;

        public: static constexpr ushort ACTION_STOPJUMPh = 62;

        public: static constexpr ushort ACTION_MOCKERY = 63;

        public: static constexpr ushort ACTION_MOCKERYi = 64;

        public: static constexpr ushort ACTION_OUF5 = 65;

        public: static constexpr ushort ACTION_BALLOON = 66;

        public: static constexpr ushort ACTION_STOPOVER = 67;

        public: static constexpr ushort ACTION_MARCHOVER = 68;

        public: static constexpr ushort ACTION_TURNOVER = 69;

        public: static constexpr ushort ACTION_RECEDEq = 70;

        public: static constexpr ushort ACTION_ADVANCEq = 71;

        public: static constexpr ushort ACTION_STOPECRASE = 72;

        public: static constexpr ushort ACTION_MARCHECRASE = 73;

        public: static constexpr ushort ACTION_TELEPORTE = 74;

        public: static constexpr ushort ACTION_CLEAR2 = 75;

        public: static constexpr ushort ACTION_CLEAR3 = 76;

        public: static constexpr ushort ACTION_CLEAR4 = 77;

        public: static constexpr ushort ACTION_CLEAR5 = 78;

        public: static constexpr ushort ACTION_CLEAR6 = 79;

        public: static constexpr ushort ACTION_CLEAR7 = 80;

        public: static constexpr ushort ACTION_CLEAR8 = 81;

        public: static constexpr ushort ACTION_SWITCH = 82;

        public: static constexpr ushort ACTION_MOCKERYp = 83;

        public: static constexpr ushort ACTION_NON = 84;

        public: static constexpr ushort ACTION_SLOWDOWNSKATE = 85;

        public: static constexpr ushort ACTION_TAKEDYNAMITE = 86;

        public: static constexpr ushort ACTION_PUTDYNAMITE = 87;

        public: static constexpr ushort DIR_LEFT = 1;

        public: static constexpr ushort DIR_RIGHT = 2;

        public: static constexpr ushort SEC_SHIELD = 1;

        public: static constexpr ushort SEC_POWER = 2;

        public: static constexpr ushort SEC_CLOUD = 3;

        public: static constexpr ushort SEC_HIDE = 4;

        public: static constexpr ushort TYPE_ASCENSEUR = 1;

        public: static constexpr ushort TYPE_BOMBEDOWN = 2;

        public: static constexpr ushort TYPE_BOMBEUP = 3;

        public: static constexpr ushort TYPE_BULLDOZER = 4;

        public: static constexpr ushort TYPE_TRESOR = 5;

        public: static constexpr ushort TYPE_EGG = 6;

        public: static constexpr ushort TYPE_GOAL = 7;

        public: static constexpr ushort TYPE_EXPLO1 = 8;

        public: static constexpr ushort TYPE_EXPLO2 = 9;

        public: static constexpr ushort TYPE_EXPLO3 = 10;

        public: static constexpr ushort TYPE_EXPLO4 = 11;

        public: static constexpr ushort TYPE_CAISSE = 12;

        public: static constexpr ushort TYPE_HELICO = 13;

        public: static constexpr ushort TYPE_PLOUF = 14;

        public: static constexpr ushort TYPE_BLUP = 15;

        public: static constexpr ushort TYPE_BOMBEMOVE = 16;

        public: static constexpr ushort TYPE_POISSON = 17;

        public: static constexpr ushort TYPE_TOMATES = 18;

        public: static constexpr ushort TYPE_JEEP = 19;

        public: static constexpr ushort TYPE_OISEAU = 20;

        public: static constexpr ushort TYPE_CLE = 21;

        public: static constexpr ushort TYPE_DOOR = 22;

        public: static constexpr ushort TYPE_BALLE = 23;

        public: static constexpr ushort TYPE_SKATE = 24;

        public: static constexpr ushort TYPE_SHIELD = 25;

        public: static constexpr ushort TYPE_POWER = 26;

        public: static constexpr ushort TYPE_MAGICTRACK = 27;

        public: static constexpr ushort TYPE_TANK = 28;

        public: static constexpr ushort TYPE_BULLET = 29;

        public: static constexpr ushort TYPE_DRINK = 30;

        public: static constexpr ushort TYPE_CHARGE = 31;

        public: static constexpr ushort TYPE_BLUPIHELICO = 32;

        public: static constexpr ushort TYPE_BLUPITANK = 33;

        public: static constexpr ushort TYPE_GLU = 34;

        public: static constexpr ushort TYPE_TIPLOUF = 35;

        public: static constexpr ushort TYPE_POLLUTION = 36;

        public: static constexpr ushort TYPE_CLEAR = 37;

        public: static constexpr ushort TYPE_ELECTRO = 38;

        public: static constexpr ushort TYPE_TRESORTRACK = 39;

        public: static constexpr ushort TYPE_INVERT = 40;

        public: static constexpr ushort TYPE_INVERTSTART = 41;

        public: static constexpr ushort TYPE_INVERTSTOP = 42;

        public: static constexpr ushort TYPE_GUEPE = 44;

        public: static constexpr ushort TYPE_OVER = 46;

        public: static constexpr ushort TYPE_ASCENSEURs = 47;

        public: static constexpr ushort TYPE_ASCENSEURsi = 48;

        public: static constexpr ushort TYPE_CLE1 = 49;

        public: static constexpr ushort TYPE_CLE2 = 50;

        public: static constexpr ushort TYPE_CLE3 = 51;

        public: static constexpr ushort TYPE_BRIDGE = 52;

        public: static constexpr ushort TYPE_TENTACULE = 53;

        public: static constexpr ushort TYPE_CREATURE = 54;

        public: static constexpr ushort TYPE_DYNAMITE = 55;

        public: static constexpr ushort TYPE_DYNAMITEf = 56;

        public: static constexpr ushort TYPE_SHIELDTRACK = 57;

        public: static constexpr ushort TYPE_HIDETRACK = 58;

        public: static constexpr ushort TYPE_EXPLO5 = 90;

        public: static constexpr ushort TYPE_EXPLO6 = 91;

        public: static constexpr ushort TYPE_EXPLO7 = 92;

        public: static constexpr ushort TYPE_EXPLO8 = 93;

        public: static constexpr ushort TYPE_EXPLO9 = 94;

        public: static constexpr ushort TYPE_EXPLO10 = 95;

        public: static constexpr ushort TYPE_BOMBEFOLLOW1 = 96;

        public: static constexpr ushort TYPE_BOMBEFOLLOW2 = 97;

        public: static constexpr ushort TYPE_SPLOUTCH1 = 98;

        public: static constexpr ushort TYPE_SPLOUTCH2 = 99;

        public: static constexpr ushort TYPE_SPLOUTCH3 = 100;

        public: static constexpr ushort TYPE_BOMBEPERSO1 = 200;

        public: static constexpr ushort TYPE_BOMBEPERSO2 = 201;

        public: static constexpr ushort TYPE_BOMBEPERSO3 = 202;

        public: static constexpr ushort TYPE_BOMBEPERSO4 = 203;

        public: static constexpr ushort STEP_STOPSTART = 1;

        public: static constexpr ushort STEP_ADVANCE = 2;

        public: static constexpr ushort STEP_STOPEND = 3;

        public: static constexpr ushort STEP_RECEDE = 4;

        public: static constexpr ushort DECOR_EXPLO1 = 1;

        public: static constexpr ushort DECOR_EXPLO2 = 2;

        public: static constexpr ushort DECOR_EXPLO3 = 3;

        public: static constexpr ushort DECOR_EXPLO4 = 4;

        public: static constexpr ushort DECOR_BALLOON = 5;

        public: static constexpr ushort SOUND_CLICK = 0;

        public: static constexpr ushort SOUND_JUMP1 = 1;

        public: static constexpr ushort SOUND_JUMP2 = 2;

        public: static constexpr ushort SOUND_JUMPEND = 3;

        public: static constexpr ushort SOUND_JUMPTOC = 4;

        public: static constexpr ushort SOUND_TURN = 5;

        public: static constexpr ushort SOUND_VERTIGO = 6;

        public: static constexpr ushort SOUND_DOWN = 7;

        public: static constexpr ushort SOUND_FALL = 8;

        public: static constexpr ushort SOUND_NEW = 9;

        public: static constexpr ushort SOUND_BOUM = 10;

        public: static constexpr ushort SOUND_TRESOR = 11;

        public: static constexpr ushort SOUND_EGG = 12;

        public: static constexpr ushort SOUND_ENDKO = 13;

        public: static constexpr ushort SOUND_ENDOK = 14;

        public: static constexpr ushort SOUND_HELICOSTART = 15;

        public: static constexpr ushort SOUND_HELICOHIGH = 16;

        public: static constexpr ushort SOUND_HELICOSTOP = 17;

        public: static constexpr ushort SOUND_HELICOLOW = 18;

        public: static constexpr ushort SOUND_LASTTRESOR = 19;

        public: static constexpr ushort SOUND_UP = 20;

        public: static constexpr ushort SOUND_LOOKUP = 21;

        public: static constexpr ushort SOUND_JUMP0 = 22;

        public: static constexpr ushort SOUND_PLOUF = 23;

        public: static constexpr ushort SOUND_BLUP = 24;

        public: static constexpr ushort SOUND_SURF = 25;

        public: static constexpr ushort SOUND_DROWN = 26;

        public: static constexpr ushort SOUND_ERROR = 27;

        public: static constexpr ushort SOUND_JEEPSTART = 28;

        public: static constexpr ushort SOUND_JEEPHIGH = 29;

        public: static constexpr ushort SOUND_JEEPSTOP = 30;

        public: static constexpr ushort SOUND_JEEPLOW = 31;

        public: static constexpr ushort SOUND_BYE = 32;

        public: static constexpr ushort SOUND_DOOR = 33;

        public: static constexpr ushort SOUND_SUSPENDTOC = 34;

        public: static constexpr ushort SOUND_SUSPENDJUMP = 35;

        public: static constexpr ushort SOUND_SINGE = 36;

        public: static constexpr ushort SOUND_PATIENT = 37;

        public: static constexpr ushort SOUND_PUSH = 38;

        public: static constexpr ushort SOUND_POP = 39;

        public: static constexpr ushort SOUND_JUMPAIE = 40;

        public: static constexpr ushort SOUND_RESSORT = 41;

        public: static constexpr ushort SOUND_STARTSHIELD = 42;

        public: static constexpr ushort SOUND_STOPSHIELD = 43;

        public: static constexpr ushort SOUND_STARTPOWER = 44;

        public: static constexpr ushort SOUND_STOPPOWER = 45;

        public: static constexpr ushort SOUND_OUF1 = 46;

        public: static constexpr ushort SOUND_OUF2 = 47;

        public: static constexpr ushort SOUND_OUF3 = 48;

        public: static constexpr ushort SOUND_OUF4 = 49;

        public: static constexpr ushort SOUND_SUCETTE = 50;

        public: static constexpr ushort SOUND_GLU = 51;

        public: static constexpr ushort SOUND_FIREOK = 52;

        public: static constexpr ushort SOUND_FIREKO = 53;

        public: static constexpr ushort SOUND_TAKEGLU = 54;

        public: static constexpr ushort SOUND_STARTCLOUD = 55;

        public: static constexpr ushort SOUND_STOPCLOUD = 56;

        public: static constexpr ushort SOUND_DRINK = 57;

        public: static constexpr ushort SOUND_CHARGE = 58;

        public: static constexpr ushort SOUND_ELECTRO = 59;

        public: static constexpr ushort SOUND_PERSOTAKE = 60;

        public: static constexpr ushort SOUND_PERSOPOSE = 61;

        public: static constexpr ushort SOUND_STARTHIDE = 62;

        public: static constexpr ushort SOUND_STOPHIDE = 63;

        public: static constexpr ushort SOUND_TIPLOUF = 64;

        public: static constexpr ushort SOUND_MOCKERY = 65;

        public: static constexpr ushort SOUND_INVERTSTART = 66;

        public: static constexpr ushort SOUND_INVERTSTOP = 67;

        public: static constexpr ushort SOUND_OVERSTOP = 68;

        public: static constexpr ushort SOUND_BLITZ = 69;

        public: static constexpr ushort SOUND_ECRASE = 70;

        public: static constexpr ushort SOUND_TELEPORTE = 71;

        public: static constexpr ushort SOUND_BRIDGE1 = 72;

        public: static constexpr ushort SOUND_BRIDGE2 = 73;

        public: static constexpr ushort SOUND_ANGEL = 74;

        public: static constexpr ushort SOUND_SCIE = 75;

        public: static constexpr ushort SOUND_SWITCHOFF = 76;

        public: static constexpr ushort SOUND_SWITCHON = 77;

        public: static constexpr ushort SOUND_JUMPENDb = 78;

        public: static constexpr ushort SOUND_JUMPTOCb = 79;

        public: static constexpr ushort SOUND_JUMPENDm = 80;

        public: static constexpr ushort SOUND_JUMPTOCm = 81;

        public: static constexpr ushort SOUND_JUMPENDg = 82;

        public: static constexpr ushort SOUND_JUMPTOCg = 83;

        public: static constexpr ushort SOUND_JUMPENDo = 84;

        public: static constexpr ushort SOUND_JUMPTOCo = 85;

        public: static constexpr ushort SOUND_JUMPENDk = 86;

        public: static constexpr ushort SOUND_JUMPTOCk = 87;

        public: static constexpr ushort SOUND_JUMPENDf = 88;

        public: static constexpr ushort SOUND_JUMPTOCf = 89;

        public: static constexpr ushort SOUND_JUMPENDh = 90;

        public: static constexpr ushort SOUND_JUMPTOCh = 91;

        public: static constexpr ushort SOUND_FOLLOW = 92;

        public: static constexpr ushort KEY_JUMP = 1;

        public: static constexpr ushort KEY_FIRE = 2;

        public: static constexpr ushort KEY_DOWN = 4;

        static constexpr bool getHasSound(){return true;}

        static constexpr bool getEasyMove(){ return true;}

    };

}
#endif // DEF_H
