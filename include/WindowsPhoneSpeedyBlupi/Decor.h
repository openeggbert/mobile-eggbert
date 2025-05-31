//
// Created by robertvokac on 5/24/25.
//

#ifndef DECOR_H
#define DECOR_H
#include <cmath>

#include "Game1.h"

#include "Pixmap.h"
#include "Sound.h"
#include "Tables.h"
#include "System/Random.h"
#include "Jauge.h"
#include "WindowsPhoneSpeedyBlupi/Helper.h"

// // WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// // WindowsPhoneSpeedyBlupi.Decor
// using System;
// using System.Collections.Generic;
// using Microsoft.Xna.Framework.Media;
// using WindowsPhoneSpeedyBlupi;
// using static System.Net.Mime.MediaTypeNames;
namespace WindowsPhoneSpeedyBlupi{

class Decor {

            private: struct Cellule
        {
            public: int icon;
        };

        private: struct MoveObject
        {
            public: int type;

            public: int stepAdvance;

            public: int stepRecede;

            public: int timeStopStart;

            public: int timeStopEnd;

            public: TinyPoint posStart;

            public: TinyPoint posEnd;

            public: TinyPoint posCurrent;

            public: int step;

            public: int time;

            public: int phase;

            public: int channel;

            public: int icon;
        };

        private: class ByeByeObject
        {
            public: int channel;

            public: int icon;

            public: double posX;

            public: double posY;

            public: double rotation;

            public: double phase;

            public: double animationSpeed;

            public: double rotationSpeed;

            public: double speedX;
        };

        private: static constexpr int MAXMOVEOBJECT = 200;

        private: static constexpr int MAXQUART = 441;

        private: static constexpr int SCROLL_SPEED = 8;

        private: static constexpr int SCROLL_MARGX = 80;

        private: static constexpr int SCROLL_MARGY = 40;

        private: static constexpr int BLUPIFLOOR = 2;

        private: static constexpr int BLUPIOFFY = 4 + BLUPIFLOOR;

        private: static constexpr int BLUPISURF = 12;

        private: static constexpr int BLUPISUSPEND = 12;

        private: static constexpr int OVERHEIGHT = 80;

        private: std::optional<Sound> m_sound;

        private: std::optional<Pixmap> m_pixmap;

        private: GameData m_gameData;

        private: Cellule m_decor[100][100];

        private: Cellule m_bigDecor[100][100];

        private: static constexpr int m_balleTrajLength = 1300;
        private: int m_balleTraj[m_balleTrajLength];

        private: static constexpr int m_moveTrajLength = 1300;
        private: int m_moveTraj[m_moveTrajLength];

        private: MoveObject m_moveObject[MAXMOVEOBJECT];

        private: int m_keyPress;

        private: int m_lastKeyPress;

        private: TinyPoint m_posDecor;

        private: TinyPoint m_dimDecor;

        private: int m_term;

        private: int m_music;

        private: int m_region;

        private: int m_time;

        private: bool m_bPause;

        private: TinyRect m_drawBounds;

        private: int m_nbRankCaisse;

        private: static constexpr int m_rankCaisseLength = MAXMOVEOBJECT;
        private: int m_rankCaisse[m_rankCaisseLength];

        private: int m_nbLinkCaisse;

private: static constexpr int m_linkCaisseLength = MAXMOVEOBJECT;
        private: int m_linkCaisse[m_linkCaisseLength];

        private: TinyPoint m_blupiPos;

        private: TinyPoint m_blupiLastPos;

        private: TinyPoint m_blupiValidPos;

        private: int m_blupiAction;

        private: int m_blupiDir;

        private: int m_blupiPhase;

        private: double m_blupiVitesseX;

        private: double m_blupiVitesseY;

        private: int m_blupiIcon;

        private: int m_blupiSec;

        private: int m_blupiChannel;

        private: TinyPoint m_blupiVector;

        private: int m_blupiTransport;

        private: bool m_blupiFocus;

        private: bool m_blupiAir;

