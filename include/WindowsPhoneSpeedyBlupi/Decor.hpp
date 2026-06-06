/**
 * @file   Decor.hpp
 * @brief  Declares the core gameplay simulation class Decor for Speedy Blupi.
 * @details
 *   This file is the heart of the gameplay subsystem. The Decor class owns the
 *   100×100 tile map, the Blupi player state machine, all active moving objects,
 *   door and switch state, and every gameplay mechanic: physics, collision,
 *   animation sequencing, viewport scrolling, sound triggering, and win/loss
 *   detection.
 *
 *   The class is a direct C++ port of the original XNA/Windows Phone C# Decor
 *   class. All movement constants, collision rules, animation table indices, and
 *   state-machine transitions are preserved from the original source.
 *
 *   Coordinate systems used throughout this file:
 *   - Tile coordinates: integer (x, y) in [0, MAXCELX) x [0, MAXCELY).
 *   - Game-space pixel coordinates: tile x 64 (DIMOBJX / DIMOBJY).
 *   - Screen-space: game-space plus scroll offset; handled by IPixmap, not Decor.
 *
 * @author  Original XNA/C# game by Epsitec SA; C++ port by the mobile-eggbert team
 * @date    2013 (original); 2024 (C++ port)
 * @see     IPixmap, ISound, GameData, Tables
 */
#pragma once

