#pragma once

#include "WindowsPhoneSpeedyBlupi/Config.hpp"
#include "GameData.hpp"
#include "IPixmap.hpp"
#include "ISound.hpp"
#include "Tables.hpp"
#include "System/Random.hpp"
#include "Jauge.hpp"
#include "decor/DecorAction.hpp"
#include "SharpRuntime/Prop.hpp"
#include "WindowsPhoneSpeedyBlupi/Helper.hpp"
#include "decor/ObjectType.hpp"
#include "def/PixmapChannel.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Core gameplay simulation class. Owns the level state, Blupi player state,
     *        moving objects, tile map, and all gameplay logic.
     *
     * Decor is the central gameplay subsystem. It corresponds directly to the original
     * C# Decor class and preserves all original gameplay logic, movement constants,
     * collision rules, animation tables, and state-machine transitions.
     *
     * Responsibilities:
     * - Owning and simulating the 100x100 tile map (m_decor, m_bigDecor).
     * - Owning and advancing the Blupi player state machine (position, action, direction,
     *   vehicle modes, bonuses, physics).
     * - Owning and advancing all active moving objects (m_moveObject[]).
     * - Detecting collision between Blupi and tiles, moving objects, and hazards.
     * - Handling doors, switches, teleporters, lifts (ascenseurs), and triggers.
     * - Managing the viewport scroll position relative to Blupi's position.
     * - Triggering sound effects at gameplay events via ISound.
     * - Driving animation through per-object phase counters indexed into Tables.
     * - Reading and writing level save files via Worlds helpers.
     *
     * Does not own rendering resources. Drawing is done by forwarding to IPixmap.
     * Does not own input; receives key state via KeyChange() each frame.
     *
     * Coordinate systems used by this class:
     * - Tile coordinates: integer (x,y) in range [0, MAXCELX) x [0, MAXCELY).
     *   One tile = 64x64 pixels in game-space (DIMOBJX / DIMOBJY).
     * - Game-space pixel coordinates: tile * 64, used for Blupi position,
     *   moving object positions, and collision rectangles.
     * - Screen-space: game-space + scroll offset; handled by Pixmap, not Decor.
     *
     * @note This is gameplay code. Do not change movement or collision constants
     *       without understanding the original timing and tile-based assumptions.
     * @note Many numeric values in this class come from the original game's C++ source.
     *       Treat all constants as part of the original game logic.
     * @note Animation phase counters (m_blupiPhase, MoveObject::phase) are not sprite
     *       indices. They are step counters used to index into Tables animation arrays.
     */
    class Decor
    {
        /** Reserved for future use — not yet populated. */
        enum class IconType
        {

        };

        /**
         * @brief Represents one cell of the tile map.
         *
         * Each cell stores one icon index that identifies the tile sprite to display
         * and the collision/passability properties of that tile. The icon value is
         * looked up in the background sprite sheet (PixmapChannel::Background).
         *
         * @note icon is a tile/decor identifier, not a raw texture pixel coordinate.
         */
        struct Cellule
        {
            intcs icon; ///< Tile icon index used for rendering and collision classification.
        };

        /**
         * @brief Represents a moving object (enemy, crate, projectile, collectible, etc.)
         *        currently active in the level.
         *
         * Moving objects are stored in the fixed-size pool m_moveObject[MAXMOVEOBJECT].
         * Each object moves between posStart and posEnd along a linear path, advancing
         * by stepAdvance pixels per frame and receding by stepRecede pixels per frame.
         * The object pauses at each end for timeStopStart / timeStopEnd frames.
         *
         * @note posCurrent is in game-space pixel coordinates. Do not interpret it as
         *       a tile coordinate.
         * @note phase is an animation step counter, not a sprite index.
         * @note icon and channel identify which sprite to render — they are data-table
         *       identifiers from the animation tables, not raw pixel offsets.
         */
        struct MoveObject
        {
            ObjectType    type;          ///< Type of this moving object (enemy, crate, etc.).
            intcs         stepAdvance;   ///< Pixels per frame while moving toward posEnd.
            intcs         stepRecede;    ///< Pixels per frame while moving toward posStart.
            intcs         timeStopStart; ///< Frames to wait at posStart before advancing.
            intcs         timeStopEnd;   ///< Frames to wait at posEnd before receding.
            TinyPoint     posStart;      ///< Start position in game-space pixel coordinates.
            TinyPoint     posEnd;        ///< End position in game-space pixel coordinates.
            TinyPoint     posCurrent;    ///< Current position in game-space pixel coordinates.
            intcs         step;          ///< Current movement step / distance traveled this leg.
            intcs         time;          ///< Remaining wait time at current endpoint (frames).
            intcs         phase;         ///< Animation step counter (indexes into animation tables).
            PixmapChannel channel;       ///< Sprite sheet to use when drawing this object.
            intcs         icon;          ///< Icon slot index within the sprite sheet.
        };

        /**
         * @brief Particle object produced when an object is destroyed (e.g., helicopter explodes).
         *
         * ByeByeObjects are short-lived visual effects that fly off-screen after an object
         * is destroyed. They are not part of the gameplay collision system.
         *
         * @note posX/posY are in game-space floating-point coordinates.
         * @note phase here is a floating-point animation progress counter, not a table index.
         */
        class ByeByeObject
        {
        public:
            PixmapChannel channel;       ///< Sprite sheet for this fragment.
            intcs         icon;          ///< Icon slot for this fragment.
            double        posX;          ///< Current X position in game-space (float for smooth motion).
            double        posY;          ///< Current Y position in game-space.
            double        rotation;      ///< Current rotation angle in radians.
            double        phase;         ///< Floating-point animation phase counter.
            double        animationSpeed;///< Phase increment per frame.
            double        rotationSpeed; ///< Rotation increment per frame (radians).
            double        speedX;        ///< Horizontal velocity in game-space pixels per frame.
        };

        /** Maximum number of simultaneously active moving objects. */
        static constexpr intcs MAXMOVEOBJECT = 200;
        /** Maximum number of quarter-tile entries used in the tile adjacency system. */
        static constexpr intcs MAXQUART = 441;
        /** Viewport scroll speed in game-space pixels per frame. */
        static constexpr intcs SCROLL_SPEED = 8;
        /** Horizontal margin in game-space pixels before scrolling is triggered. */
        static constexpr intcs SCROLL_MARGX = 80;
        /** Vertical margin in game-space pixels before scrolling is triggered. */
        static constexpr intcs SCROLL_MARGY = 40;
        /** Number of pixels Blupi sinks into the floor tile (ground contact offset). */
        static constexpr intcs BLUPIFLOOR = 2;
        /** Total vertical offset of Blupi's feet from the top of the bounding box. */
        static constexpr intcs BLUPIOFFY = 4 + BLUPIFLOOR;
        /** Vertical submersion depth for surfboard mode (pixels into water surface). */
        static constexpr intcs BLUPISURF = 12;
        /** Vertical offset for rope/suspension mode (pixels into the anchor point). */
        static constexpr intcs BLUPISUSPEND = 12;
        /** Extra height of the overcraft (flat) mode bounding box in game-space pixels. */
        static constexpr intcs OVERHEIGHT = 80;
        ISound*    m_sound;    ///< Audio subsystem — not owned by Decor.
        IPixmap*   m_pixmap;   ///< Rendering subsystem — not owned by Decor.
        GameData*  m_gameData; ///< Persistent game/save data — not owned by Decor.

        /** Primary tile map: 100x100 cells, each storing a decor/tile icon index. */
        Cellule m_decor[100][100]{};
        /** Secondary (large background) tile map for big-tile rendering. Same layout as m_decor. */
        Cellule m_bigDecor[100][100]{};
        /** Flat array used to mark tile positions occupied by projectile (balle) trajectories. */
        static constexpr int m_balleTrajLength = 1300;
        intcs m_balleTraj[m_balleTrajLength]{};

        /** Flat array used to mark tile positions occupied by currently moving objects. */
        static constexpr int m_moveTrajLength = 1300;
        intcs m_moveTraj[m_moveTrajLength]{};

        /** Pool of all active moving objects in the level. */
        MoveObject m_moveObject[MAXMOVEOBJECT];

        /** Current key/button press state as a bitfield of KeyPressFlags. */
        intcs m_keyPress;
        /** Key/button press state from the previous frame (for edge detection). */
        intcs m_lastKeyPress;

        /** Current top-left scroll position of the viewport in game-space pixels. */
        TinyPoint m_posDecor;
        /** Dimensions of the visible area in game-space pixels. */
        TinyPoint m_dimDecor;

        /** Level termination state: 0=running, positive=won, negative=lost. */
        intcs m_term;

        /** Index of the background music track currently playing. */
        intcs m_music;

        /** Current world region (used to select background texture and music). */
        intcs m_region;

        /** Global frame counter incremented each MoveStep() call. Used for animation timing. */
        intcs m_time;

        /** Non-zero when gameplay is paused. Do not advance simulation while paused. */
        intcs m_bPause;

        /** Current draw bounds rectangle in game-space (used for culling draw calls). */
        TinyRect m_drawBounds;

        /** Number of active crate (caisse) entries in m_rankCaisse. */
        intcs m_nbRankCaisse;

        static constexpr intcs m_rankCaisseLength = MAXMOVEOBJECT;
        /** Indices into m_moveObject[] for objects classified as crates. */
        intcs m_rankCaisse[m_rankCaisseLength]{};

        /** Number of active linked-crate entries in m_linkCaisse. */
        intcs m_nbLinkCaisse;

        static constexpr intcs m_linkCaisseLength = MAXMOVEOBJECT;
        /** Indices for crates that are linked together and must move as a group. */
        intcs m_linkCaisse[m_linkCaisseLength];

        /** Current Blupi position in game-space pixel coordinates. */
        TinyPoint m_blupiPos;

        /** Blupi position from the previous frame, used for rollback on collision. */
        TinyPoint m_blupiLastPos;

        /** Last position where Blupi was confirmed safe (no lethal hazard contact). */
        TinyPoint m_blupiValidPos;

        /**
         * @brief Current action/state of the player in the gameplay state machine.
         *
         * This is gameplay state, not a sprite index. The animation table (Tables::table_blupi)
         * maps BlupiAction + m_blupiPhase + m_blupiDir to the actual sprite icon.
         * Do not reset casually; many branches depend on the current action.
         */
        BlupiAction m_blupiAction;

        /**
         * @brief Current facing direction of the player.
         *
         * Affects animation frame selection and movement direction.
         * This is gameplay state, not a rendering flag.
         */
        Direction m_blupiDir;

        /**
         * @brief Animation step counter for Blupi's current action.
         *
         * Incremented each frame and used to index into Tables::table_blupi.
         * This is not a sprite index — it is a position within the animation sequence
         * defined by the current BlupiAction.
         */
        intcs m_blupiPhase;

        /** Horizontal velocity of Blupi in game-space pixels per frame (original frame rate). */
        double m_blupiVitesseX;

        /** Vertical velocity of Blupi in game-space pixels per frame (original frame rate). */
        double m_blupiVitesseY;

        // Sub-pixel accumulators for Blupi physics at high FPS.
        // Accumulates fractional pixel deltas so integer truncation doesn't cause height loss.
        double m_blupiSubPixelX = 0.0;
        double m_blupiSubPixelY = 0.0;

        /**
         * @brief Current sprite icon index for Blupi.
         *
         * Resolved each frame from the animation table using m_blupiAction, m_blupiPhase,
         * and m_blupiDir. This is a rendering value derived from gameplay state.
         * Do not use this as a gameplay state variable.
         */
        intcs m_blupiIcon;

        /** Currently active secret power-up bonus. Affects collision and capability. */
        SecretPower m_blupiSec;

        /** Sprite sheet channel used to draw Blupi (changes with vehicle/mode). */
        PixmapChannel m_blupiChannel;

        /** Last movement vector applied to Blupi (used for animation and collision direction). */
        TinyPoint m_blupiVector;

        /** Index of the moving object currently transporting Blupi (e.g., a lift). -1 if none. */
        intcs m_blupiTransport;

        /** True when Blupi has input focus and can receive player control. */
        bool m_blupiFocus;

        /** True when Blupi is airborne (not standing on a surface). */
        bool m_blupiAir;

        /** True when Blupi is in helicopter mode (free vertical flight). */
        bool m_blupiHelico;

        /** True when Blupi is in overcraft (flat/squashed) mode. */
        bool m_blupiOver;

        /** True when Blupi is riding the jeep vehicle. */
        bool m_blupiJeep;

        /** True when Blupi is operating the tank vehicle. */
        bool m_blupiTank;

        /** True when Blupi is riding the skateboard. */
        bool m_blupiSkate;

        /** True when Blupi is swimming (nage = swim in French). */
        bool m_blupiNage;

        /** True when Blupi is surfing on the surfboard. */
        bool m_blupiSurf;

        /** True when a wind effect is currently pushing Blupi. */
        bool m_blupiVent;

        /** True when Blupi is hanging/suspended from a rope or bar. */
        bool m_blupiSuspend;

        /** True during the hurt-jump (JumpAie) animation phase. */
        bool m_blupiJumpAie;

        /** True when the Shield secret power is active (temporary invincibility). */
        bool m_blupiShield;

        /** True when the Power secret power is active (enhanced strength). */
        bool m_blupiPower;

        /** True when the Cloud secret power is active (float/levitation). */
        bool m_blupiCloud;

        /** True when the Hide secret power is active (invisibility). */
        bool m_blupiHide;

#ifdef MODERN
        /** True when the Ghost cheat mode is active (semi-transparent, free flight, no interactions). */
        bool m_blupiGhost = false;

        /** Cheat zoom multiplier applied to hotspot zoom: 1.0 = normal, 0.75 = 25% out, 0.5 = 50% out. */
        double m_cheatZoomFactor = 1.0;
#endif

        /** True when Blupi's controls are inverted (left/right swapped). */
        bool m_blupiInvert;

        /** True when Blupi is in balloon mode (slow fall). */
        bool m_blupiBalloon;

        /** True when Blupi is in the crushed/ecrase state. */
        bool m_blupiEcrase;

        /** True when the vehicle motor sound is playing at high pitch. */
        bool m_blupiMotorHigh;

        /** SoundChannel playing the looping vehicle motor sound, if any. */
        SoundChannel m_blupiMotorSound;

        /** Last known helicopter position in game-space (used for sound positioning). */
        TinyPoint m_blupiPosHelico;

        /** Position of Blupi's magic effect visual in game-space. */
        TinyPoint m_blupiPosMagic;

        /** True if Blupi should be respawned at m_blupiStartPos at the next update. */
        bool m_blupiRestart;

        /** True if Blupi is in front of (rendered above) moving objects. */
        bool m_blupiFront;

        /** Number of active bullets (tank shots) currently in the level. */
        intcs m_blupiBullet;

        /** Number of keys Blupi currently carries (used to open locked doors). */
        intcs m_blupiCle;

        /** Index of the NPC/persona currently following Blupi, or -1 if none. */
        intcs m_blupiPerso;

        /** Number of dynamite sticks Blupi currently carries. */
        intcs m_blupiDynamite;

        /** Remaining frames Blupi cannot use rope/bar (NoBarre cooldown timer). */
        intcs m_blupiNoBarre;

        /** Remaining frames of Shield bonus invincibility. Do not reset casually. */
        intcs m_blupiTimeShield;

        /** Remaining frames of Fire power (tank fire cooldown). Do not reset casually. */
        intcs m_blupiTimeFire;

        /** Remaining frames during which lifts (ascenseurs) cannot pick up Blupi. */
        intcs m_blupiTimeNoAsc;

        /** Remaining frames of the enemy-mockery reaction animation. */
        intcs m_blupiTimeMockery;

        /** Remaining frames of the Ouf (relief) animation. Do not reset casually. */
        intcs m_blupiTimeOuf;

        /** The action Blupi was performing before the Ouf animation started. */
        BlupiAction m_blupiActionOuf;

        /** Number of valid entries in m_blupiFifoPos. */
        intcs m_blupiFifoNb;

        /** Circular FIFO buffer of recent Blupi positions used for rope/suspension animation. */
        TinyPoint m_blupiFifoPos[10];

        /** Level start/respawn position in game-space pixels. */
        TinyPoint m_blupiStartPos;

        /** Level start/respawn facing direction. */
        Direction m_blupiStartDir;

        /** Player-controlled horizontal speed (from directional input), in game-space pixels per frame. */
        double m_blupiSpeedX;

        /** Player-controlled vertical speed (from directional input), in game-space pixels per frame. */
        double m_blupiSpeedY;

        /** Horizontal speed from the previous frame (for smooth deceleration). */
        double m_blupiLastSpeedX;

        /** Vertical speed from the previous frame (for smooth deceleration). */
        double m_blupiLastSpeedY;

        /** HUD gauges: [0] = lives/energy gauge, [1] = charge/special gauge. */
        Jauge m_jauges[2];

        /** Current Blupi upgrade/experience level (affects some gameplay capabilities). */
        intcs m_blupiLevel;

        /** True once Blupi has picked up the key for the current level. */
        bool m_bFoundCle;

        /** True when the current session is a private (user-created) level, not an official mission. */
        bool m_bPrivate;

        /** True when the cheat that opens all doors is active. */
        bool m_bCheatDoors;

        /** True when the Super Blupi cheat is active (enhanced capabilities). */
        bool m_bSuperBlupi;

        /** True when the secret-path visualization cheat is active. */
        bool m_bDrawSecret;

        /** True when building the official mission map (suppresses some gameplay features). */
        bool m_buildOfficialMissions;

        /** Index of the current mission/level being played. */
        intcs m_mission;

        static constexpr intcs m_doorsLength = 200;
        /**
         * @brief Door state array for the current level.
         *
         * Each entry represents one door or switch in the level. Positive values
         * indicate an open door; zero or negative indicate closed. The interpretation
         * of specific values depends on the door type.
         *
         * @note This is level state, not save-game state. Synced with GameData on
         *       level load and save via InitializeDoors() and MemorizeDoors().
         */
        intcs m_doors[m_doorsLength]{};

        /** Number of lives Blupi currently has. Decremented on death; level ends if zero. */
        intcs m_nbVies;

        /** Number of treasures Blupi has collected in the current level. */
        intcs m_nbTresor;

        /** Total number of treasures in the current level. */
        intcs m_totalTresor;

        /** Animation phase counter for the level-goal (win-condition) sequence. */
        intcs m_goalPhase;

        /** Icon index of the last detected interactive tile during collision scanning. */
        intcs m_detectIcon;

        /** Position of the lollipop (sucette) power-up in game-space, if active. */
        TinyPoint m_sucettePos;

        /** Object type of the lollipop power-up, identifying its effect. */
        ObjectType m_sucetteType;

        /** Logical rotation angle of Blupi (used by balloon/vehicle modes), in degrees. */
        intcs m_blupiLogicRotation;

        /** Rendered rotation angle of Blupi (may lag behind logical for smooth animation). */
        intcs m_blupiRealRotation;

        /** Vertical pixel offset applied to Blupi's drawn position (vehicle-mode height adjust). */
        intcs m_blupiOffsetY;

        /** Current scroll target point — the tile or position the viewport is centering on. */
        TinyPoint m_scrollPoint;

        /** Per-frame scroll delta being applied to smooth the viewport movement. */
        TinyPoint m_scrollAdd;

        /** Icon index for the active Voyage (item-collection flight) animation. */
        intcs m_voyageIcon;

        /** Sprite sheet for the Voyage animation. */
        PixmapChannel m_voyageChannel;

        /** Current phase step of the Voyage animation. */
        intcs m_voyagePhase;

        /** Total number of phase steps in the current Voyage animation. */
        intcs m_voyageTotal;

        /** Start position of the Voyage animation arc in game-space. */
        TinyPoint m_voyageStart;

        /** End position of the Voyage animation arc in game-space. */
        TinyPoint m_voyageEnd;

        /** Current decor (level background) action state index (for animated tiles). */
        DecorAction m_decorAction;

        /** Animation phase counter for the current decor action. */
        intcs m_decorPhase;

        /** Cache of the last tile icon drawn at each of the 200 decor positions. Used to skip redundant redraws. */
        intcs m_lastDecorIcon[200]{};

        /** Target zoom level for the camera hot-spot zoom animation. */
        double m_hotSpotFinalZoom;
        /** Target X center for the camera hot-spot zoom animation. */
        double m_hotSpotFinalX;
        /** Target Y center for the camera hot-spot zoom animation. */
        double m_hotSpotFinalY;

        /** Current zoom level of the camera hot-spot (approaches Final each frame). */
        double m_hotSpotCurrentZoom;
        /** Current X center of the camera hot-spot. */
        double m_hotSpotCurrentX;
        /** Current Y center of the camera hot-spot. */
        double m_hotSpotCurrentY;

        /** Per-frame increment applied to m_hotSpotCurrentZoom. */
        double m_hotSpotStepZoom;
        /** Per-frame increment applied to m_hotSpotCurrentX. */
        double m_hotSpotStepX;
        /** Per-frame increment applied to m_hotSpotCurrentY. */
        double m_hotSpotStepY;

        /** Lag factor controlling how quickly the hot-spot zooms out. */
        double m_hotSpotOutLag;

        /** Deterministic random number generator used for gameplay effects. */
        std::unique_ptr<System::Random> m_random;

        /** Active particle fragments from destroyed objects (helicopter debris, etc.). */
        std::vector<ByeByeObject> byeByeObjects;

    public:
        [[nodiscard]] TinyRect getDrawBoundsProperty() const;
        void setDrawBoundsProperty(const TinyRect v);
        DDATA(Def::ButtonGlyph, ButtonPressed)

    private:
        static void MoveObjectCopy(MoveObject& dst, const MoveObject& src);

    public:
        Decor();

    public:
        /**
         * @brief Binds the Decor subsystem to the audio, rendering, and data objects.
         *
         * Must be called before any other method. Does not start simulation.
         *
         * @param sound   Audio subsystem (not owned).
         * @param pixmap  Rendering subsystem (not owned).
         * @param gameData Persistent game/save data (not owned).
         */
        void Create(ISound* sound, IPixmap* pixmap, GameData* gameData);

    public:
        /**
         * @brief Loads all background images for the current level region.
         *
         * Called when a new level or world is started. Selects and caches the
         * background texture corresponding to m_region via IPixmap::BackgroundCache().
         *
         * @return True on success.
         */
        bool LoadImages();

    private:
        void InitDecor();

    public:
        /**
         * @brief Prepares the simulation for gameplay start or the level-build editor.
         *
         * Initializes Blupi's starting position, direction, and all gameplay state.
         * If @p bTest is true, enters the level-build test mode with reduced restrictions.
         *
         * @param bTest True for test/editor mode; false for normal gameplay.
         */
        void PlayPrepare(bool bTest);

    private:
        void BuildPrepare();

    public:
        /**
         * @brief Checks whether the current level has ended.
         *
         * @return 0 if the level is still running, positive if won, negative if lost.
         */
        int IsTerminated();

    public:
        /**
         * @brief Advances the entire gameplay simulation by one frame.
         *
         * This is the main update method. It advances Blupi's physics and action
         * state machine, processes all moving objects, updates the scroll position,
         * drives animated tiles, and checks win/loss conditions.
         *
         * Normally called once per active gameplay update frame. Pause handling is controlled
         * by the caller and by the internal gameplay phase/state.
         *
         * @note This method preserves original frame-based timing. Many internal
         *       constants assume a base rate of 20 FPS; apply Config::ScaleTime()
         *       before any new timer values introduced by porting work.
         */
        void MoveStep();

    private:
        void ResetHotSpot();

    private:
        void MoveHotSpot();

    private:
        bool BlitzActif(intcs celx, intcs cely);

    public:
        /**
         * @brief Renders the entire visible level for the current frame.
         *
         * Draws the tile background, all moving objects, Blupi, HUD elements,
         * particle effects, and the Voyage animation. Order is determined by the
         * original game's layering rules.
         *
         * Must be called after MoveStep() each frame, within a valid Pixmap frame
         * (between IPixmap::Start() and IPixmap::Finish()).
         */
        void Build();

    private:
        void DrawInfo();

    private:
        bool IsDisplayInfo(int tableTresor);

    private:
        TinyPoint DecorNextAction();

    public:
        /**
         * @brief Sets the player-controlled horizontal movement speed.
         *
         * Called each frame by InputPad based on directional pad position.
         * Stores the speed into m_blupiSpeedX. The sign indicates direction
         * (negative = left, positive = right).
         *
         * @param speed Horizontal speed in game-space pixels per frame at 20 FPS.
         */
        void SetSpeedX(double speed);

    public:
        /**
         * @brief Sets the player-controlled vertical movement speed.
         *
         * Called each frame by InputPad based on directional pad or accelerometer.
         * Stores the speed into m_blupiSpeedY. Negative = up, positive = down.
         *
         * @param speed Vertical speed in game-space pixels per frame at 20 FPS.
         */
        void SetSpeedY(double speed);

    public:
        /**
         * @brief Updates the current key/button press state for gameplay logic.
         *
         * Called by InputPad each frame with the current bitfield of active
         * KeyPressFlags (Jump, Fire, Down). The gameplay state machine in MoveStep()
         * reads m_keyPress and m_lastKeyPress to detect new presses vs. held buttons.
         *
         * @param keyPress Current key press bitmask (combination of KeyPressFlags values).
         */
        void KeyChange(int keyPress);

    private:
        void GetBlupiInfo(bool& bHelico, bool& bJeep, bool& bSkate, bool& bNage);

    private:
        SoundChannel SoundEnviron(SoundChannel sound, int obstacle);

    private:
        void PlaySound(SoundChannel sound, TinyPoint pos);

    public:
        /** Stops all looping gameplay sounds (e.g., vehicle motor). Called on pause or level end. */
        void StopSound();

    public:
        /** Restarts looping gameplay sounds after unpausing. */
        void StartSound();

    private:
        void StopSound(SoundChannel sound);

    private:
        void AdaptMotorVehicleSound();

    private:
        void PosSound(TinyPoint pos);

    private:
        int GetRegion();

    private:
        void SetRegion(int region);

    private:
        int GetMusic();

    private:
        void SetMusic(int music);

    public:
        TinyPoint GetDim();

    public:
        void SetDim(TinyPoint dim);

    public:
        int GetMission();

    public:
        void SetMission(int mission);

    public:
        int GetNbVies();

    public:
        void SetNbVies(int nbVies);

    public:
        /**
         * @brief Loads the door state for the current mission from persistent game data.
         *
         * Called when a level is started or continued. Copies door open/closed state
         * from @p gameData into m_doors[].
         *
         * @param gameData Source of persistent door state for the selected gamer.
         */
        void InitializeDoors(GameData& gameData);

    public:
        /**
         * @brief Saves the current door state back to persistent game data.
         *
         * Called when the player wins or the game is saved. Copies m_doors[]
         * back to @p gameData so progress is persisted.
         *
         * @param gameData Destination for door state for the selected gamer.
         */
        void MemorizeDoors(GameData& gameData);

    public:
        /**
         * @brief Returns the short label text for a cheat button glyph.
         *
         * Used by the rendering code to draw cheat-menu button labels.
         *
         * @param glyph The cheat button whose label is requested.
         * @return Short text label (e.g., "1A", "2B").
         */
        static std::string GetCheatTinyText(Def::ButtonGlyph glyph);

    public:
        /**
         * @brief Activates a cheat code effect in the current level.
         *
         * Cheats directly modify gameplay state (doors, Blupi mode, object state, etc.).
         * Each cheat is identified by a Tables::CheatCodes value.
         *
         * @param cheat The cheat to activate.
         */
        void CheatAction(Tables::CheatCodes cheat);

    private:
        void SetBuildOfficialMissions(bool bMode);

    private:
        void BlupiSearchIcon();

    private:
        bool BlupiIsGround();

    private:
        TinyRect BlupiRect(TinyPoint pos);

    private:
        void BlupiAdjust();

    private:
        bool BlupiBloque(TinyPoint pos, int dir);

    private:
        void BlupiStep();

#ifdef MODERN
    private:
        void BlupiGhostStep();
#endif

    private:
        void BlupiDead(BlupiAction action1, std::optional<BlupiAction> action2 = std::nullopt);

    private:
        TinyPoint GetPosDecor(TinyPoint pos);

    private:
        void BlupiAddFifo(TinyPoint pos);

    private:
        bool DecorDetect(TinyRect rect);

    private:
        bool DecorDetect(TinyRect rect, bool bCaisse);

    private:
        bool TestPath(const TinyRect& rect, const TinyPoint& start, TinyPoint& end);

    private:
        void MoveObjectPollution();

    private:
        void MoveObjectPlouf(TinyPoint pos);

    private:
        void MoveObjectTiplouf(TinyPoint pos);

    private:
        void MoveObjectBlup(TinyPoint pos);

    private:
        int IsWorld(TinyPoint pos);

    private:
        void ActiveSwitch(bool bState, TinyPoint cel);

    private:
        int GetTypeBarre(TinyPoint pos);

    private:
        bool IsLave(TinyPoint pos);

    private:
        bool IsPiege(TinyPoint pos);

    private:
        bool IsGoutte(TinyPoint pos, bool bAlways);

    private:
        bool IsScie(TinyPoint pos);

    private:
        bool IsSwitch(TinyPoint pos, TinyPoint& celSwitch);

    private:
        bool IsEcraseur(TinyPoint pos);

    private:
        bool IsBlitz(TinyPoint pos, bool bAlways);

    private:
        bool IsRessort(TinyPoint pos);

    private:
        bool IsTemp(TinyPoint pos);

    private:
        bool IsBridge(TinyPoint pos, TinyPoint& celBridge);

    private:
        int IsDoor(TinyPoint pos, TinyPoint& celPorte);

    private:
        int IsTeleporte(TinyPoint pos);

    private:
        bool SearchTeleporte(TinyPoint pos, TinyPoint& newpos);

    private:
        bool IsNormalJump(TinyPoint pos);

    private:
        bool IsSurfWater(TinyPoint pos);

    private:
        bool IsDeepWater(TinyPoint pos);

    private:
        bool IsOutWater(TinyPoint pos);

    private:
        bool IsPassIcon(int icon);

    private:
        bool IsBlocIcon(int icon);

    private:
        void FlushBalleTraj();

    private:
        void SetBalleTraj(TinyPoint pos);

    private:
        bool IsBalleTraj(TinyPoint pos);

    private:
        void FlushMoveTraj();

    private:
        void SetMoveTraj(TinyPoint pos);

    private:
        bool IsMoveTraj(TinyPoint pos);

    private:
        int SearchDistRight(TinyPoint pos, TinyPoint dir, ObjectType type);

    private:
        bool IsVentillo(TinyPoint pos);

    private:
        void NetStopCloud(int rank);

    private:
        void StartSploutchGlu(TinyPoint pos);

    private:
        int ObjectStart(TinyPoint pos, ObjectType type, int speed);

    private:
        bool ObjectDelete(TinyPoint pos, ObjectType type);

    private:
        void ModifDecor(TinyPoint pos, int icon);

    private:
        void MoveObjectStep();

    private:
        void MoveObjectStepLine(int i);

    private:
        void MoveObjectStepIcon(int i);

    private:
        void DynamiteStart(int i, int dx, int dy);

    private:
        int AscenseurDetect(TinyRect rect, TinyPoint oldpos, TinyPoint newpos);

    private:
        void AscenseurVertigo(int i, bool& bVertigoLeft, bool& bVertigoRight);

    private:
        bool AscenseurShift(int i);

    private:
        void AscenseurSynchro(int i);

    private:
        void UpdateCaisse();

    private:
        bool TestPushCaisse(int i, TinyPoint pos, bool bPop);

    private:
        bool TestPushOneCaisse(int i, TinyPoint move, int b);

    private:
        void SearchLinkCaisse(int rank, bool bPop);

    private:
        bool AddLinkCaisse(int rank);

    private:
        int CaisseInFront();

    private:
        int CaisseGetMove(int max);

    private:
        int MockeryDetect(TinyPoint pos);

    private:
        bool BlupiElectro(TinyPoint pos);

    private:
        void MoveObjectFollow(TinyPoint pos);

    private:
        int MoveObjectDetect(TinyPoint pos, bool& bNear);

    private:
        int MoveAscenseurDetect(TinyPoint pos, int height);

    private:
        int MoveChargeDetect(TinyPoint pos);

    private:
        int MovePersoDetect(TinyPoint pos);

    private:
        int MoveObjectDelete(TinyPoint cel);

    private:
        int MoveObjectFree();

    private:
        int SortGetType(ObjectType type);

    private:
        void MoveObjectSort();

    private:
        void MoveObjectPriority(int i);

    private:
        int MoveObjectSearch(TinyPoint pos);

    private:
        int MoveObjectSearch(TinyPoint pos, std::optional<ObjectType> type);

    private:
        void ByeByeHelico();

    private:
        void ByeByeAdd(PixmapChannel channel, int icon, TinyPoint pos, double rotationSpeed, double animationSpeed);

    private:
        void ByeByeStep();

    private:
        void ByeByeDraw(TinyPoint posDecor);

    private:
        TinyPoint VoyageGetPosVie(int nbVies);

    private:
        void VoyageInit(TinyPoint start, TinyPoint end, int icon, PixmapChannel channel);

    private:
        void VoyageStep();

    private:
        void VoyageDraw();

    private:
        bool IsFloatingObject(int i);

    private:
        bool IsRightBorder(int x, int y, int dx, int dy);

    private:
        bool IsFromage(int x, int y);

    private:
        bool IsGrotte(int x, int y);

    private:
        void AdaptMidBorder(int x, int y);

    private:
        void AdaptBorder(TinyPoint cel);

    public:
        /** Deletes the current-game (quick-save) file from persistent storage. */
        void CurrentDelete();

    public:
        /**
         * @brief Writes a quick-save snapshot of the current level state.
         *
         * Serialises all Blupi and level state to the current-game save file so the
         * player can continue from the exact same position after the app is closed.
         *
         * @return True on success.
         */
        bool CurrentWrite();

    public:
        /**
         * @brief Reads and restores a quick-save snapshot.
         *
         * Deserialises the current-game save file and applies all state to the
         * simulation. Call PlayPrepare() after this if needed to complete setup.
         *
         * @return True on success.
         */
        bool CurrentRead();

    public:
        /**
         * @brief Loads a level file from the official or user mission set.
         *
         * @param gamer Gamer slot index.
         * @param rank  Mission/level rank.
         * @param bUser True for user-created levels; false for official missions.
         * @return True if the level was loaded successfully.
         */
        bool Read(int gamer, int rank, bool bUser);

    private:
        bool Delete(int gamer, int rank, bool bUser);

    private:
        bool FileExist(int gamer, int rank, bool bUser);

    private:
        bool SearchWorld(int world, TinyPoint& blupi, Direction& dir);

    private:
        bool SearchDoor(int n, TinyPoint& cel, TinyPoint& blupi);

    private:
        bool SearchGold(int n, TinyPoint& cel);

    public:
        /**
         * @brief Initialises the main-world switch state for the world-selection level.
         *
         * Called when loading the main-world (hub) level. Sets door/switch states based
         * on how far the player has progressed (@p lastWorld).
         *
         * @param lastWorld Index of the last world the player completed.
         */
        void MainSwitchInitialize(int lastWorld);

    public:
        /**
         * @brief Opens or closes doors in the current level based on the gamer's progress.
         *
         * Adjusts m_doors[] to reflect which doors should be accessible given the
         * current save-game state. Pass true for private (user-created) levels.
         *
         * @param bPrivate True for user-created levels where all doors are always open.
         */
        void AdaptDoors(bool bPrivate);

        bool IsGhost();

#ifdef MODERN
        /**
         * @brief Sets the cheat zoom multiplier applied to the hotspot zoom.
         *
         * @param factor 1.0 = normal, 0.75 = 25% zoomed out, 0.5 = 50% zoomed out.
         */
        void SetCheatZoom(double factor);

        /** Returns the current game time counter (frame ticks since level start). */
        int GetTime() const { return m_time; }

        /** Returns the current Blupi position in game-space pixel coordinates. */
        TinyPoint GetBlupiPos() const { return m_blupiPos; }

        /** Returns Blupi's current horizontal velocity (sub-pixels per frame). */
        double GetBlupiVX() const { return m_blupiVitesseX; }

        /** Returns Blupi's current vertical velocity (sub-pixels per frame). */
        double GetBlupiVY() const { return m_blupiVitesseY; }

        /** Returns true if Blupi is currently airborne. */
        bool GetBlupiAir() const { return m_blupiAir; }

        /** Returns true if Blupi is on the helicopter. */
        bool GetBlupiHelico() const { return m_blupiHelico; }

        /** Returns true if Blupi is on skates. */
        bool GetBlupiSkate() const { return m_blupiSkate; }

        /** Returns true if Blupi is swimming. */
        bool GetBlupiNage() const { return m_blupiNage; }

        /** Returns the current region index (MODERN debug helper). */
        int GetRegionDebug() { return GetRegion(); }
#endif

    private:
        void OpenDoorsTresor();

    private:
        void OpenDoor(TinyPoint cel);

    private:
        void OpenDoorsWin();

    private:
        void OpenGoldsWin();

    private:
        void DoorsLost();
    };
}