        private: bool m_blupiHelico;

        private: bool m_blupiOver;

        private: bool m_blupiJeep;

        private: bool m_blupiTank;

        private: bool m_blupiSkate;

        private: bool m_blupiNage;

        private: bool m_blupiSurf;

        private: bool m_blupiVent;

        private: bool m_blupiSuspend;

        private: bool m_blupiJumpAie;

        private: bool m_blupiShield;

        private: bool m_blupiPower;

        private: bool m_blupiCloud;

        private: bool m_blupiHide;

        private: bool m_blupiInvert;

        private: bool m_blupiBalloon;

        private: bool m_blupiEcrase;

        private: bool m_blupiMotorHigh;

        private: int m_blupiMotorSound;

        private: TinyPoint m_blupiPosHelico;

        private: TinyPoint m_blupiPosMagic;

        private: bool m_blupiRestart;

        private: bool m_blupiFront;

        private: int m_blupiBullet;

        private: int m_blupiCle;

        private: int m_blupiPerso;

        private: int m_blupiDynamite;

        private: int m_blupiNoBarre;

        private: int m_blupiTimeShield;

        private: int m_blupiTimeFire;

        private: int m_blupiTimeNoAsc;

        private: int m_blupiTimeMockery;

        private: int m_blupiTimeOuf;

        private: int m_blupiActionOuf;

        private: int m_blupiFifoNb;

        private: TinyPoint m_blupiFifoPos[10];

        private: TinyPoint m_blupiStartPos;

        private: int m_blupiStartDir;

        private: double m_blupiSpeedX;

        private: double m_blupiSpeedY;

        private: double m_blupiLastSpeedX;

        private: double m_blupiLastSpeedY;

        private: Jauge m_jauges[2];

        private: int m_blupiLevel;

        private: bool m_bFoundCle;

        private: bool m_bPrivate;

        private: bool m_bCheatDoors;

        private: bool m_bSuperBlupi;

        private: bool m_bDrawSecret;

        private: bool m_buildOfficialMissions;

        private: int m_mission;

        private: static constexpr int m_doorsLength = 200;
        private: int m_doors[m_doorsLength];

        private: int m_nbVies;

        private: int m_nbTresor;

        private: int m_totalTresor;

        private: int m_goalPhase;

        private: int m_detectIcon;

        private: TinyPoint m_sucettePos;

        private: int m_sucetteType;

        private: int m_blupiLogicRotation;

        private: int m_blupiRealRotation;

        private: int m_blupiOffsetY;

        private: TinyPoint m_scrollPoint;

        private: TinyPoint m_scrollAdd;

        private: int m_voyageIcon;

        private: int m_voyageChannel;

        private: int m_voyagePhase;

        private: int m_voyageTotal;

        private: TinyPoint m_voyageStart;

        private: TinyPoint m_voyageEnd;

        private: int m_decorAction;

        private: int m_decorPhase;

        private: int m_lastDecorIcon[200];

        private: double m_hotSpotFinalZoom;

        private: double m_hotSpotFinalX;

        private: double m_hotSpotFinalY;

        private: double m_hotSpotCurrentZoom;

        private: double m_hotSpotCurrentX;

        private: double m_hotSpotCurrentY;

        private: double m_hotSpotStepZoom;

        private: double m_hotSpotStepX;

        private: double m_hotSpotStepY;

        private: double m_hotSpotOutLag;

        private:
    System::Random m_random;

        private: std::vector<ByeByeObject> byeByeObjects;

public:
    [[nodiscard]] TinyRect getDrawBounds() const;

public:
    void setDrawBounds(const TinyRect &v);

    ddata(Def::ButtonGlyph, ButtonPressed)

        private: static void MoveObjectCopy(MoveObject& dst, const MoveObject &src);

        public: Decor();

        public: void Create(Sound& sound, Pixmap& pixmap, GameData& gameData);

        public: bool LoadImages();

        private: void InitDecor();

        public: void PlayPrepare(bool bTest);