#include "WindowsPhoneSpeedyBlupi/Config.hpp"
#include "GameData.hpp"
#include "IPixmap.hpp"
#include "ISound.hpp"
#include "Tables.hpp"
#include "System/Random.hpp"
#include "Jauge.hpp"
#include "decor/DecorAction.hpp"
#include "decor/DoorKeyFlags.hpp"
#include "SharpRuntime/Prop.hpp"
#include "WindowsPhoneSpeedyBlupi/Helper.hpp"
#include "decor/ObjectType.hpp"
#include "def/PixmapChannel.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @class  Decor
     * @brief  Core gameplay simulation class. Owns the level state, Blupi player state,
     *         moving objects, tile map, and all gameplay logic.
     * @details
     *   Decor is the central gameplay subsystem. It corresponds directly to the original
     *   C# Decor class and preserves all original gameplay logic, movement constants,
     *   collision rules, animation tables, and state-machine transitions.
     *
     *   Responsibilities:
     *   - Owning and simulating the 100x100 tile map (m_decor, m_bigDecor).
     *   - Owning and advancing the Blupi player state machine (position, action, direction,
     *     vehicle modes, bonuses, physics).
     *   - Owning and advancing all active moving objects (m_moveObject[]).
     *   - Detecting collision between Blupi and tiles, moving objects, and hazards.
     *   - Handling doors, switches, teleporters, lifts (ascenseurs), and triggers.
     *   - Managing the viewport scroll position relative to Blupi's position.
     *   - Triggering sound effects at gameplay events via ISound.
     *   - Driving animation through per-object phase counters indexed into Tables.
     *   - Reading and writing level save files via Worlds helpers.
     *
     *   Does not own rendering resources. Drawing is done by forwarding to IPixmap.
     *   Does not own input; receives key state via KeyChange() each frame.
     *
     *   Coordinate systems used by this class:
     *   - Tile coordinates: integer (x,y) in range [0, MAXCELX) x [0, MAXCELY).
     *     One tile = 64x64 pixels in game-space (DIMOBJX / DIMOBJY).
     *   - Game-space pixel coordinates: tile * 64, used for Blupi position,
     *     moving object positions, and collision rectangles.
     *   - Screen-space: game-space + scroll offset; handled by Pixmap, not Decor.
     *
     * @note   This is gameplay code. Do not change movement or collision constants
     *         without understanding the original timing and tile-based assumptions.
     * @note   Many numeric values in this class come from the original game's C++ source.
     *         Treat all constants as part of the original game logic.
     * @note   Animation phase counters (m_blupiPhase, MoveObject::phase) are not sprite
     *         indices. They are step counters used to index into Tables animation arrays.
     * @warning Do not add new timer values without applying Config::ScaleTime() — all
     *          original timing assumes a base rate of 20 FPS.
     * @see    IPixmap, ISound, GameData, Tables, BlupiAction, ObjectType
     */
    class Decor
    {
        /**
         * @enum   IconType
         * @brief  Reserved enumeration for future icon type classification.
         * @details
         *   This enumeration is a placeholder ported from the original C# codebase.
         *   It is not yet populated with values and has no effect on current gameplay.
         * @note   Do not remove; may be populated in future porting work.
         */
        enum class IconType
        {

        };

        /**
         * @struct Cellule
         * @brief  Represents one cell of the tile map.
         * @details
         *   Each cell stores one icon index that identifies the tile sprite to display
         *   and the collision/passability properties of that tile. The icon value is
         *   looked up in the background sprite sheet (PixmapChannel::Background).
         *
         *   The 100x100 grid of Cellule entries in m_decor (and m_bigDecor) constitutes
         *   the full level layout. Tile collision type is derived entirely from the icon
         *   value via IsPassIcon() and IsBlocIcon().
         *
         * @note icon is a tile/decor identifier, not a raw texture pixel coordinate.
         * @see  Decor::m_decor, Decor::IsPassIcon(), Decor::IsBlocIcon()
         */
        struct Cellule
        {
            intcs icon; ///< Tile icon index used for rendering and collision classification.
        };

        /**
         * @struct MoveObject
         * @brief  Represents a moving object (enemy, crate, projectile, collectible, etc.)
         *         currently active in the level.
         * @details
         *   Moving objects are stored in the fixed-size pool m_moveObject[MAXMOVEOBJECT].
         *   Each object moves between posStart and posEnd along a linear path, advancing
         *   by stepAdvance pixels per frame and receding by stepRecede pixels per frame.
         *   The object pauses at each end for timeStopStart / timeStopEnd frames before
         *   reversing direction.
         *
         *   The object's visual representation is identified by (channel, icon), which are
         *   looked up in the animation tables at each frame via MoveObjectStepIcon().
         *
         * @note posCurrent is in game-space pixel coordinates. Do not interpret it as
         *       a tile coordinate.
         * @note phase is an animation step counter, not a sprite index.
         * @note icon and channel identify which sprite to render — they are data-table
         *       identifiers from the animation tables, not raw pixel offsets.
         * @see  Decor::m_moveObject, Decor::MoveObjectStep(), Decor::MoveObjectStepLine()
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
         * @class  ByeByeObject
         * @brief  Particle fragment produced when an object is destroyed (e.g., helicopter explodes).
         * @details
         *   ByeByeObject instances are short-lived visual-only effects. When an object such as
         *   a helicopter is destroyed, ByeByeHelico() creates several ByeByeObject fragments that
         *   fly off-screen via ByeByeStep(). They have no effect on gameplay collision, physics,
         *   or game state. They are stored in the byeByeObjects vector and removed when they leave
         *   the visible area or exhaust their animation.
         *
         * @note posX/posY are in game-space floating-point coordinates, not tile coordinates.
         * @note phase is a floating-point animation progress counter, not a table index.
         * @see  Decor::byeByeObjects, Decor::ByeByeHelico(), Decor::ByeByeStep(), Decor::ByeByeDraw()
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

        double m_blupiSubPixelX = 0.0; ///< @brief Sub-pixel accumulator for Blupi's horizontal position; prevents integer-truncation drift at high FPS.
        double m_blupiSubPixelY = 0.0; ///< @brief Sub-pixel accumulator for Blupi's vertical position; same purpose as m_blupiSubPixelX.

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
        DoorKeyFlags m_blupiCle;

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
        /**
         * @brief Returns the current draw-bounds rectangle used for culling.
         * @return Copy of m_drawBounds in game-space coordinates.
         */
        [[nodiscard]] TinyRect getDrawBoundsProperty() const;

        /**
         * @brief Sets the draw-bounds rectangle used for culling draw calls.
         * @param[in] v  New draw bounds in game-space coordinates.
         */
        void setDrawBoundsProperty(const TinyRect v);

        /** @brief Data property storing the last virtual button pressed by the player. */
        DDATA(Def::ButtonGlyph, ButtonPressed)

    private:
        /**
         * @brief Copies all fields from one MoveObject into another.
         * @param[out] dst  Destination MoveObject to overwrite.
         * @param[in]  src  Source MoveObject to copy from.
         */
        static void MoveObjectCopy(MoveObject& dst, const MoveObject& src);

    public:
        /**
         * @brief Constructs a Decor instance with default-initialised state.
         * @note  Call Create() before any simulation methods.
         */
        Decor();

    public:
        /**
         * @brief Binds the Decor subsystem to the audio, rendering, and data objects.
         * @details
         *   Must be called before any other method. Stores non-owning pointers to the
         *   three subsystems. Does not start simulation or load any level.
         * @param[in] sound     Audio subsystem pointer (not owned by Decor).
         * @param[in] pixmap    Rendering subsystem pointer (not owned by Decor).
         * @param[in] gameData  Persistent game/save data pointer (not owned by Decor).
         * @pre  All three pointers must remain valid for the lifetime of this Decor instance.
         * @post m_sound, m_pixmap, and m_gameData are set; simulation is not yet active.
         * @warning Calling any simulation method before Create() results in undefined behaviour.
         * @see  PlayPrepare(), Read()
         */
        void Create(ISound* sound, IPixmap* pixmap, GameData* gameData);

    public:
        /**
         * @brief Loads all background images for the current level region.
         * @details
         *   Called when a new level or world is started. Selects and caches the
         *   background texture corresponding to m_region via IPixmap::BackgroundCache().
         * @return True on success; false if the texture could not be loaded.
         * @pre  m_region must be set (via SetRegion() or Read()) before calling.
         * @post The background atlas for m_region is resident in GPU memory.
         * @see  SetRegion()
         */
        bool LoadImages();

    private:
        /**
         * @brief Resets the tile map and all moving-object state to a clean baseline.
         * @note  Called internally during level load and PlayPrepare(). Not for external use.
         */
        void InitDecor();

    public:
        /**
         * @brief Prepares the simulation for gameplay start or the level-build editor.
         * @details
         *   Initializes Blupi's starting position, direction, and all gameplay state.
         *   If @p bTest is true, enters the level-build test mode with reduced restrictions.
         * @param[in] bTest  True for test/editor mode; false for normal gameplay.
         * @pre  Read() must have been called successfully before PlayPrepare().
         * @post The simulation is ready to accept MoveStep() and Build() calls.
         * @see  Read(), MoveStep(), Build()
         */
        void PlayPrepare(bool bTest);

    private:
        /**
         * @brief Prepares internal state for the level-build editor mode.
         * @note  Called by PlayPrepare() when bTest is false and the editor is active.
         *        Sets up reduced gameplay restrictions appropriate for level editing.
         */
        void BuildPrepare();

    public:
        /**
         * @brief Checks whether the current level has ended.
         * @return 0 if the level is still running.
         * @retval 0   Level is still active; continue calling MoveStep().
         * @retval >0  Level won; transition to the win screen.
         * @retval <0  Level lost; transition to the death/game-over screen.
         * @see  MoveStep()
         */
        int IsTerminated();

    public:
        /**
         * @brief Advances the entire gameplay simulation by one frame.
         * @details
         *   This is the main update method. It advances Blupi's physics and action
         *   state machine, processes all moving objects, updates the scroll position,
         *   drives animated tiles, and checks win/loss conditions.
         *
         *   Normally called once per active gameplay update frame. Pause handling is
         *   controlled by the caller and by the internal gameplay phase/state.
         * @pre  PlayPrepare() must have been called. KeyChange(), SetSpeedX(), SetSpeedY()
         *       must be updated before each call.
         * @post m_blupiPos, m_posDecor, m_moveObject[], m_time, and m_term are updated.
         * @note This method preserves original frame-based timing. Many internal
         *       constants assume a base rate of 20 FPS; apply Config::ScaleTime()
         *       before introducing any new timer values.
         * @see  Build(), KeyChange(), SetSpeedX(), SetSpeedY(), IsTerminated()
         */
        void MoveStep();

    private:
        /**
         * @brief Resets the camera hot-spot zoom to the default (no zoom, centered on Blupi).
         * @post  m_hotSpotCurrentZoom, m_hotSpotCurrentX, m_hotSpotCurrentY are set to defaults.
         */
        void ResetHotSpot();

    private:
        /**
         * @brief Advances the camera hot-spot zoom animation by one frame.
         * @details
         *   Interpolates m_hotSpotCurrentZoom/X/Y toward m_hotSpotFinalZoom/X/Y
         *   using the per-frame step values. Called from MoveStep() each frame.
         */
        void MoveHotSpot();

    private:
        /**
         * @brief Tests whether a blitz (lightning hazard) tile is active at the given tile cell.
         * @param[in] celx  Tile x-coordinate.
         * @param[in] cely  Tile y-coordinate.
         * @return True if the cell contains an active blitz hazard.
         */
        bool BlitzActif(intcs celx, intcs cely);

    public:
        /**
         * @brief Renders the entire visible level for the current frame.
         * @details
         *   Draws the tile background, all moving objects, Blupi, HUD elements,
         *   particle effects, and the Voyage animation. Draw order follows the
         *   original game's layering rules (background → objects → Blupi → HUD).
         * @pre  Must be called after MoveStep() each frame.
         * @pre  Must be called within a valid IPixmap frame (between Start() and Finish()).
         * @note Does not call IPixmap::Start() or IPixmap::Finish() itself.
         * @see  MoveStep(), IPixmap
         */
        void Build();

    private:
        /**
         * @brief Draws the HUD information overlay (lives, keys, treasures).
         * @note  Called from Build() each frame. Renders gauge widgets and icon overlays.
         */
        void DrawInfo();

    private:
        /**
         * @brief Tests whether treasure info should be displayed for the given table entry.
         * @param[in] tableTresor  Index into the treasure table to test.
         * @return True if the info overlay for this treasure entry is currently shown.
         */
        bool IsDisplayInfo(int tableTresor);

    private:
        /**
         * @brief Returns the next tile position to apply the current decor action animation.
         * @details
         *   Iterates the active DecorAction state to find the next tile cell that needs
         *   to have a shake or electric animation frame applied.
         * @return Tile coordinates of the next cell to animate, or an invalid point when done.
         */
        TinyPoint DecorNextAction();

    public:
        /**
         * @brief Sets the player-controlled horizontal movement speed.
         *
         * Called each frame by InputPad based on directional pad position.
         * Stores the speed into m_blupiSpeedX. The sign indicates direction
         * (negative = left, positive = right).
         *
         * @param[in] speed  Horizontal speed in game-space pixels per frame at 20 FPS.
         */
        void SetSpeedX(double speed);

    public:
        /**
         * @brief Sets the player-controlled vertical movement speed.
         *
         * Called each frame by InputPad based on directional pad or accelerometer.
         * Stores the speed into m_blupiSpeedY. Negative = up, positive = down.
         *
         * @param[in] speed  Vertical speed in game-space pixels per frame at 20 FPS.
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
         * @param[in] keyPress  Current key press bitmask (combination of KeyPressFlags values).
         */
        void KeyChange(int keyPress);

    private:
        /**
         * @brief Retrieves Blupi's current vehicle/mode flags in one call.
         * @param[out] bHelico  Set to true if Blupi is in helicopter mode.
         * @param[out] bJeep    Set to true if Blupi is in jeep mode.
         * @param[out] bSkate   Set to true if Blupi is on skates.
         * @param[out] bNage    Set to true if Blupi is swimming.
         */
        void GetBlupiInfo(bool& bHelico, bool& bJeep, bool& bSkate, bool& bNage);

    private:
        /**
         * @brief Selects the appropriate sound for the current environment / surface.
         * @details
         *   Maps a generic sound channel to a variant appropriate for the tile type
         *   (e.g., footstep sounds differ on grass, stone, water).
         * @param[in] sound     Base sound channel to potentially override.
         * @param[in] obstacle  Tile icon of the surface Blupi is on.
         * @return  The most appropriate SoundChannel for the current environment.
         */
        SoundChannel SoundEnviron(SoundChannel sound, int obstacle);

    private:
        /**
         * @brief Triggers a one-shot sound effect at a given game-space position.
         * @details
         *   Calls ISound with stereo panning derived from @p pos relative to the
         *   current viewport center.
         * @param[in] sound  Sound channel to play.
         * @param[in] pos    Game-space position of the sound source (for panning).
         */
        void PlaySound(SoundChannel sound, TinyPoint pos);

    public:
        /**
         * @brief Stops all looping gameplay sounds (e.g., vehicle motor).
         * @details
         *   Called on pause or level end to silence any sounds that loop for the
         *   duration of gameplay. Individual one-shot effects are not affected.
         * @post All looping sound channels started by Decor are silenced.
         * @see  StartSound()
         */
        void StopSound();

    public:
        /**
         * @brief Restarts looping gameplay sounds after unpausing.
         * @details
         *   Re-enables the vehicle motor loop and any other looping sounds that were
         *   silenced by StopSound(). Should be called when gameplay resumes.
         * @pre  StopSound() must have been called before StartSound() is meaningful.
         * @see  StopSound()
         */
        void StartSound();

    private:
        /**
         * @brief Stops a specific looping sound channel.
         * @param[in] sound  The looping sound channel to stop.
         */
        void StopSound(SoundChannel sound);

    private:
        /**
         * @brief Adjusts vehicle motor sound pitch based on Blupi's current speed.
         * @details
         *   Switches between low and high motor sound variants when Blupi's speed
         *   crosses the threshold. Updates m_blupiMotorHigh accordingly.
         */
        void AdaptMotorVehicleSound();

    private:
        /**
         * @brief Updates the stereo panning of all active looping sounds based on position.
         * @param[in] pos  Current game-space position used as the sound source.
         */
        void PosSound(TinyPoint pos);

    private:
        /**
         * @brief Returns the current world region index.
         * @return Region index stored in m_region.
         */
        int GetRegion();

    private:
        /**
         * @brief Sets the current world region index.
         * @param[in] region  New region index; controls background texture and music selection.
         */
        void SetRegion(int region);

    private:
        /**
         * @brief Returns the current background music track index.
         * @return Music track index stored in m_music.
         */
        int GetMusic();

    private:
        /**
         * @brief Sets the background music track index and triggers a music change.
         * @param[in] music  New music track index.
         */
        void SetMusic(int music);

    public:
        /**
         * @brief Returns the visible area dimensions of the viewport in game-space pixels.
         * @return Dimensions stored in m_dimDecor.
         */
        TinyPoint GetDim();

    public:
        /**
         * @brief Sets the visible area dimensions of the viewport in game-space pixels.
         * @param[in] dim  New viewport dimensions.
         */
        void SetDim(TinyPoint dim);

    public:
        /**
         * @brief Returns the index of the current mission/level.
         * @return Mission index stored in m_mission.
         */
        int GetMission();

    public:
        /**
         * @brief Sets the current mission/level index.
         * @param[in] mission  Level index to load or begin.
         */
        void SetMission(int mission);

    public:
        /**
         * @brief Returns the number of lives Blupi currently has.
         * @return Lives count stored in m_nbVies.
         */
        int GetNbVies();

    public:
        /**
         * @brief Sets the number of lives Blupi currently has.
         * @param[in] nbVies  New life count; must be non-negative.
         */
        void SetNbVies(int nbVies);

    public:
        /**
         * @brief Loads the door state for the current mission from persistent game data.
         * @details
         *   Called when a level is started or continued. Copies door open/closed state
         *   from @p gameData into m_doors[].
         * @param[in] gameData  Source of persistent door state for the selected gamer.
         * @pre  m_mission must be set to the correct level index before calling.
         * @post m_doors[] is populated with the door state for the current mission.
         * @see  MemorizeDoors()
         */
        void InitializeDoors(GameData& gameData);

    public:
        /**
         * @brief Saves the current door state back to persistent game data.
         * @details
         *   Called when the player wins or the game is saved. Copies m_doors[]
         *   back to @p gameData so progress is persisted.
         * @param[in,out] gameData  Destination for door state for the selected gamer;
         *                          the door fields in gameData are overwritten.
         * @post The door state in gameData reflects m_doors[] at the time of the call.
         * @see  InitializeDoors()
         */
        void MemorizeDoors(GameData& gameData);

    public:
        /**
         * @brief Returns the short label text for a cheat button glyph.
         * @details
         *   Used by the rendering code to draw cheat-menu button labels over virtual
         *   gamepad buttons. Each label is typically two characters (e.g., "1A", "2B").
         * @param[in] glyph  The cheat button whose label is requested.
         * @return Short text label string for the given glyph.
         * @see  Def::ButtonGlyph
         */
        static std::string GetCheatTinyText(Def::ButtonGlyph glyph);

    public:
        /**
         * @brief Activates a cheat code effect in the current level.
         *
         * Cheats directly modify gameplay state (doors, Blupi mode, object state, etc.).
         * Each cheat is identified by a Tables::CheatCodes value.
         *
         * @param[in] cheat  The cheat code to activate; selects the specific effect to apply.
         * @see   Tables::CheatCodes
         */
        void CheatAction(Tables::CheatCodes cheat);

    private:
        /**
         * @brief Enables or disables the official-mission-build mode.
         * @param[in] bMode  True to enable official-mission building; false to disable.
         * @note  When enabled, certain gameplay features are suppressed for the editor.
         */
        void SetBuildOfficialMissions(bool bMode);

    private:
        /**
         * @brief Resolves Blupi's current sprite icon from the animation table.
         * @details
         *   Reads m_blupiAction, m_blupiPhase, and m_blupiDir to look up the correct
         *   icon index in Tables::table_blupi and stores it in m_blupiIcon.
         * @post m_blupiIcon reflects the correct sprite for the current frame.
         */
        void BlupiSearchIcon();

    private:
        /**
         * @brief Tests whether Blupi is currently standing on a solid surface.
         * @return True if Blupi's feet are resting on a passable tile below.
         */
        bool BlupiIsGround();

    private:
        /**
         * @brief Computes Blupi's collision bounding rectangle for a given position.
         * @param[in] pos  Game-space position to compute the bounding box around.
         * @return Rectangle in game-space enclosing Blupi at @p pos.
         * @note  The rectangle accounts for BLUPIOFFY and vehicle-mode size overrides.
         */
        TinyRect BlupiRect(TinyPoint pos);

    private:
        /**
         * @brief Adjusts Blupi's position to resolve minor tile-edge penetration.
         * @details
         *   Called after movement to push Blupi out of any tile he has slightly
         *   overlapped. Ensures Blupi does not clip into solid geometry.
         * @post m_blupiPos is adjusted so that BlupiRect() does not overlap any blocking tile.
         */
        void BlupiAdjust();

    private:
        /**
         * @brief Tests whether Blupi is blocked (cannot move) in a given direction.
         * @param[in] pos  Proposed new position in game-space.
         * @param[in] dir  Direction of movement (e.g., left/right/up/down constant).
         * @return True if movement to @p pos in @p dir is blocked by a solid tile or object.
         */
        bool BlupiBloque(TinyPoint pos, int dir);

    private:
        /**
         * @brief Advances Blupi's position and action state machine by one frame.
         * @details
         *   Applies physics, processes key input, handles transitions between actions
         *   (walking, jumping, vehicle modes), and invokes hazard checks. This is the
         *   core of Blupi's per-frame simulation.
         * @pre  m_keyPress and m_blupiSpeedX/Y must be up to date before the call.
         * @post m_blupiPos, m_blupiAction, m_blupiPhase, and related state are updated.
         */
        void BlupiStep();

#ifdef MODERN
    private:
        /**
         * @brief Advances Blupi in ghost/cheat free-flight mode by one frame.
         * @details
         *   In ghost mode Blupi moves freely in all directions without collision or
         *   hazard checks. Available only in MODERN build configuration.
         * @note  MODERN-only. Not compiled in LEGACY builds.
         */
        void BlupiGhostStep();
#endif

    private:
        /**
         * @brief Triggers Blupi's death animation and transitions to a dead state.
         * @details
         *   Sets Blupi's action to @p action1 (and optionally @p action2 for a two-phase
         *   death sequence). Decrements lives and schedules a respawn.
         * @param[in] action1  Primary death animation to play (e.g., BlupiAction::Dead).
         * @param[in] action2  Optional secondary death animation played after action1 completes.
         */
        void BlupiDead(BlupiAction action1, std::optional<BlupiAction> action2 = std::nullopt);

    private:
        /**
         * @brief Converts a game-space pixel position to a scroll-adjusted screen position.
         * @param[in] pos  Game-space pixel position.
         * @return Position relative to the current scroll origin (m_posDecor).
         */
        TinyPoint GetPosDecor(TinyPoint pos);

    private:
        /**
         * @brief Appends a position entry to the rope/suspension FIFO trail buffer.
         * @param[in] pos  Game-space position to record for rope animation.
         * @note  The buffer has a fixed capacity of 10 entries; oldest entries are overwritten.
         */
        void BlupiAddFifo(TinyPoint pos);

    private:
        /**
         * @brief Tests whether any blocking tile overlaps the given rectangle.
         * @param[in] rect  Rectangle in game-space to test against tile collision.
         * @return True if any blocking (non-passable) tile overlaps @p rect.
         */
        bool DecorDetect(TinyRect rect);

    private:
        /**
         * @brief Tests whether any blocking tile (optionally including crates) overlaps a rectangle.
         * @param[in] rect    Rectangle in game-space to test.
         * @param[in] bCaisse True to also treat crate (caisse) tiles as blocking.
         * @return True if a blocking tile overlaps @p rect.
         */
        bool DecorDetect(TinyRect rect, bool bCaisse);

    private:
        /**
         * @brief Tests whether a movement path is clear of blocking tiles.
         * @param[in]  rect   Bounding rectangle of the moving entity.
         * @param[in]  start  Start position of the path in game-space.
         * @param[out] end    Furthest reachable position along the path (set on return).
         * @return True if the full path from @p start to original @p end is clear;
         *         false if an obstacle was found, with @p end set to the last clear position.
         */
        bool TestPath(const TinyRect& rect, const TinyPoint& start, TinyPoint& end);

    private:
        /**
         * @brief Applies pollution damage to moving objects that overlap pollution tiles.
         * @details
         *   Scans m_moveObject[] for objects positioned on pollution tiles and
         *   triggers destruction or state transitions as required.
         */
        void MoveObjectPollution();

    private:
        /**
         * @brief Spawns a water-entry splash (plouf) effect at the given position.
         * @param[in] pos  Game-space position where the splash appears.
         */
        void MoveObjectPlouf(TinyPoint pos);

    private:
        /**
         * @brief Spawns a small water-tip (tiplouf) splash effect at the given position.
         * @param[in] pos  Game-space position of the splash.
         */
        void MoveObjectTiplouf(TinyPoint pos);

    private:
        /**
         * @brief Spawns a bubble (blup) particle effect at the given position.
         * @param[in] pos  Game-space position where the bubble appears.
         */
        void MoveObjectBlup(TinyPoint pos);

    private:
        /**
         * @brief Returns the world/region index for the tile cell at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return World index (>= 0) if the tile belongs to a world region; -1 if none.
         */
        int IsWorld(TinyPoint pos);

    private:
        /**
         * @brief Activates or deactivates a switch tile and all tiles linked to it.
         * @param[in] bState  True to activate (open); false to deactivate (close).
         * @param[in] cel     Tile coordinates of the switch to toggle.
         */
        void ActiveSwitch(bool bState, TinyPoint cel);

    private:
        /**
         * @brief Returns the bar (rope/rail) type at the given game-space position.
         * @param[in] pos  Game-space pixel position to query.
         * @return Bar type identifier (0 = none; positive = specific bar/rail type).
         */
        int GetTypeBarre(TinyPoint pos);

    private:
        /**
         * @brief Tests whether Blupi is touching lava (lave) at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if a lava tile overlaps the position.
         */
        bool IsLave(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a trap (piege) hazard is present at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if an active trap tile is found at @p pos.
         */
        bool IsPiege(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a drip (goutte) hazard is present at the given position.
         * @param[in] pos     Game-space pixel position to test.
         * @param[in] bAlways True to test even when the hazard is in an inactive cycle.
         * @return True if a drip hazard is found at @p pos.
         */
        bool IsGoutte(TinyPoint pos, bool bAlways);

    private:
        /**
         * @brief Tests whether a saw (scie) hazard is present at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if a saw hazard tile overlaps @p pos.
         */
        bool IsScie(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a switch tile is present at the given position.
         * @param[in]  pos        Game-space pixel position to test.
         * @param[out] celSwitch  Set to the tile coordinates of the switch if found.
         * @return True if a switch tile is found; @p celSwitch is valid only when true.
         */
        bool IsSwitch(TinyPoint pos, TinyPoint& celSwitch);

    private:
        /**
         * @brief Tests whether a crusher (ecraseur) hazard is active at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if an active crusher hazard overlaps @p pos.
         */
        bool IsEcraseur(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a lightning (blitz) hazard is present at the given position.
         * @param[in] pos     Game-space pixel position to test.
         * @param[in] bAlways True to test even when the blitz is in an inactive phase.
         * @return True if a lightning hazard is found at @p pos.
         */
        bool IsBlitz(TinyPoint pos, bool bAlways);

    private:
        /**
         * @brief Tests whether a spring (ressort) tile is present at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if a spring tile is found, causing an upward bounce.
         */
        bool IsRessort(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a temporary / timed platform tile is present at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if a temporary tile is found (disappears after Blupi steps on it).
         */
        bool IsTemp(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a bridge tile is present at or near the given position.
         * @param[in]  pos       Game-space pixel position to test.
         * @param[out] celBridge Set to the tile coordinates of the bridge cell if found.
         * @return True if a bridge tile is found; @p celBridge is valid only when true.
         */
        bool IsBridge(TinyPoint pos, TinyPoint& celBridge);

    private:
        /**
         * @brief Tests whether a door is present at the given position.
         * @param[in]  pos      Game-space pixel position to test.
         * @param[out] celPorte Set to the tile coordinates of the door if found.
         * @return Door index (>= 0) if a door is found; -1 if none.
         *         @p celPorte is valid only when the return value is >= 0.
         */
        int IsDoor(TinyPoint pos, TinyPoint& celPorte);

    private:
        /**
         * @brief Tests whether a teleporter tile is present at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return Teleporter index (>= 0) if found; -1 if no teleporter at @p pos.
         */
        int IsTeleporte(TinyPoint pos);

    private:
        /**
         * @brief Finds the destination position of a teleporter paired with @p pos.
         * @param[in]  pos     Game-space position of the source teleporter.
         * @param[out] newpos  Set to the exit teleporter position on success.
         * @return True if a matching teleporter destination was found; @p newpos is valid only when true.
         */
        bool SearchTeleporte(TinyPoint pos, TinyPoint& newpos);

    private:
        /**
         * @brief Tests whether the tile at the given position allows a normal (non-vehicle) jump.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if a normal jump is permitted from this tile position.
         */
        bool IsNormalJump(TinyPoint pos);

    private:
        /**
         * @brief Tests whether the tile at the given position is shallow/surf water.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if a surf-water tile (Blupi uses surfboard) is at @p pos.
         */
        bool IsSurfWater(TinyPoint pos);

    private:
        /**
         * @brief Tests whether the tile at the given position is deep water.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if a deep-water tile (Blupi swims) is at @p pos.
         */
        bool IsDeepWater(TinyPoint pos);

    private:
        /**
         * @brief Tests whether the tile at the given position is the water/land boundary.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if the tile is the transition boundary between water and dry land.
         */
        bool IsOutWater(TinyPoint pos);

    private:
        /**
         * @brief Tests whether an icon index represents a passable (walk-through) tile.
         * @param[in] icon  Tile icon index to classify.
         * @return True if the tile is passable (Blupi can walk through it).
         * @see  IsBlocIcon()
         */
        bool IsPassIcon(int icon);

    private:
        /**
         * @brief Tests whether an icon index represents a blocking (solid) tile.
         * @param[in] icon  Tile icon index to classify.
         * @return True if the tile is solid (Blupi cannot pass through it).
         * @see  IsPassIcon()
         */
        bool IsBlocIcon(int icon);

    private:
        /**
         * @brief Clears the projectile (balle) trajectory occupancy array.
         * @post m_balleTraj[] is zeroed; all previously recorded bullet positions are removed.
         */
        void FlushBalleTraj();

    private:
        /**
         * @brief Marks a position in the projectile trajectory occupancy array.
         * @param[in] pos  Game-space position to record as occupied by a bullet.
         */
        void SetBalleTraj(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a position is occupied by a bullet in the trajectory array.
         * @param[in] pos  Game-space position to test.
         * @return True if a bullet trajectory passes through @p pos.
         */
        bool IsBalleTraj(TinyPoint pos);

    private:
        /**
         * @brief Clears the moving-object trajectory occupancy array.
         * @post m_moveTraj[] is zeroed; all previously recorded object positions are removed.
         */
        void FlushMoveTraj();

    private:
        /**
         * @brief Marks a position in the moving-object trajectory occupancy array.
         * @param[in] pos  Game-space position to record as occupied by a moving object.
         */
        void SetMoveTraj(TinyPoint pos);

    private:
        /**
         * @brief Tests whether a position is occupied by a moving object in the trajectory array.
         * @param[in] pos  Game-space position to test.
         * @return True if a moving object occupies @p pos.
         */
        bool IsMoveTraj(TinyPoint pos);

    private:
        /**
         * @brief Searches rightward for the nearest available placement distance for an object.
         * @param[in] pos   Starting game-space position.
         * @param[in] dir   Direction vector for the search.
         * @param[in] type  ObjectType to check for conflicts.
         * @return Distance in game-space pixels to the nearest free position, or -1 if none.
         */
        int SearchDistRight(TinyPoint pos, TinyPoint dir, ObjectType type);

    private:
        /**
         * @brief Tests whether a ventilator (fan) tile is active at the given position.
         * @param[in] pos  Game-space pixel position to test.
         * @return True if an active fan tile that pushes Blupi is found at @p pos.
         */
        bool IsVentillo(TinyPoint pos);

    private:
        /**
         * @brief Stops the cloud net (filet nuage) object at the given pool index.
         * @param[in] rank  Index into m_moveObject[] of the cloud-net object to stop.
         */
        void NetStopCloud(int rank);

    private:
        /**
         * @brief Spawns a sploutch-glu (glue splash) effect at the given position.
         * @param[in] pos  Game-space position where the glue splash appears.
         */
        void StartSploutchGlu(TinyPoint pos);

    private:
        /**
         * @brief Creates and activates a new moving object in the pool.
         * @param[in] pos   Game-space position at which to spawn the object.
         * @param[in] type  Type of object to create.
         * @param[in] speed Movement speed for the new object (pixels per frame).
         * @return Index into m_moveObject[] of the newly created object, or -1 on failure.
         * @note  Returns -1 if the pool (MAXMOVEOBJECT) is full.
         */
        int ObjectStart(TinyPoint pos, ObjectType type, int speed);

    private:
        /**
         * @brief Removes a moving object of the given type from the pool at @p pos.
         * @param[in] pos   Game-space position of the object to delete.
         * @param[in] type  Type of object to match for deletion.
         * @return True if an object was found and removed; false if no match was found.
         */
        bool ObjectDelete(TinyPoint pos, ObjectType type);

    private:
        /**
         * @brief Sets the tile icon at a given position and marks the cell as dirty.
         * @param[in] pos   Tile coordinates (in game-space) of the cell to modify.
         * @param[in] icon  New tile icon index to assign to the cell.
         * @post The tile at @p pos in m_decor is updated and will be redrawn next frame.
         */
        void ModifDecor(TinyPoint pos, int icon);

    private:
        /**
         * @brief Advances all active moving objects by one simulation frame.
         * @details
         *   Iterates m_moveObject[] and calls MoveObjectStepLine() and
         *   MoveObjectStepIcon() for each active entry. Handles object lifetime,
         *   collision response, and object-specific AI.
         */
        void MoveObjectStep();

    private:
        /**
         * @brief Advances the linear movement of one moving object by one frame.
         * @param[in] i  Index into m_moveObject[] of the object to advance.
         * @details
         *   Moves the object along its posStart–posEnd path, handling bounce,
         *   pause timers (timeStopStart/timeStopEnd), and direction reversal.
         */
        void MoveObjectStepLine(int i);

    private:
        /**
         * @brief Advances the animation icon counter of one moving object by one frame.
         * @param[in] i  Index into m_moveObject[] of the object to update.
         * @details
         *   Increments the object's phase counter and looks up the new icon in the
         *   animation tables, updating channel and icon fields.
         */
        void MoveObjectStepIcon(int i);

    private:
        /**
         * @brief Initiates a dynamite explosion from the object at pool index @p i.
         * @param[in] i   Index into m_moveObject[] of the dynamite object.
         * @param[in] dx  Tile x-offset of the explosion epicenter relative to the object.
         * @param[in] dy  Tile y-offset of the explosion epicenter relative to the object.
         */
        void DynamiteStart(int i, int dx, int dy);

    private:
        /**
         * @brief Detects whether Blupi has stepped onto or under a lift (ascenseur).
         * @param[in] rect    Bounding rectangle of Blupi at the new position.
         * @param[in] oldpos  Blupi's position from the previous frame.
         * @param[in] newpos  Blupi's proposed new position.
         * @return Index of the lift object in m_moveObject[] that Blupi has entered,
         *         or -1 if no lift interaction occurred.
         */
        int AscenseurDetect(TinyRect rect, TinyPoint oldpos, TinyPoint newpos);

    private:
        /**
         * @brief Tests whether the lift at index @p i causes a vertigo (near-edge) state.
         * @param[in]  i             Index into m_moveObject[] of the lift.
         * @param[out] bVertigoLeft  Set to true if Blupi is near the left edge of the lift.
         * @param[out] bVertigoRight Set to true if Blupi is near the right edge of the lift.
         */
        void AscenseurVertigo(int i, bool& bVertigoLeft, bool& bVertigoRight);

    private:
        /**
         * @brief Shifts Blupi's position to stay centered on the lift at index @p i.
         * @param[in] i  Index into m_moveObject[] of the lift.
         * @return True if Blupi's position was adjusted; false if no shift was needed.
         */
        bool AscenseurShift(int i);

    private:
        /**
         * @brief Synchronises connected lifts so they move together.
         * @param[in] i  Index into m_moveObject[] of the primary lift.
         * @details
         *   Some lifts are logically paired; this ensures their posCurrent values
         *   remain consistent so Blupi does not phase through or bounce unexpectedly.
         */
        void AscenseurSynchro(int i);

    private:
        /**
         * @brief Refreshes the crate (caisse) pool indices in m_rankCaisse[].
         * @details
         *   Scans m_moveObject[] for objects classified as crates and rebuilds
         *   m_rankCaisse[] and m_nbRankCaisse to reflect the current pool state.
         */
        void UpdateCaisse();

    private:
        /**
         * @brief Tests whether Blupi can push the crate at pool index @p i.
         * @param[in] i     Index into m_moveObject[] of the crate.
         * @param[in] pos   Proposed push position in game-space.
         * @param[in] bPop  True to also test whether the crate can pop out (spring/fall).
         * @return True if the push is possible without a blocking collision.
         */
        bool TestPushCaisse(int i, TinyPoint pos, bool bPop);

    private:
        /**
         * @brief Tests whether a single crate at pool index @p i can move by vector @p move.
         * @param[in] i     Index into m_moveObject[] of the crate.
         * @param[in] move  Movement vector (pixels) to test.
         * @param[in] b     Bitmask of crates already tested in this chain (prevents cycles).
         * @return True if the crate can move by @p move without obstruction.
         */
        bool TestPushOneCaisse(int i, TinyPoint move, int b);

    private:
        /**
         * @brief Finds all crates linked (group-movable) to the crate at @p rank.
         * @param[in] rank  Index into m_rankCaisse[] of the starting crate.
         * @param[in] bPop  True to also collect crates that can pop/spring.
         * @post m_linkCaisse[] and m_nbLinkCaisse are updated with the linked set.
         */
        void SearchLinkCaisse(int rank, bool bPop);

    private:
        /**
         * @brief Adds a crate to the linked-crate group for group movement.
         * @param[in] rank  Index into m_rankCaisse[] of the crate to add.
         * @return True if the crate was successfully added to m_linkCaisse[].
         */
        bool AddLinkCaisse(int rank);

    private:
        /**
         * @brief Returns the pool index of the crate directly in front of Blupi.
         * @return Index into m_moveObject[] of the frontmost crate, or -1 if none.
         */
        int CaisseInFront();

    private:
        /**
         * @brief Computes how far the current crate group can be pushed before hitting an obstacle.
         * @param[in] max  Maximum movement distance to consider (in game-space pixels).
         * @return Actual push distance in game-space pixels (0 to @p max).
         */
        int CaisseGetMove(int max);

    private:
        /**
         * @brief Detects whether a mockery (taunt) object is near Blupi's position.
         * @param[in] pos  Game-space position to test.
         * @return Pool index of the mockery object if one is nearby; -1 if none found.
         */
        int MockeryDetect(TinyPoint pos);

    private:
        /**
         * @brief Applies an electric hazard at @p pos to Blupi if he is touching it.
         * @param[in] pos  Game-space position of the electric hazard source.
         * @return True if Blupi was electrocuted (triggered death or damage).
         */
        bool BlupiElectro(TinyPoint pos);

    private:
        /**
         * @brief Makes follower NPC objects track Blupi's current position.
         * @param[in] pos  Blupi's current game-space position for the NPC to approach.
         */
        void MoveObjectFollow(TinyPoint pos);

    private:
        /**
         * @brief Detects the moving object nearest to the given position.
         * @param[in]  pos    Game-space position to search around.
         * @param[out] bNear  Set to true if the nearest object is within contact range.
         * @return Pool index of the nearest object, or -1 if no object was found.
         */
        int MoveObjectDetect(TinyPoint pos, bool& bNear);

    private:
        /**
         * @brief Detects a lift (ascenseur) object at or near the given position.
         * @param[in] pos     Game-space position to test.
         * @param[in] height  Height of the bounding area to search within (pixels).
         * @return Pool index of the matching lift, or -1 if no lift was found.
         */
        int MoveAscenseurDetect(TinyPoint pos, int height);

    private:
        /**
         * @brief Detects a charge (charging enemy) object near the given position.
         * @param[in] pos  Game-space position to test.
         * @return Pool index of the charging object if found; -1 otherwise.
         */
        int MoveChargeDetect(TinyPoint pos);

    private:
        /**
         * @brief Detects an NPC (perso) following object near the given position.
         * @param[in] pos  Game-space position to test.
         * @return Pool index of the following NPC if found; -1 otherwise.
         */
        int MovePersoDetect(TinyPoint pos);

    private:
        /**
         * @brief Removes all moving objects whose tile cell matches @p cel.
         * @param[in] cel  Tile coordinates (game-space / 64) of the cell to clear.
         * @return Number of objects removed.
         */
        int MoveObjectDelete(TinyPoint cel);

    private:
        /**
         * @brief Finds a free (inactive) slot in m_moveObject[].
         * @return Index of an unused MoveObject slot, or -1 if the pool is full.
         */
        int MoveObjectFree();

    private:
        /**
         * @brief Returns the draw-order sort priority for the given object type.
         * @param[in] type  ObjectType whose sort priority is requested.
         * @return Positive integer priority; higher values are drawn on top of lower values.
         */
        int SortGetType(ObjectType type);

    private:
        /**
         * @brief Sorts m_moveObject[] by draw priority so objects are layered correctly.
         * @details
         *   Uses SortGetType() to assign priorities and reorders the pool accordingly.
         *   Must be called before Build() draws objects to ensure correct Z-ordering.
         */
        void MoveObjectSort();

    private:
        /**
         * @brief Raises the draw priority of the object at pool index @p i to the front.
         * @param[in] i  Index into m_moveObject[] of the object to promote.
         */
        void MoveObjectPriority(int i);

    private:
        /**
         * @brief Searches for a moving object at or near the given game-space position.
         * @param[in] pos  Game-space position to search around.
         * @return Pool index of the nearest object at @p pos, or -1 if none found.
         */
        int MoveObjectSearch(TinyPoint pos);

    private:
        /**
         * @brief Searches for a moving object of a specific type at the given position.
         * @param[in] pos   Game-space position to search around.
         * @param[in] type  Optional object type filter; std::nullopt matches any type.
         * @return Pool index of the first matching object, or -1 if none found.
         */
        int MoveObjectSearch(TinyPoint pos, std::optional<ObjectType> type);

    private:
        /**
         * @brief Triggers the helicopter destruction particle effect.
         * @details
         *   Creates several ByeByeObject fragments at the helicopter's last known position
         *   (m_blupiPosHelico) and adds them to byeByeObjects for rendering by ByeByeDraw().
         */
        void ByeByeHelico();

    private:
        /**
         * @brief Adds a single particle fragment to the ByeByeObject pool.
         * @param[in] channel        Sprite sheet for the fragment.
         * @param[in] icon           Icon slot for the fragment within the sprite sheet.
         * @param[in] pos            Initial game-space position of the fragment.
         * @param[in] rotationSpeed  Angular velocity in radians per frame.
         * @param[in] animationSpeed Phase increment per frame.
         */
        void ByeByeAdd(PixmapChannel channel, int icon, TinyPoint pos, double rotationSpeed, double animationSpeed);

    private:
        /**
         * @brief Advances all active ByeByeObject fragments by one frame.
         * @details
         *   Updates position, rotation, and phase for each fragment.
         *   Removes fragments that have moved off-screen or exceeded their lifetime.
         */
        void ByeByeStep();

    private:
        /**
         * @brief Draws all active ByeByeObject fragments to the screen.
         * @param[in] posDecor  Current scroll offset in game-space, used to convert to screen coordinates.
         */
        void ByeByeDraw(TinyPoint posDecor);

    private:
        /**
         * @brief Computes the HUD life-icon position for the given life count.
         * @param[in] nbVies  Number of lives to position the icon for.
         * @return Game-space position of the life icon in the HUD.
         */
        TinyPoint VoyageGetPosVie(int nbVies);

    private:
        /**
         * @brief Initialises a Voyage (item-collection arc) animation.
         * @param[in] start    Start position of the arc in game-space.
         * @param[in] end      End position of the arc in game-space.
         * @param[in] icon     Sprite icon used for the travelling collectible.
         * @param[in] channel  Sprite sheet for the travelling collectible.
         * @post m_voyageStart, m_voyageEnd, m_voyageIcon, m_voyageChannel, and related counters
         *       are initialised; VoyageStep() will advance the animation each frame.
         */
        void VoyageInit(TinyPoint start, TinyPoint end, int icon, PixmapChannel channel);

    private:
        /**
         * @brief Advances the Voyage animation arc by one frame.
         * @post m_voyagePhase is incremented; animation ends when m_voyagePhase >= m_voyageTotal.
         */
        void VoyageStep();

    private:
        /**
         * @brief Draws the current Voyage animation frame to the screen.
         * @note  Has no effect if no Voyage animation is currently active.
         */
        void VoyageDraw();

    private:
        /**
         * @brief Tests whether the moving object at pool index @p i floats on water.
         * @param[in] i  Index into m_moveObject[] to test.
         * @return True if the object type at @p i is a floating object (e.g., raft, barrel).
         */
        bool IsFloatingObject(int i);

    private:
        /**
         * @brief Tests whether the given tile cell is on the right border of a terrain feature.
         * @param[in] x   Tile x-coordinate.
         * @param[in] y   Tile y-coordinate.
         * @param[in] dx  Horizontal search direction.
         * @param[in] dy  Vertical search direction.
         * @return True if the tile at (x, y) is on the right border of the feature.
         */
        bool IsRightBorder(int x, int y, int dx, int dy);

    private:
        /**
         * @brief Tests whether the tile at (x, y) is a fromage (cheese) tile.
         * @param[in] x  Tile x-coordinate.
         * @param[in] y  Tile y-coordinate.
         * @return True if the tile contains a fromage icon.
         */
        bool IsFromage(int x, int y);

    private:
        /**
         * @brief Tests whether the tile at (x, y) is a grotte (cave/rock) tile.
         * @param[in] x  Tile x-coordinate.
         * @param[in] y  Tile y-coordinate.
         * @return True if the tile contains a grotte icon.
         */
        bool IsGrotte(int x, int y);

    private:
        /**
         * @brief Adapts the border tile icons for a mid-section of a terrain feature.
         * @param[in] x  Tile x-coordinate of the cell to adapt.
         * @param[in] y  Tile y-coordinate of the cell to adapt.
         * @note  Selects the correct mid-border icon variant based on adjacent tile state.
         */
        void AdaptMidBorder(int x, int y);

    private:
        /**
         * @brief Adapts all border tile icons around the given tile cell.
         * @param[in] cel  Tile coordinates of the cell whose neighbours need border adaptation.
         * @details
         *   Called after ModifDecor() changes a tile to keep border/edge icons consistent
         *   with their neighbours. Calls AdaptMidBorder() and IsRightBorder() internally.
         */
        void AdaptBorder(TinyPoint cel);

    public:
        /**
         * @brief Deletes the current-game (quick-save) file from persistent storage.
         * @note  Silently succeeds if no quick-save file exists.
         * @see   CurrentWrite(), CurrentRead()
         */
        void CurrentDelete();

    public:
        /**
         * @brief Writes a quick-save snapshot of the current level state.
         * @details
         *   Serialises all Blupi and level state to the current-game save file so the
         *   player can continue from the exact same position after the app is closed.
         *   The file format mirrors the regular level file format used by Read().
         * @return True on success; false on I/O error.
         * @pre  The level must be loaded and PlayPrepare() must have been called.
         * @see  CurrentRead(), CurrentDelete()
         */
        bool CurrentWrite();

    public:
        /**
         * @brief Reads and restores a quick-save snapshot.
         * @details
         *   Deserialises the current-game save file and applies all state to the
         *   simulation. The level is restored to the exact saved frame, including
         *   Blupi's position, keys, lives, and door state.
         * @return True on success; false if no save file exists or on parse error.
         * @post All simulation state reflects the saved snapshot; call MoveStep()/Build()
         *       normally after this to continue.
         * @see  CurrentWrite(), CurrentDelete()
         */
        bool CurrentRead();

    public:
        /**
         * @brief Loads a level file from the official or user mission set.
         * @details
         *   Parses the level data file for the given gamer slot and mission rank,
         *   populating m_decor, m_bigDecor, and the moving-object pool.
         * @param[in] gamer  Gamer slot index (0-based).
         * @param[in] rank   Mission/level rank (0-based index within the set).
         * @param[in] bUser  True for user-created levels; false for official missions.
         * @return True if the level was loaded successfully; false on I/O or parse error.
         * @pre  Create() must have been called before Read().
         * @post m_decor, m_bigDecor, and m_moveObject[] reflect the loaded level.
         */
        bool Read(int gamer, int rank, bool bUser);

    private:
        /**
         * @brief Deletes a level save file for the given gamer slot and rank.
         * @param[in] gamer  Gamer slot index (0-based).
         * @param[in] rank   Mission/level rank.
         * @param[in] bUser  True for user-created levels; false for official missions.
         * @return True if the file was successfully deleted or did not exist.
         */
        bool Delete(int gamer, int rank, bool bUser);

    private:
        /**
         * @brief Tests whether a level save file exists for the given gamer slot and rank.
         * @param[in] gamer  Gamer slot index (0-based).
         * @param[in] rank   Mission/level rank.
         * @param[in] bUser  True for user-created levels; false for official missions.
         * @return True if the file exists on disk.
         */
        bool FileExist(int gamer, int rank, bool bUser);

    private:
        /**
         * @brief Finds the starting tile and direction for the given world index.
         * @param[in]  world  World index to search for in the tile map.
         * @param[out] blupi  Set to the starting game-space position for this world.
         * @param[out] dir    Set to the starting facing direction for this world.
         * @return True if the world start marker was found; @p blupi and @p dir are valid only when true.
         */
        bool SearchWorld(int world, TinyPoint& blupi, Direction& dir);

    private:
        /**
         * @brief Finds the door tile and Blupi entry position for door number @p n.
         * @param[in]  n      Zero-based door index to search for.
         * @param[out] cel    Set to the tile coordinates of the door.
         * @param[out] blupi  Set to the game-space entry position for this door.
         * @return True if door @p n was found; @p cel and @p blupi are valid only when true.
         */
        bool SearchDoor(int n, TinyPoint& cel, TinyPoint& blupi);

    private:
        /**
         * @brief Finds the gold (treasure) tile for the given treasure index.
         * @param[in]  n    Zero-based treasure index to search for.
         * @param[out] cel  Set to the tile coordinates of the treasure.
         * @return True if treasure @p n was found; @p cel is valid only when true.
         */
        bool SearchGold(int n, TinyPoint& cel);

    public:
        /**
         * @brief Initialises the main-world switch state for the world-selection level.
         * @details
         *   Called when loading the main-world (hub) level. Sets door/switch states based
         *   on how far the player has progressed (@p lastWorld). Worlds already completed
         *   have their entry doors opened; future worlds remain locked.
         * @param[in] lastWorld  Index of the last world the player completed (0-based).
         * @pre  The hub level map must already be loaded via Read().
         * @post m_doors[] is updated to match the player's progression state.
         */
        void MainSwitchInitialize(int lastWorld);

    public:
        /**
         * @brief Opens or closes doors in the current level based on the gamer's progress.
         * @details
         *   Adjusts m_doors[] to reflect which doors should be accessible given the
         *   current save-game state. In private (user-created) levels, all doors are always
         *   treated as open.
         * @param[in] bPrivate  True for user-created levels where all doors are always open.
         * @post m_doors[] reflects the accessible doors for the current game state.
         * @see  InitializeDoors(), MemorizeDoors()
         */
        void AdaptDoors(bool bPrivate);

        /**
         * @brief Tests whether ghost (free-flight cheat) mode is currently active.
         * @return True if m_blupiGhost is true (MODERN builds only); always false in LEGACY builds.
         * @note  In LEGACY builds this method always returns false.
         */
        bool IsGhost();

#ifdef MODERN
        /**
         * @brief Sets the cheat zoom multiplier applied to the hotspot zoom.
         * @details
         *   Adjusts how far the camera pulls back. The value is multiplied with the
         *   normal hotspot zoom to produce the final camera zoom level.
         * @param[in] factor  Zoom multiplier: 1.0 = normal view, 0.75 = 25% out, 0.5 = 50% out.
         * @note  MODERN-only. Has no effect in LEGACY builds.
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
        /**
         * @brief Opens all doors that require a treasure to be collected.
         * @details
         *   Scans m_doors[] for treasure-gated doors and opens any whose treasure
         *   condition is now satisfied (m_nbTresor == m_totalTresor or per-door count).
         */
        void OpenDoorsTresor();

    private:
        /**
         * @brief Opens the door tile at the given tile coordinates.
         * @param[in] cel  Tile coordinates of the door to open.
         * @post The door tile at @p cel is replaced with its open-state icon and
         *       the corresponding entry in m_doors[] is set to the open value.
         */
        void OpenDoor(TinyPoint cel);

    private:
        /**
         * @brief Opens all win-condition doors when the level is completed.
         * @details
         *   Called when the player satisfies the win condition. Opens any remaining
         *   doors that are gated by the win state so Blupi can reach the exit.
         */
        void OpenDoorsWin();

    private:
        /**
         * @brief Opens all gold-chest tiles when the win condition is met.
         * @details
         *   Triggers visual gold-chest animations and marks the chest tiles as collected.
         *   Called from OpenDoorsWin() as part of the level-complete sequence.
         */
        void OpenGoldsWin();

    private:
        /**
         * @brief Closes all doors as a penalty when Blupi loses a life.
         * @details
         *   Resets m_doors[] entries for doors that were opened by Blupi during the
         *   current attempt, so replaying after death starts with doors in the correct state.
         */
        void DoorsLost();
    };
}
