#pragma once

#include <any>

#include "Decor.hpp"
#include "Def.hpp"
#include "IGame1.hpp"
#include "Jauge.hpp"
#include "IPixmap.hpp"
#include "ISound.hpp"
#include "InputPad.hpp"
#include "def/ContinueMission.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceManager.hpp"
#include "System/TimeSpan.hpp"
#include "System/EventArgs.hpp"

#define readonly mutable

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Top-level game class. Owns all major subsystems and drives the game loop.
     *
     * Game1 is the entry point for the game session. It inherits from both the XNA/CNA
     * Game base class (which provides the platform game loop via Initialize / LoadContent /
     * Update / Draw) and IGame1 (which allows subsystems to call back into Game1 without
     * a circular dependency).
     *
     * Ownership:
     * - GraphicsDeviceManager (graphics): manages the display device and window.
     * - Pixmap (pixmap): rendering/sprite drawing.
     * - Sound (sound): audio playback.
     * - Decor (decor): core gameplay simulation.
     * - InputPad (inputPad): input handling.
     * - GameData (gameData): persistent player progress.
     * - Jauge (waitJauge): loading progress bar shown during resource loading.
     *
     * Game phase state machine:
     * - Game1 drives the top-level Def::Phase state machine, which selects which
     *   UI screens, overlays, and gameplay are active each frame.
     * - Phase transitions are performed exclusively through SetPhase().
     *
     * @note This is the port infrastructure entry point. The original game logic
     *       lives in Decor, not here.
     * @note The `readonly` macro expands to `mutable` for C# porting compatibility.
     */
    class Game1 : public Microsoft::Xna::Framework::Game, public IGame1
    {
        static constexpr int waitTableLength = 24;
        /**
         * @brief Lookup table mapping loading progress fractions to gauge fill levels.
         *
         * Each pair is (threshold, level): when the loading progress fraction reaches
         * or exceeds threshold, the wait gauge is set to level (0..100).
         * Preserves the original game's non-linear loading bar behavior.
         */
        //pairs: threshold, level
        static constexpr double waitTable[waitTableLength] =
        {
            0.1, 7.0, 0.2, 20.0, 0.25, 22.0, 0.45, 50.0, 0.6, 53.0,
            0.65, 58.0, 0.68, 60.0, 0.8, 70.0, 0.84, 75.0, 0.9, 84.0,
            0.94, 91.0, 1.0, 100.0
        };

        static constexpr int cheatGesteLength = 10;

        /**
         * @brief Required gesture sequence to unlock the cheat menu.
         *
         * The player must tap these ten cheat buttons in order to reveal the
         * cheat code input panel. This is the original game's cheat unlock sequence.
         */
        static constexpr Def::ButtonGlyph cheatGeste[cheatGesteLength] =
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

        readonly Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager graphics; ///< Manages the graphics device and display window.

        readonly std::shared_ptr<IPixmap> pixmap; ///< Rendering subsystem (owned).

        readonly std::shared_ptr<ISound> sound; ///< Audio subsystem (owned).

        readonly Decor decor; ///< Core gameplay simulation (owned).

        readonly InputPad inputPad; ///< Input handling subsystem (owned).

        readonly GameData gameData; ///< Persistent player progress and settings (owned).

        /** Current high-level game phase (main menu, playing, paused, won, lost, etc.). */
        Def::Phase phase;

        /** Timestamp of when the current phase started — used for timed transitions. */
        System::TimeSpan startTime;

        /** First mission index queued to start on the next Update (used for level transitions). */
        int missionToStart1;

        /** Second mission index queued to start (some transitions require two steps). */
        int missionToStart2;

        /** Index of the currently active mission/level. */
        int mission = 0;

        /** How far into the cheat gesture sequence the player has progressed. */
        int cheatGesteIndex = 0;

        /** Whether a continue-mission request is pending, active, or absent. */
        ContinueMissionType continueMission = ContinueMissionType::None;

        /** Loading progress gauge shown during the Wait phase. */
        Jauge waitJauge;

        /** Current loading progress fraction [0,1] used to drive waitJauge. */
        double waitProgress = 0.0;

        /** True when the game is running in trial/demo mode (limited levels). */
        bool isTrialMode = false;;

        /** True when trial mode is being simulated in a full build (for testing). */
        bool simulateTrialMode{false};

        /** True when the in-game settings (PlaySetup) overlay is active. */
        bool playSetup = false;

        /** Frame counter for the current phase — used for timed UI animations and transitions. */
        int phaseTime{0};

        /** The phase to transition to after the current fade-out animation completes. */
        Def::Phase fadeOutPhase;

        /** The mission to load after the current fade-out animation completes. */
        int fadeOutMission{0};

#ifndef LEGACY
        /** Current game speed multiplier: Normal, Fast, Faster, Fastest. */
        GameSpeed gameSpeed{GameSpeed::Normal};
#endif

    public:
        [[nodiscard]] bool getIsRankingModeProperty() const override;

        [[nodiscard]] bool getIsTrialModeProperty() const override;

        Game1();
        virtual ~Game1();

    protected:
        void Initialize() override;

        void LoadContent() override;

        void UnloadContent() override;

        void OnDeactivated(std::any sender, System::EventArgs args) override;

        void OnActivated(std::any sender, System::EventArgs args) override;

        void OnExiting(Microsoft::Xna::Framework::ExitingEventArgs args) override;

        void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        void MissionBack() override;

        void StartMission(int mission) override;

        void ContinueMission() override;

        void CheatAction(Def::ButtonGlyph glyph) override;

    protected:
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        void DrawBackgroundFade() override;

        void DrawButtonsBackground() override;

        void DrawButtonsText() override;

        void DrawButtonGamerText(Def::ButtonGlyph glyph, int gamer) override;

        void DrawTextRightButton(Def::ButtonGlyph glyph, int res) override;

        void DrawTextRightButton(Def::ButtonGlyph glyph, std::string text) override;

        void DrawTextUnderButton(Def::ButtonGlyph glyph, int res) override;

        void DrawWaitProgress() override;

        void DrawDebug() override;

        void SetGamer(int gamer) override;

        void SetPhase(Def::Phase phase) override;

        void SetPhase(Def::Phase phase, int mission) override;

        void MemorizeGamerProgress() override;

    public:
        void ToggleFullScreen() override;
        bool IsFullScreen() override;

#ifndef LEGACY
        void SetGameSpeed(GameSpeed speed) override;
        [[nodiscard]] GameSpeed getGameSpeed() const override;
#endif

        Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager getGraphics() override;

        [[nodiscard]] Microsoft::Xna::Framework::Content::ContentManager& getContentProperty() override;

        [[nodiscard]] Microsoft::Xna::Framework::Graphics::GraphicsDevice& getGraphicsDeviceProperty() override;
        GetTypeNameHPP()
    };
};