        private: void BuildPrepare();

        public: int IsTerminated();

        public: void MoveStep();

        private: void ResetHotSpot();

        private: void MoveHotSpot();

        private: bool BlitzActif(int celx, int cely);

        public: void Build();

        private: void DrawInfo();

        private: bool IsDisplayInfo(int tableTresor);

        private: TinyPoint DecorNextAction();

        public: void SetSpeedX(double speed);

        public: void SetSpeedY(double speed);

        public: void KeyChange(int keyPress);

        private: void GetBlupiInfo(bool& bHelico, bool& bJeep, bool& bSkate, bool& bNage);

        private: int SoundEnviron(int sound, int obstacle);

        private: void PlaySound(int sound, TinyPoint pos);

        public: void StopSound();

        public: void StartSound();

        private: void StopSound(int sound);

        private: void AdaptMotorVehicleSound();

        private: void PosSound(TinyPoint pos);

        private: int GetRegion();

        private: void SetRegion(int region);

        private: int GetMusic();

        private: void SetMusic(int music);

        public: TinyPoint GetDim();

        public: void SetDim(TinyPoint dim);

        public: int GetMission();

        public: void SetMission(int mission);

        public: int GetNbVies();

        public: void SetNbVies(int nbVies);

        public: void InitializeDoors(GameData gameData);

        public: void MemorizeDoors(GameData gameData);

        public: static string GetCheatTinyText(Def::ButtonGlyph glyph);

        public: void CheatAction(Tables::CheatCodes cheat);

        private: void SetBuildOfficialMissions(bool bMode);

        private: void BlupiSearchIcon();

        private: bool BlupiIsGround();

        private: TinyRect BlupiRect(TinyPoint pos);

        private: void BlupiAdjust();

        private: bool BlupiBloque(TinyPoint pos, int dir);

        private: void BlupiStep();

        private: void BlupiDead(int action1, int action2);

        private: TinyPoint GetPosDecor(TinyPoint pos);

        private: void BlupiAddFifo(TinyPoint pos);

        private: bool DecorDetect(TinyRect rect);

        private: bool DecorDetect(TinyRect rect, bool bCaisse);

        private: bool TestPath(const TinyRect& rect, const TinyPoint& start, TinyPoint& end);

        private: void MoveObjectPollution();

        private: void MoveObjectPlouf(TinyPoint pos);

        private: void MoveObjectTiplouf(TinyPoint pos);

        private: void MoveObjectBlup(TinyPoint pos);

        private: int IsWorld(TinyPoint pos);

        private: void ActiveSwitch(bool bState, TinyPoint cel);

        private: int GetTypeBarre(TinyPoint pos);

        private: bool IsLave(TinyPoint pos);

        private: bool IsPiege(TinyPoint pos);

        private: bool IsGoutte(TinyPoint pos, bool bAlways);

        private: bool IsScie(TinyPoint pos);

        private: bool IsSwitch(TinyPoint pos, TinyPoint& celSwitch);

        private: bool IsEcraseur(TinyPoint pos);

        private: bool IsBlitz(TinyPoint pos, bool bAlways);

        private: bool IsRessort(TinyPoint pos);

        private: bool IsTemp(TinyPoint pos);

        private: bool IsBridge(TinyPoint pos, TinyPoint& celBridge);

        private: int IsDoor(TinyPoint pos, TinyPoint& celPorte);

        private: int IsTeleporte(TinyPoint pos);

        private: bool SearchTeleporte(TinyPoint pos, TinyPoint& newpos);

        private: bool IsNormalJump(TinyPoint pos);

        private: bool IsSurfWater(TinyPoint pos);

        private: bool IsDeepWater(TinyPoint pos);

        private: bool IsOutWater(TinyPoint pos);

        private: bool IsPassIcon(int icon);

        private: bool IsBlocIcon(int icon);

        private: void FlushBalleTraj();

        private: void SetBalleTraj(TinyPoint pos);

