//
// Created by robertvokac on 3/27/25.
//

#ifndef GAME1_H
#define GAME1_H

#include <any>

#include "Decor.h"
#include "Def.h"
#include "Game1I.h"
#include "Jauge.h"
#include "PixmapI.h"
#include "SoundI.h"
#include "InputPad.h"
#include "Microsoft/Xna/Framework/Game.h"
#include "Microsoft/Xna/Framework/GameTime.h"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceManager.h"
#include "System/TimeSpan.h"
#include "System/Runtime/CompilerServices/EventArgs.h"

#define readonly mutable

namespace WindowsPhoneSpeedyBlupi {
class Game1 : public Microsoft::Xna::Framework::Game, public Game1I {
public:
private: static constexpr int waitTableLength = 24;
        private: static constexpr double waitTable[waitTableLength] =
        {
        0.1, 7.0, 0.2, 20.0, 0.25, 22.0, 0.45, 50.0, 0.6, 53.0,
        0.65, 58.0, 0.68, 60.0, 0.8, 70.0, 0.84, 75.0, 0.9, 84.0,
        0.94, 91.0, 1.0, 100.0
        };

private: static constexpr int cheatGesteLength = 10;

        private: static constexpr Def::ButtonGlyph cheatGeste[cheatGesteLength] =
        {
        Def::ButtonGlyph::Cheat12,
        Def::ButtonGlyph::Cheat22,
        Def::ButtonGlyph::Cheat32,
        Def::ButtonGlyph::Cheat12,
        Def::ButtonGlyph::Cheat11,
        Def::ButtonGlyph::Cheat21,
        Def::ButtonGlyph::Cheat22,
        Def::ButtonGlyph::Cheat21,
        Def::ButtonGlyph::Cheat31,
        Def::ButtonGlyph::Cheat32
        };

        private: readonly Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager graphics;

        private: readonly std::unique_ptr<PixmapI> pixmap;

        private: readonly std::unique_ptr<SoundI> sound;

        private: readonly Decor decor;

        private: readonly InputPad inputPad;

        private: readonly GameData gameData;

private: Def::Phase phase;

private:
    System::TimeSpan startTime;

        private: int missionToStart1;

        private: int missionToStart2;

        private: int mission = 0;

        private: int cheatGesteIndex =0;

        private: int continueMission = 0;

private: Jauge waitJauge;

        private: double waitProgress = 0.0;

        private: bool isTrialMode = false;;

        private: bool simulateTrialMode{false};

        private: bool playSetup = false;

        private: int phaseTime{0};

private: Def::Phase fadeOutPhase;

        private: int fadeOutMission{0};

public:
    public: [[nodiscard]] bool getIsRankingMode() const;

    public: [[nodiscard]] bool getIsTrialMode() const;

    Game1();
    virtual ~Game1();

protected: void Initialize() override;

        protected: void LoadContent () override;

        protected: void UnloadContent () override;

        protected: void OnDeactivated(std::any sender, System::Runtime::CompilerServices::EventArgs args) override;

protected: void OnActivated(std::any sender, System::Runtime::CompilerServices::EventArgs args) override;

        protected: void OnExiting(Microsoft::Xna::Framework::ExitingEventArgs args);

protected:void Update(const Microsoft::Xna::Framework::GameTime& gameTime) override;

        private: void MissionBack();

        private: void StartMission(int mission);

        private: void ContinueMission();

        private: void CheatAction(Def::ButtonGlyph glyph);

        protected: void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

        private: void DrawBackgroundFade();

        private: void DrawButtonsBackground();

        private: void DrawButtonsText();

        private: void DrawButtonGamerText(Def::ButtonGlyph glyph, int gamer);

        private: void DrawTextRightButton(Def::ButtonGlyph glyph, int res);

        private: void DrawTextRightButton(Def::ButtonGlyph glyph, string text);

        private: void DrawTextUnderButton(Def::ButtonGlyph glyph, int res);

        private: void DrawWaitProgress();

        private: void DrawDebug();

        private: void SetGamer(int gamer);

        private: void SetPhase(Def::Phase phase);

        private: void SetPhase(Def::Phase phase, int mission);

        private: void MemorizeGamerProgress();

        public: void ToggleFullScreen();
public: bool IsFullScreen();

public:
    Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager getGraphics();

};

}

#endif //GAME1_H
