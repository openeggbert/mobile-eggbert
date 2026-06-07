/**
 * @file Game1.hpp
 * @brief Declaration of the top-level Game1 class for Speedy Blupi.
 *
 * @details This file declares Game1, the central coordinator of the Speedy Blupi
 * game session.  It owns all major subsystems (graphics, audio, gameplay simulation,
 * input, and persistent data) and implements the XNA-style game loop
 * (Initialize / LoadContent / Update / Draw).  A phase state machine governs
 * which screen or mode is currently active; every transition is funnelled through
 * SetPhase() to keep side-effects centralised.
 *
 * This is a C++ port of the original XNA/Windows Phone C# codebase.
 */

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
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "System/TimeSpan.hpp"
#include "System/EventArgs.hpp"

/// @brief Expands to @c mutable, mirroring the C# @c readonly keyword during porting.
/// @note This macro is a porting shim only.  Do not use it in new code.
#define readonly mutable

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @class Game1
     * @brief Top-level game class.  Owns all major subsystems and drives the game loop.
     *
     * @details Game1 is the entry point for the game session.  It inherits from both
     * the XNA/CNA Game base class (which provides the platform game loop via
     * Initialize / LoadContent / Update / Draw) and IGame1 (which allows subsystems
     * to call back into Game1 without a circular dependency).
     *
     * **Subsystem ownership**
     * | Field        | Type                      | Responsibility                                    |
     * |--------------|---------------------------|---------------------------------------------------|
     * | graphics     | GraphicsDeviceManager     | Manages the display device and window.            |
     * | pixmap       | IPixmap (Pixmap)          | Sprite rendering and background caching.          |
     * | sound        | ISound (Sound)            | Audio playback and music.                         |
     * | decor        | Decor                     | Core gameplay simulation (map, entities, physics).|
     * | inputPad     | InputPad                  | Touch / mouse / gamepad input handling.           |
     * | gameData     | GameData                  | Persistent player progress and settings.          |
     * | waitJauge    | Jauge                     | Loading progress bar shown during Wait phase.     |
     *
     * **Phase state machine**
     *
     * Game1 drives the top-level Def::Phase state machine.  The active phase
     * determines which UI screens, overlays, and gameplay subsystems are updated and
     * drawn each frame.  Every phase transition must go through SetPhase() — direct
     * assignment to `phase` is not permitted — so that background assets are loaded,
     * InputPad is reconfigured, and optional fade-out animations are triggered
     * consistently.
     *
     * See Game1.cpp for the full phase transition graph.
     *
     * @note This is the port infrastructure entry point.  The original game logic
     *       lives in Decor, not here.
     * @note The @c readonly macro expands to @c mutable for C# porting compatibility.
     *
     * @see Decor
     * @see IGame1
     * @see Def::Phase
     */
    class Game1 : public Microsoft::Xna::Framework::Game, public IGame1
    {
        /// @brief Number of elements in #waitTable (12 threshold/level pairs × 2 = 24).
        static constexpr int waitTableLength = 24;

        /**
         * @brief Lookup table mapping loading-progress fractions to gauge fill levels.
         *
         * @details The table is a flat array of 12 interleaved pairs:
         * @code
         *   { threshold₀, level₀,  threshold₁, level₁, … }
         * @endcode
         * During the Wait phase, DrawWaitProgress() walks this table and sets the
         * waitJauge to the first @c level whose paired @c threshold is greater than or
         * equal to #waitProgress.  The resulting curve is intentionally non-linear so
         * the progress bar accelerates during fast loads and decelerates near the end,
         * matching the behaviour of the original Windows Phone release.
         *
         * @note Thresholds must be strictly ascending; the final threshold must be 1.0
         *       so the gauge always reaches 100 when loading finishes.
         * @see DrawWaitProgress()
         */
        //pairs: threshold, level
        static constexpr double waitTable[waitTableLength] =
        {
            0.1, 7.0, 0.2, 20.0, 0.25, 22.0, 0.45, 50.0, 0.6, 53.0,
            0.65, 58.0, 0.68, 60.0, 0.8, 70.0, 0.84, 75.0, 0.9, 84.0,
            0.94, 91.0, 1.0, 100.0
        };

        /// @brief Number of button taps in the cheat unlock gesture sequence.
        static constexpr int cheatGesteLength = 10;

        /**
         * @brief Required gesture sequence to unlock the cheat menu.
         *
         * @details The player must tap these ten Cheat grid buttons in this exact order
         * to reveal the cheat code input panel.  #cheatGesteIndex tracks how many
         * leading buttons of this sequence have been pressed correctly.  Any incorrect
         * press resets #cheatGesteIndex to zero.  This preserves the original game's
         * cheat-unlock mechanism.
         *
         * @see CheatAction()
         * @see cheatGesteIndex
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

        readonly Microsoft::Xna::Framework::GraphicsDeviceManager graphics; ///< @brief Manages the graphics device and display window.

        readonly std::shared_ptr<IPixmap> pixmap; ///< @brief Rendering subsystem; shared with Decor and InputPad.

        readonly std::shared_ptr<ISound> sound; ///< @brief Audio subsystem; shared with Decor.

        readonly Decor decor; ///< @brief Core gameplay simulation (map, entities, physics, doors).

        readonly InputPad inputPad; ///< @brief Translates raw input events into ButtonGlyph presses.

        readonly GameData gameData; ///< @brief Persistent player progress, settings, and save data.

        /** @brief Current high-level game phase.
         *
         *  @details Controls which UI screens, overlays, and gameplay are active.
         *  Must only be modified through SetPhase() to maintain invariants.
         *  @see SetPhase(Def::Phase)
         *  @see SetPhase(Def::Phase, int)
         */
        Def::Phase phase;

        /** @brief Absolute game-time timestamp at which the current Wait phase began.
         *
         *  @details Used exclusively in the Wait phase to compute #waitProgress.
         *  The elapsed ticks since this value determine how far along the artificial
         *  loading delay the game is.
         *  @see waitProgress
         */
        System::TimeSpan startTime;

        /** @brief First mission index queued for the next frame's SetPhase call.
         *
         *  @details When a menu-to-Play transition needs a fade-out animation, the
         *  real mission number is stored here so the Draw pass can promote it to
         *  #missionToStart2 after the background has been refreshed.
         *  @retval -1 No mission is queued.
         *  @see missionToStart2
         */
        int missionToStart1;

        /** @brief Second mission index consumed at the top of the next Update.
         *
         *  @details After #missionToStart1 has been promoted (via Draw), Update picks
         *  up this value and calls SetPhase(Phase::Play, missionToStart2).
         *  @retval -1 No mission is queued.
         *  @see missionToStart1
         */
        int missionToStart2;

        /** @brief Index of the currently active mission/level (1-based).
         *
         *  @details Level numbers encode world and sub-level:
         *  the tens digit is the world and the units digit is the sub-level within
         *  that world (e.g. 21 = world 2, sub-level 1).  Mission 1 is the tutorial.
         */
        int mission = 0;

        /** @brief How many leading entries of #cheatGeste the player has matched so far.
         *
         *  @details Incremented on each correct cheat-grid button press; reset to zero
         *  on any incorrect button press.  When it reaches #cheatGesteLength the cheat
         *  menu is revealed and this index resets.
         *  @see cheatGeste
         *  @see CheatAction()
         */
        int cheatGesteIndex = 0;

        /** @brief State of a pending continue-mission request.
         *
         *  @details Set to @c Pending when the app is reactivated (OnActivated).
         *  Draw advances it to @c Active so that Update can safely attempt to restore
         *  the saved game via Decor::CurrentRead().  Resets to @c None after
         *  the attempt (successful or not).
         *  @see ContinueMission()
         *  @see OnActivated()
         */
        ContinueMissionType continueMission = ContinueMissionType::None;

        /** @brief Loading-screen progress bar widget displayed during the Wait phase.
         *
         *  @details Positioned at (196, 426), zoomed 2×, yellow style.  Its fill
         *  level is driven by DrawWaitProgress() using the #waitTable lookup.
         *  @see waitProgress
         *  @see waitTable
         *  @see DrawWaitProgress()
         */
        Jauge waitJauge;

        /** @brief Current loading-progress fraction in the range [0.0, 1.0].
         *
         *  @details Updated each frame during the Wait phase based on elapsed time.
         *  When it exceeds 1.0 the Wait phase ends and the Init phase begins.
         *  @see waitTable
         *  @see DrawWaitProgress()
         */
        double waitProgress = 0.0;

        /** @brief True when the game is running in trial/demo mode.
         *
         *  @details Re-queried from the platform each time SetPhase() is called.
         *  In trial mode only the first world's levels are accessible; attempting to
         *  enter later levels transitions to Phase::Trial instead.
         *  @see simulateTrialMode
         *  @see getIsTrialModeProperty()
         *  @see getIsRankingModeProperty()
         */
        bool isTrialMode = false;;

        /** @brief True when trial mode is being artificially forced in a full build.
         *
         *  @details Toggled by the Cheat6 cheat action.  When true,
         *  getIsRankingModeProperty() returns @c true regardless of platform licensing.
         *  Intended for QA testing of the trial-mode paywall screens.
         *  @warning Enabling this in a release build exposes the trial paywall to
         *           legitimate purchasers.  It is toggled at runtime only.
         *  @see getIsRankingModeProperty()
         *  @see CheatAction()
         */
        bool simulateTrialMode{false};

        /** @brief True while the in-game settings overlay (PlaySetup phase) is active.
         *
         *  @details Set by SetPhase() to @c true when entering Phase::PlaySetup and
         *  @c false for every other phase.  Used by the SetupReturn button handler to
         *  decide whether to return to gameplay (Play) or to the main menu (Init).
         *  @see SetPhase(Def::Phase, int)
         */
        bool playSetup = false;

        /** @brief Frame counter for the current phase.
         *
         *  @details Incremented once per Update call and reset to zero by SetPhase().
         *  Used to drive timed UI entry/exit animations in DrawBackgroundFade() and
         *  to trigger the deferred fade-out transition in Update.
         *  @see fadeOutPhase
         *  @see DrawBackgroundFade()
         */
        int phaseTime{0};

        /** @brief The phase to transition to once the current fade-out animation expires.
         *
         *  @details When SetPhase() is called from a phase that supports animated
         *  transitions (Init, MainSetup, PlaySetup, Pause, Resume), the requested
         *  target phase is stored here instead of being applied immediately.  Update
         *  watches this value and commits the real transition after
         *  Config::ScaleTime(20) frames.
         *  @retval Def::Phase::None No pending transition; the fade-out is not active.
         *  @see fadeOutMission
         *  @see phaseTime
         */
        Def::Phase fadeOutPhase;

        /** @brief Mission index to load once the pending fade-out transition fires.
         *
         *  @details Stored alongside #fadeOutPhase so that the correct mission number
         *  is available when SetPhase() eventually applies the deferred transition.
         *  @see fadeOutPhase
         */
        int fadeOutMission{0};

#ifndef LEGACY
        /** @brief Current game-speed multiplier applied to Decor::MoveStep() calls.
         *
         *  @details Slow runs one simulation step every other frame; Normal runs one
         *  step per frame; Fast/Faster/Fastest run 2/4/8 steps per frame respectively.
         *  Only compiled when the @c LEGACY macro is not defined.
         *  @see SetGameSpeed()
         *  @see getGameSpeed()
         */
        GameSpeed gameSpeed{GameSpeed::Normal};
#endif

    public:
        /**
         * @brief Returns whether ranking (high-score) mode is active.
         *
         * @details Returns @c true when either the platform reports trial mode or
         * #simulateTrialMode is set, making ranking features visible on the main menu.
         *
         * @return @c true if ranking mode is active; @c false otherwise.
         *
         * @note This differs from getIsTrialModeProperty(): ranking mode can be
         *       simulated in a full build via the Cheat6 action.
         * @see simulateTrialMode
         * @see getIsTrialModeProperty()
         */
        [[nodiscard]] bool getIsRankingModeProperty() const override;

        /**
         * @brief Returns whether the platform reports genuine trial/demo mode.
         *
         * @details Currently always returns @c false in this port (full build).
         * The original Windows Phone implementation queried the Marketplace licence.
         *
         * @return @c false unconditionally in the current port.
         *
         * @see getIsRankingModeProperty()
         */
        [[nodiscard]] bool getIsTrialModeProperty() const override;

        /**
         * @brief Constructs the game, initialises all subsystems, and queues Phase::First.
         *
         * @details Construction order:
         * 1. GraphicsDeviceManager is attached to this Game instance.
         * 2. Pixmap and Sound are allocated and given cross-references.
         * 3. Decor is created with pointers to Sound, Pixmap, and GameData.
         * 4. InputPad is created with pointers to Decor, Pixmap, Sound, and GameData.
         * 5. Tables::Init() populates static game data tables.
         * 6. The Exiting event is wired to OnExiting().
         * 7. Touch/mouse cursor visibility is configured based on hardware capabilities.
         * 8. Content root is set to "Content"; target elapsed time is configured from
         *    Config::TIME_SCALE.
         * 9. waitJauge is positioned and hidden until the Wait phase.
         * 10. SetPhase(Phase::First) is called to start the boot sequence.
         *
         * @post phase == Def::Phase::First.
         * @post missionToStart1 == -1 and missionToStart2 == -1.
         * @post waitJauge is visible and positioned at (196, 426) with zoom 2.0.
         */
        Game1();

        /**
         * @brief Destroys the Game1 object and releases all owned resources.
         *
         * @details Subsystems held by @c shared_ptr are released in reverse
         * construction order.  Any platform-specific teardown that could not be
         * deferred to destructors is handled by OnExiting().
         *
         * @see OnExiting()
         */
        virtual ~Game1();

    protected:
        /**
         * @brief Performs platform-level initialisation of the XNA/CNA framework.
         *
         * @details Delegates to Microsoft::Xna::Framework::Game::Initialize() to
         * set up the graphics device, input services, and the game-services container.
         * Game-specific initialisation (content loading, subsystem startup) is
         * deferred to LoadContent() and the Phase::First handling in Update().
         *
         * @post The graphics device is ready; the game loop may begin.
         * @see LoadContent()
         */
        void Initialize() override;

        /**
         * @brief Loads the initial background asset required before Phase::First runs.
         *
         * @details Calls @c pixmap->BackgroundCache("wait") to pre-load the loading
         * screen background so it is available when the Wait phase begins.  The
         * heavier per-phase asset loads (Init, Pause, Play backgrounds) happen inside
         * SetPhase() on demand.
         *
         * @pre The graphics device is initialised (Initialize() has returned).
         * @post The "wait" background image is resident in the pixmap cache.
         * @see Initialize()
         * @see SetPhase(Def::Phase, int)
         */
        void LoadContent() override;

        /**
         * @brief Releases content managed by the ContentManager.
         *
         * @details Currently a no-op; content lifetime is managed by the pixmap and
         * sound subsystems directly.  Provided to satisfy the base-class contract.
         */
        void UnloadContent() override;

        /**
         * @brief Saves gameplay state when the application is deactivated (loses focus).
         *
         * @details If the game is currently in Phase::Play, the live gameplay state is
         * serialised via Decor::CurrentWrite() so it can be restored when the app
         * returns (see OnActivated()).  For any other phase the stale save file is
         * deleted via Decor::CurrentDelete() to avoid restoring an inconsistent state.
         * Calls Game::OnDeactivated() after performing the save.
         *
         * @param[in] sender  The object that raised the Deactivated event (may be null).
         * @param[in] args    Event arguments (unused in this implementation).
         *
         * @post If phase was Phase::Play: save file contains the current game state.
         * @post Otherwise: save file is absent.
         * @see OnActivated()
         * @see ContinueMission()
         */
        void OnDeactivated(System::Object* sender, const System::EventArgs& args) override;

        /**
         * @brief Signals that a continue-mission attempt should be made after reactivation.
         *
         * @details Sets #continueMission to @c ContinueMissionType::Pending.  The
         * Draw() method will advance it to @c Active on the next rendered frame, at
         * which point Update() will call Decor::CurrentRead() to attempt a restore.
         * Calls Game::OnActivated() after recording the intent.
         *
         * @param[in] sender  The object that raised the Activated event (may be null).
         * @param[in] args    Event arguments (unused in this implementation).
         *
         * @post continueMission == ContinueMissionType::Pending.
         * @see OnDeactivated()
         * @see ContinueMission()
         */
        void OnActivated(System::Object* sender, const System::EventArgs& args) override;

        /**
         * @brief Cleans up persistent state when the application exits.
         *
         * @details Deletes the current-game save file via Decor::CurrentDelete() so
         * that a clean restart is guaranteed on the next launch.  Invoked from the
         * Exiting event delegate wired in the constructor.
         *
         * @param[in] args  Exit event arguments (unused in this implementation).
         *
         * @post The current-game save file is absent.
         * @see Game1()
         * @see OnDeactivated()
         */
        void OnExiting(System::Object* sender, const System::EventArgs& args) override;

        /**
         * @brief Advances the game state by one logical frame.
         *
         * @details Update is the heart of the game loop.  Its responsibilities are:
         * - Handle the hardware Back button (pause, return to menu, or exit).
         * - Drive the two-stage fade-out/mission-start pipeline.
         * - On Phase::First: trigger resource loading and transition to Phase::Wait.
         * - On Phase::Wait: compute #waitProgress and transition to Phase::Init when done.
         * - For all other phases: poll InputPad for button presses and dispatch them
         *   to phase transitions, settings toggles, cheat gestures, and gameplay
         *   actions.
         * - On Phase::Play: forward the pressed button to Decor, run one or more
         *   simulation steps (respecting #gameSpeed), and handle level-end outcomes
         *   (lost / win / next mission).
         *
         * @param[in,out] gameTime  Elapsed- and total-time snapshot for this frame.
         *                          The total time is used to derive #waitProgress.
         *
         * @pre The game loop has been started; Initialize() and LoadContent() have
         *      returned successfully.
         * @post Game state (phase, mission, gameData) may have been updated.
         *
         * @see Draw()
         * @see SetPhase(Def::Phase, int)
         * @see MissionBack()
         * @see StartMission()
         * @see ContinueMission()
         * @see CheatAction()
         */
        void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        /**
         * @brief Returns to the beginning of the current world or to the main menu.
         *
         * @details If the current mission is 1 (tutorial) the game returns to Init.
         * Otherwise the target is the first sub-level of the current world
         * (i.e. @c mission rounded down to the nearest multiple of 10, or 1 if
         * mission is already a round multiple — e.g. mission 23 → 20, mission 20 → 1).
         *
         * @pre phase == Def::Phase::Pause and mission >= 1.
         * @post SetPhase() has been called with the computed back destination.
         *
         * @see Update()
         * @see mission
         */
        void MissionBack() override;

        /**
         * @brief Loads and starts the specified mission.
         *
         * @details In trial mode, any mission beyond world 2 (mission > 20) that is
         * not the first sub-level of a world redirects to Phase::Trial instead.
         * Otherwise the mission data is read from disk via Decor::Read(), images are
         * loaded, doors and lives are initialised from #gameData, and gameplay begins.
         * Updates #gameData with the new last-world value for missions other than 1.
         *
         * @param[in] mission  1-based mission number to start.  Format: tens = world,
         *                     units = sub-level within world.
         *
         * @pre mission >= 1.
         * @pre phase == Def::Phase::Play (StartMission is always called from SetPhase
         *      when transitioning to Play with a positive mission number).
         * @post If not redirected to Trial: Decor is loaded and running for @p mission.
         * @post If redirected: phase == Def::Phase::Trial.
         *
         * @see SetPhase(Def::Phase, int)
         * @see MissionBack()
         */
        void StartMission(int mission) override;

        /**
         * @brief Resumes the game from a serialised save rather than starting fresh.
         *
         * @details Calls SetPhase(Phase::Play, -2) to enter Play without loading a
         * new map, then recovers the mission number from Decor::GetMission().  Images
         * are loaded and audio is started, but Decor::Read() is NOT called — the map
         * state was already restored by Decor::CurrentRead() in the Wait phase.
         *
         * @pre continueMission == ContinueMissionType::Active (called only from Update
         *      after a successful Decor::CurrentRead()).
         * @post phase == Def::Phase::Play.
         * @post mission reflects the restored level.
         *
         * @see OnActivated()
         * @see StartMission()
         */
        void ContinueMission() override;

        /**
         * @brief Dispatches a cheat-code button press to the appropriate cheat action.
         *
         * @details Maps Cheat1–Cheat9 ButtonGlyph values to the following effects:
         * - Cheat1: Decor::CheatAction(OpenDoors)
         * - Cheat2: Decor::CheatAction(SuperBlupi)
         * - Cheat3: Decor::CheatAction(ShowSecret)
         * - Cheat4: Decor::CheatAction(LayEgg)
         * - Cheat5: GameData::Reset()
         * - Cheat6: Toggle #simulateTrialMode
         * - Cheat7: Decor::CheatAction(CleanAll)
         * - Cheat8: Decor::CheatAction(AllTreasure)
         * - Cheat9: Decor::CheatAction(EndGoal)
         *
         * @param[in] glyph  The cheat button that was pressed.
         *                   Must be in range [ButtonGlyph::Cheat1, ButtonGlyph::Cheat9].
         *
         * @pre The cheat menu is visible (InputPad::setShowCheatMenuProperty was set
         *      to true after completing the cheat gesture).
         * @post The corresponding game or simulation state has been modified.
         *
         * @see cheatGeste
         * @see simulateTrialMode
         */
        void CheatAction(Def::ButtonGlyph glyph) override;

    protected:
        /**
         * @brief Renders the current frame.
         *
         * @details Draw is called once per frame after Update.  Its responsibilities:
         * 1. If #continueMission is @c Pending, advance it to @c Active so that
         *    Update can safely perform the restore on the next logical frame.
         * 2. For all non-Play phases: draw the static background, optionally trigger
         *    #missionToStart1 → #missionToStart2 promotion, then draw the fade
         *    animation, interactive buttons, and overlaid text.
         * 3. For Phase::Play: delegate rendering entirely to Decor::Build() and
         *    InputPad::Draw().
         * 4. For Phase::Wait: overlay the loading-progress gauge.
         * 5. Flush the sprite batch via pixmap::EndBatch().
         *
         * @param[in] gameTime  Elapsed- and total-time snapshot for this frame
         *                      (passed through to the base-class Draw).
         *
         * @pre pixmap is initialised and LoadContent() has returned.
         * @post All visible game elements for the current frame have been submitted
         *       to the render pipeline.
         *
         * @see Update()
         * @see DrawBackgroundFade()
         * @see DrawButtonsBackground()
         * @see DrawButtonsText()
         * @see DrawWaitProgress()
         */
        void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

    private:
        /**
         * @brief Draws animated background images for the current phase transition.
         *
         * @details Renders entrance and exit animations for the SpeedyBlupi, BlupiYoupie,
         * and Gear background layers.  The animation style depends on the current
         * #phase and whether a #fadeOutPhase is pending:
         * - Phase::Init (entering): BlupiYoupie scales and fades in with a spin;
         *   SpeedyBlupi slides in from the right.
         * - Phase::Init (exiting toward MainSetup): SpeedyBlupi reverses its slide;
         *   BlupiYoupie zooms out.
         * - Phase::Init (exiting toward other phases): BlupiYoupie zooms out fast.
         * - Phase::Pause / Phase::Resume: BlupiYoupie rotates in or slides away
         *   depending on the destination.
         * - Phase::MainSetup / Phase::PlaySetup: SpeedyBlupi and two counter-rotating
         *   gear overlays animate in/out.
         * - Phase::Lost: BlupiYoupie spins in with a high rotation count.
         * - Phase::Win: BlupiYoupie pulsates with a sine-wave scale.
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post Animated background icons have been submitted to pixmap.
         *
         * @note This method is called only when #fadeOutPhase != Phase::None or the
         *       phase-entry animation is still in progress (phaseTime < threshold).
         * @see Draw()
         * @see phaseTime
         * @see fadeOutPhase
         */
        void DrawBackgroundFade() override;

        /**
         * @brief Draws semi-transparent panel backgrounds behind the Init-phase buttons.
         *
         * @details Renders two translucent Pad sprites (opacity 0.3) behind the gamer
         * selector buttons (left panel) and the Play/Setup/Buy/Ranking buttons (right
         * panel).  The right panel height expands if trial or ranking mode is active.
         * Only active during Phase::Init.
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post Panel backgrounds have been submitted to pixmap for Phase::Init.
         *
         * @see Draw()
         * @see DrawButtonsText()
         */
        void DrawButtonsBackground() override;

        /**
         * @brief Draws text labels adjacent to interactive buttons for the current phase.
         *
         * @details Dispatches to the appropriate per-phase label drawing:
         * - Phase::Init: gamer names/stats, Play, Setup, optionally Buy and Ranking.
         * - Phase::Pause: Menu, Back (if not mission 1), Setup, Restart (if applicable),
         *   Continue labels.
         * - Phase::Resume: Menu and Continue labels.
         * - Phase::MainSetup / Phase::PlaySetup: all settings toggle labels; MainSetup
         *   also draws the per-gamer Reset label with the selected gamer letter.
         * - Phase::Trial: multi-line trial explanation text plus Buy and Back labels.
         * - Phase::Ranking: Back label.
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post Text labels have been submitted to pixmap for the current phase.
         *
         * @see Draw()
         * @see DrawButtonsBackground()
         * @see DrawButtonGamerText()
         * @see DrawTextRightButton(Def::ButtonGlyph, int)
         * @see DrawTextUnderButton()
         */
        void DrawButtonsText() override;

        /**
         * @brief Draws the gamer name, door count, and life count adjacent to a gamer button.
         *
         * @details Renders four lines of text to the right of the button identified by
         * @p glyph: the gamer letter (e.g. "Gamer A"), main-door count, secondary-door
         * count, and remaining lives.  Text is queried from #gameData via
         * GameData::GetGamerInfo().
         *
         * @param[in] glyph  The button glyph identifying the gamer slot
         *                   (InitGamerA, InitGamerB, or InitGamerC).
         * @param[in] gamer  Zero-based gamer slot index (0 = A, 1 = B, 2 = C).
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @pre @p gamer is in the range [0, 2].
         * @post Four text lines have been submitted to pixmap to the right of @p glyph.
         *
         * @see DrawButtonsText()
         */
        void DrawButtonGamerText(Def::ButtonGlyph glyph, int gamer) override;

        /**
         * @brief Draws localised text to the right of a button (resource-ID overload).
         *
         * @details Loads the string identified by @p res from MyResource and delegates
         * to DrawTextRightButton(Def::ButtonGlyph, std::string).
         *
         * @param[in] glyph  The button whose right edge is used as the text anchor.
         * @param[in] res    MyResource string ID.
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post Text has been submitted to pixmap to the right of @p glyph.
         *
         * @see DrawTextRightButton(Def::ButtonGlyph, std::string)
         */
        void DrawTextRightButton(Def::ButtonGlyph glyph, int res) override;

        /**
         * @brief Draws text to the right of a button (string overload).
         *
         * @details If @p text contains a single newline the string is split into two
         * lines, each drawn at 0.7 scale and vertically centred relative to the
         * button.  Single-line text is drawn centred in one pass.
         *
         * @param[in] glyph  The button whose right edge is used as the text anchor.
         * @param[in] text   UTF-8 string to draw; may contain one @c '\\n' separator.
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post Text has been submitted to pixmap to the right of @p glyph.
         *
         * @see DrawTextRightButton(Def::ButtonGlyph, int)
         */
        void DrawTextRightButton(Def::ButtonGlyph glyph, std::string text) override;

        /**
         * @brief Draws localised text centred below a button.
         *
         * @details Loads the string identified by @p res from MyResource and renders
         * it horizontally centred below the bottom edge of @p glyph's bounding rect.
         *
         * @param[in] glyph  The button below which the text is drawn.
         * @param[in] res    MyResource string ID.
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post Text has been submitted to pixmap below @p glyph.
         *
         * @see DrawButtonsText()
         */
        void DrawTextUnderButton(Def::ButtonGlyph glyph, int res) override;

        /**
         * @brief Draws the loading-screen progress gauge during Phase::Wait.
         *
         * @details Does nothing if a continue-mission is pending or active
         * (#continueMission != None).  Otherwise walks #waitTable to find the gauge
         * level corresponding to the current #waitProgress and calls
         * Jauge::SetLevel() followed by Jauge::Draw().
         *
         * @pre phase == Def::Phase::Wait.
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post waitJauge fill level reflects #waitProgress; the gauge sprite has been
         *       submitted to pixmap.
         *
         * @see waitProgress
         * @see waitTable
         * @see waitJauge
         * @see Draw()
         */
        void DrawWaitProgress() override;

        /**
         * @brief Draws a debug overlay showing the total touch/click event count.
         *
         * @details Renders InputPad::getTotalTouchOrClickProperty() as text at screen
         * position (10, 20) at scale 1.0.  Not called from the normal render path;
         * must be wired in manually during debugging.
         *
         * @pre pixmap is in a valid BeginBatch() / EndBatch() block.
         * @post Debug text has been submitted to pixmap.
         *
         * @note Intended for development use only.  Not called during normal gameplay.
         */
        void DrawDebug() override;

        /**
         * @brief Selects the active gamer slot and persists the choice.
         *
         * @details Updates GameData::setSelectedGamerProperty() and writes the change
         * to disk via GameData::Write().
         *
         * @param[in] gamer  Zero-based gamer slot index (0 = A, 1 = B, 2 = C).
         *
         * @pre @p gamer is in the range [0, 2].
         * @post gameData reflects the new selected gamer; changes are persisted.
         *
         * @see Update()
         */
        void SetGamer(int gamer) override;

        /**
         * @brief Transitions to a new phase with mission index 0.
         *
         * @details Convenience overload — delegates to SetPhase(phase, 0).
         *
         * @param[in] phase  Target phase.
         *
         * @see SetPhase(Def::Phase, int)
         */
        void SetPhase(Def::Phase phase) override;

        /**
         * @brief Transitions to a new phase, optionally loading a specific mission.
         *
         * @details This is the sole legal mechanism for changing the active phase.
         * It handles three concerns:
         *
         * 1. **Fade-out deferral** — If the current phase supports exit animations
         *    (Init, MainSetup, PlaySetup, Pause, Resume) and no other fade is active,
         *    the real transition is deferred: #fadeOutPhase and #fadeOutMission are
         *    recorded, #phaseTime is reset, and the method returns early.  Update()
         *    will commit the transition after Config::ScaleTime(20) frames.
         *
         * 2. **Two-stage mission loading** — When the target is Phase::Play with a
         *    non-negative mission and #missionToStart2 is not already set, the mission
         *    is staged in #missionToStart1 for Draw() to promote to #missionToStart2
         *    (ensuring the new background texture is resident before gameplay starts).
         *
         * 3. **Immediate commit** — For all other transitions the method:
         *    - Updates #phase and resets #phaseTime, #missionToStart2.
         *    - Calls InputPad::setPhaseProperty() and updates #playSetup.
         *    - Re-queries #isTrialMode from the platform.
         *    - Stops any playing sound via Decor::StopSound().
         *    - Calls pixmap::BackgroundCache() with the appropriate asset name.
         *    - If the target is Phase::Play with @p mission > 0, calls StartMission().
         *
         * @param[in] phase    Target phase.
         * @param[in] mission  Mission to load when @p phase is Phase::Play.
         *                     Pass 0 or negative to skip loading (continue current or
         *                     deferred mission).  Special value -2 bypasses all
         *                     staging and loads via ContinueMission() instead.
         *
         * @post If not deferred: this->phase == @p phase, phaseTime == 0.
         * @post If deferred: fadeOutPhase == @p phase, phaseTime == 0.
         *
         * @note Calling this method with the same phase that is already active resets
         *       the phase timer and reloads background assets.
         * @warning Never assign to the @c phase member directly; always use this method.
         *
         * @see SetPhase(Def::Phase)
         * @see StartMission()
         * @see fadeOutPhase
         * @see missionToStart1
         */
        void SetPhase(Def::Phase phase, int mission) override;

        /**
         * @brief Saves the current number of lives and door progress to #gameData.
         *
         * @details Reads Decor::GetNbVies() and copies door-open state via
         * Decor::MemorizeDoors(), then writes the updated record to disk.  Should be
         * called whenever a mission ends (lost, won, or advancing to the next level)
         * so progress is not lost on sudden termination.
         *
         * @pre phase == Def::Phase::Play; Decor has live state for the current mission.
         * @post gameData reflects the latest lives and door counts; changes persisted.
         *
         * @see Update()
         * @see GameData::Write()
         */
        void MemorizeGamerProgress() override;

    public:
        /**
         * @brief Toggles the window between windowed and full-screen modes.
         *
         * @details Delegates to GraphicsDeviceManager::ToggleFullScreen().
         * The new state is reflected immediately by the platform.
         *
         * @post IsFullScreen() returns the logical complement of its pre-call value.
         *
         * @see IsFullScreen()
         */
        void ToggleFullScreen() override;

        /**
         * @brief Returns whether the game is currently running in full-screen mode.
         *
         * @return @c true if the window is full-screen; @c false if windowed.
         *
         * @see ToggleFullScreen()
         */
        bool IsFullScreen() override;

#ifndef LEGACY
        /**
         * @brief Sets the game simulation speed.
         *
         * @details Updates #gameSpeed to @p speed.  The new speed takes effect on the
         * next Update() call.  Invalid enum values are silently ignored.
         * Only compiled when the @c LEGACY macro is not defined.
         *
         * @param[in] speed  Desired speed: Slow, Normal, Fast, Faster, or Fastest.
         *                   Any value not in this set is ignored.
         *
         * @post gameSpeed == @p speed if @p speed is valid; unchanged otherwise.
         *
         * @see getGameSpeed()
         * @see GameSpeed
         */
        void SetGameSpeed(GameSpeed speed) override;

        /**
         * @brief Returns the current game simulation speed.
         *
         * @details Only compiled when the @c LEGACY macro is not defined.
         *
         * @return The active #gameSpeed value.
         *
         * @see SetGameSpeed()
         * @see GameSpeed
         */
        [[nodiscard]] GameSpeed getGameSpeed() const override;
#endif

        /**
         * @brief Returns a copy of the GraphicsDeviceManager.
         *
         * @details Provided to satisfy the IGame1 interface so that subsystems can
         * access display parameters without a direct dependency on Game1.
         *
         * @return A copy of the #graphics member.
         *
         * @note Returning by value is intentional; the returned object shares the
         *       underlying platform handle with the original.
         */
        Microsoft::Xna::Framework::GraphicsDeviceManager getGraphics() override;

        /**
         * @brief Returns a reference to the game's ContentManager.
         *
         * @details Delegates to Microsoft::Xna::Framework::Game::getContentProperty().
         * Provided to satisfy the IGame1 interface.
         *
         * @return A reference to the ContentManager used to load XNB assets.
         *
         * @see getGraphicsDeviceProperty()
         */
        [[nodiscard]] Microsoft::Xna::Framework::Content::ContentManager& getContentProperty() override;

        /**
         * @brief Returns a reference to the graphics device.
         *
         * @details Delegates to Microsoft::Xna::Framework::Game::getGraphicsDeviceProperty().
         * Provided to satisfy the IGame1 interface.
         *
         * @return A reference to the active GraphicsDevice.
         *
         * @see getContentProperty()
         */
        [[nodiscard]] Microsoft::Xna::Framework::Graphics::GraphicsDevice& getGraphicsDeviceProperty() override;

        GetTypeNameHPP()
    };
};