        private: bool IsBalleTraj(TinyPoint pos);

        private: void FlushMoveTraj();

        private: void SetMoveTraj(TinyPoint pos);

        private: bool IsMoveTraj(TinyPoint pos);

        private: int SearchDistRight(TinyPoint pos, TinyPoint dir, int type);

        private: bool IsVentillo(TinyPoint pos);

        private: void NetStopCloud(int rank);

        private: void StartSploutchGlu(TinyPoint pos);

        private: int ObjectStart(TinyPoint pos, int type, int speed);

        private: bool ObjectDelete(TinyPoint pos, int type);

        private: void ModifDecor(TinyPoint pos, int icon);

        private: void MoveObjectStep();

        private: void MoveObjectStepLine(int i);

        private: void MoveObjectStepIcon(int i);

        private: void DynamiteStart(int i, int dx, int dy);

        private: int AscenseurDetect(TinyRect rect, TinyPoint oldpos, TinyPoint newpos);

        private: void AscenseurVertigo(int i, bool& bVertigoLeft, bool& bVertigoRight);

        private: bool AscenseurShift(int i);

        private: void AscenseurSynchro(int i);

        private: void UpdateCaisse();

        private: bool TestPushCaisse(int i, TinyPoint pos, bool bPop);

        private: bool TestPushOneCaisse(int i, TinyPoint move, int b);

        private: void SearchLinkCaisse(int rank, bool bPop);

        private: bool AddLinkCaisse(int rank);

        private: int CaisseInFront();

        private: int CaisseGetMove(int max);

        private: int MockeryDetect(TinyPoint pos);

        private: bool BlupiElectro(TinyPoint pos);

        private: void MoveObjectFollow(TinyPoint pos);

        private: int MoveObjectDetect(TinyPoint pos, bool& bNear);

        private: int MoveAscenseurDetect(TinyPoint pos, int height);

        private: int MoveChargeDetect(TinyPoint pos);

        private: int MovePersoDetect(TinyPoint pos);

        private: int MoveObjectDelete(TinyPoint cel);

        private: int MoveObjectFree();

        private: int SortGetType(int type);

        private: void MoveObjectSort();

        private: void MoveObjectPriority(int i);

        private: int MoveObjectSearch(TinyPoint pos);

        private: int MoveObjectSearch(TinyPoint pos, int type);

        private: void ByeByeHelico();

        private: void ByeByeAdd(int channel, int icon, TinyPoint pos, double rotationSpeed, double animationSpeed);

        private: void ByeByeStep();

        private: void ByeByeDraw(TinyPoint posDecor);

        private: TinyPoint VoyageGetPosVie(int nbVies);

        private: void VoyageInit(TinyPoint start, TinyPoint end, int icon, int channel);

        private: void VoyageStep();

        private: void VoyageDraw();
        private: bool IsFloatingObject(int i);

        private: bool IsRightBorder(int x, int y, int dx, int dy);

        private: bool IsFromage(int x, int y);

        private: bool IsGrotte(int x, int y);

        private: void AdaptMidBorder(int x, int y);

        private: void AdaptBorder(TinyPoint cel);

        public: void CurrentDelete();

        public: bool CurrentWrite();

        public: bool CurrentRead();

        public: bool Read(int gamer, int rank, bool bUser);

        private: bool Delete(int gamer, int rank, bool bUser);

        private: bool FileExist(int gamer, int rank, bool bUser);

        private: bool SearchWorld(int world, TinyPoint& blupi, int& dir);

        private: bool SearchDoor(int n, TinyPoint& cel, TinyPoint& blupi);

        private: bool SearchGold(int n, TinyPoint& cel);

        public: void MainSwitchInitialize(int lastWorld);

        public: void AdaptDoors(bool bPrivate);

        private: void OpenDoorsTresor();

        private: void OpenDoor(TinyPoint cel);

        private: void OpenDoorsWin();

        private: void OpenGoldsWin();

        private: void DoorsLost();

};
}


#endif //DECOR_H










