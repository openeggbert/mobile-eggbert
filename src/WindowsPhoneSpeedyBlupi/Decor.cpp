/**
 * @file   Decor.cpp
 * @brief  Implements the Speedy Blupi gameplay simulation declared in Decor.hpp.
 * @details
 *   This translation unit is the largest and most complex file in the project
 *   (~11 000 lines). It contains the full per-frame gameplay simulation: tile
 *   physics and collision, the Blupi player action state machine, every moving
 *   object AI, lift (ascenseur) and teleporter mechanics, crate (caisse) pushing,
 *   particle/effect spawning, sound triggering, viewport scrolling, and level I/O.
 *
 *   The public contract of every method is documented in Decor.hpp; the comments
 *   in this file are strictly implementation-level (algorithm sketches, invariants,
 *   side effects, and warnings) and intentionally do not repeat the header.
 *
 *   ## Per-frame tick: MoveStep()
 *   MoveStep() is the single entry point that advances the whole simulation by one
 *   frame. It runs its sub-systems in a fixed order so that later stages observe the
 *   results of earlier ones:
 *     1. m_time is incremented (it drives every animation phase counter).
 *     2. MoveHotSpot() eases the camera zoom/pan toward its target.
 *     3. BlupiStep() runs the player physics + action state machine. This reads the
 *        latest input (m_keyPress, m_blupiSpeedX/Y), applies gravity/velocity,
 *        resolves tile collision via BlupiAdjust()/BlupiBloque(), handles vehicle
 *        and hazard transitions, and may attach Blupi to a lift or teleport him.
 *     4. MoveObjectStep() advances every active moving object: linear motion
 *        (MoveObjectStepLine), per-type AI and lifetime (MoveObjectStepIcon),
 *        crate bookkeeping (UpdateCaisse), and pollution damage (MoveObjectPollution).
 *     5. ByeByeStep() advances and expires destruction particles.
 *     6. The scroll position (m_posDecor) is eased toward Blupi.
 *     7. Win/loss conditions update m_term (read later by IsTerminated()).
 *   Build() is a separate call made after MoveStep(); it only renders and never
 *   mutates simulation state.
 *
 *   ## Tile-overlap collision resolution
 *   Collision is tile-based, not swept. The moving entity (Blupi or an object) is
 *   represented by an axis-aligned rectangle in game-space pixels. DecorDetect()
 *   converts that rectangle to the range of tile cells it overlaps and tests each
 *   cell's icon via IsBlocIcon()/IsPassIcon(); a single blocking cell makes the
 *   rectangle "occupied". Movement is applied first, then BlupiAdjust() pushes Blupi
 *   back out of any penetrated tile by nudging him along the minor axis until
 *   DecorDetect() reports clear; BlupiBloque() answers the directional "can I move
 *   here?" query used before committing a step. TestPath() does a stepwise march
 *   from start toward end and returns the furthest clear position for objects that
 *   must stop on contact rather than be pushed out.
 *
 *   ## Object AI state machines
 *   Two styles coexist. Most enemies/effects are *table-driven*: a MoveObject moves
 *   linearly between posStart and posEnd (advance/recede speeds + end-dwell timers in
 *   MoveObjectStepLine) while its animation/lifetime is a phase counter indexed into
 *   the Tables animation arrays by MoveObjectStepIcon(). A minority of behaviours are
 *   *hand-coded* with bespoke logic keyed on ObjectType inside MoveObjectStepIcon()
 *   and its helpers (dynamite, charging enemies, followers, cloud-nets, crates).
 *
 *   ## Teleporter pairing
 *   Teleporters are not stored in a side table; they are discovered from the tile map
 *   on demand. IsTeleporte() reports the teleporter index at a position, and
 *   SearchTeleporte() scans the whole map for the *other* teleporter cell sharing the
 *   same pairing and returns its position as the exit. Pairing is therefore implicit
 *   in the tile icons rather than indexed.
 *
 *   ## Particle / effect objects
 *   Two pools exist. Short-lived water/glue/bubble splashes and most one-shot effects
 *   are ordinary MoveObjects spawned through ObjectStart() (e.g. MoveObjectPlouf,
 *   MoveObjectBlup, StartSploutchGlu); they expire when their phase counter runs past
 *   the end of their animation table and the slot is freed. Destruction debris from a
 *   helicopter uses the separate float-based ByeByeObject pool (ByeByeHelico/ByeByeAdd
 *   spawn, ByeByeStep advances and culls off-screen fragments, ByeByeDraw renders); it
 *   is purely visual and never touches gameplay state.
 *
 * @note  This is a direct port of the original C# game. Every numeric constant is part
 *        of the original game logic; do not tune them without understanding the 20 FPS
 *        timing assumptions (see Config::ScaleTime()).
 * @see   Decor.hpp, IPixmap, ISound, GameData, Tables
 */
#include "WindowsPhoneSpeedyBlupi/Decor.hpp"

#include <iomanip>
#include <string>

#include "WindowsPhoneSpeedyBlupi/Config.hpp"

#include "CNA/Logger.hpp"
#include "System/String.hpp"
#include "WindowsPhoneSpeedyBlupi/Misc.hpp"
#include "WindowsPhoneSpeedyBlupi/MyResource.hpp"
#include "WindowsPhoneSpeedyBlupi/Text.hpp"
#include "WindowsPhoneSpeedyBlupi/Worlds.hpp"
#include "WindowsPhoneSpeedyBlupi/def/SoundChannel.hpp"

/**
 * @brief Maximum number of egg collectibles considered in a single egg scan.
 * @details File-local constant used by the egg-related gameplay paths in this
 *          translation unit; it caps how many egg objects one scan inspects. Not
 *          exposed in the header.
 */
static constexpr SharpRuntime::intcs MAX_EGG_COUNT = 10;

namespace WindowsPhoneSpeedyBlupi
{
    using log = CNA::Logger;
    TinyRect Decor::getDrawBoundsProperty() const { return m_drawBounds; }
    void Decor::setDrawBoundsProperty(const TinyRect v) { m_drawBounds = v; }
    IDATA(Def::ButtonGlyph, ButtonPressed, Decor)

    void Decor::MoveObjectCopy(MoveObject& dst, const MoveObject& src)
    {
        dst.type = src.type;
        dst.stepAdvance = src.stepAdvance;
        dst.stepRecede = src.stepRecede;
        dst.timeStopStart = src.timeStopStart;
        dst.timeStopEnd = src.timeStopEnd;
        dst.posStart = src.posStart;
        dst.posEnd = src.posEnd;
        dst.posCurrent = src.posCurrent;
        dst.step = src.step;
        dst.time = src.time;
        dst.phase = src.phase;
        dst.channel = src.channel;
        dst.icon = src.icon;
    }

    Decor::Decor() : m_sound(nullptr), m_pixmap(nullptr), m_gameData(nullptr),
                     m_moveObject{},
                     m_keyPress(0),
                     m_lastKeyPress(0),
                     m_term(0),
                     m_music(0), m_region(0),
                     m_bPause(false),
                     m_nbRankCaisse(0),
                     m_rankCaisse{},
                     m_nbLinkCaisse(0),
                     m_linkCaisse{},
                     m_blupiAction(BlupiAction::None),
                     m_blupiDir(Direction::None), m_blupiPhase(0),
                     m_blupiVitesseX(0),
                     m_blupiVitesseY(0),
                     m_blupiIcon(0),
                     m_blupiSec(SecretPower::None),
                     m_blupiChannel(PixmapChannel::PixmapChannel0),
                     m_blupiTransport(0),
                     m_blupiFocus(false),
                     m_blupiAir(false),
                     m_blupiHelico(false),
                     m_blupiOver(false),
                     m_blupiJeep(false), m_blupiTank(false),
                     m_blupiSkate(false), m_blupiNage(false),
                     m_blupiSurf(false),
                     m_blupiVent(false),
                     m_blupiSuspend(false),
                     m_blupiJumpAie(false),
                     m_blupiShield(false),
                     m_blupiPower(false),
                     m_blupiCloud(false),
                     m_blupiHide(false),
                     m_blupiInvert(false),
                     m_blupiBalloon(false),
                     m_blupiEcrase(false),
                     m_blupiMotorHigh(false),
                     m_blupiMotorSound(SoundChannel::SoundChannel0),
                     m_blupiRestart(false),
                     m_blupiFront(false),
                     m_blupiBullet(0), m_blupiCle(DoorKeyFlags::None),
                     m_blupiPerso(0),
                     m_blupiDynamite(0),
                     m_blupiNoBarre(0),
                     m_blupiTimeShield(0),
                     m_blupiTimeFire(0),
                     m_blupiTimeNoAsc(0),
                     m_blupiTimeMockery(0),
                     m_blupiTimeOuf(0),
                     m_blupiActionOuf(BlupiAction::None),
                     m_blupiFifoNb(0), m_blupiStartDir(Direction::None),
                     m_blupiSpeedX(0),
                     m_blupiSpeedY(0),
                     m_blupiLastSpeedX(0),
                     m_blupiLastSpeedY(0),
                     m_blupiLevel(0),
                     m_bFoundCle(false),
                     m_bPrivate(false),
                     m_mission(0),
                     m_doors{}, m_nbVies(0),
                     m_nbTresor(0),
                     m_totalTresor(0),
                     m_goalPhase(0),
                     m_detectIcon(0),
                     m_sucetteType(ObjectType::ObjectType0),
                     m_blupiLogicRotation(0),
                     m_blupiRealRotation(0),
                     m_blupiOffsetY(0),
                     m_voyageIcon(0),
                     m_voyageChannel(PixmapChannel::PixmapChannel0), m_voyagePhase(0),
                     m_voyageTotal(0),
                     m_decorAction(DecorAction::None),
                     m_decorPhase(0),
                     m_hotSpotStepZoom(0),
                     m_hotSpotStepX(0),
                     m_hotSpotStepY(0),
                     m_hotSpotOutLag(0),
                     ButtonPressed_(Def::ButtonGlyph::None)
    {
        for (int i = 0; i < 200; i++)
        {
            m_lastDecorIcon[i] = 0;
        }
        m_drawBounds.Left = 0;
        m_drawBounds.Right = 640;
        m_drawBounds.Top = 0;
        m_drawBounds.Bottom = 480;
        m_time = 0;
        m_bCheatDoors = false;
        m_bSuperBlupi = false;
        m_bDrawSecret = false;
        m_buildOfficialMissions = false;
        m_hotSpotFinalZoom = 1.0;
        m_hotSpotFinalX = 320.0;
        m_hotSpotFinalY = 240.0;
        m_hotSpotCurrentZoom = 1.0;
        m_hotSpotCurrentX = 320.0;
        m_hotSpotCurrentY = 240.0;
        m_random = std::make_unique<System::Random>();
    }

    void Decor::Create(ISound* sound, IPixmap* pixmap, GameData* gameData)
    {
        m_sound = sound;
        m_pixmap = pixmap;
        m_gameData = gameData;
        m_keyPress = 0;
        m_lastKeyPress = 0;
        m_blupiMotorSound = SoundChannel::SoundChannel0;
        InitDecor();
        TinyPoint pos{};
        pos.X = 90;
        pos.Y = 450;
        m_jauges[0] = Jauge();
        m_jauges[0].Create(m_pixmap, m_sound, pos, JaugeMode::Red, false);
        m_jauges[0].SetHide(true);
        pos.X = 90;
        pos.Y = 428;
        m_jauges[1] = Jauge();
        m_jauges[1].Create(m_pixmap, m_sound, pos, JaugeMode::Yellow, false);
        m_jauges[1].SetHide(true);
    }

    bool Decor::LoadImages()
    {
        std::ostringstream oss;
        oss << "decor" << std::setw(3) << std::setfill('0') << m_region;
        string name = oss.str();
        m_pixmap->BackgroundCache(name);
        return true;
    }


    /**
     * @note Resets the whole 100x100 map to icon -1 (empty), then stamps a small
     *       built-in starter platform (cells (1..7,4)) and two demo moving objects in
     *       slots 0/1. This baseline is overwritten by Read() when a real level loads;
     *       it only matters when no level file is present.
     * @note Also resets every Blupi flag, the scroll target, the bye-bye particle pool,
     *       and clears the bullet/move trajectory occupancy arrays.
     */
    void Decor::InitDecor()
    {
        m_posDecor.X = 0;
        m_posDecor.Y = 0;
        m_dimDecor.X = 100;
        m_dimDecor.Y = 100;
        m_music = 1;
        m_region = 2;
        m_decorAction = DecorAction::None;
        for (int i = 0; i < 100; i++)
        {
            for (int j = 0; j < 100; j++)
            {
                m_decor[i][j].icon = -1;
                m_bigDecor[i][j].icon = -1;
            }
        }
        m_decor[1][4].icon = 40;
        m_decor[2][4].icon = 38;
        m_decor[3][4].icon = 38;
        m_decor[4][4].icon = 38;
        m_decor[5][4].icon = 38;
        m_decor[6][4].icon = 38;
        m_decor[7][4].icon = 39;
        for (int k = 0; k < MAXMOVEOBJECT; k++)
        {
            m_moveObject[k].type = ObjectType::ObjectType0;
        }
        FlushBalleTraj();
        FlushMoveTraj();
        m_moveObject[0].type = ObjectType::ObjectType5;
        m_moveObject[0].stepAdvance = 1;
        m_moveObject[0].stepRecede = 1;
        m_moveObject[0].timeStopStart = 0;
        m_moveObject[0].timeStopEnd = 0;
        m_moveObject[0].posStart.X = 258;
        m_moveObject[0].posStart.Y = 196;
        m_moveObject[0].posEnd = m_moveObject[0].posStart;
        m_moveObject[0].posCurrent = m_moveObject[0].posStart;
        m_moveObject[0].phase = 0;
        m_moveObject[0].step = 1;
        m_moveObject[0].time = 0;
        m_moveObject[0].channel = PixmapChannel::Element;
        m_moveObject[0].icon = 0;
        m_moveObject[1].type = ObjectType::ObjectType7;
        m_moveObject[1].stepAdvance = 1;
        m_moveObject[1].stepRecede = 1;
        m_moveObject[1].timeStopStart = 0;
        m_moveObject[1].timeStopEnd = 0;
        m_moveObject[1].posStart.X = 450;
        m_moveObject[1].posStart.Y = 196;
        m_moveObject[1].posEnd = m_moveObject[1].posStart;
        m_moveObject[1].posCurrent = m_moveObject[1].posStart;
        m_moveObject[1].phase = 0;
        m_moveObject[1].step = 1;
        m_moveObject[1].time = 0;
        m_moveObject[1].channel = PixmapChannel::Element;
        m_moveObject[1].icon = 29;
        m_blupiStartPos.X = 66;
        m_blupiStartPos.Y = 192 + BLUPIOFFY;
        m_blupiStartDir = Direction::Right;
        m_blupiAction = BlupiAction::Stop;
        m_blupiPhase = 0;
        m_blupiIcon = 0;
        m_blupiChannel = PixmapChannel::Blupi;
        m_blupiFocus = true;
        m_blupiAir = false;
        m_blupiHelico = false;
        m_blupiOver = false;
        m_blupiJeep = false;
        m_blupiTank = false;
        m_blupiSkate = false;
        m_blupiNage = false;
        m_blupiSurf = false;
        m_blupiSuspend = false;
        m_blupiJumpAie = false;
        m_blupiShield = false;
        m_blupiPower = false;
        m_blupiCloud = false;
        m_blupiHide = false;
#ifdef MODERN
        m_blupiGhost = false;
#endif
        m_blupiInvert = false;
        m_blupiBalloon = false;
        m_blupiEcrase = false;
        m_blupiMotorHigh = false;
        m_blupiPosHelico.X = -1;
        m_blupiActionOuf = BlupiAction::None;
        m_blupiTimeNoAsc = 0;
        m_blupiTimeMockery = 0;
        m_blupiVitesseX = 0.0;
        m_blupiVitesseY = 0.0;
        m_blupiValidPos = m_blupiStartPos;
        m_blupiFront = false;
        m_blupiBullet = 0;
        m_blupiCle = DoorKeyFlags::None;
        m_blupiPerso = 0;
        m_blupiDynamite = 0;
        m_nbTresor = 0;
        m_totalTresor = 1;
        m_goalPhase = 0;
        m_scrollPoint = m_blupiStartPos;
        m_scrollAdd.X = 0;
        m_scrollAdd.Y = 0;
        m_term = 0;
        byeByeObjects.clear();
    }

    /**
     * @note Counts treasures by scanning the loaded object pool for ObjectType5 to
     *       initialise m_totalTresor (the win-condition target). A set of decorative
     *       object types is given a randomised starting phase so their animations are
     *       not all in lock-step. Editor-only marker objects (ObjectType23) are demoted
     *       to ObjectType0 (inactive) so they do not run during play.
     * @note Ends by running one MoveStep() and then snapping the scroll target onto
     *       Blupi so the first rendered frame is already centred (no initial pan).
     */
    void Decor::PlayPrepare(bool bTest)
    {
        if (bTest)
        {
            m_nbVies = 3;
        }
        m_blupiPos = m_blupiStartPos;
        m_blupiDir = m_blupiStartDir;
        if (m_blupiDir == Direction::Left)
        {
            m_blupiIcon = 4;
        }
        else
        {
            m_blupiIcon = 0;
        }
        m_blupiAction = BlupiAction::Stop;
        m_blupiPhase = 0;
        m_blupiFocus = true;
        m_blupiAir = false;
        m_blupiHelico = false;
        m_blupiOver = false;
        m_blupiJeep = false;
        m_blupiTank = false;
        m_blupiNage = false;
        m_blupiSurf = false;
        m_blupiSuspend = false;
        m_blupiJumpAie = false;
        m_blupiShield = false;
        m_blupiPower = false;
        m_blupiCloud = false;
        m_blupiHide = false;
#ifdef MODERN
        m_blupiGhost = false;
#endif
        m_blupiInvert = false;
        m_blupiBalloon = false;
        m_blupiEcrase = false;
        m_blupiMotorHigh = false;
        m_blupiActionOuf = BlupiAction::None;
        m_blupiTimeNoAsc = 0;
        m_blupiTimeMockery = 0;
        m_blupiValidPos = m_blupiPos;
        m_blupiBullet = 0;
        m_blupiCle = DoorKeyFlags::None;
        m_blupiPerso = 0;
        m_blupiDynamite = 0;
        m_nbTresor = 0;
        m_totalTresor = 0;
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType5)
            {
                m_totalTresor++;
            }
            m_moveObject[i].posCurrent = m_moveObject[i].posStart;
            m_moveObject[i].step = 1;
            m_moveObject[i].phase = 0;
            m_moveObject[i].time = 0;
            if (m_moveObject[i].type == ObjectType::ObjectType5 || m_moveObject[i].type == ObjectType::ObjectType6 || m_moveObject[i].type == ObjectType::ObjectType25 || m_moveObject[i].
                type == ObjectType::ObjectType26 || m_moveObject[i].type == ObjectType::ObjectType40 || m_moveObject[i].type == ObjectType::ObjectType2 || m_moveObject[i].type == ObjectType::ObjectType3 ||
                m_moveObject[i].type == ObjectType::ObjectType96 || m_moveObject[i].type == ObjectType::ObjectType97)
            {
                m_moveObject[i].phase = m_random.get()->Next(23);
            }
            if (m_moveObject[i].type == ObjectType::ObjectType23)
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
        }
        m_goalPhase = 0;
        MoveObjectSort();
        m_scrollPoint = m_blupiPos;
        m_scrollAdd.X = 0;
        m_scrollAdd.Y = 0;
        m_blupiPosHelico.X = -1;
        m_blupiMotorSound = SoundChannel::SoundChannel0;
        m_blupiFront = false;
        m_blupiNoBarre = 0;
        m_blupiValidPos = m_blupiPos;
        m_blupiFifoNb = 0;
        m_blupiTimeFire = 0;
        m_voyageIcon = -1;
        m_jauges[0].SetHide(true);
        m_jauges[1].SetHide(true);
        m_bFoundCle = false;
        m_term = 0;
        m_time = 0;
        m_bPause = false;
        MoveStep();
        m_scrollPoint.X = m_blupiPos.X + 30 + m_scrollAdd.X;
        m_scrollPoint.Y = m_blupiPos.Y + 30 + m_scrollAdd.Y;
    }

    void Decor::BuildPrepare()
    {
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            m_moveObject[i].posCurrent = m_moveObject[i].posStart;
            m_moveObject[i].step = 1;
            m_moveObject[i].time = 0;
            m_moveObject[i].phase = 0;
            if (m_moveObject[i].type == ObjectType::ObjectType23)
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
        }
        m_voyageIcon = -1;
        m_time = 0;
        m_bPause = false;
    }

    int Decor::IsTerminated()
    {
        return m_term;
    }

    /**
     * @note Fixed sub-system order: objects first (MoveObjectStep), then particles
     *       (ByeByeStep), then the player (BlupiStep) so Blupi reacts to objects already
     *       moved this frame, then camera (MoveHotSpot) and motor sound. The whole body
     *       is wrapped in a catch-all so a single bad frame cannot crash the game loop;
     *       the exception is intentionally swallowed (state may be left mid-update).
     * @warning Swallowing the exception means a partially-updated frame can be rendered;
     *          this matches the original behaviour and is not a place to add new logic.
     */
    void Decor::MoveStep()
    {
        try
        {
            MoveObjectStep();
            ByeByeStep();
            BlupiStep();
            MoveHotSpot();
            AdaptMotorVehicleSound();
        }
        catch (std::exception e)
        {
        }
    }

    void Decor::ResetHotSpot()
    {
        m_pixmap->SetHotSpot(1.0, getDrawBoundsProperty().getWidthProperty() / 2,
                             getDrawBoundsProperty().getHeightProperty() / 2);
    }

    /**
     * @note Decides whether to zoom in (flag): true only while Blupi is walking
     *       horizontally, focused, not in helicopter/power mode, and auto-zoom is enabled
     *       in settings. m_hotSpotOutLag adds hysteresis so the camera lingers zoomed-in
     *       for ScaleTime(30) frames after horizontal motion stops, avoiding zoom jitter.
     * @note The final zoom/centre target is interpolated toward each frame using fixed
     *       per-frame step sizes scaled by Config::SPEED_SCALE; the MODERN cheat zoom
     *       factor is folded into the target zoom. The current values are clamped so they
     *       never overshoot the target.
     */
    void Decor::MoveHotSpot()
    {
        bool flag = false;
        if (m_blupiSpeedX != 0.0)
        {
            flag = true;
        }
        if (!m_blupiFocus)
        {
            flag = false;
        }
        if (m_blupiHelico || m_blupiPower)
        {
            flag = false;
        }
        if (!m_gameData->getAutoZoomProperty())
        {
            flag = false;
        }
        if (flag)
        {
            m_hotSpotOutLag = Config::ScaleTime(30);
        }
        else if (m_hotSpotOutLag > 0.0)
        {
            if (!m_blupiFocus || m_blupiSpeedY != 0.0)
            {
                m_hotSpotOutLag = 0.0;
            }
            else
            {
                m_hotSpotOutLag -= 1.0;
            }
            if (m_hotSpotOutLag > 0.0)
            {
                flag = true;
            }
        }
        if (flag)
        {
            m_hotSpotFinalZoom = 1.3;
            m_hotSpotFinalX = m_blupiPos.X + 30 - m_posDecor.X;
            m_hotSpotFinalY = m_blupiPos.Y + 30 - m_posDecor.Y;
        }
        else
        {
            m_hotSpotFinalZoom = 1.0;
            m_hotSpotFinalX = getDrawBoundsProperty().getWidthProperty() / 2;
            m_hotSpotFinalY = getDrawBoundsProperty().getHeightProperty() / 2;
        }
#ifdef MODERN
        m_hotSpotFinalZoom *= m_cheatZoomFactor;
#endif
        m_hotSpotStepZoom = Config::SPEED_SCALE / 30.0;
        m_hotSpotStepX = 10.0 * Config::SPEED_SCALE;
        m_hotSpotStepY = 10.0 * Config::SPEED_SCALE;
        if (m_hotSpotCurrentZoom < m_hotSpotFinalZoom)
        {
            m_hotSpotCurrentZoom = std::min(m_hotSpotCurrentZoom + m_hotSpotStepZoom, m_hotSpotFinalZoom);
        }
        if (m_hotSpotCurrentZoom > m_hotSpotFinalZoom)
        {
            m_hotSpotCurrentZoom = std::max(m_hotSpotCurrentZoom - m_hotSpotStepZoom, m_hotSpotFinalZoom);
        }
        if (m_hotSpotCurrentX < m_hotSpotFinalX)
        {
            m_hotSpotCurrentX = std::min(m_hotSpotCurrentX + m_hotSpotStepX, m_hotSpotFinalX);
        }
        if (m_hotSpotCurrentX > m_hotSpotFinalX)
        {
            m_hotSpotCurrentX = std::max(m_hotSpotCurrentX - m_hotSpotStepX, m_hotSpotFinalX);
        }
        m_pixmap->SetHotSpot(m_hotSpotCurrentZoom, m_hotSpotCurrentX, m_hotSpotCurrentY);
    }

    /**
     * @note The lightning is on a 100-tick cycle (m_time / ScaleDiv(1) % 100): it is
     *       "active" only on even ticks in the first half of the cycle (num < 50), giving
     *       a flicker. When a blitz emitter (icon 304) sits in the cell above, the zap
     *       sound is played on a fixed set of cycle ticks so the audio lines up with the
     *       visible strikes.
     */
    bool Decor::BlitzActif(intcs celx, intcs cely)
    {
        TinyPoint pos{celx * 64, cely * 64};
        int num = (m_time / Config::ScaleDiv(1)) % 100;
        if (m_decor[celx][cely - 1].icon == 304 && (num == 0 || num == 7 || num == 18 || num == 25 || num == 33 || num
            == 44) && cely > 0)
        {
            PlaySound(SoundChannel::SoundChannel69, pos);
        }
        if (num % 2 == 0)
        {
            return num < 50;
        }
        return false;
    }

    /**
     * @note Renders in strict back-to-front layers: (1) the tiled parallax background
     *       blitted in a 3x2 grid of 640x480 panels, (2) the big-decor tile layer
     *       (m_bigDecor) culled to the cells overlapping the visible draw bounds, (3) the
     *       foreground decor/objects/Blupi, then (4) HUD and particle/voyage overlays.
     *       Tiles are iterated by converting the scroll offset (posDecor) to a cell range
     *       padded by a one-tile margin so partially-visible edge tiles are drawn.
     * @note MODERN cheat-zoom widens the iterated cell range by zoomExtraX/Y tiles so the
     *       enlarged (zoomed-out) viewport is filled with real tiles instead of empty space.
     * @note Several icons get per-icon vertical nudges (e.g. animated water icon 203 is
     *       resolved through Tables::table_marine for its current frame); these offsets are
     *       cosmetic alignment carried over from the original game.
     */
    void Decor::Build()
    {
        TinyPoint posDecor = DecorNextAction();
        TinyPoint pos{
            posDecor.X * 2 / 3,
            pos.Y = posDecor.Y * 2 / 3
        };
        int num = 1;
        TinyPoint tinyPoint;
        tinyPoint.X = m_drawBounds.Left;
        TinyRect rect{};
        rect.Left = pos.X % 640;
        rect.Right = 640;
#ifdef MODERN
        // When zoomed out, the hotspot transform shrinks all world sprites toward the
        // screen centre.  Extra world tiles must be rendered on all sides so that the
        // enlarged visible area is filled with actual content instead of black/empty space.
        int zoomExtraX = 0;
        int zoomExtraY = 0;
        if (m_cheatZoomFactor > 0.0 && m_cheatZoomFactor < 1.0)
        {
            // How many extra pixels are needed outside each screen edge after the
            // inverse zoom:  extraPx = screenHalf * (1/zoom - 1)
            // Round up to the nearest 64-pixel tile boundary.
            double invScale = 1.0 / m_cheatZoomFactor - 1.0;
            zoomExtraX = static_cast<int>(
                std::ceil(m_drawBounds.getWidthProperty()  * 0.5 * invScale / 64.0));
            zoomExtraY = static_cast<int>(
                std::ceil(m_drawBounds.getHeightProperty() * 0.5 * invScale / 64.0));
        }
#endif
        for (int i = 0; i < 3; i++)
        {
            tinyPoint.Y = m_drawBounds.Top;
            rect.Top = pos.Y % 480;
            rect.Bottom = 480;
            for (int j = 0; j < 2; j++)
            {
                m_pixmap->DrawPart(PixmapChannel::Background, tinyPoint, rect);
                tinyPoint.Y += rect.getHeightProperty() - num;
                rect.Top = 0;
                rect.Bottom = 480;
            }
            tinyPoint.X += rect.getWidthProperty() - num;
            rect.Left = 0;
            rect.Right = 640;
            if (tinyPoint.X > m_drawBounds.Right)
            {
                break;
            }
        }
#ifdef MODERN
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64 - 64 - zoomExtraX * 64;
        for (int i = posDecor.X / 64 - 1 - zoomExtraX; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 3 + zoomExtraX; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64 + 2 - 64 - zoomExtraY * 64;
            for (int j = posDecor.Y / 64 - 1 - zoomExtraY; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2 + zoomExtraY; j++)
#else
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64 - 64;
        for (int i = posDecor.X / 64 - 1; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 3; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64 + 2 - 64;
            for (int j = posDecor.Y / 64 - 1; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2; j++)
#endif
            {
                if (i >= 0 && i < 100 && j >= 0 && j < 100)
                {
                    int num2 = m_bigDecor[i][j].icon;
                    PixmapChannel channel = PixmapChannel::Explosion;
                    if (num2 != -1)
                    {
                        pos.X = tinyPoint.X;
                        pos.Y = tinyPoint.Y;
                        if (num2 == 203)
                        {
                            num2 = Tables::table_marine[m_time / Config::ScaleDiv(3) % 11];
                            channel = PixmapChannel::Object;
                        }
                        if (num2 >= 66 && num2 <= 68)
                        {
                            pos.Y -= 13;
                        }
                        if (num2 >= 87 && num2 <= 89)
                        {
                            pos.Y -= 2;
                        }
                        m_pixmap->QuickIcon(channel, num2, pos);
                    }
                }
                tinyPoint.Y += 64;
            }
            tinyPoint.X += 64;
        }
#ifdef MODERN
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64 - zoomExtraX * 64;
        for (int i = posDecor.X / 64 - zoomExtraX; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 2 + zoomExtraX; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64 - zoomExtraY * 64;
            for (int j = posDecor.Y / 64 - zoomExtraY; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2 + zoomExtraY; j++)
#else
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64;
        for (int i = posDecor.X / 64; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 2; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64;
            for (int j = posDecor.Y / 64; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2; j++)
#endif
            {
                if (i >= 0 && i < 100 && j >= 0 && j < 100 && m_decor[i][j].icon != -1)
                {
                    int num2 = m_decor[i][j].icon;
                    if (num2 == 384 || num2 == 385)
                    {
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, tinyPoint);
                    }
                }
                tinyPoint.Y += 64;
            }
            tinyPoint.X += 64;
        }
        m_blupiSec = SecretPower::None;
        if (!m_blupiFront)
        {
            double rotation = 0.0;
            if (m_blupiNage || m_blupiSurf || m_blupiHelico || m_blupiJeep)
            {
                rotation = m_blupiRealRotation;
            }
            tinyPoint.X = m_drawBounds.Left + m_blupiPos.X - posDecor.X;
            tinyPoint.Y = m_drawBounds.Top + m_blupiPos.Y - posDecor.Y;
            if (m_blupiJeep)
            {
                tinyPoint.Y += m_blupiOffsetY;
            }
            if (m_blupiShield)
            {
                m_blupiSec = SecretPower::Shield;
                if (m_blupiTimeShield > 25 || (m_time / Config::ScaleDiv(1)) % 4 < 2)
                {
                    int num2 = Tables::table_shield_blupi[m_time / Config::ScaleDiv(2) % 16];
                    tinyPoint.Y -= 2;
                    m_pixmap->QuickIcon(PixmapChannel::Element, num2, tinyPoint);
                    tinyPoint.Y += 2;
                    num2 = Tables::table_shieldloop[m_time / Config::ScaleDiv(2) % 5];
                    m_pixmap->QuickIcon(PixmapChannel::Element, num2, tinyPoint);
                }
                m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, 1.0, rotation);
            }
            else if (m_blupiPower)
            {
                m_blupiSec = SecretPower::Power;
                if (m_blupiTimeShield > 25 || (m_time / Config::ScaleDiv(1)) % 4 < 2)
                {
                    int num2 = Tables::table_magicloop[m_time / Config::ScaleDiv(2) % 5];
                    m_pixmap->QuickIcon(PixmapChannel::Element, num2, tinyPoint);
                }
                m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, 1.0, rotation);
            }
            else if (m_blupiCloud)
            {
                m_blupiSec = SecretPower::Cloud;
                if (m_blupiTimeShield > 25 || (m_time / Config::ScaleDiv(1)) % 4 < 2)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        int num2 = 48 + (m_time / Config::ScaleDiv(1) + k) % 6;
                        pos.X = tinyPoint.X - 34;
                        pos.Y = tinyPoint.Y - 34;
                        m_pixmap->QuickIcon(PixmapChannel::Explosion, num2, pos);
                    }
                }
                m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, 1.0, rotation);
            }
#ifdef MODERN
            else if (m_blupiGhost)
            {
                static constexpr double kGhostBlupiOpacity = 0.75;
                m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, kGhostBlupiOpacity, rotation);
            }
#endif
            else if (m_blupiHide)
            {
                m_blupiSec = SecretPower::Hide;
                if (m_blupiTimeShield > 25 || (m_time / Config::ScaleDiv(1)) % 4 < 2)
                {
                    m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, 0.3, rotation);
                }
                else
                {
                    m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, 1.0, rotation);
                }
            }
            else
            {
                m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, 1.0, rotation);
            }
        }
        for (int num3 = MAXMOVEOBJECT - 1; num3 >= 0; num3--)
        {
#ifdef MODERN
            if (m_moveObject[num3].type != ObjectType::ObjectType0 && m_moveObject[num3].posCurrent.X >= posDecor.X - 64 - zoomExtraX * 64 && m_moveObject[num3]
                .posCurrent.Y >= posDecor.Y - 64 - zoomExtraY * 64 && m_moveObject[num3].posCurrent.X <= posDecor.X + m_drawBounds.
                getWidthProperty() + zoomExtraX * 64 && m_moveObject[num3].posCurrent.Y <= posDecor.Y + m_drawBounds.getHeightProperty() + zoomExtraY * 64
                && (m_moveObject[num3].type < ObjectType::ObjectType8 || m_moveObject[num3].type > ObjectType::ObjectType11) && (m_moveObject[num3].type < ObjectType::ObjectType90 ||
                    m_moveObject[num3].type > ObjectType::ObjectType95) && (m_moveObject[num3].type < ObjectType::ObjectType98 || m_moveObject[num3].type > ObjectType::ObjectType100) &&
                m_moveObject[num3].type != ObjectType::ObjectType53 && m_moveObject[num3].type != ObjectType::ObjectType1 && m_moveObject[num3].type != ObjectType::ObjectType47 &&
                m_moveObject[num3].type != ObjectType::ObjectType48)
#else
            if (m_moveObject[num3].type != ObjectType::ObjectType0 && m_moveObject[num3].posCurrent.X >= posDecor.X - 64 && m_moveObject[num3]
                .posCurrent.Y >= posDecor.Y - 64 && m_moveObject[num3].posCurrent.X <= posDecor.X + m_drawBounds.
                getWidthProperty() && m_moveObject[num3].posCurrent.Y <= posDecor.Y + m_drawBounds.getHeightProperty()
                && (m_moveObject[num3].type < ObjectType::ObjectType8 || m_moveObject[num3].type > ObjectType::ObjectType11) && (m_moveObject[num3].type < ObjectType::ObjectType90 ||
                    m_moveObject[num3].type > ObjectType::ObjectType95) && (m_moveObject[num3].type < ObjectType::ObjectType98 || m_moveObject[num3].type > ObjectType::ObjectType100) &&
                m_moveObject[num3].type != ObjectType::ObjectType53 && m_moveObject[num3].type != ObjectType::ObjectType1 && m_moveObject[num3].type != ObjectType::ObjectType47 &&
                m_moveObject[num3].type != ObjectType::ObjectType48)
#endif
            {
                tinyPoint.X = m_drawBounds.Left + m_moveObject[num3].posCurrent.X - posDecor.X;
                tinyPoint.Y = m_drawBounds.Top + m_moveObject[num3].posCurrent.Y - posDecor.Y;
                if (m_moveObject[num3].type == ObjectType::ObjectType4 || m_moveObject[num3].type == ObjectType::ObjectType32 || m_moveObject[num3].type == ObjectType::ObjectType33)
                {
                    tinyPoint.X += 2;
                    tinyPoint.Y += BLUPIOFFY;
                }
                if (m_moveObject[num3].type == ObjectType::ObjectType54)
                {
                    tinyPoint.Y += BLUPIOFFY;
                }
                double opacity = 1.0;
                if (m_moveObject[num3].type == ObjectType::ObjectType58)
                {
                    opacity = (double)(20 - m_moveObject[num3].phase) * 0.3 / 20.0;
                }
                m_pixmap->QuickIcon(m_moveObject[num3].channel, m_moveObject[num3].icon, tinyPoint, opacity, 0.0);
                if (m_moveObject[num3].type == ObjectType::ObjectType30)
                {
                    for (int l = 0; l < Tables::table_drinkoffsetLength; l++)
                    {
                        int num4 = (m_time / Config::ScaleDiv(1) + Tables::table_drinkoffset[l]) % 50;
                        int rank = Tables::table_drinkeffect[num4 % 5];
                        TinyPoint tinyPoint2;
                        tinyPoint2.X = tinyPoint.X + 2;
                        tinyPoint2.Y = tinyPoint.Y - num4 * 3;
                        TinyPoint pos2 = tinyPoint2;
                        double opacity2 = (50.0 - (double)num4) / 50.0;
                        m_pixmap->QuickIcon(PixmapChannel::Element, rank, pos2, opacity2, 0.0);
                    }
                }
                if (m_bDrawSecret && m_moveObject[num3].type == ObjectType::ObjectType12 && m_moveObject[num3].icon != 32 && m_moveObject[
                    num3].icon != 33 && m_moveObject[num3].icon != 34)
                {
                    m_pixmap->QuickIcon(PixmapChannel::Object, 214, tinyPoint);
                }
            }
        }
#ifdef MODERN
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64 - zoomExtraX * 64;
        for (int i = posDecor.X / 64 - zoomExtraX; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 2 + zoomExtraX; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64 - zoomExtraY * 64;
            for (int j = posDecor.Y / 64 - zoomExtraY; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2 + zoomExtraY; j++)
#else
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64;
        for (int i = posDecor.X / 64; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 2; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64;
            for (int j = posDecor.Y / 64; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2; j++)
#endif
            {
                if (i >= 0 && i < 100 && j >= 0 && j < 100 && m_decor[i][j].icon != -1)
                {
                    int num2 = m_decor[i][j].icon;
                    pos.X = tinyPoint.X;
                    pos.Y = tinyPoint.Y;
                    if ((num2 >= 107 && num2 <= 109) || num2 == 157)
                    {
                        pos.Y -= 7;
                    }
                    if (num2 == 211)
                    {
                        num2 = Tables::table_ressort[(m_time / Config::ScaleDiv(2) + i * 7) % 8];
                    }
                    if (num2 == 214 && !m_bDrawSecret)
                    {
                        num2 = -1;
                    }
                    if (num2 == 364)
                    {
                        pos.Y -= 2;
                    }
                    switch (num2)
                    {
                    default:
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                        break;
                    case 68:
                    case 91:
                    case 92:
                    case 110:
                    case 111:
                    case 112:
                    case 113:
                    case 114:
                    case 115:
                    case 116:
                    case 117:
                    case 118:
                    case 119:
                    case 120:
                    case 121:
                    case 122:
                    case 123:
                    case 124:
                    case 125:
                    case 126:
                    case 127:
                    case 128:
                    case 129:
                    case 130:
                    case 131:
                    case 132:
                    case 133:
                    case 134:
                    case 135:
                    case 136:
                    case 137:
                    case 305:
                    case 317:
                    case 324:
                    case 373:
                    case 378:
                    case 384:
                    case 385:
                    case 404:
                    case 410:
                        break;
                    }
                }
                tinyPoint.Y += 64;
            }
            tinyPoint.X += 64;
        }
        for (int num3 = 0; num3 < MAXMOVEOBJECT; num3++)
        {
#ifdef MODERN
            if ((m_moveObject[num3].type == ObjectType::ObjectType1 || m_moveObject[num3].type == ObjectType::ObjectType47 || m_moveObject[num3].type == ObjectType::ObjectType48) &&
                m_moveObject[num3].posCurrent.X >= posDecor.X - 64 - zoomExtraX * 64 && m_moveObject[num3].posCurrent.Y >= posDecor.Y - 64 - zoomExtraY * 64
                && m_moveObject[num3].posCurrent.X <= posDecor.X + m_drawBounds.getWidthProperty() + zoomExtraX * 64 && m_moveObject[num3]
                .posCurrent.Y <= posDecor.Y + m_drawBounds.getHeightProperty() + zoomExtraY * 64)
#else
            if ((m_moveObject[num3].type == ObjectType::ObjectType1 || m_moveObject[num3].type == ObjectType::ObjectType47 || m_moveObject[num3].type == ObjectType::ObjectType48) &&
                m_moveObject[num3].posCurrent.X >= posDecor.X - 64 && m_moveObject[num3].posCurrent.Y >= posDecor.Y - 64
                && m_moveObject[num3].posCurrent.X <= posDecor.X + m_drawBounds.getWidthProperty() && m_moveObject[num3]
                .posCurrent.Y <= posDecor.Y + m_drawBounds.getHeightProperty())
#endif
            {
                tinyPoint.X = m_drawBounds.Left + m_moveObject[num3].posCurrent.X - posDecor.X;
                tinyPoint.Y = m_drawBounds.Top + m_moveObject[num3].posCurrent.Y - posDecor.Y;
                m_pixmap->QuickIcon(m_moveObject[num3].channel, m_moveObject[num3].icon, tinyPoint);
            }
        }
#ifdef MODERN
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64 - zoomExtraX * 64;
        for (int i = posDecor.X / 64 - zoomExtraX; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 2 + zoomExtraX; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64 - zoomExtraY * 64;
            for (int j = posDecor.Y / 64 - zoomExtraY; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2 + zoomExtraY; j++)
#else
        tinyPoint.X = m_drawBounds.Left - posDecor.X % 64;
        for (int i = posDecor.X / 64; i < posDecor.X / 64 + m_drawBounds.getWidthProperty() / 64 + 2; i++)
        {
            tinyPoint.Y = m_drawBounds.Top - posDecor.Y % 64;
            for (int j = posDecor.Y / 64; j < posDecor.Y / 64 + m_drawBounds.getHeightProperty() / 64 + 2; j++)
#endif
            {
                if (i >= 0 && i < 100 && j >= 0 && j < 100 && m_decor[i][j].icon != -1)
                {
                    int num2 = m_decor[i][j].icon;
                    pos = tinyPoint;
                    if (num2 == 68)
                    {
                        num2 = Tables::table_decor_lave[(i * 13 + j * 7 + m_time / Config::ScaleDiv(2)) % 8];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 373)
                    {
                        num2 = ((!m_blupiFocus)
                                    ? Tables::table_decor_piege2[(i * 13 + j * 7 + m_time / Config::ScaleDiv(2)) % 4]
                                    : Tables::table_decor_piege1[(i * 13 + j * 7 + m_time / Config::ScaleDiv(4)) % 16]);
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 404 || num2 == 410)
                    {
                        num2 = Tables::table_decor_goutte[(i * 13 + j * 7 + m_time / Config::ScaleDiv(2)) % 48];
                        pos.Y -= 9;
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                        if (num2 >= 404 && num2 <= 407)
                        {
                            m_decor[i][j].icon = 404;
                        }
                        else
                        {
                            m_decor[i][j].icon = 410;
                        }
                    }
                    if (num2 == 317)
                    {
                        num2 = Tables::table_decor_ecraseur[m_time / Config::ScaleDiv(3) % 10];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 378)
                    {
                        num2 = Tables::table_decor_scie[(i * 13 + j * 7 + m_time / Config::ScaleDiv(1)) % 6];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 324)
                    {
                        num2 = Tables::table_decor_temp[m_time / Config::ScaleDiv(4) % 20];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 92)
                    {
                        num2 = Tables::table_decor_eau1[(i * 13 + j * 7 + m_time / Config::ScaleDiv(3)) % 6];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 91)
                    {
                        int num5 = 3 + (i * 17 + j * 13) % 3;
                        num2 = Tables::table_decor_eau2[(i * 11 + j * 7 + m_time / Config::ScaleDiv(num5)) % 6];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 305 && BlitzActif(i, j))
                    {
                        num2 = m_random.get()->Next(305, 308);
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 110)
                    {
                        num2 = Tables::table_decor_ventg[m_time / Config::ScaleDiv(1) % 4];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 114)
                    {
                        num2 = Tables::table_decor_ventd[m_time / Config::ScaleDiv(1) % 4];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 118)
                    {
                        num2 = Tables::table_decor_venth[m_time / Config::ScaleDiv(1) % 4];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 122)
                    {
                        num2 = Tables::table_decor_ventb[m_time / Config::ScaleDiv(1) % 4];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 126)
                    {
                        num2 = Tables::table_decor_ventillog[m_time / Config::ScaleDiv(2) % 3];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 129)
                    {
                        num2 = Tables::table_decor_ventillod[m_time / Config::ScaleDiv(2) % 3];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 132)
                    {
                        num2 = Tables::table_decor_ventilloh[m_time / Config::ScaleDiv(2) % 3];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                    if (num2 == 135)
                    {
                        num2 = Tables::table_decor_ventillob[m_time / Config::ScaleDiv(2) % 3];
                        m_pixmap->QuickIcon(PixmapChannel::Object, num2, pos);
                    }
                }
                tinyPoint.Y += 64;
            }
            tinyPoint.X += 64;
        }
        ByeByeDraw(posDecor);
        for (int num3 = 0; num3 < MAXMOVEOBJECT; num3++)
        {
#ifdef MODERN
            if (m_moveObject[num3].type != ObjectType::ObjectType0 && m_moveObject[num3].posCurrent.X >= posDecor.X - 64 - zoomExtraX * 64 && m_moveObject[num3]
                .posCurrent.Y >= posDecor.Y - 64 - zoomExtraY * 64 && m_moveObject[num3].posCurrent.X <= posDecor.X + m_drawBounds.
                getWidthProperty() + zoomExtraX * 64 && m_moveObject[num3].posCurrent.Y <= posDecor.Y + m_drawBounds.getHeightProperty() + zoomExtraY * 64
                && ((m_moveObject[num3].type >= ObjectType::ObjectType8 && m_moveObject[num3].type <= ObjectType::ObjectType11) || (m_moveObject[num3].type >= ObjectType::ObjectType90 &&
                        m_moveObject[num3].type <= ObjectType::ObjectType95) || (m_moveObject[num3].type >= ObjectType::ObjectType98 && m_moveObject[num3].type <=
                        ObjectType::ObjectType100)
                    || m_moveObject[num3].type == ObjectType::ObjectType53))
#else
            if (m_moveObject[num3].type != ObjectType::ObjectType0 && m_moveObject[num3].posCurrent.X >= posDecor.X - 64 && m_moveObject[num3]
                .posCurrent.Y >= posDecor.Y - 64 && m_moveObject[num3].posCurrent.X <= posDecor.X + m_drawBounds.
                getWidthProperty() && m_moveObject[num3].posCurrent.Y <= posDecor.Y + m_drawBounds.getHeightProperty()
                && ((m_moveObject[num3].type >= ObjectType::ObjectType8 && m_moveObject[num3].type <= ObjectType::ObjectType11) || (m_moveObject[num3].type >= ObjectType::ObjectType90 &&
                        m_moveObject[num3].type <= ObjectType::ObjectType95) || (m_moveObject[num3].type >= ObjectType::ObjectType98 && m_moveObject[num3].type <=
                        ObjectType::ObjectType100)
                    || m_moveObject[num3].type == ObjectType::ObjectType53))
#endif
            {
                tinyPoint.X = m_drawBounds.Left + m_moveObject[num3].posCurrent.X - posDecor.X;
                tinyPoint.Y = m_drawBounds.Top + m_moveObject[num3].posCurrent.Y - posDecor.Y;
                m_pixmap->QuickIcon(m_moveObject[num3].channel, m_moveObject[num3].icon, tinyPoint);
            }
        }
        if (m_blupiFront)
        {
            tinyPoint.X = m_drawBounds.Left + m_blupiPos.X - posDecor.X;
            tinyPoint.Y = m_drawBounds.Top + m_blupiPos.Y - posDecor.Y;
#ifdef MODERN
            if (m_blupiGhost)
            {
                static constexpr double kGhostBlupiOpacity = 0.75;
                m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint, kGhostBlupiOpacity, 0.0);
            }
            else
#endif
            m_pixmap->QuickIcon(m_blupiChannel, m_blupiIcon, tinyPoint);
        }
        DrawInfo();
        VoyageDraw();
        m_time++;
    }

    /**
     * @note Lays out HUD glyphs at fixed screen coordinates: a row of life icons, a row
     *       of bullet pips, follower/dynamite/key indicators (keys gated by the m_blupiCle
     *       bit flags), and the treasure counter (suppressed on the hub/world-select levels
     *       where m_mission % 10 == 0). The two gauges are drawn only when not hidden.
     * @note For the training missions (11-14) it picks the matching tutorial hint table and
     *       walks its 6-field records {x0,x1,y0,y1,condition,stringId}: when Blupi's tile is
     *       inside the box and IsDisplayInfo(condition) holds, the localized hint string is
     *       shown, auto-scaled down to fit 640px and offset by 10000 when accelerometer
     *       controls are active (to select the accelerometer-worded variant).
     */
    void Decor::DrawInfo()
    {
        TinyPoint pos;
        pos.X = 210;
        pos.Y = 417;
        for (int i = 0; i < m_nbVies; i++)
        {
            m_pixmap->HudIcon(PixmapChannel::Blupi, 48, pos);
            pos.X += 16;
        }
        pos.X = 570;
        pos.Y = 442;
        for (int i = 0; i < m_blupiBullet; i++)
        {
            m_pixmap->HudIcon(PixmapChannel::Element, 176, pos);
            pos.X += 4;
        }
        if (m_blupiPerso > 0)
        {
            pos.X = 0;
            pos.Y = 438;
            m_pixmap->HudIcon(PixmapChannel::Button, 108, pos);
            string text = Helper::formatString("= {0}", STRING_VECTOR(std::to_string(m_blupiPerso)));
            pos.X = 32;
            pos.Y = 452;
            Text::DrawText(*m_pixmap, pos, text, 0.7);
        }
        if (m_blupiDynamite > 0)
        {
            pos.X = 505;
            pos.Y = 414;
            m_pixmap->HudIcon(PixmapChannel::Element, 252, pos);
        }
        if ((m_blupiCle & DoorKeyFlags::Key1) != DoorKeyFlags::None)
        {
            pos.X = 520;
            pos.Y = 418;
            m_pixmap->HudIcon(PixmapChannel::Element, 215, pos);
        }
        if ((m_blupiCle & DoorKeyFlags::Key2) != DoorKeyFlags::None)
        {
            pos.X = 530;
            pos.Y = 418;
            m_pixmap->HudIcon(PixmapChannel::Element, 222, pos);
        }
        if ((m_blupiCle & DoorKeyFlags::Key3) != DoorKeyFlags::None)
        {
            pos.X = 540;
            pos.Y = 418;
            m_pixmap->HudIcon(PixmapChannel::Element, 229, pos);
        }
        if ((m_mission != 1 && m_mission % 10 != 0) || m_bPrivate)
        {
            TinyRect tinyRect = TinyRect();
            tinyRect.Left = 410 + m_pixmap->getOriginProperty().X;
            tinyRect.Right = 510 + m_pixmap->getOriginProperty().X;
            tinyRect.Top = 445;
            tinyRect.Bottom = 480;
            TinyRect rect = tinyRect;
            m_pixmap->DrawIcon(PixmapChannel::Pad, 15, rect, 0.6, false);
            string text = Helper::formatString("{0}/{1}", std::vector{TO_STRING(m_nbTresor), TO_STRING(m_totalTresor)});
            pos.X = 460;
            pos.Y = 450;
            Text::DrawTextCenter(*m_pixmap, pos, text, 1.0);
        }
        for (int i = 0; i < 2; i++)
        {
            if (!m_jauges[i].GetHide())
            {
                m_jauges[i].Draw();
            }
        }

        short* array = nullptr;
        if (m_mission == 11)
        {
            array = Tables::table_training1;
        }
        if (m_mission == 12)
        {
            array = Tables::table_training2;
        }
        if (m_mission == 13)
        {
            array = Tables::table_training3;
        }
        if (m_mission == 14)
        {
            array = Tables::table_training4;
        }
        if (array == nullptr || m_bPrivate)
        {
            return;
        }
        int num = (m_blupiPos.X + 30) / 64;
        int num2 = (m_blupiPos.Y + 30) / 64;
        for (int i = 0; array[i] != -1; i += 6)
        {
            if (num >= array[i] && num <= array[i + 1] && num2 >= array[i + 2] && num2 <= array[i + 3] && IsDisplayInfo(
                array[i + 4]))
            {
                int num3 = 0;
                if (m_gameData->getAccelActiveProperty())
                {
                    num3 = 10000;
                }
                string text = MyResource::LoadString(array[i + 5] + num3);
                if (text != "")
                {
                    TinyRect drawBounds = m_pixmap->getDrawBoundsProperty();
                    TinyRect tinyRect2{};
                    tinyRect2.Left = 0;
                    tinyRect2.Right = drawBounds.getWidthProperty();
                    tinyRect2.Top = 0;
                    tinyRect2.Bottom = 40;
                    TinyRect rect2 = tinyRect2;
                    m_pixmap->DrawIcon(PixmapChannel::Pad, 15, rect2, 1.0, false);
                    double num4 = Text::GetTextWidth(text, 1.0);
                    double num5 = std::min(640.0 / num4, 1.0);
                    pos.X = 320;
                    pos.Y = 5 + (int)((1.0 - num5) * 35.0 * 0.6);
                    Text::DrawTextCenter(*m_pixmap, pos, text, num5);
                }
                break;
            }
        }
    }

    bool Decor::IsDisplayInfo(int tableTresor)
    {
        if (tableTresor >= 0)
        {
            return m_nbTresor == tableTresor;
        }
        switch (tableTresor)
        {
        case -2:
            if (!m_blupiHelico && !m_blupiSkate && !m_blupiTank)
            {
                return !m_blupiJeep;
            }
            return false;
        case -3:
            if (!m_blupiHelico && !m_blupiSkate && !m_blupiTank)
            {
                return m_blupiJeep;
            }
            return true;
        case -4:
            return m_blupiDynamite == 0;
        case -5:
            return m_blupiDynamite > 0;
        default:
            return true;
        }
    }

    /**
     * @note Drives the screen-shake / forced-pan effect. Tables::table_decor_action is a
     *       flat record list: each record is {actionId, frameCount, (dx,dy) per frame...}.
     *       It walks records by their variable stride until it finds the active
     *       m_decorAction, then returns the base scroll offset displaced by 3x the current
     *       frame's (dx,dy), advancing m_decorPhase. When the frames are exhausted the
     *       action is cleared back to None. The returned offset is clamped to the level
     *       bounds (6400 minus the draw width/height).
     * @note Returns m_posDecor unchanged when no action is active or the game is paused;
     *       this function does not mutate m_posDecor itself, only the returned copy.
     */
    TinyPoint Decor::DecorNextAction()
    {
        int i = 0;
        if (m_decorAction == DecorAction::None || m_bPause)
        {
            return m_posDecor;
        }
        TinyPoint posDecor = m_posDecor;
        for (; Tables::table_decor_action[i] != 0; i += 2 + Tables::table_decor_action[i + 1] * 2)
        {
            if (ToRaw(m_decorAction) != Tables::table_decor_action[i])
            {
                continue;
            }
            if (m_decorPhase < Tables::table_decor_action[i + 1])
            {
                posDecor.X += 3 * Tables::table_decor_action[i + 2 + m_decorPhase * 2];
                posDecor.Y += 3 * Tables::table_decor_action[i + 2 + m_decorPhase * 2 + 1];
                int num = ((m_dimDecor.X != 0) ? (6400 - m_drawBounds.getWidthProperty()) : 0);
                if (posDecor.X < 0)
                {
                    posDecor.X = 0;
                }
                if (posDecor.X > num)
                {
                    posDecor.X = num;
                }
                num = ((m_dimDecor.Y != 0) ? (6400 - m_drawBounds.getHeightProperty()) : 0);
                if (posDecor.Y < 0)
                {
                    posDecor.Y = 0;
                }
                if (posDecor.Y > num)
                {
                    posDecor.Y = num;
                }
                m_decorPhase++;
            }
            else
            {
                m_decorAction = DecorAction::None;
            }
            break;
        }
        return posDecor;
    }

    void Decor::SetSpeedX(double speed)
    {
        if (m_blupiInvert)
        {
            speed = 0.0 - speed;
        }
        m_blupiSpeedX = speed;
    }

    void Decor::SetSpeedY(double speed)
    {
        m_blupiSpeedY = speed;
    }

    void Decor::KeyChange(int keyPress)
    {
        m_keyPress = keyPress;
    }

    void Decor::GetBlupiInfo(bool& bHelico, bool& bJeep, bool& bSkate, bool& bNage)
    {
        bHelico = m_blupiHelico;
        bJeep = m_blupiJeep | m_blupiTank;
        bSkate = m_blupiSkate;
        bNage = m_blupiNage | m_blupiSurf;
    }

    /**
     * @note Maps the two generic footstep channels (SoundChannel3/4) to a surface-specific
     *       variant by classifying the obstacle (tile) icon into material ranges (grass,
     *       stone, wood, metal, sand, etc.). Each material range routes to a dedicated pair
     *       of footstep sounds. Any channel other than 3/4, or an icon outside every range,
     *       is returned unchanged.
     */
    SoundChannel Decor::SoundEnviron(SoundChannel sound, int obstacle)
    {
        if ((obstacle >= 32 && obstacle <= 34) || (obstacle >= 41 && obstacle <= 47) || (obstacle >= 139 && obstacle <=
            143))
        {
            switch (sound)
            {
            case SoundChannel::SoundChannel4:
                return SoundChannel::SoundChannel79;
            case SoundChannel::SoundChannel3:
                return SoundChannel::SoundChannel78;
            }
        }
        if ((obstacle >= 1 && obstacle <= 28) || (obstacle >= 78 && obstacle <= 90) || (obstacle >= 250 && obstacle <=
            260) || (obstacle >= 311 && obstacle <= 316) || (obstacle >= 324 && obstacle <= 329))
        {
            switch (sound)
            {
            case SoundChannel::SoundChannel4:
                return SoundChannel::SoundChannel81;
            case SoundChannel::SoundChannel3:
                return SoundChannel::SoundChannel80;
            }
        }
        if ((obstacle >= 284 && obstacle <= 303) || obstacle == 338)
        {
            switch (sound)
            {
            case SoundChannel::SoundChannel4:
                return SoundChannel::SoundChannel83;
            case SoundChannel::SoundChannel3:
                return SoundChannel::SoundChannel82;
            }
        }
        if (obstacle >= 341 && obstacle <= 363)
        {
            switch (sound)
            {
            case SoundChannel::SoundChannel4:
                return SoundChannel::SoundChannel85;
            case SoundChannel::SoundChannel3:
                return SoundChannel::SoundChannel84;
            }
        }
        if (obstacle >= 215 && obstacle <= 234)
        {
            switch (sound)
            {
            case SoundChannel::SoundChannel4:
                return SoundChannel::SoundChannel87;
            case SoundChannel::SoundChannel3:
                return SoundChannel::SoundChannel86;
            }
        }
        if (obstacle >= 246 && obstacle <= 249)
        {
            switch (sound)
            {
            case SoundChannel::SoundChannel4:
                return SoundChannel::SoundChannel89;
            case SoundChannel::SoundChannel3:
                return SoundChannel::SoundChannel88;
            }
        }
        if (obstacle >= 107 && obstacle <= 109)
        {
            switch (sound)
            {
            case SoundChannel::SoundChannel4:
                return SoundChannel::SoundChannel91;
            case SoundChannel::SoundChannel3:
                return SoundChannel::SoundChannel90;
            }
        }
        return sound;
    }

    /**
     * @note When the Hide secret power is active, Blupi's own action/footstep/effect sounds
     *       are suppressed: the large disjunction lists every "Blupi-produced" channel id
     *       and the sound only plays if it is NOT one of them (so ambient/environment
     *       sounds still play while hidden). Panning is computed by subtracting the scroll
     *       origin so the position becomes screen-relative before forwarding to ISound.
     */
    void Decor::PlaySound(SoundChannel sound, TinyPoint pos)
    {
        int ranSound = ToRaw(sound);
        if (!m_blupiHide || (
            ranSound != 1 &&
             ranSound != 2 &&
             ranSound != 3 &&
             ranSound != 4 &&
             ranSound != 5 &&
             ranSound != 6 &&
             ranSound != 7 &&
             ranSound != 20 &&
             ranSound != 21 &&
             ranSound != 22 &&
             ranSound != 23 &&
             ranSound != 24 &&
             ranSound != 25 &&
             ranSound != 27 &&
             ranSound != 32 &&
             ranSound != 34 &&
             ranSound != 35 &&
             ranSound != 36 &&
             ranSound != 37 &&
             ranSound != 38 &&
             ranSound != 39 &&
             ranSound != 40 &&
             ranSound != 46 &&
             ranSound != 47 &&
             ranSound != 48 &&
             ranSound != 49 &&
             ranSound != 64 &&
             ranSound != 65 &&
            
             ranSound != 78 &&
             ranSound != 79 &&
             ranSound != 80 &&
             ranSound != 81 &&
             ranSound != 82 &&
             ranSound != 83 &&
             ranSound != 84 &&
            
             ranSound != 85 &&
             ranSound != 86 &&
             ranSound != 87 &&
             ranSound != 88 &&
             ranSound != 89 &&
             ranSound != 90 &&
             ranSound != 91)
            )
        {
            pos.X -= m_posDecor.X;
            pos.Y -= m_posDecor.Y;
            m_sound->PlayImage(sound, pos);
        }
    }

    void Decor::StopSound()
    {
        m_blupiMotorSound = SoundChannel::SoundChannel0;
        m_sound->StopAll();
    }

    void Decor::StartSound()
    {
        AdaptMotorVehicleSound();
    }

    void Decor::StopSound(SoundChannel sound)
    {
        m_sound->Stop(sound);
    }

    /**
     * @note Selects the looping motor channel for the current vehicle (helicopter vs.
     *       jeep/tank/overcraft) and pitch (m_blupiMotorHigh chooses the high/low variant).
     *       When the desired loop differs from the one currently playing (m_blupiMotorSound)
     *       it crossfades: plays a one-shot start sound when going from silence to a motor,
     *       a one-shot stop sound when going from a motor to silence, stops the old loop,
     *       then starts the new loop (PlayImage with loop flag) panned to Blupi's screen
     *       position. No-ops when the desired loop already matches the active one.
     */
    void Decor::AdaptMotorVehicleSound()
    {
        SoundChannel num = SoundChannel::SoundChannel0;
        SoundChannel channel = SoundChannel::SoundChannel0;
        SoundChannel channel2 = SoundChannel::SoundChannel0;
        if (m_blupiHelico)
        {
            num = (m_blupiMotorHigh ? SoundChannel::SoundChannel16 : SoundChannel::SoundChannel18);
            channel = SoundChannel::SoundChannel15;
            channel2 = SoundChannel::SoundChannel17;
        }
        else if (m_blupiJeep || m_blupiOver || m_blupiTank)
        {
            num = (m_blupiMotorHigh ? SoundChannel::SoundChannel29 : SoundChannel::SoundChannel31);
            channel = SoundChannel::SoundChannel28;
            channel2 = SoundChannel::SoundChannel30;
        }
        if (m_blupiMotorSound != num)
        {
            TinyPoint blupiPos = m_blupiPos;
            blupiPos.X -= m_posDecor.X;
            blupiPos.Y -= m_posDecor.Y;
            if (m_blupiMotorSound == SoundChannel::SoundChannel0 && num != SoundChannel::SoundChannel0)
            {
                m_sound->PlayImage(channel, blupiPos);
            }
            if (m_blupiMotorSound != SoundChannel::SoundChannel0 && num == SoundChannel::SoundChannel0)
            {
                m_sound->PlayImage(channel2, blupiPos);
            }
            if (m_blupiMotorSound != SoundChannel::SoundChannel0)
            {
                m_sound->Stop(m_blupiMotorSound);
            }
            m_blupiMotorSound = num;
            if (m_blupiMotorSound != SoundChannel::SoundChannel0)
            {
                m_sound->PlayImage(m_blupiMotorSound, blupiPos, -1, true);
            }
        }
    }

    void Decor::PosSound(TinyPoint pos)
    {
        if (m_blupiMotorSound != SoundChannel::SoundChannel0)
        {
            pos.X -= m_posDecor.X;
            pos.Y -= m_posDecor.Y;
            m_sound->PosImage(m_blupiMotorSound, pos);
        }
    }

    int Decor::GetRegion()
    {
        return m_region;
    }

    void Decor::SetRegion(int region)
    {
        m_region = region;
    }

    int Decor::GetMusic()
    {
        return m_music;
    }

    void Decor::SetMusic(int music)
    {
        m_music = music;
    }

    TinyPoint Decor::GetDim()
    {
        return m_dimDecor;
    }

    void Decor::SetDim(TinyPoint dim)
    {
        m_dimDecor = dim;
    }

    int Decor::GetMission()
    {
        return m_mission;
    }

    void Decor::SetMission(int mission)
    {
        m_mission = mission;
    }

    int Decor::GetNbVies()
    {
        return m_nbVies;
    }

    void Decor::SetNbVies(int nbVies)
    {
        m_nbVies = nbVies;
    }

    void Decor::InitializeDoors(GameData& gameData)
    {
        gameData.GetDoors(m_doors);
        const int doorIndex = m_mission + 1;
        if (doorIndex >= 0 && doorIndex < 200)
        {
            log::Debug("Decor::InitializeDoors mission=" + std::to_string(m_mission)
                + " doorIndex=" + std::to_string(doorIndex)
                + " state=" + std::to_string(m_doors[doorIndex]));
        }
        else
        {
            log::Debug("Decor::InitializeDoors mission=" + std::to_string(m_mission)
                + " doorIndex=" + std::to_string(doorIndex)
                + " out_of_range");
        }
    }

    void Decor::MemorizeDoors(GameData& gameData)
    {
        gameData.SetDoors(m_doors);
        const int doorIndex = m_mission + 1;
        if (doorIndex >= 0 && doorIndex < 200)
        {
            log::Debug("Decor::MemorizeDoors mission=" + std::to_string(m_mission)
                + " doorIndex=" + std::to_string(doorIndex)
                + " state=" + std::to_string(m_doors[doorIndex]));
        }
        else
        {
            log::Debug("Decor::MemorizeDoors mission=" + std::to_string(m_mission)
                + " doorIndex=" + std::to_string(doorIndex)
                + " out_of_range");
        }
    }

    string Decor::GetCheatTinyText(Def::ButtonGlyph glyph)
    {
        switch (glyph)
        {
        case Def::ButtonGlyph::Cheat1:
            return "D";
        case Def::ButtonGlyph::Cheat2:
            return "B";
        case Def::ButtonGlyph::Cheat3:
            return "S";
        case Def::ButtonGlyph::Cheat4:
            return "E";
        case Def::ButtonGlyph::Cheat5:
            return "R";
        case Def::ButtonGlyph::Cheat6:
            return "T";
        case Def::ButtonGlyph::Cheat7:
            return "C";
        case Def::ButtonGlyph::Cheat8:
            return "T";
        case Def::ButtonGlyph::Cheat9:
            return "G";
        default:
            return "";
        }
    }

    /**
     * @note One linear if-chain, one branch per cheat code (the codes are not mutually
     *       exclusive in form but the player triggers one at a time). Toggle cheats flip a
     *       bool and re-derive dependent state (e.g. OpenDoors flips m_bCheatDoors then calls
     *       AdaptDoors). CleanAll converts every enemy/hazard object in the pool into an
     *       explosion object (ObjectType8), kicks the small screen-shake, and plays the
     *       explosion sound. The vehicle cheats (Skate, etc.) clear all other vehicle/mode
     *       flags first so the modes stay mutually exclusive, then stop the now-irrelevant
     *       looping motor sounds.
     */
    void Decor::CheatAction(Tables::CheatCodes cheat)
    {
        if (cheat == Tables::CheatCodes::OpenDoors)
        {
            m_bCheatDoors = !m_bCheatDoors;
            AdaptDoors(m_bPrivate);
        }
        if (cheat == Tables::CheatCodes::ShowSecret)
        {
            m_bDrawSecret = !m_bDrawSecret;
        }
        if (cheat == Tables::CheatCodes::SuperBlupi)
        {
            m_bSuperBlupi = !m_bSuperBlupi;
        }
        if (cheat == Tables::CheatCodes::LayEgg)
        {
            m_nbVies = 9;
        }
        if (cheat == Tables::CheatCodes::CleanAll)
        {
            for (int i = 0; i < MAXMOVEOBJECT; i++)
            {
                if (
                    m_moveObject[i].type == ObjectType::ObjectType2 ||
                    m_moveObject[i].type == ObjectType::ObjectType3 ||
                    m_moveObject[i].type == ObjectType::ObjectType96 ||
                    m_moveObject[i].type == ObjectType::ObjectType97 ||
                    m_moveObject[i].type == ObjectType::ObjectType4 ||
                    m_moveObject[i].type == ObjectType::ObjectType16 ||
                    m_moveObject[i].type == ObjectType::ObjectType17 ||
                    m_moveObject[i].type == ObjectType::ObjectType20 ||
                    m_moveObject[i].type == ObjectType::ObjectType44 ||
                    m_moveObject[i].type == ObjectType::ObjectType54 ||
                    m_moveObject[i].type == ObjectType::ObjectType32 ||
                    m_moveObject[i].type == ObjectType::ObjectType33)
                {
                    m_decorAction = DecorAction::SmallShake;
                    m_decorPhase = 0;
                    m_moveObject[i].type = ObjectType::ObjectType8;
                    m_moveObject[i].phase = 0;
                    m_moveObject[i].posCurrent.X -= 34;
                    m_moveObject[i].posCurrent.Y -= 34;
                    m_moveObject[i].posStart = m_moveObject[i].posCurrent;
                    m_moveObject[i].posEnd = m_moveObject[i].posCurrent;
                    MoveObjectStepIcon(i);
                    PlaySound(SoundChannel::SoundChannel10, m_moveObject[i].posCurrent);
                }
            }
        }
        if (cheat == Tables::CheatCodes::Skate)
        {
            m_blupiAir = false;
            m_blupiHelico = false;
            m_blupiOver = false;
            m_blupiJeep = false;
            m_blupiTank = false;
            m_blupiSkate = true;
            m_blupiNage = false;
            m_blupiSurf = false;
            m_blupiVent = false;
            m_blupiSuspend = false;
            StopSound(SoundChannel::SoundChannel16);
            StopSound(SoundChannel::SoundChannel18);
            StopSound(SoundChannel::SoundChannel29);
            StopSound(SoundChannel::SoundChannel31);
        }
        if (cheat == Tables::CheatCodes::Copter)
        {
            m_blupiAir = false;
            m_blupiHelico = true;
            m_blupiOver = false;
            m_blupiJeep = false;
            m_blupiTank = false;
            m_blupiSkate = false;
            m_blupiNage = false;
            m_blupiSurf = false;
            m_blupiVent = false;
            m_blupiSuspend = false;
        }
        if (cheat == Tables::CheatCodes::Jeep)
        {
            m_blupiAir = false;
            m_blupiHelico = false;
            m_blupiOver = false;
            m_blupiJeep = true;
            m_blupiTank = false;
            m_blupiSkate = false;
            m_blupiNage = false;
            m_blupiSurf = false;
            m_blupiVent = false;
            m_blupiSuspend = false;
            m_blupiCloud = false;
            m_blupiHide = false;
        }
        if (cheat == Tables::CheatCodes::AllTreasure)
        {
            for (int i = 0; i < MAXMOVEOBJECT; i++)
            {
                if (m_moveObject[i].type == ObjectType::ObjectType5)
                {
                    m_moveObject[i].type = ObjectType::ObjectType0;
                    m_nbTresor++;
                    OpenDoorsTresor();
                    PlaySound(SoundChannel::SoundChannel11, m_moveObject[i].posCurrent);
                }
            }
        }
        if (cheat == Tables::CheatCodes::EndGoal)
        {
            for (int i = 0; i < MAXMOVEOBJECT; i++)
            {
                if (m_moveObject[i].type != ObjectType::ObjectType7 && m_moveObject[i].type != ObjectType::ObjectType21)
                {
                    continue;
                }
                m_blupiPos = m_moveObject[i].posCurrent;
                if (m_nbTresor >= m_totalTresor)
                {
                    if (m_moveObject[i].type == ObjectType::ObjectType21)
                    {
                        m_bFoundCle = true;
                    }
                    StopSound(SoundChannel::SoundChannel16);
                    StopSound(SoundChannel::SoundChannel18);
                    StopSound(SoundChannel::SoundChannel29);
                    StopSound(SoundChannel::SoundChannel31);
                    PlaySound(SoundChannel::SoundChannel14, m_moveObject[i].posCurrent);
                    m_blupiAction = BlupiAction::Win;
                    m_blupiPhase = 0;
                    m_blupiFocus = false;
                    m_blupiFront = true;
                    m_blupiAir = false;
                    m_blupiHelico = false;
                    m_blupiOver = false;
                    m_blupiJeep = false;
                    m_blupiTank = false;
                    m_blupiSkate = false;
                    m_blupiNage = false;
                    m_blupiSurf = false;
                    m_blupiVent = false;
                    m_blupiSuspend = false;
                    m_blupiShield = false;
                    m_blupiPower = false;
                    m_blupiCloud = false;
                    m_blupiHide = false;
                    m_blupiInvert = false;
                    m_blupiBalloon = false;
                    m_blupiEcrase = false;
                }
                else
                {
                    PlaySound(SoundChannel::SoundChannel13, m_moveObject[i].posCurrent);
                }
                m_goalPhase = 50;
            }
        }
        if (cheat == Tables::CheatCodes::RoundShield)
        {
            PlaySound(SoundChannel::SoundChannel42, m_blupiPos);
            m_blupiShield = true;
            m_blupiPower = false;
            m_blupiCloud = false;
            m_blupiHide = false;
            m_blupiTimeShield = 100;
            m_blupiPosMagic = m_blupiPos;
            m_jauges[1].SetHide(false);
        }
        if (cheat == Tables::CheatCodes::Lollipop)
        {
            m_blupiAction = BlupiAction::Sucette;
            m_blupiPhase = 0;
            m_blupiHelico = false;
            m_blupiOver = false;
            m_blupiJeep = false;
            m_blupiTank = false;
            m_blupiSkate = false;
            m_blupiShield = false;
            m_blupiPower = false;
            m_blupiCloud = false;
            m_blupiHide = false;
            m_blupiFocus = false;
            PlaySound(SoundChannel::SoundChannel50, m_blupiPos);
        }
        if (cheat == Tables::CheatCodes::Bombs)
        {
            m_blupiPerso = 10;
            PlaySound(SoundChannel::SoundChannel60, m_blupiPos);
        }
        if (cheat == Tables::CheatCodes::BirdLime)
        {
            m_blupiBullet = 10;
        }
        if (cheat == Tables::CheatCodes::Tank)
        {
            m_blupiAir = false;
            m_blupiHelico = false;
            m_blupiOver = false;
            m_blupiJeep = false;
            m_blupiTank = true;
            m_blupiSkate = false;
            m_blupiNage = false;
            m_blupiSurf = false;
            m_blupiVent = false;
            m_blupiSuspend = false;
            m_blupiCloud = false;
            m_blupiHide = false;
            if (Config::FPS != Fps::Fps20)
            {
                m_blupiVitesseY = 0.0;
                m_blupiSubPixelY = 0.0;
            }
        }
        if (cheat == Tables::CheatCodes::PowerCharge)
        {
            m_blupiAction = BlupiAction::Charge;
            m_blupiPhase = 0;
            m_blupiHelico = false;
            m_blupiOver = false;
            m_blupiJeep = false;
            m_blupiTank = false;
            m_blupiSkate = false;
            m_blupiShield = false;
            m_blupiPower = false;
            m_blupiCloud = false;
            m_blupiHide = false;
            m_blupiJumpAie = false;
            m_blupiFocus = false;
            PlaySound(SoundChannel::SoundChannel58, m_blupiPos);
        }
        if (cheat == Tables::CheatCodes::Drink)
        {
            m_blupiAction = BlupiAction::Drink;
            m_blupiPhase = 0;
            m_blupiHelico = false;
            m_blupiOver = false;
            m_blupiJeep = false;
            m_blupiTank = false;
            m_blupiSkate = false;
            m_blupiShield = false;
            m_blupiPower = false;
            m_blupiCloud = false;
            m_blupiHide = false;
            m_blupiJumpAie = false;
            m_blupiFocus = false;
            PlaySound(SoundChannel::SoundChannel57, m_blupiPos);
        }
        if (cheat == Tables::CheatCodes::Overcraft)
        {
            m_blupiAir = false;
            m_blupiHelico = false;
            m_blupiOver = true;
            m_blupiJeep = false;
            m_blupiTank = false;
            m_blupiSkate = false;
            m_blupiNage = false;
            m_blupiSurf = false;
            m_blupiVent = false;
            m_blupiSuspend = false;
        }
        if (cheat == Tables::CheatCodes::Dynamite)
        {
            m_blupiDynamite = 1;
            PlaySound(SoundChannel::SoundChannel60, m_blupiPos);
        }
        if (cheat == Tables::CheatCodes::WeelKeys)
        {
            m_blupiCle = m_blupiCle | DoorKeyFlags::All;
        }
#ifdef MODERN
        if (cheat == Tables::CheatCodes::Ghost)
        {
            if (!m_blupiGhost)
            {
                m_blupiGhost = true;
                m_blupiHelico = false;
                m_blupiOver = false;
                m_blupiJeep = false;
                m_blupiTank = false;
                m_blupiSkate = false;
                m_blupiNage = false;
                m_blupiSurf = false;
                m_blupiSuspend = false;
                m_blupiAir = false;
                m_blupiVitesseX = 0.0;
                m_blupiVitesseY = 0.0;
                m_blupiSubPixelX = 0.0;
                m_blupiSubPixelY = 0.0;
                m_blupiVector = TinyPoint();
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            else
            {
                if (!DecorDetect(BlupiRect(m_blupiPos)))
                {
                    m_blupiGhost = false;
                    m_blupiVitesseX = 0.0;
                    m_blupiVitesseY = 0.0;
                    m_blupiSubPixelX = 0.0;
                    m_blupiSubPixelY = 0.0;
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                    m_blupiAir = !BlupiIsGround();
                }
            }
        }
#endif
        if (!m_blupiShield && !m_blupiHide && !m_blupiCloud && !m_blupiPower)
        {
            m_jauges[1].SetHide(true);
        }
        if (!m_blupiHelico && !m_blupiOver)
        {
            StopSound(SoundChannel::SoundChannel16);
            StopSound(SoundChannel::SoundChannel18);
        }
        if (!m_blupiJeep && !m_blupiTank)
        {
            StopSound(SoundChannel::SoundChannel29);
            StopSound(SoundChannel::SoundChannel31);
        }
    }

    void Decor::SetBuildOfficialMissions(bool bMode)
    {
        m_buildOfficialMissions = bMode;
    }

    /**
     * @note Resolves the sprite icon (m_blupiIcon) and channel (m_blupiChannel) for the
     *       current frame in three stages:
     *       1. Action remapping: the base m_blupiAction is rewritten into a mode-specific
     *          variant for whichever vehicle/state flag is set (helico/over/jeep/tank/skate/
     *          nage/surf/suspend/balloon/ecrase/vent). These blocks are hand-coded per mode
     *          and also compute m_blupiLogicRotation/m_blupiRealRotation for the rotating
     *          modes (swim direction, surf bob, helicopter tilt).
     *       2. Table lookup: Tables::table_blupi is a record list {actionId, frameCount,
     *          holdFrame, icon0..iconN}; it scans by stride to the matching action and picks
     *          the frame from the phase counter (num6), clamping at holdFrame for one-shot
     *          animations and wrapping by frameCount for looping ones. Slow-walk phases are
     *          halved so the walk cycle looks right at low speed.
     *       3. Direction/mirror fix-up: left-facing icons are mapped through Tables::table_mirror
     *          (with a few suspend-pose special cases), and certain effect actions switch the
     *          channel to Element instead of Blupi.
     * @note Side effect: triggers ambient idle sounds (SoundChannel37/36) and surf splash
     *       particles (MoveObjectTiplouf) at specific phase values, so this is not a pure
     *       icon lookup.
     */
    void Decor::BlupiSearchIcon()
    {
        int i = 0;
        int num = 2;
        BlupiAction blupi_action = m_blupiAction;
        if (m_blupiVent && !m_blupiHelico && !m_blupiOver)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::Vertigo;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::Push;
            }
        }
        if (m_blupiHelico)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopHelico;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchHelico;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnHelico;
            }
            if (blupi_action == BlupiAction::Advance)
            {
                blupi_action = BlupiAction::StopHelico;
            }
            if (blupi_action == BlupiAction::Recede)
            {
                blupi_action = BlupiAction::StopHelico;
            }
            m_blupiRealRotation = (int)(m_blupiVitesseX * 2.0);
        }
        if (m_blupiOver)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopOver;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchOver;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnOver;
            }
            if (blupi_action == BlupiAction::Advance)
            {
                blupi_action = BlupiAction::StopOver;
            }
            if (blupi_action == BlupiAction::Recede)
            {
                blupi_action = BlupiAction::StopOver;
            }
        }
        if (m_blupiJeep)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopJeep;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchJeep;
            }
            if (blupi_action == BlupiAction::Advance)
            {
                blupi_action = BlupiAction::MarchJeep;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnJeep;
            }
        }
        if (m_blupiTank)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopTank;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchTank;
            }
            if (blupi_action == BlupiAction::Advance)
            {
                blupi_action = BlupiAction::MarchTank;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnTank;
            }
        }
        if (m_blupiSkate)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopSkate;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchSkate;
            }
            if (blupi_action == BlupiAction::Advance)
            {
                blupi_action = BlupiAction::MarchSkate;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnSkate;
            }
            if (blupi_action == BlupiAction::TurnAir)
            {
                blupi_action = BlupiAction::TurnSkate;
            }
            if (blupi_action == BlupiAction::Jump)
            {
                blupi_action = BlupiAction::JumpSkate;
            }
            if (blupi_action == BlupiAction::Air)
            {
                blupi_action = BlupiAction::AirSkate;
            }
            if (blupi_action == BlupiAction::MarchSkate && m_blupiSpeedX == 0.0 && std::abs(m_blupiVitesseX) > 5.0)
            {
                blupi_action = BlupiAction::SlowdownSkate;
            }
        }
        if (m_blupiNage)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopNage;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchNage;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnNage;
            }
            int num3 = 0;
            int num4 = 0;
            if (m_blupiPos.X > m_blupiLastPos.X)
            {
                num3 = 1;
            }
            if (m_blupiPos.X < m_blupiLastPos.X)
            {
                num3 = -1;
            }
            if (m_blupiPos.Y > m_blupiLastPos.Y)
            {
                num4 = 1;
            }
            if (m_blupiPos.Y < m_blupiLastPos.Y)
            {
                num4 = -1;
            }
            int num5 = ((num4 >= 0 || num3 != 0)
                            ? ((num4 < 0 && num3 != 0)
                                   ? 45
                                   : ((num4 == 0 && num3 != 0)
                                          ? 90
                                          : ((num4 > 0 && num3 != 0) ? 135 : ((num4 > 0 && num3 == 0) ? 180 : 0))))
                            : 0);
            num5 += 15;
            if (blupi_action == BlupiAction::TurnNage)
            {
                num5 = 90;
            }
            m_blupiLogicRotation = Misc::Approach(m_blupiLogicRotation, num5, 10);
            if (m_blupiDir == Direction::Right)
            {
                m_blupiRealRotation = m_blupiLogicRotation - 90;
            }
            else
            {
                m_blupiRealRotation = 90 - m_blupiLogicRotation;
            }
            m_blupiRealRotation += (int)(std::sin((double)m_time / 6.0) * 20.0);
        }
        if (m_blupiSurf)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopSurf;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchSurf;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnSurf;
            }
            m_blupiLogicRotation = Misc::Approach(m_blupiLogicRotation, 0, 10);
            m_blupiRealRotation = m_blupiLogicRotation;
            m_blupiRealRotation += (int)(std::sin((double)m_time / 10.0) * 10.0);
        }
        if (m_blupiSuspend)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopSuspend;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchSuspend;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::TurnSuspend;
            }
            if (blupi_action == BlupiAction::Jump)
            {
                blupi_action = BlupiAction::JumpSuspend;
            }
        }
        if (m_blupiBalloon)
        {
            blupi_action = BlupiAction::Balloon;
        }
        if (m_blupiEcrase)
        {
            if (blupi_action == BlupiAction::Stop)
            {
                blupi_action = BlupiAction::StopEcrase;
            }
            if (blupi_action == BlupiAction::March)
            {
                blupi_action = BlupiAction::MarchEcrase;
            }
            if (blupi_action == BlupiAction::Turn)
            {
                blupi_action = BlupiAction::MarchEcrase;
            }
        }
        int scaledPhase = m_blupiPhase / Config::ScaleDiv(1);
        if (blupi_action == BlupiAction::Stop && (scaledPhase % 330 == 125 || scaledPhase % 330 == 129 || scaledPhase % 330 == 135 ||
            scaledPhase % 330 == 139 || scaledPhase % 330 == 215 || scaledPhase % 330 == 219 || scaledPhase % 330 ==
            225 || scaledPhase % 330 == 229 || scaledPhase % 330 == 235 || scaledPhase % 330 == 239 || scaledPhase %
            330 == 245 || scaledPhase % 330 == 249 || scaledPhase % 330 == 255 || scaledPhase % 330 == 259 ||
            scaledPhase % 330 == 265 || scaledPhase % 330 == 269))
        {
            PlaySound(SoundChannel::SoundChannel37, m_blupiPos);
        }
        if (blupi_action == BlupiAction::StopSuspend && (scaledPhase % 328 == 118 || scaledPhase % 328 == 230 || scaledPhase % 328 == 278))
        {
            PlaySound(SoundChannel::SoundChannel36, m_blupiPos);
        }
        if ((blupi_action == BlupiAction::StopSurf || blupi_action == BlupiAction::MarchSurf) && scaledPhase % 12 == 0 && m_blupiSurf)
        {
            MoveObjectTiplouf(m_blupiPos);
        }
        int num6 = scaledPhase;
        if (!m_blupiHelico && ((m_blupiSpeedX > 0.1 && m_blupiSpeedX < 0.75) || (m_blupiSpeedX < -0.1 && m_blupiSpeedX >
            -0.75)))
        {
            num6 /= 2;
        }
        for (; Tables::table_blupi[i] != 0; i += Tables::table_blupi[i + 1] + 3)
        {
            if (ToRaw(blupi_action) == Tables::table_blupi[i])
            {
                int num7 = ((Tables::table_blupi[i + 2] == 0 || num6 <= Tables::table_blupi[i + 2])
                                ? (num6 % Tables::table_blupi[i + 1])
                                : Tables::table_blupi[i + 2]);
                num = Tables::table_blupi[i + 3 + num7];
                break;
            }
        }
        if (blupi_action == BlupiAction::Clear1 || blupi_action == BlupiAction::Clear2 || blupi_action == BlupiAction::Clear3 || blupi_action == BlupiAction::Glu || (blupi_action == BlupiAction::Electro && num < 266))
        {
            m_blupiChannel = PixmapChannel::Element;
        }
        else
        {
            m_blupiChannel = PixmapChannel::Blupi;
        }
        Direction num8 = m_blupiDir;
        if (m_blupiInvert)
        {
            if (m_blupiDir == Direction::Right)
            {
                num8 = Direction::Left;
            }
            if (m_blupiDir == Direction::Left)
            {
                num8 = Direction::Right;
            }
        }
        if (num8 == Direction::Left && m_blupiChannel == PixmapChannel::Blupi)
        {
            if (blupi_action == BlupiAction::StopSuspend)
            {
                if (num == 144)
                {
                    num = 158;
                }
                if (num == 143)
                {
                    num = 145;
                }
                if (num == 151)
                {
                    num = 146;
                }
            }
            if (num >= 0 && num < 335)
            {
                num = Tables::table_mirror[num];
            }
        }
        if (num8 == Direction::Left && m_blupiChannel == PixmapChannel::Element && num >= 168 && num <= 171)
        {
            num += 4;
        }
        m_blupiIcon = num;
        m_blupiPhase++;
    }

    /**
     * @note Probes a 1-pixel-tall strip just below Blupi's feet (Y+60-2..Y+60-1) for a
     *       blocking tile. Returns false unconditionally while Blupi is being carried by a
     *       transport (m_blupiTransport != -1) so a lift counts as airborne for ground tests.
     */
    bool Decor::BlupiIsGround()
    {
        if (m_blupiTransport == -1)
        {
            TinyRect rect = BlupiRect(m_blupiPos);
            rect.Top = m_blupiPos.Y + 60 - 2;
            rect.Bottom = m_blupiPos.Y + 60 - 1;
            return DecorDetect(rect);
        }
        return false;
    }

    TinyRect Decor::BlupiRect(TinyPoint pos)
    {
        TinyRect result = TinyRect();
        if (m_blupiNage || m_blupiSurf)
        {
            result.Left = pos.X + 12;
            result.Right = pos.X + 60 - 12;
            if (m_blupiAction == BlupiAction::Stop)
            {
                result.Top = pos.Y + 5;
                result.Bottom = pos.Y + 60 - 10;
            }
            else
            {
                result.Top = pos.Y + 15;
                result.Bottom = pos.Y + 60 - 10;
            }
        }
        else if (m_blupiJeep)
        {
            result.Left = pos.X + 2;
            result.Right = pos.X + 60 - 2;
            result.Top = pos.Y + 10;
            result.Bottom = pos.Y + 60 - 2;
        }
        else if (m_blupiTank)
        {
            result.Left = pos.X + 2;
            result.Right = pos.X + 60 - 2;
            result.Top = pos.Y + 10;
            result.Bottom = pos.Y + 60 - 2;
        }
        else if (m_blupiOver)
        {
            result.Left = pos.X + 2;
            result.Right = pos.X + 60 - 2;
            result.Top = pos.Y + 2;
            result.Bottom = pos.Y + 60 - 2;
        }
        else if (m_blupiBalloon)
        {
            result.Left = pos.X + 10;
            result.Right = pos.X + 60 - 10;
            result.Top = pos.Y + 5;
            result.Bottom = pos.Y + 60 - 2;
        }
        else if (m_blupiEcrase)
        {
            result.Left = pos.X + 5;
            result.Right = pos.X + 60 - 5;
            result.Top = pos.Y + 39;
            result.Bottom = pos.Y + 60 - 2;
        }
        else
        {
            result.Left = pos.X + 12;
            result.Right = pos.X + 60 - 12;
            result.Top = pos.Y + 11;
            result.Bottom = pos.Y + 60 - 2;
        }
        return result;
    }

    /**
     * @note Penetration recovery. If Blupi's box is already clear it returns immediately.
     *       Otherwise it nudges him out of the overlap in fixed phases, each a bounded loop
     *       of up to 50 two-pixel steps (so worst case ~100px of correction): first push
     *       down (test a thin top edge), then push right, then push left, then two further
     *       horizontal passes using only the leading vertical edge. Each phase stops as soon
     *       as the probed edge is clear of any blocking tile.
     * @warning The 50-iteration cap means very deep penetration may not fully resolve; the
     *          caps are part of the original tuning and should not be raised casually.
     */
    void Decor::BlupiAdjust()
    {
        TinyRect tinyRect = BlupiRect(m_blupiPos);
        if (!DecorDetect(tinyRect))
        {
            return;
        }
        for (int i = 0; i < 50; i++)
        {
            TinyRect rect = tinyRect;
            rect.Bottom = rect.Top + 2;
            rect.Left = m_blupiPos.X + 12;
            rect.Right = m_blupiPos.X + 60 - 12;
            if (!DecorDetect(rect))
            {
                break;
            }
            tinyRect.Top += 2;
            tinyRect.Bottom += 2;
            m_blupiPos.Y += 2;
        }
        for (int i = 0; i < 50; i++)
        {
            TinyRect rect = tinyRect;
            rect.Right = rect.Left + 2;
            rect.Top = m_blupiPos.Y + 11;
            rect.Bottom = m_blupiPos.Y + 60 - 2;
            if (!DecorDetect(rect))
            {
                break;
            }
            tinyRect.Left += 2;
            tinyRect.Right += 2;
            m_blupiPos.X += 2;
        }
        for (int i = 0; i < 50; i++)
        {
            TinyRect rect = tinyRect;
            rect.Left = rect.Right - 2;
            rect.Top = m_blupiPos.Y + 11;
            rect.Bottom = m_blupiPos.Y + 60 - 2;
            if (!DecorDetect(rect))
            {
                break;
            }
            tinyRect.Left -= 2;
            tinyRect.Right -= 2;
            m_blupiPos.X -= 2;
        }
        for (int i = 0; i < 50; i++)
        {
            TinyRect rect = tinyRect;
            rect.Right = rect.Left + 2;
            if (!DecorDetect(rect))
            {
                break;
            }
            tinyRect.Left += 2;
            tinyRect.Right += 2;
            m_blupiPos.X += 2;
        }
        for (int i = 0; i < 50; i++)
        {
            TinyRect rect = tinyRect;
            rect.Left = rect.Right - 2;
            if (!DecorDetect(rect))
            {
                break;
            }
            tinyRect.Left -= 2;
            tinyRect.Right -= 2;
            m_blupiPos.X -= 2;
        }
    }

    // Checks whether Blupi is blocked by a solid tile in the given horizontal direction.
    // Only tests a narrow 2-pixel wide column on the leading edge of the bounding box,
    // in the lower 20 pixels of Blupi's height (foot-level collision).
    // dir > 0 = moving right, dir < 0 = moving left.
    bool Decor::BlupiBloque(TinyPoint pos, int dir)
    {
        TinyRect rect = BlupiRect(pos);
        rect.Top = rect.Bottom - 20;
        rect.Bottom -= 2;
        if (dir > 0)
        {
            rect.Left = rect.Right - 2;
        }
        if (dir < 0)
        {
            rect.Right = rect.Left + 2;
        }
        return DecorDetect(rect);
    }

#ifdef MODERN
    void Decor::BlupiGhostStep()
    {
        // Ghost mode: free flying movement based on directional input, no gravity,
        // no collision response. Only world bounds are enforced.
        int dx = 0;
        int dy = 0;
        if (Config::FPS == Fps::Fps20)
        {
            dx = static_cast<int>(m_blupiSpeedX * 4.0);
            dy = static_cast<int>(m_blupiSpeedY * 4.0);
        }
        else
        {
            m_blupiSubPixelX += m_blupiSpeedX * Config::SPEED_SCALE * 4.0;
            m_blupiSubPixelY += m_blupiSpeedY * Config::SPEED_SCALE * 4.0;
            dx = static_cast<int>(std::floor(m_blupiSubPixelX));
            dy = static_cast<int>(std::floor(m_blupiSubPixelY));
            m_blupiSubPixelX -= dx;
            m_blupiSubPixelY -= dy;
        }

        if (dx != 0)
        {
            m_blupiDir = (dx < 0) ? Direction::Left : Direction::Right;
        }

        m_blupiPos.X += dx;
        m_blupiPos.Y += dy;

        // Clamp to world bounds (tile map is 100x100 tiles at 64px each = 6400px).
        m_blupiPos.X = std::max(0, std::min(m_blupiPos.X, 6400 - 64));
        m_blupiPos.Y = std::max(0, std::min(m_blupiPos.Y, 6400 - 64));

        // Update animation: march when moving, stop when idle.
        if (dx != 0 || dy != 0)
        {
            if (m_blupiAction != BlupiAction::March)
            {
                m_blupiAction = BlupiAction::March;
                m_blupiPhase = 0;
            }
        }
        else
        {
            if (m_blupiAction != BlupiAction::Stop)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
        }

        BlupiSearchIcon();

        // Update camera/scroll so it follows Blupi during ghost flight (mirrors normal BlupiStep scroll logic).
        TinyPoint ghostEnd;
        ghostEnd.X = m_blupiPos.X + 30 + m_scrollAdd.X;
        ghostEnd.Y = m_blupiPos.Y + 30 + m_scrollAdd.Y;
        int ghostSpeed = SCROLL_SPEED;
        int ghostDx = std::abs(m_scrollPoint.X - ghostEnd.X);
        int ghostDy = std::abs(m_scrollPoint.Y - ghostEnd.Y);
        if (ghostDx > SCROLL_MARGX * 2) { ghostSpeed += (ghostDx - SCROLL_MARGX * 2) / 4; }
        if (ghostDy > SCROLL_MARGY * 2) { ghostSpeed += (ghostDy - SCROLL_MARGY * 2) / 4; }
        if (m_scrollPoint.X < ghostEnd.X) { m_scrollPoint.X += ghostSpeed; if (m_scrollPoint.X >= ghostEnd.X) m_scrollPoint.X = ghostEnd.X; }
        if (m_scrollPoint.X > ghostEnd.X) { m_scrollPoint.X -= ghostSpeed; if (m_scrollPoint.X <= ghostEnd.X) m_scrollPoint.X = ghostEnd.X; }
        if (m_scrollPoint.Y < ghostEnd.Y) { m_scrollPoint.Y += ghostSpeed; if (m_scrollPoint.Y >= ghostEnd.Y) m_scrollPoint.Y = ghostEnd.Y; }
        if (m_scrollPoint.Y > ghostEnd.Y) { m_scrollPoint.Y -= ghostSpeed; if (m_scrollPoint.Y <= ghostEnd.Y) m_scrollPoint.Y = ghostEnd.Y; }
        m_posDecor = GetPosDecor(m_scrollPoint);
    }
#endif

    /**
     * @note Core Blupi physics + action state machine; the single largest hand-coded state
     *       machine in the file. Per-frame order of operations:
     *         1. BlupiAdjust() resolves any stuck-in-wall situation left by the previous frame.
     *         2. The proposed move is end = m_blupiPos + m_blupiVector.
     *         3. Wind/ventilator tile effects are applied when Blupi is on an air-vent tile.
     *         4. Ground contact (flag2) and ceiling (flag3) are probed via DecorDetect().
     *         5. Hazard/feature detection runs: lava, traps, drips, saws, springs, switches,
     *            bridges, doors, teleporters, lifts, water entry/exit, etc. Each can force an
     *            action change, death (BlupiDead), a bounce, or a teleport (SearchTeleporte).
     *         6. A large switch on m_blupiAction executes the branch for the current action
     *            (march / jump / air / helico / jeep / tank / skate / swim / surf / suspend /
     *            balloon / death / clear / ...). Each branch reads m_blupiSpeedX/Y plus state
     *            and produces the next m_blupiVector, possibly transitioning to another action.
     *         7. BlupiAdjust() snaps Blupi back out of geometry after the move is committed.
     *       BlupiSearchIcon() is invoked to refresh the sprite, and the scroll target is eased
     *       toward Blupi at the end.
     * @note m_blupiVector is both the input (this frame's intended displacement) and is
     *       recomputed here for the next frame. m_blupiValidPos is updated only while Blupi is
     *       in a non-lethal position so death can respawn him at the last safe spot.
     * @warning This method mutates a large amount of member state and has many early-exit and
     *          fall-through paths between actions; the action transitions are interdependent
     *          and ordering-sensitive. Treat it as ported game logic, not free-form code.
     */
    void Decor::BlupiStep()
    {
#ifdef MODERN
        if (m_blupiGhost)
        {
            BlupiGhostStep();
            return;
        }
#endif
        TinyPoint celSwitch;
        TinyPoint celBridge;
        BlupiAdjust();
        m_blupiLastPos = m_blupiPos;
        TinyPoint end = m_blupiPos;
        bool flag = m_blupiAir;
        BlupiAction blupiAction = m_blupiAction;
        bool bVertigoLeft = false;
        bool bVertigoRight = false;
        end.X += m_blupiVector.X;
        end.Y += m_blupiVector.Y;
        if (m_blupiFocus && (end.Y + 30) / 64 >= 99)
        {
            BlupiDead(BlupiAction::Clear2, {});
            m_blupiRestart = true;
            m_blupiAir = true;
            m_blupiPos.Y = m_blupiPos.Y / 64 * 64 + BLUPIOFFY;
            PlaySound(SoundChannel::SoundChannel8, m_blupiPos);
            return;
        }
        TinyRect rect = TinyRect();
        if (m_blupiVector.X != 0 || m_blupiVector.Y != 0)
        {
            rect = BlupiRect(m_blupiPos);
            rect.Top = m_blupiPos.Y + 11;
            rect.Bottom = m_blupiPos.Y + 60 - 2;
            TestPath(rect, m_blupiPos, end);
        }
        m_blupiVent = false;
        int icon;
        if (m_blupiTransport == -1 && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && m_blupiFocus)
        {
            icon = m_decor[(end.X + 30) / 64][(end.Y + 30) / 64].icon;
            if (icon == 110)
            {
                if (Config::FPS == Fps::Fps20) { end.X -= 9; }
                else { m_blupiSubPixelX -= 9.0 * Config::SPEED_SCALE; int wpx110 = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpx110; end.X += wpx110; }
            }
            if (icon == 114)
            {
                if (Config::FPS == Fps::Fps20) { end.X += 9; }
                else { m_blupiSubPixelX += 9.0 * Config::SPEED_SCALE; int wpx114 = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpx114; end.X += wpx114; }
            }
            if (icon == 118)
            {
                if (Config::FPS == Fps::Fps20) { end.Y -= 20; }
                else { m_blupiSubPixelY -= 20.0 * Config::SPEED_SCALE; int wpy118 = static_cast<int>(std::floor(m_blupiSubPixelY)); m_blupiSubPixelY -= wpy118; end.Y += wpy118; }
            }
            if (icon == 122)
            {
                if (Config::FPS == Fps::Fps20) { end.Y += 20; }
                else { m_blupiSubPixelY += 20.0 * Config::SPEED_SCALE; int wpy122 = static_cast<int>(std::floor(m_blupiSubPixelY)); m_blupiSubPixelY -= wpy122; end.Y += wpy122; }
            }
            if (icon >= 110 && icon <= 125)
            {
                m_blupiVent = true;
                rect.Left = m_blupiPos.X + 12;
                rect.Right = m_blupiPos.X + 60 - 12;
                rect.Top = m_blupiPos.Y + 11;
                rect.Bottom = m_blupiPos.Y + 60 - 2;
                TestPath(rect, m_blupiPos, end);
            }
        }
        bool flag2;
        if (m_blupiTransport == -1)
        {
            rect = BlupiRect(end);
            rect.Top = end.Y + 60 - 2;
            rect.Bottom = end.Y + 60 - 1;
            flag2 = !DecorDetect(rect);
        }
        else
        {
            flag2 = false;
        }
        rect = BlupiRect(end);
        rect.Top = end.Y + 10;
        rect.Bottom = end.Y + 20;
        bool flag3 = DecorDetect(rect);
        int detectIcon = m_detectIcon;
        if (!m_blupiAir && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !
            m_blupiTank && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && flag2 && m_blupiFocus)
        {
            if (m_blupiFocus)
            {
                m_blupiAction = BlupiAction::Air;
                m_blupiPhase = 0;
            }
            m_blupiVitesseY = 1.0;
            m_blupiAir = true;
            flag = true;
        }
        if (!m_blupiNage && !m_blupiSurf && !m_blupiSuspend && !m_blupiAir && IsRessort(end))
        {
            if ((m_blupiHelico || m_blupiOver) && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                m_blupiHelico = false;
                m_blupiOver = false;
                celSwitch.X = end.X - 34;
                celSwitch.Y = end.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType9, 0);
                m_decorAction = DecorAction::SmallShake;
                m_decorPhase = 0;
                StopSound(SoundChannel::SoundChannel16);
                StopSound(SoundChannel::SoundChannel18);
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel10, m_blupiPos);
            }
            if (m_blupiJeep && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                m_blupiJeep = false;
                celSwitch.X = end.X - 34;
                celSwitch.Y = end.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType9, 0);
                m_decorAction = DecorAction::SmallShake;
                m_decorPhase = 0;
                StopSound(SoundChannel::SoundChannel16);
                StopSound(SoundChannel::SoundChannel18);
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel10, m_blupiPos);
            }
            if (m_blupiTank && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                m_blupiTank = false;
                celSwitch.X = end.X - 34;
                celSwitch.Y = end.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType9, 0);
                m_decorAction = DecorAction::SmallShake;
                m_decorPhase = 0;
                StopSound(SoundChannel::SoundChannel16);
                StopSound(SoundChannel::SoundChannel18);
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel10, m_blupiPos);
            }
            if (m_blupiSkate && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                m_blupiSkate = false;
                celSwitch.X = end.X - 34;
                celSwitch.Y = end.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType9, 0);
                m_decorAction = DecorAction::SmallShake;
                m_decorPhase = 0;
                StopSound(SoundChannel::SoundChannel16);
                StopSound(SoundChannel::SoundChannel18);
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel10, m_blupiPos);
            }
            if (m_blupiFocus && m_blupiAction != BlupiAction::Clear1 && m_blupiAction != BlupiAction::Clear2 && m_blupiAction != BlupiAction::Clear3 && m_blupiAction != BlupiAction::Clear4
                && m_blupiAction != BlupiAction::Clear5 && m_blupiAction != BlupiAction::Clear6 && m_blupiAction != BlupiAction::Clear7 && m_blupiAction != BlupiAction::Clear8)
            {
                m_blupiAction = BlupiAction::Air;
                m_blupiPhase = 0;
            }
            if (((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0 && m_blupiFocus)
            {
                m_blupiVitesseY = (m_blupiPower ? (-25) : (-19));
            }
            else
            {
                m_blupiVitesseY = (m_blupiPower ? (-16) : (-10));
            }
            m_blupiAir = true;
            m_blupiSubPixelY = 0.0;
            flag = true;
            PlaySound(SoundChannel::SoundChannel41, end);
        }
        if (((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0 && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !
            m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
            m_blupiFocus)
        {
            if (m_blupiAction != BlupiAction::Jump && m_blupiAction != BlupiAction::Turn && !m_blupiAir)
            {
                m_blupiAction = BlupiAction::Jump;
                m_blupiPhase = 0;
            }
            if (m_blupiAction == BlupiAction::Jump && m_blupiPhase == Config::ScaleTime(3))
            {
                m_blupiAction = BlupiAction::Air;
                m_blupiPhase = 0;
                if (m_blupiSkate)
                {
                    PlaySound(SoundChannel::SoundChannel1, end);
                    m_blupiVitesseY = (m_blupiPower ? (-17) : (-13));
                }
                else
                {
                    PlaySound(SoundChannel::SoundChannel1, end);
                    if (IsNormalJump(end))
                    {
                        m_blupiVitesseY = (m_blupiPower ? (-26) : (-16));
                    }
                    else
                    {
                        m_blupiVitesseY = (m_blupiPower ? (-16) : (-12));
                    }
                }
                m_blupiAir = true;
                m_blupiSubPixelY = 0.0;
                flag = true;
            }
        }
        if (m_blupiAir)
        {
            if (flag3 && m_blupiVitesseY < 0.0)
            {
                if (m_blupiVitesseY < -14.0 && m_blupiAction != BlupiAction::Clear1 && m_blupiAction != BlupiAction::Clear2 && m_blupiAction != BlupiAction::Clear3 &&
                    m_blupiAction != BlupiAction::Clear4 && m_blupiAction != BlupiAction::Clear5 && m_blupiAction != BlupiAction::Clear6 && m_blupiAction != BlupiAction::Clear7 &&
                    m_blupiAction != BlupiAction::Clear8 && !m_blupiSkate)
                {
                    m_blupiJumpAie = true;
                    PlaySound(SoundChannel::SoundChannel40, end);
                }
                else
                {
                    PlaySound(SoundEnviron(SoundChannel::SoundChannel4, detectIcon), end);
                }
                m_blupiVitesseY = 1.0;
            }
            if (Config::FPS == Fps::Fps20)
            {
                end.Y += (int)(m_blupiVitesseY * 2.0);
                if (m_blupiVitesseY < 20.0) { m_blupiVitesseY += 2.0; }
            }
            else
            {
                m_blupiSubPixelY += m_blupiVitesseY * 2.0 * Config::SPEED_SCALE;
                int wholePixelsY = static_cast<int>(std::floor(m_blupiSubPixelY));
                m_blupiSubPixelY -= wholePixelsY;
                end.Y += wholePixelsY;
                if (m_blupiVitesseY < 20.0) { m_blupiVitesseY += 2.0 * Config::SPEED_SCALE; }
            }
            rect = BlupiRect(end);
            rect.Top = end.Y + 60 - 30;
            rect.Bottom = end.Y + 60 - 1;
            if (m_blupiVitesseY >= 0.0 && DecorDetect(rect))
            {
                end.Y = end.Y / 32 * 32 + BLUPIOFFY;
                if (!IsRessort(end))
                {
                    PlaySound(SoundEnviron(SoundChannel::SoundChannel3, m_detectIcon), end);
                }
                if (m_blupiFocus)
                {
                    if (m_blupiVitesseY > 20.0)
                    {
                        m_blupiAction = BlupiAction::StopJump;
                    }
                    else
                    {
                        m_blupiAction = BlupiAction::StopJump;
                    }
                    m_blupiPhase = 0;
                }
                m_blupiAir = false;
                m_blupiSubPixelY = 0.0;
                if (m_blupiJumpAie)
                {
                    m_blupiJumpAie = false;
                    m_blupiAction = BlupiAction::JumpAie;
                    m_blupiPhase = 0;
                }
            }
            rect.Left = end.X + 20;
            rect.Right = end.X + 60 - 20;
            rect.Top = end.Y + 60 - 33;
            rect.Bottom = end.Y + 60 - 1;
            icon = AscenseurDetect(rect, m_blupiPos, end);
            if (m_blupiVitesseY >= 0.0 && icon != -1)
            {
                m_blupiTransport = icon;
                flag2 = false;
                PlaySound(SoundChannel::SoundChannel3, end);
                end.Y = m_moveObject[icon].posCurrent.Y - 64 + BLUPIOFFY;
                if (m_blupiFocus)
                {
                    if (m_blupiVitesseY > 20.0)
                    {
                        m_blupiAction = BlupiAction::StopJump;
                    }
                    else
                    {
                        m_blupiAction = BlupiAction::StopJump;
                    }
                    m_blupiPhase = 0;
                }
                m_blupiAir = false;
                m_blupiSubPixelY = 0.0;
                if (m_blupiJumpAie)
                {
                    m_blupiJumpAie = false;
                    m_blupiAction = BlupiAction::JumpAie;
                    m_blupiPhase = 0;
                }
            }
        }
        if (m_blupiAction == BlupiAction::JumpAie && m_blupiPhase == Config::ScaleTime(30))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        if (m_blupiAction == BlupiAction::Charge && m_blupiPhase == Config::ScaleTime(64))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
            m_blupiCloud = true;
            m_blupiTimeShield = 100;
            m_jauges[1].SetHide(false);
            PlaySound(SoundChannel::SoundChannel55, end);
        }
        if (m_blupiAction == BlupiAction::HelicoGlu)
        {
            if (m_blupiPhase == Config::ScaleTime(8))
            {
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = m_blupiPos.Y + 40;
                if (m_blupiVitesseY > 0.0)
                {
                    celSwitch.Y += (int)(m_blupiVitesseY * 4.0);
                }
                m_blupiVitesseY -= 10.0 * Config::SPEED_SCALE;
                if (ObjectStart(celSwitch, ObjectType::ObjectType23, 55) != -1)
                {
                    PlaySound(SoundChannel::SoundChannel52, m_blupiPos);
                    m_blupiTimeFire = Config::ScaleTime(10);
                    m_blupiBullet--;
                }
            }
            if (m_blupiPhase == Config::ScaleTime(14))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
        }
        if ((m_blupiAction == BlupiAction::Ouf1a || m_blupiAction == BlupiAction::Ouf1b) && m_blupiPhase == Config::ScaleTime(29))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        if (m_blupiAction == BlupiAction::Ouf2 && m_blupiPhase == Config::ScaleTime(32))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        if (m_blupiAction == BlupiAction::Ouf3 && m_blupiPhase == Config::ScaleTime(34))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        bool bNear;
        if (m_blupiAction == BlupiAction::Ouf4 && m_blupiPhase == Config::ScaleTime(40))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
            icon = MoveObjectDetect(end, bNear);
            if (icon != -1 && !bNear && end.Y - BLUPIFLOOR == m_moveObject[icon].posCurrent.Y)
            {
                if (m_blupiDir == Direction::Right && end.X < m_moveObject[icon].posCurrent.X)
                {
                    celSwitch.X = end.X - 16;
                    celSwitch.Y = end.Y;
                    int num = MoveObjectDetect(celSwitch, bNear);
                    if (num == -1)
                    {
                        m_blupiAction = BlupiAction::Recede;
                        m_blupiPhase = 0;
                    }
                }
                if (m_blupiDir == Direction::Left && end.X > m_moveObject[icon].posCurrent.X)
                {
                    celSwitch.X = end.X + 16;
                    celSwitch.Y = end.Y;
                    int num = MoveObjectDetect(celSwitch, bNear);
                    if (num == -1)
                    {
                        m_blupiAction = BlupiAction::Recede;
                        m_blupiPhase = 0;
                    }
                }
            }
        }
        if (m_blupiAction == BlupiAction::Ouf5)
        {
            if (m_blupiPhase == Config::ScaleTime(4))
            {
                PlaySound(SoundChannel::SoundChannel47, m_blupiPos);
            }
            if (m_blupiPhase == Config::ScaleTime(44))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
        }
        if (m_blupiAction == BlupiAction::Mockery)
        {
            if (m_blupiPhase == Config::ScaleTime(1))
            {
                PlaySound(SoundChannel::SoundChannel65, m_blupiPos);
                m_blupiTimeMockery = Config::ScaleTime(300);
            }
            if (m_blupiPhase == Config::ScaleTime(92))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
        }
        if (m_blupiAction == BlupiAction::Mockeryi)
        {
            if (m_blupiPhase == Config::ScaleTime(6))
            {
                PlaySound(SoundChannel::SoundChannel65, m_blupiPos);
                m_blupiTimeMockery = Config::ScaleTime(300);
            }
            if (m_blupiPhase == Config::ScaleTime(104))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
        }
        if (m_blupiAction == BlupiAction::Mockeryp)
        {
            if (m_blupiPhase == Config::ScaleTime(4))
            {
                PlaySound(SoundChannel::SoundChannel47, m_blupiPos);
            }
            if (m_blupiPhase == Config::ScaleTime(60))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
        }
        if (m_blupiAction == BlupiAction::Non && m_blupiPhase == Config::ScaleTime(18))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        if (m_blupiAction == BlupiAction::StopMarch && m_blupiPhase == Config::ScaleTime(3))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
        }
        if (m_blupiAction == BlupiAction::StopJump && m_blupiPhase == Config::ScaleTime(5))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
        }
        if (m_blupiAction == BlupiAction::StopJumph && m_blupiPhase == Config::ScaleTime(2))
        {
            m_blupiAction = BlupiAction::Air;
            m_blupiPhase = 0;
            m_blupiVitesseY = -12.0;
            m_blupiAir = true;
            flag = true;
        }
        if (m_blupiAction == BlupiAction::Sucette && m_blupiPhase == Config::ScaleTime(32))
        {
            ObjectStart(m_sucettePos, m_sucetteType, 0);
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
            m_blupiPower = true;
            m_blupiTimeShield = 100;
            m_blupiPosMagic = m_blupiPos;
            m_jauges[1].SetHide(false);
            PlaySound(SoundChannel::SoundChannel44, end);
        }
        if (m_blupiAction == BlupiAction::Drink && m_blupiPhase == Config::ScaleTime(36))
        {
            ObjectStart(m_sucettePos, m_sucetteType, 0);
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
            m_blupiHide = true;
            m_blupiTimeShield = 100;
            m_blupiPosMagic = m_blupiPos;
            m_jauges[1].SetHide(false);
            PlaySound(SoundChannel::SoundChannel62, end);
        }
        if (m_blupiSpeedY < 0.0 && m_blupiLastSpeedY == 0.0 && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::Jump && m_blupiAction
            != BlupiAction::Air && m_blupiAction != BlupiAction::Vertigo && m_blupiAction != BlupiAction::Advance && m_blupiAction != BlupiAction::Recede && !m_blupiAir && !m_blupiHelico &&
            !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !
            m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            m_blupiAction = BlupiAction::Up;
            m_blupiPhase = 0;
        }
        if (m_blupiSpeedY == 0.0 && m_blupiLastSpeedY < 0.0 && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::Jump && m_blupiAction
            != BlupiAction::Air && m_blupiAction != BlupiAction::Vertigo && m_blupiAction != BlupiAction::Advance && m_blupiAction != BlupiAction::Recede && !m_blupiAir && !m_blupiHelico &&
            !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !
            m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
        }
        if (m_blupiSpeedY > 0.0 && m_blupiLastSpeedY == 0.0 && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::Jump && m_blupiAction
            != BlupiAction::Air && m_blupiAction != BlupiAction::Down && m_blupiAction != BlupiAction::StopPop && m_blupiAction != BlupiAction::Vertigo && m_blupiAction != BlupiAction::Advance &&
            m_blupiAction != BlupiAction::Recede && !m_blupiAir && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase &&
            !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
            m_blupiFocus)
        {
            m_blupiAction = BlupiAction::Down;
            m_blupiPhase = 0;
        }
        if (m_blupiSpeedY > 0.0 && m_blupiSpeedX == 0.0 && (m_keyPress & ToRaw(KeyPressFlags::Jump)) == 0 && m_blupiAction != BlupiAction::Turn && m_blupiAction
            != BlupiAction::Jump && m_blupiAction != BlupiAction::Air && m_blupiAction != BlupiAction::Down && m_blupiAction != BlupiAction::StopPop && m_blupiAction != BlupiAction::Vertigo &&
            m_blupiAction != BlupiAction::Advance && m_blupiAction != BlupiAction::Recede && !m_blupiAir && !m_blupiHelico && !m_blupiOver && !
            m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !
            m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            m_blupiAction = BlupiAction::Down;
            m_blupiPhase = 0;
        }
        if (m_blupiSpeedY == 0.0 && m_blupiLastSpeedY > 0.0 && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::Jump &&
            m_blupiAction != BlupiAction::Air && m_blupiAction != BlupiAction::Vertigo && m_blupiAction != BlupiAction::Advance && m_blupiAction != BlupiAction::Recede && !m_blupiAir && !m_blupiHelico &&
            !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !
            m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
        }
        if (m_blupiAction == BlupiAction::Up && m_blupiPhase == Config::ScaleTime(4))
        {
            m_scrollAdd.Y = -150;
            PlaySound(SoundChannel::SoundChannel21, end);
        }
        if (m_blupiAction == BlupiAction::Down && m_blupiPhase == Config::ScaleTime(4))
        {
            m_scrollAdd.Y = 150;
            PlaySound(SoundChannel::SoundChannel7, end);
        }
        if (!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
            m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            if (m_blupiSpeedY > 0.0 && m_blupiSpeedX == 0.0 && (m_keyPress & ToRaw(KeyPressFlags::Jump)) == 0 && m_blupiAction != BlupiAction::StopPop &&
                m_blupiDir == Direction::Left && (icon = CaisseInFront()) != -1)
            {
                end.X = m_moveObject[icon].posCurrent.X + 64 - 5;
                m_blupiAction = BlupiAction::StopPop;
                m_blupiPhase = 0;
                m_scrollAdd.Y = 0;
                PlaySound(SoundChannel::SoundChannel39, end);
            }
            if (m_blupiSpeedY > 0.0 && m_blupiSpeedX > 0.0 && (m_keyPress & ToRaw(KeyPressFlags::Jump)) == 0 && m_blupiAction != BlupiAction::Pop &&
                m_blupiDir == Direction::Left && (icon = CaisseInFront()) != -1)
            {
                m_blupiAction = BlupiAction::Pop;
                m_blupiPhase = 0;
                m_scrollAdd.Y = 0;
                PlaySound(SoundChannel::SoundChannel39, end);
            }
            if (m_blupiSpeedY > 0.0 && m_blupiSpeedX == 0.0 && (m_keyPress & ToRaw(KeyPressFlags::Jump)) == 0 && m_blupiAction != BlupiAction::StopPop &&
                m_blupiDir == Direction::Right && (icon = CaisseInFront()) != -1)
            {
                end.X = m_moveObject[icon].posCurrent.X - 60 + 5;
                m_blupiAction = BlupiAction::StopPop;
                m_blupiPhase = 0;
                m_scrollAdd.Y = 0;
                PlaySound(SoundChannel::SoundChannel39, end);
            }
            if (m_blupiSpeedY > 0.0 && m_blupiSpeedX < 0.0 && (m_keyPress & ToRaw(KeyPressFlags::Jump)) == 0 && m_blupiAction != BlupiAction::Pop && m_blupiDir
                == Direction::Right && (icon = CaisseInFront()) != -1)
            {
                m_blupiAction = BlupiAction::Pop;
                m_blupiPhase = 0;
                m_scrollAdd.Y = 0;
                PlaySound(SoundChannel::SoundChannel39, end);
            }
            if (m_blupiAction == BlupiAction::Pop && m_blupiActionOuf != BlupiAction::Ouf3)
            {
                m_blupiActionOuf = BlupiAction::Ouf3;
                m_blupiTimeOuf = 0;
            }
        }
        if (m_blupiAction != BlupiAction::StopPop && m_blupiAction != BlupiAction::Pop && m_blupiFocus)
        {
            if (m_blupiSpeedX < 0.0 && m_blupiLastSpeedX == 0.0 && !m_blupiAir && m_blupiSpeedY != 0.0)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            if (m_blupiSpeedX == 0.0 && m_blupiLastSpeedX < 0.0 && m_blupiSpeedY != 0.0)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            if (m_blupiSpeedX > 0.0 && m_blupiLastSpeedX == 0.0 && !m_blupiAir && m_blupiSpeedY != 0.0)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            if (m_blupiSpeedX == 0.0 && m_blupiLastSpeedX > 0.0 && m_blupiSpeedY != 0.0)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
        }
        int num2;
        int num3;
        if (m_blupiSpeedX < 0.0 && m_blupiFocus)
        {
            if (m_blupiDir == Direction::Right && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::TurnAir && m_blupiAction != BlupiAction::Up && m_blupiAction != BlupiAction::Down
                && m_blupiAction != BlupiAction::Pop && ((!m_blupiJeep && !m_blupiTank && !m_blupiSkate) || std::abs(m_blupiVitesseX)
                    <= 8.0))
            {
                if (m_blupiAir)
                {
                    PlaySound(SoundChannel::SoundChannel5, end);
                    m_blupiAction = BlupiAction::TurnAir;
                    m_blupiPhase = 0;
                    m_blupiDir = Direction::Left;
                }
                else
                {
                    PlaySound(SoundChannel::SoundChannel5, end);
                    m_blupiAction = BlupiAction::Turn;
                    m_blupiPhase = 0;
                }
            }
            if (m_blupiDir == Direction::Left && m_blupiAction != BlupiAction::March && m_blupiAction != BlupiAction::Push && m_blupiAction != BlupiAction::Jump && m_blupiAction != BlupiAction::Up
                && m_blupiAction != BlupiAction::Down && m_blupiAction != BlupiAction::Pop && !m_blupiAir)
            {
                m_blupiAction = BlupiAction::March;
                m_blupiPhase = 0;
            }
            if (m_blupiDir == Direction::Left && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::Jump && m_blupiAction != BlupiAction::Up && m_blupiAction != BlupiAction::Down
                && m_blupiAction != BlupiAction::Pop && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !
                m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend)
            {
                if (m_blupiAction == BlupiAction::Push)
                {
                    if (Config::FPS == Fps::Fps20) { end.X -= CaisseGetMove(5); }
                    else { m_blupiSubPixelX -= CaisseGetMove(5) * Config::SPEED_SCALE; int wholePixelsX = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wholePixelsX; end.X += wholePixelsX; }
                }
                else
                {
                    num2 = m_blupiPhase;
                    if (num2 > 3 || m_blupiAir)
                    {
                        num2 = 3;
                    }
                    num3 = Tables::table_vitesse_march[num2];
                    if (m_blupiPower)
                    {
                        num3 *= 3;
                        num3 /= 2;
                    }
                    if (Config::FPS == Fps::Fps20) { end.X += Misc::Speed(m_blupiSpeedX, num3); }
                    else { m_blupiSubPixelX += Misc::Speed(m_blupiSpeedX, num3) * Config::SPEED_SCALE; int wholePixelsX = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wholePixelsX; end.X += wholePixelsX; }
                }
            }
            if (m_blupiDir == Direction::Right && m_blupiAction == BlupiAction::Pop)
            {
                if (Config::FPS == Fps::Fps20) { end.X -= CaisseGetMove(3); }
                else { m_blupiSubPixelX -= CaisseGetMove(3) * Config::SPEED_SCALE; int wholePixelsXPop = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wholePixelsXPop; end.X += wholePixelsXPop; }
            }
        }
        if (m_blupiSpeedX > 0.0 && m_blupiFocus)
        {
            if (m_blupiDir == Direction::Left && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::TurnAir && m_blupiAction != BlupiAction::Up && m_blupiAction != BlupiAction::Down
                && m_blupiAction != BlupiAction::Pop && ((!m_blupiJeep && !m_blupiTank && !m_blupiSkate) || std::abs(m_blupiVitesseX)
                    <= 8.0))
            {
                if (m_blupiAir)
                {
                    PlaySound(SoundChannel::SoundChannel5, end);
                    m_blupiAction = BlupiAction::TurnAir;
                    m_blupiPhase = 0;
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    PlaySound(SoundChannel::SoundChannel5, end);
                    m_blupiAction = BlupiAction::Turn;
                    m_blupiPhase = 0;
                }
            }
            if (m_blupiDir == Direction::Right && m_blupiAction != BlupiAction::March && m_blupiAction != BlupiAction::Push && m_blupiAction != BlupiAction::Jump && m_blupiAction != BlupiAction::Up
                && m_blupiAction != BlupiAction::Down && m_blupiAction != BlupiAction::Pop && !m_blupiAir)
            {
                m_blupiAction = BlupiAction::March;
                m_blupiPhase = 0;
            }
            if (m_blupiDir == Direction::Right && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::Jump && m_blupiAction != BlupiAction::Up && m_blupiAction != BlupiAction::Down
                && m_blupiAction != BlupiAction::Pop && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !
                m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend)
            {
                if (m_blupiAction == BlupiAction::Push)
                {
                    if (Config::FPS == Fps::Fps20) { end.X += CaisseGetMove(5); }
                    else { m_blupiSubPixelX += CaisseGetMove(5) * Config::SPEED_SCALE; int wholePixelsX = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wholePixelsX; end.X += wholePixelsX; }
                }
                else
                {
                    num2 = m_blupiPhase;
                    if (num2 > 3 || m_blupiAir)
                    {
                        num2 = 3;
                    }
                    num3 = Tables::table_vitesse_march[num2];
                    if (m_blupiPower)
                    {
                        num3 *= 3;
                        num3 /= 2;
                    }
                    if (Config::FPS == Fps::Fps20) { end.X += Misc::Speed(m_blupiSpeedX, num3); }
                    else { m_blupiSubPixelX += Misc::Speed(m_blupiSpeedX, num3) * Config::SPEED_SCALE; int wholePixelsX = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wholePixelsX; end.X += wholePixelsX; }
                }
            }
            if (m_blupiDir == Direction::Left && m_blupiAction == BlupiAction::Pop)
            {
                if (Config::FPS == Fps::Fps20) { end.X += CaisseGetMove(3); }
                else { m_blupiSubPixelX += CaisseGetMove(3) * Config::SPEED_SCALE; int wholePixelsXPop = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wholePixelsXPop; end.X += wholePixelsXPop; }
            }
        }
        if (m_blupiHelico)
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(10))
            {
                m_blupiAction = BlupiAction::March;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
        }
        else if (m_blupiOver)
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(7))
            {
                m_blupiAction = BlupiAction::March;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
        }
        else if (m_blupiJeep)
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(7))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
        }
        else if (m_blupiTank)
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(12))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
        }
        else if (m_blupiSkate)
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(14))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
        }
        else if (m_blupiNage || m_blupiSurf)
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(10))
            {
                m_blupiAction = BlupiAction::March;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
        }
        else if (m_blupiSuspend)
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(10))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
        }
        else
        {
            if (m_blupiAction == BlupiAction::Turn && m_blupiPhase == Config::ScaleTime(6))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                if (m_blupiDir == Direction::Left)
                {
                    m_blupiDir = Direction::Right;
                }
                else
                {
                    m_blupiDir = Direction::Left;
                }
            }
            if (m_blupiAction == BlupiAction::TurnAir && m_blupiPhase == Config::ScaleTime(6))
            {
                m_blupiAction = BlupiAction::Air;
                m_blupiPhase = 0;
            }
        }
        if (!m_blupiSuspend && m_blupiAction == BlupiAction::Jump && m_blupiPhase == Config::ScaleTime(3))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
        }
        if (m_blupiSpeedX == 0.0 && m_blupiSpeedY == 0.0 && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !
            m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf &&
            m_blupiFocus)
        {
            if (m_blupiAction == BlupiAction::Push || m_blupiAction == BlupiAction::Up)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            if (m_blupiAction == BlupiAction::March)
            {
                if (m_blupiSuspend || m_blupiPhase < Config::ScaleTime(10))
                {
                    m_blupiAction = BlupiAction::Stop;
                }
                else
                {
                    m_blupiAction = BlupiAction::StopMarch;
                }
                m_blupiPhase = 0;
            }
            if (m_blupiAction == BlupiAction::Down)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                PlaySound(SoundChannel::SoundChannel20, end);
            }
            m_scrollAdd.Y = 0;
            if (blupiAction == BlupiAction::Push)
            {
                StopSound(SoundChannel::SoundChannel38);
            }
            if (blupiAction == BlupiAction::Pop || blupiAction == BlupiAction::StopPop)
            {
                StopSound(SoundChannel::SoundChannel39);
            }
        }
        if (!m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !
            m_blupiSurf && m_blupiFocus)
        {
            if (m_blupiAction == BlupiAction::Recede && m_blupiDir == Direction::Left)
            {
                if (Config::FPS == Fps::Fps20) { end.X += 4; }
                else { m_blupiSubPixelX += 4.0 * Config::SPEED_SCALE; int wpxR = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpxR; end.X += wpxR; }
            }
            if (m_blupiAction == BlupiAction::Recede && m_blupiDir == Direction::Right)
            {
                if (Config::FPS == Fps::Fps20) { end.X -= 4; }
                else { m_blupiSubPixelX -= 4.0 * Config::SPEED_SCALE; int wpxR = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpxR; end.X += wpxR; }
            }
            if (m_blupiAction == BlupiAction::Advance && m_blupiDir == Direction::Left)
            {
                if (Config::FPS == Fps::Fps20) { end.X -= 4; }
                else { m_blupiSubPixelX -= 4.0 * Config::SPEED_SCALE; int wpxA = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpxA; end.X += wpxA; }
            }
            if (m_blupiAction == BlupiAction::Advance && m_blupiDir == Direction::Right)
            {
                if (Config::FPS == Fps::Fps20) { end.X += 4; }
                else { m_blupiSubPixelX += 4.0 * Config::SPEED_SCALE; int wpxA = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpxA; end.X += wpxA; }
            }
        }
        if ((m_keyPress & -3) == 0 && m_blupiSpeedX == 0.0 && m_blupiSpeedY == 0.0 && (m_blupiJeep || m_blupiTank ||
            m_blupiSkate) && m_blupiFocus)
        {
            if (m_blupiAction == BlupiAction::Advance && m_blupiDir == Direction::Left)
            {
                if (Config::FPS == Fps::Fps20) { end.X -= 5; }
                else { m_blupiSubPixelX -= 5.0 * Config::SPEED_SCALE; int wpxJA = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpxJA; end.X += wpxJA; }
            }
            if (m_blupiAction == BlupiAction::Advance && m_blupiDir == Direction::Right)
            {
                if (Config::FPS == Fps::Fps20) { end.X += 5; }
                else { m_blupiSubPixelX += 5.0 * Config::SPEED_SCALE; int wpxJA = static_cast<int>(std::floor(m_blupiSubPixelX)); m_blupiSubPixelX -= wpxJA; end.X += wpxJA; }
            }
        }
        if ((m_keyPress & -3) == 0 && m_blupiSpeedX == 0.0 && m_blupiSpeedY == 0.0 && m_blupiNage && m_blupiFocus &&
            m_blupiAction == BlupiAction::March)
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
        }
        if ((m_keyPress & -3) == 0 && m_blupiSpeedX == 0.0 && m_blupiSpeedY == 0.0 && m_blupiSurf && m_blupiFocus &&
            m_blupiAction == BlupiAction::March)
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
        }
        if (m_blupiHelico && (m_blupiFocus || m_blupiAction == BlupiAction::HelicoGlu))
        {
            if (((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Fire)) != 0 && m_blupiTimeFire == 0 && m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::HelicoGlu &&
                flag2)
            {
                if (m_blupiBullet == 0)
                {
                    PlaySound(SoundChannel::SoundChannel53, m_blupiPos);
                }
                else
                {
                    m_blupiAction = BlupiAction::HelicoGlu;
                    m_blupiPhase = 0;
                    m_blupiFocus = false;
                }
            }
            m_blupiMotorHigh = flag2;
            if (m_blupiAction != BlupiAction::Turn && m_blupiAction != BlupiAction::HelicoGlu)
            {
                if (flag2)
                {
                    m_blupiAction = BlupiAction::March;
                }
                else
                {
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                    m_blupiVitesseY = 0.0;
                }
            }
            if (Def::getEasyMoveProperty())
            {
                if (m_blupiSpeedY <= -1.0 || ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
                {
                    if (m_blupiVitesseY > -7.0)
                    {
                        m_blupiVitesseY -= 0.5;
                    }
                    if (m_blupiVitesseY == -0.5)
                    {
                        m_blupiVitesseY = -1.0;
                    }
                }
                else if (m_blupiSpeedY >= 1.0)
                {
                    if (m_blupiVitesseY < 8.0)
                    {
                        m_blupiVitesseY += 0.5;
                    }
                }
                else
                {
                    if (m_blupiVitesseY > 1.0)
                    {
                        m_blupiVitesseY -= 1.0;
                    }
                    if (m_blupiVitesseY < 1.0)
                    {
                        m_blupiVitesseY += 1.0;
                    }
                }
                end.Y += (int)m_blupiVitesseY;
            }
            else
            {
                if (m_blupiSpeedY <= -1.0 || ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
                {
                    if (m_blupiVitesseY > -10.0)
                    {
                        m_blupiVitesseY -= 0.5;
                    }
                }
                else if (m_blupiSpeedY >= 1.0)
                {
                    if (m_blupiVitesseY < 12.0)
                    {
                        m_blupiVitesseY += 0.5;
                    }
                }
                else
                {
                    if (m_blupiVitesseY > 1.0)
                    {
                        m_blupiVitesseY -= 1.0;
                    }
                    if (m_blupiVitesseY < 1.0)
                    {
                        m_blupiVitesseY += 1.0;
                    }
                }
                end.Y += (int)m_blupiVitesseY;
            }
            if (Def::getEasyMoveProperty())
            {
                if (m_blupiSpeedX <= -1.0)
                {
                    int num4 = (int)(m_blupiSpeedX * 12.0);
                    if (m_blupiVitesseX > (double)num4)
                    {
                        m_blupiVitesseX -= 0.5;
                    }
                    celSwitch.X = end.X + (int)m_blupiVitesseX;
                    celSwitch.Y = end.Y;
                    if (BlupiBloque(celSwitch, -1))
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                else if (m_blupiSpeedX >= 1.0)
                {
                    int num5 = (int)(m_blupiSpeedX * 12.0);
                    if (m_blupiVitesseX < (double)num5)
                    {
                        m_blupiVitesseX += 0.5;
                    }
                    celSwitch.X = end.X + (int)m_blupiVitesseX;
                    celSwitch.Y = end.Y;
                    if (BlupiBloque(celSwitch, 1))
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                else
                {
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX -= 2.0;
                        if (m_blupiVitesseX < 0.0)
                        {
                            m_blupiVitesseX = 0.0;
                        }
                    }
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX += 2.0;
                        if (m_blupiVitesseX > 0.0)
                        {
                            m_blupiVitesseX = 0.0;
                        }
                    }
                }
                end.X += (int)m_blupiVitesseX;
            }
            else
            {
                if (m_blupiSpeedX <= -1.0)
                {
                    int num6 = (int)(m_blupiSpeedX * 16.0);
                    if (m_blupiVitesseX > (double)num6)
                    {
                        m_blupiVitesseX -= 1.0;
                    }
                    celSwitch.X = end.X + (int)m_blupiVitesseX;
                    celSwitch.Y = end.Y;
                    if (BlupiBloque(celSwitch, -1))
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                else if (m_blupiSpeedX >= 1.0)
                {
                    int num7 = (int)(m_blupiSpeedX * 16.0);
                    if (m_blupiVitesseX < (double)num7)
                    {
                        m_blupiVitesseX += 1.0;
                    }
                    celSwitch.X = end.X + (int)m_blupiVitesseX;
                    celSwitch.Y = end.Y;
                    if (BlupiBloque(celSwitch, 1))
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                else
                {
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX -= 2.0;
                        if (m_blupiVitesseX < 0.0)
                        {
                            m_blupiVitesseX = 0.0;
                        }
                    }
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX += 2.0;
                        if (m_blupiVitesseX > 0.0)
                        {
                            m_blupiVitesseX = 0.0;
                        }
                    }
                }
                end.X += (int)m_blupiVitesseX;
            }
            MoveObjectPollution();
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && !flag2 && m_blupiTransport == -1)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                rect.Left = m_blupiPos.X + 20;
                rect.Right = m_blupiPos.X + 22;
                rect.Top = m_blupiPos.Y + 60 - 2;
                rect.Bottom = m_blupiPos.Y + 60;
                bVertigoLeft = !DecorDetect(rect);
                rect.Left = m_blupiPos.X + 60 - 22;
                rect.Right = m_blupiPos.X + 60 - 20;
                rect.Top = m_blupiPos.Y + 60 - 2;
                rect.Bottom = m_blupiPos.Y + 60;
                bVertigoRight = !DecorDetect(rect);
                if (!bVertigoLeft && !bVertigoRight)
                {
                    celSwitch.X = m_blupiPos.X;
                    celSwitch.Y = m_blupiPos.Y - BLUPIFLOOR;
                    ObjectStart(celSwitch, ObjectType::ObjectType13, 0);
                    m_blupiHelico = false;
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                    m_blupiPosHelico = m_blupiPos;
                    m_blupiFocus = true;
                    StopSound(SoundChannel::SoundChannel16);
                    StopSound(SoundChannel::SoundChannel18);
                    PlaySound(SoundChannel::SoundChannel17, m_blupiPos);
                }
            }
        }
        if (m_blupiOver && (m_blupiFocus || m_blupiAction == BlupiAction::HelicoGlu))
        {
            m_blupiMotorHigh = flag2;
            if (m_blupiAction != BlupiAction::Turn)
            {
                if (flag2)
                {
                    m_blupiAction = BlupiAction::March;
                }
                else
                {
                    if (m_blupiAction != BlupiAction::Stop)
                    {
                        m_blupiAction = BlupiAction::Stop;
                        m_blupiPhase = 0;
                    }
                    m_blupiVitesseY = 0.0;
                }
            }
            rect = BlupiRect(end);
            rect.Top = end.Y + 60 - 2;
            rect.Bottom = end.Y + 60 + OVERHEIGHT - 1;
            bool flag4 = !DecorDetect(rect);
            icon = MoveAscenseurDetect(m_blupiPos, OVERHEIGHT);
            if (icon != -1)
            {
                flag4 = false;
            }
            if ((m_blupiSpeedY < 0.0 || ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0) && !flag4)
            {
                if (m_blupiVitesseY == 0.0 && icon != -1)
                {
                    m_blupiVitesseY = -5.0;
                }
                else if (m_blupiVitesseY > -5.0)
                {
                    m_blupiVitesseY -= 1.0;
                }
            }
            else if (m_blupiSpeedY > 0.0)
            {
                if (m_blupiVitesseY < 12.0)
                {
                    m_blupiVitesseY += 5.0;
                }
            }
            else if (m_blupiVitesseY < 12.0 && m_time % Config::ScaleTime(2) == 0)
            {
                m_blupiVitesseY += 1.0;
            }
            end.Y += (int)m_blupiVitesseY;
            if (m_blupiSpeedX < 0.0 && flag2)
            {
                int num8 = (int)(m_blupiSpeedX * 12.0);
                if (m_blupiVitesseX > (double)num8)
                {
                    m_blupiVitesseX -= 1.0;
                }
                celSwitch.X = end.X + (int)m_blupiVitesseX;
                celSwitch.Y = end.Y;
                if (BlupiBloque(celSwitch, -1))
                {
                    m_blupiVitesseX = 0.0;
                }
            }
            else if (m_blupiSpeedX > 0.0 && flag2)
            {
                int num9 = (int)(m_blupiSpeedX * 12.0);
                if (m_blupiVitesseX < (double)num9)
                {
                    m_blupiVitesseX += 1.0;
                }
                celSwitch.X = end.X + (int)m_blupiVitesseX;
                celSwitch.Y = end.Y;
                if (BlupiBloque(celSwitch, 1))
                {
                    m_blupiVitesseX = 0.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 1.0;
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 1.0;
                }
            }
            end.X += (int)m_blupiVitesseX;
            MoveObjectPollution();
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && !flag2 && m_blupiTransport == -1)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                rect.Left = m_blupiPos.X + 20;
                rect.Right = m_blupiPos.X + 22;
                rect.Top = m_blupiPos.Y + 60 - 2;
                rect.Bottom = m_blupiPos.Y + 60;
                bVertigoLeft = !DecorDetect(rect);
                rect.Left = m_blupiPos.X + 60 - 22;
                rect.Right = m_blupiPos.X + 60 - 20;
                rect.Top = m_blupiPos.Y + 60 - 2;
                rect.Bottom = m_blupiPos.Y + 60;
                bVertigoRight = !DecorDetect(rect);
                if (!bVertigoLeft && !bVertigoRight)
                {
                    celSwitch.X = m_blupiPos.X;
                    celSwitch.Y = m_blupiPos.Y - BLUPIFLOOR;
                    ObjectStart(celSwitch, ObjectType::ObjectType46, 0);
                    m_blupiOver = false;
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                    m_blupiPosHelico = m_blupiPos;
                    m_blupiFocus = true;
                    StopSound(SoundChannel::SoundChannel16);
                    StopSound(SoundChannel::SoundChannel18);
                    PlaySound(SoundChannel::SoundChannel17, m_blupiPos);
                }
            }
        }
        if (m_blupiBalloon && m_blupiFocus)
        {
            if (m_blupiSpeedY < 0.0 || ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
            {
                if (m_blupiVitesseY > -5.0 && m_time % Config::ScaleTime(6) == 0)
                {
                    m_blupiVitesseY -= 1.0;
                }
            }
            else if (m_blupiSpeedY > 0.0)
            {
                if (m_blupiVitesseY < 0.0 && m_time % Config::ScaleTime(6) == 0)
                {
                    m_blupiVitesseY += 1.0;
                }
            }
            else if (m_blupiVitesseY > -3.0 && m_time % Config::ScaleTime(6) == 0)
            {
                m_blupiVitesseY -= 1.0;
            }
            end.Y += (int)m_blupiVitesseY;
            if (m_blupiSpeedX < 0.0)
            {
                int num10 = (int)(m_blupiSpeedX * 10.0);
                if (m_blupiVitesseX > (double)num10)
                {
                    m_blupiVitesseX -= 1.0;
                }
                celSwitch.X = end.X + (int)m_blupiVitesseX;
                celSwitch.Y = end.Y;
                if (BlupiBloque(celSwitch, -1))
                {
                    m_blupiVitesseX = 0.0;
                }
            }
            else if (m_blupiSpeedX > 0.0)
            {
                int num11 = (int)(m_blupiSpeedX * 10.0);
                if (m_blupiVitesseX < (double)num11)
                {
                    m_blupiVitesseX += 1.0;
                }
                celSwitch.X = end.X + (int)m_blupiVitesseX;
                celSwitch.Y = end.Y;
                if (BlupiBloque(celSwitch, 1))
                {
                    m_blupiVitesseX = 0.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 2.0;
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 2.0;
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
            }
            end.X += (int)m_blupiVitesseX;
        }
        if (m_blupiEcrase && m_blupiFocus)
        {
            if (flag2)
            {
                if (m_blupiVitesseY < 2.0)
                {
                    m_blupiVitesseY += 1.0;
                }
            }
            else
            {
                m_blupiVitesseY = 0.0;
            }
            end.Y += (int)m_blupiVitesseY;
            num2 = ((!flag2) ? 4 : 7);
            num2 = (int)((double)num2 * m_blupiSpeedX);
            if (m_blupiSpeedX < 0.0)
            {
                if (m_blupiVitesseX > (double)num2)
                {
                    m_blupiVitesseX -= 1.0;
                }
                celSwitch.X = end.X + (int)m_blupiVitesseX;
                celSwitch.Y = end.Y;
                if (BlupiBloque(celSwitch, -1))
                {
                    m_blupiVitesseX = 0.0;
                }
            }
            else if (m_blupiSpeedX > 0.0)
            {
                if (m_blupiVitesseX < (double)num2)
                {
                    m_blupiVitesseX += 1.0;
                }
                celSwitch.X = end.X + (int)m_blupiVitesseX;
                celSwitch.Y = end.Y;
                if (BlupiBloque(celSwitch, 1))
                {
                    m_blupiVitesseX = 0.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 2.0;
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 2.0;
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
            }
            if (m_blupiVitesseX == 0.0 && !flag2)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            end.X += (int)m_blupiVitesseX;
        }
        if (m_blupiJeep && m_blupiFocus)
        {
            if (m_blupiVitesseX == 0.0 && m_blupiAction == BlupiAction::March)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            m_blupiMotorHigh = m_blupiAction != BlupiAction::Stop;
            rect = BlupiRect(end);
            rect.Right -= 40;
            rect.Top = end.Y + 60 - 2;
            rect.Bottom = end.Y + 60 - 1;
            bool flag5 = !DecorDetect(rect);
            rect.Left += 40;
            rect.Right += 40;
            bool flag6 = !DecorDetect(rect);
            if (flag2)
            {
                if (m_blupiVitesseY < 50.0)
                {
                    m_blupiVitesseY += 5.0;
                }
            }
            else
            {
                if (m_blupiVitesseY != 0.0)
                {
                    PlaySound(SoundChannel::SoundChannel3, m_blupiPos);
                }
                m_blupiVitesseY = 0.0;
            }
            end.Y += (int)m_blupiVitesseY;
            if (m_blupiTransport == -1)
            {
                rect.Left = end.X + 20;
                rect.Right = end.X + 60 - 20;
                rect.Top = end.Y + 60 - 35;
                rect.Bottom = end.Y + 60 - 1;
                icon = AscenseurDetect(rect, m_blupiPos, end);
                if (m_blupiVitesseY >= 0.0 && icon != -1)
                {
                    m_blupiTransport = icon;
                    flag2 = false;
                    PlaySound(SoundChannel::SoundChannel3, end);
                    end.Y = m_moveObject[icon].posCurrent.Y - 64 + BLUPIOFFY;
                }
            }
            if (flag5 && !flag6)
            {
                int num12 = -20;
                if (m_blupiVitesseX > (double)num12)
                {
                    m_blupiVitesseX -= 1.0;
                }
            }
            else if (!flag5 && flag6)
            {
                int num13 = 20;
                if (m_blupiVitesseX < (double)num13)
                {
                    m_blupiVitesseX += 1.0;
                }
            }
            else if (m_blupiSpeedX < 0.0)
            {
                int num14 = (int)(m_blupiSpeedX * 20.0);
                if (m_blupiVitesseX > (double)num14)
                {
                    m_blupiVitesseX -= 1.0;
                }
            }
            else if (m_blupiSpeedX > 0.0)
            {
                int num15 = (int)(m_blupiSpeedX * 20.0);
                if (m_blupiVitesseX < (double)num15)
                {
                    m_blupiVitesseX += 1.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 2.0;
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 2.0;
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
            }
            if (m_blupiAction == BlupiAction::Turn)
            {
                m_blupiVitesseX = 0.0;
            }
            end.X += (int)m_blupiVitesseX;
            if (flag5 && !flag6)
            {
                m_blupiRealRotation = Misc::Approach(m_blupiRealRotation, -45, 5);
            }
            else if (!flag5 && flag6)
            {
                m_blupiRealRotation = Misc::Approach(m_blupiRealRotation, 45, 5);
            }
            else if (!flag2)
            {
                m_blupiRealRotation = 0;
            }
            m_blupiOffsetY = std::abs(m_blupiRealRotation / 2);
            MoveObjectPollution();
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && !flag2 && m_blupiTransport == -1)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = m_blupiPos.Y - BLUPIFLOOR;
                ObjectStart(celSwitch, ObjectType::ObjectType19, 0);
                m_blupiJeep = false;
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiPosHelico = m_blupiPos;
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel30, m_blupiPos);
            }
        }
        if (m_blupiTank && m_blupiFocus)
        {
            if (m_blupiAction == BlupiAction::FireTank && m_blupiPhase == Config::ScaleTime(6))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            if (((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Fire)) != 0 && m_blupiTimeFire == 0 && m_blupiAction != BlupiAction::Turn)
            {
                if (m_blupiBullet == 0)
                {
                    PlaySound(SoundChannel::SoundChannel53, m_blupiPos);
                }
                else
                {
                    if (m_blupiDir == Direction::Left)
                    {
                        celSwitch.X = m_blupiPos.X - 35;
                        celSwitch.Y = m_blupiPos.Y;
                        num3 = -5;
                        m_blupiVitesseX += 12.0;
                    }
                    else
                    {
                        celSwitch.X = m_blupiPos.X + 35;
                        celSwitch.Y = m_blupiPos.Y;
                        num3 = 5;
                        m_blupiVitesseX -= 12.0;
                    }
                    if (ObjectStart(celSwitch, ObjectType::ObjectType23, num3) != -1)
                    {
                        m_blupiAction = BlupiAction::FireTank;
                        m_blupiPhase = 0;
                        PlaySound(SoundChannel::SoundChannel52, m_blupiPos);
                        m_blupiTimeFire = Config::ScaleTime(10);
                        m_blupiBullet--;
                    }
                }
            }
            if (m_blupiVitesseX == 0.0 && m_blupiAction == BlupiAction::March)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            m_blupiMotorHigh = m_blupiAction != BlupiAction::Stop;
            if (flag2)
            {
                if (m_blupiVitesseY < 50.0)
                {
                    m_blupiVitesseY += 5.0;
                }
            }
            else
            {
                if (m_blupiVitesseY != 0.0)
                {
                    PlaySound(SoundChannel::SoundChannel3, m_blupiPos);
                }
                m_blupiVitesseY = 0.0;
                m_blupiSubPixelY = 0.0;
            }
            if (Config::FPS == Fps::Fps20)
            {
                end.Y += (int)m_blupiVitesseY;
            }
            else
            {
                m_blupiSubPixelY += m_blupiVitesseY * Config::SPEED_SCALE;
                int wholePixelsY = static_cast<int>(std::floor(m_blupiSubPixelY));
                m_blupiSubPixelY -= wholePixelsY;
                end.Y += wholePixelsY;
            }
            if (m_blupiTransport == -1)
            {
                rect.Left = end.X + 20;
                rect.Right = end.X + 60 - 20;
                rect.Top = end.Y + 60 - 35;
                rect.Bottom = end.Y + 60 - 1;
                icon = AscenseurDetect(rect, m_blupiPos, end);
                if (m_blupiVitesseY >= 0.0 && icon != -1)
                {
                    m_blupiTransport = icon;
                    flag2 = false;
                    PlaySound(SoundChannel::SoundChannel3, end);
                    end.Y = m_moveObject[icon].posCurrent.Y - 64 + BLUPIOFFY;
                }
            }
            if (m_blupiSpeedX < 0.0)
            {
                int num16 = (int)(m_blupiSpeedX * 12.0);
                if (m_blupiVitesseX > (double)num16)
                {
                    m_blupiVitesseX -= 1.0;
                }
            }
            else if (m_blupiSpeedX > 0.0)
            {
                int num17 = (int)(m_blupiSpeedX * 12.0);
                if (m_blupiVitesseX < (double)num17)
                {
                    m_blupiVitesseX += 1.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 3.0;
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 3.0;
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
            }
            if (m_blupiAction == BlupiAction::Turn)
            {
                m_blupiVitesseX = 0.0;
            }
            end.X += (int)m_blupiVitesseX;
            MoveObjectPollution();
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && !flag2 && m_blupiTransport == -1)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = m_blupiPos.Y;
                ObjectStart(celSwitch, ObjectType::ObjectType28, 0);
                m_blupiTank = false;
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiPosHelico = m_blupiPos;
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel30, m_blupiPos);
            }
        }
        if (m_blupiSkate && m_blupiFocus)
        {
            if (m_blupiVitesseX == 0.0 && m_blupiAction == BlupiAction::March)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            if (m_blupiSpeedX < 0.0)
            {
                int num18 = (int)(m_blupiSpeedX * 15.0);
                if (m_blupiVitesseX > (double)num18)
                {
                    m_blupiVitesseX -= 1.0;
                }
            }
            else if (m_blupiSpeedX > 0.0)
            {
                int num19 = (int)(m_blupiSpeedX * 15.0);
                if (m_blupiVitesseX < (double)num19)
                {
                    m_blupiVitesseX += 1.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 1.0;
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 1.0;
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
            }
            if (m_blupiAction == BlupiAction::Turn)
            {
                m_blupiVitesseX = 0.0;
            }
            end.X += (int)m_blupiVitesseX;
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && !flag2 && !m_blupiAir && m_blupiTransport
                == -1 && m_blupiVitesseX < 8.0)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                m_blupiSkate = false;
                m_blupiAction = BlupiAction::DeposeSkate;
                m_blupiPhase = 0;
                m_blupiFocus = false;
                m_blupiPosHelico = m_blupiPos;
            }
        }
        if (m_blupiAction == BlupiAction::TakeSkate)
        {
            if (m_blupiPhase == Config::ScaleTime(8))
            {
                icon = MoveObjectDetect(m_blupiPos, bNear);
                if (icon != -1)
                {
                    ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                }
            }
            if (m_blupiPhase == Config::ScaleTime(20))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
        }
        if (m_blupiAction == BlupiAction::DeposeSkate)
        {
            if (m_blupiPhase == Config::ScaleTime(12))
            {
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = m_blupiPos.Y - BLUPIFLOOR + 1;
                ObjectStart(celSwitch, ObjectType::ObjectType24, 0);
            }
            if (m_blupiPhase == Config::ScaleTime(20))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
        }
        if (m_blupiNage && m_blupiFocus)
        {
            if (m_blupiTransport == -1)
            {
                if (m_blupiSpeedY < 0.0 || ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
                {
                    if (m_blupiVitesseY > -5.0)
                    {
                        m_blupiVitesseY -= 1.0;
                    }
                }
                else if (m_blupiSpeedY > 0.0)
                {
                    if (m_blupiVitesseY < 5.0)
                    {
                        m_blupiVitesseY += 1.0;
                    }
                }
                else
                {
                    num2 = ((m_blupiAction == BlupiAction::Stop) ? (-1) : 0);
                    if (m_blupiVitesseY > (double)num2)
                    {
                        m_blupiVitesseY -= 1.0;
                    }
                    if (m_blupiVitesseY < (double)num2)
                    {
                        m_blupiVitesseY += 1.0;
                    }
                }
                end.Y += (int)m_blupiVitesseY;
            }
            if (m_blupiSpeedX < 0.0)
            {
                int num20 = (int)(m_blupiSpeedX * 8.0);
                if (m_blupiVitesseX > (double)num20)
                {
                    m_blupiVitesseX -= 1.0;
                }
            }
            else if (m_blupiSpeedX > 0.0)
            {
                int num21 = (int)(m_blupiSpeedX * 8.0);
                if (m_blupiVitesseX < (double)num21)
                {
                    m_blupiVitesseX += 1.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 2.0;
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 2.0;
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
            }
            icon = Tables::table_vitesse_nage[m_blupiPhase % 14 / 2];
            end.X += (int)(m_blupiVitesseX * (double)icon / 7.0);
            if (m_time % Config::ScaleTime(70) == 0 || m_time % Config::ScaleTime(70) == Config::ScaleTime(28))
            {
                MoveObjectBlup(end);
            }
            if (m_time % Config::ScaleTime(5) == 0)
            {
                if (!m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
                {
                    m_blupiLevel--;
                }
                if (m_blupiLevel == 25)
                {
                    m_jauges[0].SetMode(JaugeMode::Red);
                }
                m_jauges[0].SetLevel(m_blupiLevel);
                if (m_blupiLevel == 0)
                {
                    m_blupiAction = BlupiAction::Drown;
                    m_blupiPhase = 0;
                    m_blupiFocus = false;
                    m_blupiHelico = false;
                    m_blupiOver = false;
                    m_blupiJeep = false;
                    m_blupiTank = false;
                    m_blupiSkate = false;
                    m_blupiNage = false;
                    m_blupiSurf = false;
                    m_blupiSuspend = false;
                    m_blupiJumpAie = false;
                    m_blupiShield = false;
                    m_blupiPower = false;
                    m_blupiCloud = false;
                    m_blupiHide = false;
                    m_blupiInvert = false;
                    m_blupiBalloon = false;
                    m_blupiEcrase = false;
                    m_blupiAir = false;
                    m_blupiRestart = true;
                    m_blupiActionOuf = BlupiAction::None;
                    m_jauges[0].SetHide(true);
                    m_jauges[1].SetHide(true);
                    PlaySound(SoundChannel::SoundChannel26, end);
                }
            }
        }
        if (m_blupiSurf && m_blupiFocus)
        {
            if (m_blupiTransport == -1)
            {
                if (m_blupiSpeedY < 0.0 || ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
                {
                    if (m_blupiVitesseY > -5.0)
                    {
                        m_blupiVitesseY -= 1.0;
                    }
                }
                else if (m_blupiSpeedY > 0.0)
                {
                    if (m_blupiVitesseY < 5.0)
                    {
                        m_blupiVitesseY += 1.0;
                    }
                }
                else
                {
                    if (m_blupiVitesseY > -2.0)
                    {
                        m_blupiVitesseY -= 1.0;
                    }
                    if (m_blupiVitesseY < -2.0)
                    {
                        m_blupiVitesseY += 1.0;
                    }
                }
                end.Y += (int)m_blupiVitesseY;
                end.Y += BLUPISURF;
                if (end.Y % 64 > 30)
                {
                    end.Y += 64 - end.Y % 64;
                }
                end.Y -= BLUPISURF;
            }
            if (m_blupiSpeedX < 0.0)
            {
                int num22 = (int)(m_blupiSpeedX * 8.0);
                if (m_blupiVitesseX > (double)num22)
                {
                    m_blupiVitesseX -= 1.0;
                }
            }
            else if (m_blupiSpeedX > 0.0)
            {
                int num23 = (int)(m_blupiSpeedX * 8.0);
                if (m_blupiVitesseX < (double)num23)
                {
                    m_blupiVitesseX += 1.0;
                }
            }
            else
            {
                if (m_blupiVitesseX > 0.0)
                {
                    m_blupiVitesseX -= 2.0;
                    if (m_blupiVitesseX < 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
                if (m_blupiVitesseX < 0.0)
                {
                    m_blupiVitesseX += 2.0;
                    if (m_blupiVitesseX > 0.0)
                    {
                        m_blupiVitesseX = 0.0;
                    }
                }
            }
            icon = Tables::table_vitesse_surf[m_blupiPhase % 12 / 2];
            end.X += (int)(m_blupiVitesseX * (double)icon / 10.0);
        }
        TinyPoint tinyPoint;
        if (m_blupiSuspend && m_blupiFocus)
        {
            if (m_blupiSpeedX < 0.0 && m_blupiAction == BlupiAction::March)
            {
                int num24 = (int)(m_blupiSpeedX * 5.0);
                end.X += num24;
            }
            if (m_blupiSpeedX > 0.0 && m_blupiAction == BlupiAction::March)
            {
                int num25 = (int)(m_blupiSpeedX * 5.0);
                end.X += num25;
            }
            icon = GetTypeBarre(end);
            if (icon == 2)
            {
                tinyPoint.X = end.X;
                tinyPoint.Y = end.Y / 64 * 64 + BLUPIOFFY;
                rect = BlupiRect(tinyPoint);
                if (!DecorDetect(rect, true))
                {
                    m_blupiSuspend = false;
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                    end = (m_blupiPos = tinyPoint);
                }
            }
            if ((m_blupiSpeedY > 0.0 && m_blupiPhase > 5) || icon == 0)
            {
                m_blupiSuspend = false;
                m_blupiAir = true;
                m_blupiAction = BlupiAction::Air;
                end.Y = end.Y; //Todo : check : Assignment made to same variable; did you mean to assign something else?
                m_blupiVitesseY = 0.0;
                m_blupiNoBarre = Config::ScaleTime(5);
                m_blupiActionOuf = BlupiAction::Ouf5;
                m_blupiTimeOuf = 0;
            }
            if ((((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0 || m_blupiSpeedY < 0.0) && m_blupiAction != BlupiAction::Jump &&
                m_blupiAction != BlupiAction::Turn)
            {
                m_blupiAction = BlupiAction::Jump;
                m_blupiPhase = 0;
            }
            if ((m_keyPress & ToRaw(KeyPressFlags::Jump)) == 0 && m_blupiSpeedY == 0.0 && m_blupiAction == BlupiAction::Jump)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
            if (m_blupiAction == BlupiAction::Jump && m_blupiPhase == Config::ScaleTime(10))
            {
                m_blupiSuspend = false;
                m_blupiAir = true;
                m_blupiAction = BlupiAction::Air;
                m_blupiPhase = 0;
                end.Y -= 2;
                m_blupiVitesseY = -11.0;
                m_blupiNoBarre = Config::ScaleTime(5);
                PlaySound(SoundChannel::SoundChannel35, end);
            }
        }
        if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && !m_blupiHelico && !m_blupiOver && !
            m_blupiBalloon && !m_blupiEcrase && !m_blupiTank && !m_blupiJeep && !m_blupiSkate && !flag2 &&
            m_blupiTransport == -1 && m_blupiFocus)
        {
            if (m_blupiDynamite > 0)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                rect.Left = end.X + 18;
                rect.Right = end.X + 20;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoLeft = !DecorDetect(rect);
                rect.Left = end.X + 60 - 20;
                rect.Right = end.X + 60 - 18;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoRight = !DecorDetect(rect);
                if (!bVertigoLeft && !bVertigoRight && ObjectStart(end, ObjectType::ObjectType56, 0) != -1)
                {
                    m_blupiAction = BlupiAction::PutDynamite;
                    m_blupiPhase = 0;
                    m_blupiFocus = false;
                    PlaySound(SoundChannel::SoundChannel61, end);
                    m_blupiDynamite--;
                }
            }
            else if (m_blupiPerso > 0)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                icon = MoveObjectDetect(end, bNear);
                if (icon == -1 || m_moveObject[icon].type != ObjectType::ObjectType200)
                {
                    rect.Left = end.X + 18;
                    rect.Right = end.X + 20;
                    rect.Top = end.Y + 60 - 2;
                    rect.Bottom = end.Y + 60;
                    bVertigoLeft = !DecorDetect(rect);
                    rect.Left = end.X + 60 - 20;
                    rect.Right = end.X + 60 - 18;
                    rect.Top = end.Y + 60 - 2;
                    rect.Bottom = end.Y + 60;
                    bVertigoRight = !DecorDetect(rect);
                    icon = MoveChargeDetect(end);
                    if (icon == -1 && !bVertigoLeft && !bVertigoRight && ObjectStart(end, ObjectType::ObjectType200, 0) != -1)
                    {
                        m_blupiAction = BlupiAction::Ouf2;
                        m_blupiPhase = 0;
                        m_blupiFocus = false;
                        PlaySound(SoundChannel::SoundChannel61, end);
                        m_blupiPerso--;
                    }
                }
                if (m_blupiFocus)
                {
                    m_blupiAction = BlupiAction::Ouf3;
                    m_blupiPhase = 0;
                    PlaySound(SoundChannel::SoundChannel27, end);
                }
            }
        }
        rect = BlupiRect(m_blupiPos);
        tinyPoint = end;
        TestPath(rect, m_blupiPos, end);
        if (flag && m_blupiPos.X == end.X && m_blupiPos.X != tinyPoint.X)
        {
            end.Y = tinyPoint.Y;
            TestPath(rect, m_blupiPos, end);
        }
        if (m_blupiVent && m_blupiPos.Y == end.Y && m_blupiPos.Y != tinyPoint.Y)
        {
            end.X = tinyPoint.X;
            TestPath(rect, m_blupiPos, end);
        }
        if (m_blupiTransport != -1 && m_blupiPos.X == end.X && m_blupiPos.X != tinyPoint.X)
        {
            end.Y = tinyPoint.Y;
            TestPath(rect, m_blupiPos, end);
        }
        if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase || m_blupiJeep || m_blupiTank ||
            m_blupiSkate || m_blupiNage)
        {
            if (m_blupiPos.X == end.X && m_blupiPos.X != tinyPoint.X)
            {
                end.Y = tinyPoint.Y;
                TestPath(rect, m_blupiPos, end);
            }
            else if (m_blupiPos.Y == end.Y && m_blupiPos.Y != tinyPoint.Y)
            {
                end.X = tinyPoint.X;
                TestPath(rect, m_blupiPos, end);
            }
        }
        TinyPoint blupiPos = m_blupiPos;
        m_blupiPos = end;
        if ((m_blupiAction == BlupiAction::Stop || m_blupiAction == BlupiAction::StopMarch || m_blupiAction == BlupiAction::Up || m_blupiAction == BlupiAction::Down) && !m_blupiAir && !
            m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !
            m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            if (m_blupiTransport != -1)
            {
                AscenseurVertigo(m_blupiTransport, bVertigoLeft, bVertigoRight);
            }
            else
            {
                rect.Left = end.X + 24;
                rect.Right = end.X + 26;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoLeft = !DecorDetect(rect);
                rect.Left = end.X + 60 - 26;
                rect.Right = end.X + 60 - 24;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoRight = !DecorDetect(rect);
            }
            if (m_blupiDir == Direction::Left && bVertigoLeft && !bVertigoRight)
            {
                if (m_blupiHelico || m_blupiOver || AscenseurShift(m_blupiTransport))
                {
                    m_blupiAction = BlupiAction::Recede;
                    m_blupiPhase = 0;
                }
                else
                {
                    PlaySound(SoundChannel::SoundChannel6, end);
                    m_blupiAction = BlupiAction::Vertigo;
                    m_blupiPhase = 0;
                }
            }
            if (m_blupiDir == Direction::Right && !bVertigoLeft && bVertigoRight)
            {
                if (m_blupiHelico || m_blupiOver || AscenseurShift(m_blupiTransport))
                {
                    m_blupiAction = BlupiAction::Recede;
                    m_blupiPhase = 0;
                }
                else
                {
                    PlaySound(SoundChannel::SoundChannel6, end);
                    m_blupiAction = BlupiAction::Vertigo;
                    m_blupiPhase = 0;
                }
            }
            if (m_blupiAction != BlupiAction::Vertigo && m_blupiAction != BlupiAction::Advance && m_blupiAction != BlupiAction::Recede && (bVertigoLeft || bVertigoRight))
            {
                if (!m_blupiHelico && !m_blupiOver)
                {
                    PlaySound(SoundChannel::SoundChannel6, end);
                }
                m_blupiAction = BlupiAction::Advance;
                m_blupiPhase = 0;
            }
        }
        if (m_blupiAction == BlupiAction::Stop && m_blupiJeep && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase &&
            !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            if (m_blupiTransport != -1)
            {
                AscenseurVertigo(m_blupiTransport, bVertigoLeft, bVertigoRight);
            }
            else
            {
                rect.Left = end.X + 2;
                rect.Right = end.X + 18;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoLeft = !DecorDetect(rect);
                rect.Left = end.X + 60 - 18;
                rect.Right = end.X + 60 - 2;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoRight = !DecorDetect(rect);
            }
            if (bVertigoLeft && !bVertigoRight)
            {
                m_blupiAction = BlupiAction::Advance;
                m_blupiPhase = 0;
            }
            if (bVertigoRight && !bVertigoLeft)
            {
                m_blupiAction = BlupiAction::Advance;
                m_blupiPhase = 0;
            }
        }
        if (m_blupiAction == BlupiAction::Stop && m_blupiTank && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase &&
            !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            if (m_blupiTransport != -1)
            {
                AscenseurVertigo(m_blupiTransport, bVertigoLeft, bVertigoRight);
            }
            else
            {
                rect.Left = end.X + 2;
                rect.Right = end.X + 18;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoLeft = !DecorDetect(rect);
                rect.Left = end.X + 60 - 18;
                rect.Right = end.X + 60 - 2;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoRight = !DecorDetect(rect);
            }
            if (bVertigoLeft && !bVertigoRight)
            {
                m_blupiAction = BlupiAction::Advance;
                m_blupiPhase = 0;
            }
            if (bVertigoRight && !bVertigoLeft)
            {
                m_blupiAction = BlupiAction::Advance;
                m_blupiPhase = 0;
            }
        }
        if (m_blupiAction == BlupiAction::Stop && m_blupiSkate && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase &&
            !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
        {
            if (m_blupiTransport != -1)
            {
                AscenseurVertigo(m_blupiTransport, bVertigoLeft, bVertigoRight);
            }
            else
            {
                rect.Left = end.X + 12;
                rect.Right = end.X + 19;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoLeft = !DecorDetect(rect);
                rect.Left = end.X + 60 - 19;
                rect.Right = end.X + 60 - 12;
                rect.Top = end.Y + 60 - 2;
                rect.Bottom = end.Y + 60;
                bVertigoRight = !DecorDetect(rect);
            }
            if (bVertigoLeft && !bVertigoRight)
            {
                m_blupiAction = BlupiAction::Advance;
                m_blupiPhase = 0;
            }
            if (bVertigoRight && !bVertigoLeft)
            {
                m_blupiAction = BlupiAction::Advance;
                m_blupiPhase = 0;
            }
        }
        if (m_blupiFocus)
        {
            if (m_blupiAction == BlupiAction::Vertigo && m_blupiPhase == Config::ScaleTime(16))
            {
                m_blupiAction = BlupiAction::Recede;
                m_blupiPhase = 0;
            }
            if (m_blupiAction == BlupiAction::Recede && m_blupiPhase == Config::ScaleTime(3))
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiActionOuf = BlupiAction::None;
            }
            num2 = Config::ScaleTime(5);
            if (m_blupiJeep)
            {
                num2 = Config::ScaleTime(10);
            }
            if (m_blupiTank)
            {
                num2 = Config::ScaleTime(10);
            }
            if (m_blupiSkate)
            {
                num2 = Config::ScaleTime(10);
            }
            if (m_blupiAction == BlupiAction::Advance && m_blupiPhase == num2)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
        }
        BlupiSearchIcon();
        if (m_blupiShield)
        {
            if (m_blupiTimeShield == 10)
            {
                PlaySound(SoundChannel::SoundChannel43, m_blupiPos);
            }
            if (m_blupiTimeShield == 0)
            {
                m_blupiShield = false;
                m_jauges[1].SetHide(true);
            }
            else if (m_time % Config::ScaleTime(5) == 0)
            {
                m_blupiTimeShield--;
                m_jauges[1].SetLevel(m_blupiTimeShield);
            }
        }
        if (m_blupiPower)
        {
            if (m_blupiTimeShield == 20)
            {
                PlaySound(SoundChannel::SoundChannel45, m_blupiPos);
            }
            if (m_blupiTimeShield == 0)
            {
                m_blupiPower = false;
                m_jauges[1].SetHide(true);
            }
            else if (m_time % Config::ScaleTime(3) == 0)
            {
                m_blupiTimeShield--;
                m_jauges[1].SetLevel(m_blupiTimeShield);
            }
        }
        if (m_blupiCloud)
        {
            if (m_blupiTimeShield == 25)
            {
                PlaySound(SoundChannel::SoundChannel56, m_blupiPos);
            }
            if (m_blupiTimeShield == 0)
            {
                m_blupiCloud = false;
                m_jauges[1].SetHide(true);
            }
            else if (m_time % Config::ScaleTime(4) == 0)
            {
                m_blupiTimeShield--;
                m_jauges[1].SetLevel(m_blupiTimeShield);
            }
        }
        if (m_blupiHide)
        {
            if (m_blupiTimeShield == 20)
            {
                PlaySound(SoundChannel::SoundChannel63, m_blupiPos);
            }
            if (m_blupiTimeShield == 0)
            {
                m_blupiHide = false;
                m_jauges[1].SetHide(true);
            }
            else if (m_time % Config::ScaleTime(4) == 0)
            {
                m_blupiTimeShield--;
                m_jauges[1].SetLevel(m_blupiTimeShield);
            }
        }
        if (m_blupiInvert)
        {
            if (m_blupiTimeShield == 0)
            {
                m_blupiInvert = false;
                m_jauges[1].SetHide(true);
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = m_blupiPos.Y + 100;
                ObjectStart(celSwitch, ObjectType::ObjectType42, -60);
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = m_blupiPos.Y - 100;
                ObjectStart(celSwitch, ObjectType::ObjectType42, 60);
                celSwitch.X = m_blupiPos.X - 100;
                celSwitch.Y = m_blupiPos.Y;
                ObjectStart(celSwitch, ObjectType::ObjectType42, 10);
                celSwitch.X = m_blupiPos.X + 100;
                celSwitch.Y = m_blupiPos.Y;
                ObjectStart(celSwitch, ObjectType::ObjectType42, -10);
                PlaySound(SoundChannel::SoundChannel67, end);
            }
            else if (m_time % Config::ScaleTime(3) == 0)
            {
                m_blupiTimeShield--;
                m_jauges[1].SetLevel(m_blupiTimeShield);
            }
        }
        if (m_blupiBalloon)
        {
            if (m_blupiTimeShield == 0)
            {
                m_blupiBalloon = false;
                m_jauges[1].SetHide(true);
                celSwitch.X = m_blupiPos.X - 34;
                celSwitch.Y = m_blupiPos.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType91, 0);
                PlaySound(SoundChannel::SoundChannel41, m_blupiPos);
            }
            else if (m_time % Config::ScaleTime(2) == 0)
            {
                m_blupiTimeShield--;
                m_jauges[1].SetLevel(m_blupiTimeShield);
            }
        }
        if (m_blupiEcrase)
        {
            if (m_blupiTimeShield == 0)
            {
                m_blupiEcrase = false;
                m_blupiAir = true;
                m_jauges[1].SetHide(true);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, -60);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, 60);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, 10);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, -10);
                celSwitch.X = m_blupiPos.X - 34;
                celSwitch.Y = m_blupiPos.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType90, 0);
                PlaySound(SoundChannel::SoundChannel41, m_blupiPos);
            }
            else if (m_time % Config::ScaleTime(2) == 0)
            {
                m_blupiTimeShield--;
                m_jauges[1].SetLevel(m_blupiTimeShield);
            }
        }
        if (m_blupiPower && std::abs(m_blupiPos.X - m_blupiPosMagic.X) + std::abs(m_blupiPos.Y - m_blupiPosMagic.Y) >=
            40)
        {
            icon = MoveObjectFree();
            if (icon != -1)
            {
                m_moveObject[icon].type = ObjectType::ObjectType27;
                m_moveObject[icon].phase = 0;
                m_moveObject[icon].posCurrent = m_blupiPos;
                m_moveObject[icon].posStart = m_moveObject[icon].posCurrent;
                m_moveObject[icon].posEnd = m_moveObject[icon].posCurrent;
                m_moveObject[icon].step = 1;
                m_moveObject[icon].time = 0;
                MoveObjectStepIcon(icon);
                m_blupiPosMagic = m_blupiPos;
            }
        }
        if (m_blupiShield && std::abs(m_blupiPos.X - m_blupiPosMagic.X) + std::abs(m_blupiPos.Y - m_blupiPosMagic.Y) >=
            40)
        {
            icon = MoveObjectFree();
            if (icon != -1)
            {
                m_moveObject[icon].type = ObjectType::ObjectType57;
                m_moveObject[icon].phase = 0;
                m_moveObject[icon].posCurrent = m_blupiPos;
                m_moveObject[icon].posStart = m_moveObject[icon].posCurrent;
                m_moveObject[icon].posEnd = m_moveObject[icon].posCurrent;
                m_moveObject[icon].step = 1;
                m_moveObject[icon].time = 0;
                MoveObjectStepIcon(icon);
                m_blupiPosMagic = m_blupiPos;
            }
        }
        if (m_blupiHide && std::abs(m_blupiPos.X - m_blupiPosMagic.X) + std::abs(m_blupiPos.Y - m_blupiPosMagic.Y) >=
            10)
        {
            icon = MoveObjectFree();
            if (icon != -1)
            {
                m_moveObject[icon].type = ObjectType::ObjectType58;
                m_moveObject[icon].icon = m_blupiIcon;
                m_moveObject[icon].channel = PixmapChannel::Blupi;
                m_moveObject[icon].phase = 0;
                m_moveObject[icon].posCurrent = m_blupiPos;
                m_moveObject[icon].posStart = m_moveObject[icon].posCurrent;
                m_moveObject[icon].posEnd = m_moveObject[icon].posCurrent;
                m_moveObject[icon].step = 1;
                m_moveObject[icon].time = 0;
                MoveObjectStepIcon(icon);
                m_blupiPosMagic = m_blupiPos;
            }
        }
        if (m_blupiTimeNoAsc > 0)
        {
            m_blupiTimeNoAsc--;
        }
        if (m_blupiHelico && m_blupiPos.Y > 2 && m_blupiFocus && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
        {
            rect = BlupiRect(end);
            rect.Top = end.Y + 4;
            rect.Bottom = end.Y + 20;
            if (DecorDetect(rect))
            {
                ByeByeHelico();
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiHelico = false;
                celSwitch.X = m_blupiPos.X - 34;
                celSwitch.Y = m_blupiPos.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType9, 0);
                m_decorAction = DecorAction::SmallShake;
                m_decorPhase = 0;
                StopSound(SoundChannel::SoundChannel16);
                StopSound(SoundChannel::SoundChannel18);
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel10, m_blupiPos);
            }
        }
        if (!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
            m_blupiSkate && m_blupiFocus)
        {
            if (!m_blupiNage && !m_blupiSurf && IsSurfWater(m_blupiPos))
            {
                m_scrollAdd.X = 0;
                m_scrollAdd.Y = 0;
                m_blupiAir = false;
                m_blupiNage = false;
                m_blupiSurf = true;
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiVitesseX = 0.0;
                m_blupiVitesseY = 0.0;
                MoveObjectPlouf(m_blupiPos);
                if (m_blupiTransport != -1)
                {
                    m_blupiPos.Y -= 10;
                    m_blupiTransport = -1;
                }
                if (m_blupiCloud)
                {
                    m_blupiCloud = false;
                    m_jauges[1].SetHide(true);
                }
            }
            if (!m_blupiNage && !IsSurfWater(m_blupiPos) && IsDeepWater(m_blupiPos))
            {
                if (!m_blupiSurf)
                {
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                    m_blupiVitesseX = 0.0;
                    m_blupiVitesseY = 0.0;
                    MoveObjectPlouf(m_blupiPos);
                }
                m_blupiAir = false;
                m_blupiSurf = false;
                m_blupiNage = true;
                m_blupiLevel = 100;
                m_jauges[0].SetLevel(m_blupiLevel);
                m_jauges[0].SetMode(JaugeMode::Blue);
                m_jauges[0].SetHide(false);
            }
            if (m_blupiNage && IsSurfWater(m_blupiPos))
            {
                m_blupiAir = false;
                m_blupiNage = false;
                m_blupiSurf = true;
                PlaySound(SoundChannel::SoundChannel25, m_blupiPos);
                m_jauges[0].SetHide(true);
            }
            tinyPoint.X = m_blupiPos.X;
            tinyPoint.Y = m_blupiPos.Y - 60;
            if ((m_blupiSurf || m_blupiNage) && (m_blupiPos.Y % 64 == 64 - BLUPISURF || m_blupiPos.Y % 64 == 32) &&
                IsOutWater(tinyPoint) && ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
            {
                m_blupiNage = false;
                m_blupiSurf = false;
                m_blupiAir = true;
                m_blupiAction = BlupiAction::Air;
                m_blupiPhase = 0;
                m_blupiVitesseX = 0.0;
                if (m_blupiPower)
                {
                    m_blupiVitesseY = -16.0;
                }
                else
                {
                    m_blupiVitesseY = -12.0;
                }
                MoveObjectTiplouf(m_blupiPos);
                PlaySound(SoundChannel::SoundChannel22, m_blupiPos);
                m_jauges[0].SetHide(true);
            }
            if ((m_blupiSurf || m_blupiNage) && IsOutWater(m_blupiPos))
            {
                if (m_blupiVitesseY < 0.0)
                {
                    if (m_blupiTransport == -1)
                    {
                        m_blupiPos = blupiPos;
                    }
                    else
                    {
                        m_blupiTransport = -1;
                        m_blupiNage = false;
                        m_blupiSurf = false;
                        m_blupiAir = true;
                        m_blupiAction = BlupiAction::Air;
                        m_blupiPhase = 0;
                        m_blupiPos.Y -= 10;
                        m_blupiVitesseX = 0.0;
                        m_blupiVitesseY = -10.0;
                        PlaySound(SoundChannel::SoundChannel22, m_blupiPos);
                        m_jauges[0].SetHide(true);
                    }
                }
                else if (m_blupiVitesseY > 0.0)
                {
                    m_blupiNage = false;
                    m_blupiSurf = false;
                    m_blupiAir = false;
                    m_blupiAction = BlupiAction::Air;
                    m_blupiPhase = 0;
                    m_blupiPos.Y += 30;
                    m_blupiVitesseX = 0.0;
                    m_blupiVitesseY = 0.0;
                    PlaySound(SoundChannel::SoundChannel22, m_blupiPos);
                    m_jauges[0].SetHide(true);
                }
                else
                {
                    m_blupiNage = false;
                    m_blupiSurf = false;
                    m_blupiAir = false;
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                    m_blupiPos.Y -= 10;
                    m_blupiVitesseX = 0.0;
                    m_blupiVitesseY = 0.0;
                    PlaySound(SoundChannel::SoundChannel22, m_blupiPos);
                    m_jauges[0].SetHide(true);
                }
            }
            if ((m_blupiSurf || m_blupiNage) && m_blupiActionOuf != BlupiAction::Ouf1a)
            {
                m_blupiActionOuf = BlupiAction::Ouf1a;
                m_blupiTimeOuf = 0;
            }
        }
        if ((m_blupiHelico || m_blupiOver || m_blupiJeep || m_blupiTank || m_blupiSkate) && m_blupiFocus && (
            IsSurfWater(m_blupiPos) || IsDeepWater(m_blupiPos)))
        {
            ByeByeHelico();
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiHelico = false;
            m_blupiOver = false;
            m_blupiJeep = false;
            m_blupiTank = false;
            m_blupiSkate = false;
            celSwitch.X = m_blupiPos.X - 34;
            celSwitch.Y = m_blupiPos.Y - 34;
            ObjectStart(celSwitch, ObjectType::ObjectType9, 0);
            m_decorAction = DecorAction::SmallShake;
            m_decorPhase = 0;
            StopSound(SoundChannel::SoundChannel16);
            StopSound(SoundChannel::SoundChannel18);
            StopSound(SoundChannel::SoundChannel29);
            StopSound(SoundChannel::SoundChannel31);
            PlaySound(SoundChannel::SoundChannel10, m_blupiPos);
        }
        if (m_blupiFocus && !m_blupiSuspend && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !
            m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && m_blupiNoBarre == 0 &&
            GetTypeBarre(m_blupiPos) == 1)
        {
            tinyPoint.X = m_blupiPos.X;
            tinyPoint.Y = (m_blupiPos.Y + 22) / 64 * 64 + BLUPISUSPEND;
            rect = BlupiRect(tinyPoint);
            if (!DecorDetect(rect, true))
            {
                m_blupiPos = tinyPoint;
                m_blupiSuspend = true;
                m_blupiAir = false;
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiActionOuf = BlupiAction::None;
                PlaySound(SoundChannel::SoundChannel34, m_blupiPos);
            }
        }
        if (m_blupiNoBarre > 0)
        {
            m_blupiNoBarre--;
        }
        if (IsVentillo(m_blupiPos))
        {
            if (m_blupiFocus && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                BlupiDead(BlupiAction::Clear1, BlupiAction::Clear2);
            }
            celSwitch.X = m_blupiPos.X - 34;
            celSwitch.Y = m_blupiPos.Y - 34;
            ObjectStart(celSwitch, ObjectType::ObjectType11, 0);
            m_decorAction = DecorAction::BigShake;
            m_decorPhase = 0;
            StopSound(SoundChannel::SoundChannel16);
            StopSound(SoundChannel::SoundChannel18);
            StopSound(SoundChannel::SoundChannel29);
            StopSound(SoundChannel::SoundChannel31);
            PlaySound(SoundChannel::SoundChannel10, m_blupiPos);
        }
        if (m_blupiAction != BlupiAction::Bye && m_blupiFocus)
        {
            icon = IsWorld(m_blupiPos);
            if (icon != -1)
            {
                StopSound(SoundChannel::SoundChannel16);
                StopSound(SoundChannel::SoundChannel18);
                StopSound(SoundChannel::SoundChannel29);
                StopSound(SoundChannel::SoundChannel31);
                PlaySound(SoundChannel::SoundChannel32, m_blupiPos);
                m_blupiAction = BlupiAction::Bye;
                m_blupiPhase = 0;
                m_blupiFocus = false;
                m_blupiFront = true;
            }
        }
        int num26 = MoveObjectDetect(m_blupiPos, bNear);
        TinyPoint tinyPoint2;
        if (m_blupiAction != BlupiAction::Clear1 && m_blupiAction != BlupiAction::Clear2 && m_blupiAction != BlupiAction::Clear3 && m_blupiAction != BlupiAction::Clear4 &&
            m_blupiAction != BlupiAction::Clear5 && m_blupiAction != BlupiAction::Clear6 && m_blupiAction != BlupiAction::Clear7 && m_blupiAction != BlupiAction::Clear8)
        {
            if (IsLave(m_blupiPos) && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                BlupiDead(BlupiAction::Clear3, {});
                m_blupiRestart = true;
                m_blupiPos.Y = m_blupiPos.Y / 64 * 64 + BLUPIOFFY;
                PlaySound(SoundChannel::SoundChannel8, m_blupiPos);
            }
            if (IsPiege(m_blupiPos) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_blupiHide &&
                !m_bSuperBlupi && m_blupiFocus)
            {
                BlupiDead(BlupiAction::Glu, {});
                m_blupiRestart = true;
                m_blupiAir = true;
                ObjectStart(m_blupiPos, ObjectType::ObjectType53, 0);
                PlaySound(SoundChannel::SoundChannel51, m_blupiPos);
            }
            if (IsGoutte(m_blupiPos, false) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !
                m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
            {
                BlupiDead(BlupiAction::Glu, {});
                m_blupiRestart = true;
                m_blupiAir = true;
                PlaySound(SoundChannel::SoundChannel51, m_blupiPos);
            }
            if (IsScie(m_blupiPos) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_blupiHide &&
                !m_bSuperBlupi && m_blupiFocus)
            {
                BlupiDead(BlupiAction::Clear4, {});
                m_blupiFront = true;
                m_blupiRestart = true;
                m_blupiAir = true;
            }
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && (num26 == -1 || !bNear) &&
                IsSwitch(m_blupiPos, celSwitch) && !m_blupiOver && !m_blupiBalloon && !m_blupiJeep && !m_blupiTank && !
                m_blupiSkate && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                ActiveSwitch(m_decor[celSwitch.X][celSwitch.Y].icon == 385, celSwitch);
                m_blupiAction = BlupiAction::Switch;
                m_blupiPhase = 0;
                m_blupiFocus = false;
                m_blupiVitesseX = 0.0;
                m_blupiVitesseY = 0.0;
            }
            if (IsBlitz(m_blupiPos, false) && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                BlupiDead(BlupiAction::Clear1, {});
                m_blupiRestart = true;
                m_blupiAir = true;
                m_blupiPos.Y = m_blupiPos.Y / 64 * 64 + BLUPIOFFY;
                PlaySound(SoundChannel::SoundChannel8, m_blupiPos);
            }
            if (IsEcraseur(m_blupiPos) && !m_blupiEcrase && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi &&
                m_blupiFocus)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiVitesseX = 0.0;
                m_blupiVitesseY = 0.0;
                m_blupiEcrase = true;
                m_blupiBalloon = false;
                m_blupiAir = false;
                m_blupiHelico = false;
                m_blupiOver = false;
                m_blupiJeep = false;
                m_blupiTank = false;
                m_blupiSkate = false;
                m_blupiNage = false;
                m_blupiSurf = false;
                m_blupiSuspend = false;
                m_blupiJumpAie = false;
                m_blupiShield = false;
                m_blupiPower = false;
                m_blupiCloud = false;
                m_blupiHide = false;
                m_blupiTimeShield = 100;
                m_blupiPosMagic = m_blupiPos;
                m_jauges[1].SetHide(false);
                if (!m_blupiJeep && !m_blupiTank)
                {
                    StopSound(SoundChannel::SoundChannel16);
                    StopSound(SoundChannel::SoundChannel18);
                    StopSound(SoundChannel::SoundChannel29);
                    StopSound(SoundChannel::SoundChannel31);
                }
                PlaySound(SoundChannel::SoundChannel70, m_blupiPos);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, -60);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, 60);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, 10);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, -10);
                celSwitch.X = m_blupiPos.X - 34;
                celSwitch.Y = m_blupiPos.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType90, 0);
                m_decorAction = DecorAction::BigShake;
                m_decorPhase = 0;
            }
            if (IsTeleporte(m_blupiPos) != -1 && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase &&
                !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiAir && m_blupiFocus && m_blupiPosHelico.X == -
                1)
            {
                m_blupiAction = BlupiAction::Teleporte;
                m_blupiPhase = 0;
                m_blupiVitesseX = 0.0;
                m_blupiVitesseY = 0.0;
                m_blupiFocus = false;
                m_blupiPos.X = m_blupiPos.X / 64 * 64;
                PlaySound(SoundChannel::SoundChannel71, m_blupiPos);
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = m_blupiPos.Y - 5;
                ObjectStart(celSwitch, ObjectType::ObjectType92, 0);
            }
            if (IsBridge(m_blupiPos, celBridge) && m_blupiFocus)
            {
                celBridge.X *= 64;
                celBridge.Y *= 64;
                ObjectStart(celBridge, ObjectType::ObjectType52, 0);
            }
            int num = IsDoor(m_blupiPos, celBridge);
            const DoorKeyFlags doorKeyMask = ToDoorKeyFlags(1 << (num - 334));
            if (num != -1 && (m_blupiCle & doorKeyMask) != DoorKeyFlags::None)
            {
                OpenDoor(celBridge);
                m_blupiCle = ToDoorKeyFlags(ToRaw(m_blupiCle) & ~ToRaw(doorKeyMask));
                celSwitch.X = 520;
                celSwitch.Y = 418;
                tinyPoint2.X = celBridge.X * 64 - m_posDecor.X;
                tinyPoint2.Y = celBridge.Y * 64 - m_posDecor.Y;
                VoyageInit(celSwitch, m_pixmap->HotSpotToHud(tinyPoint2), 214 + (num - 334) * 7, PixmapChannel::Element);
            }
        }
        if (!m_blupiHelico && !m_blupiSuspend && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiSkate && !
            m_blupiJeep && !m_blupiTank && !m_blupiJeep && m_blupiFocus)
        {
            icon = MockeryDetect(m_blupiPos);
            if (icon != 0)
            {
                m_blupiActionOuf = ToBlupiAction(icon);
                m_blupiTimeOuf = 0;
            }
        }
        MoveObjectFollow(m_blupiPos);
        icon = num26;
        if (icon != -1 && !bNear && m_moveObject[icon].type == ObjectType::ObjectType2 && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon &&
            !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !
            m_blupiSuspend && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
        {
            m_blupiActionOuf = BlupiAction::Ouf4;
            m_blupiTimeOuf = 0;
        }
        if (icon != -1 && bNear)
        {
            if (m_moveObject[icon].type == ObjectType::ObjectType13 && (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction ||
                    IsFloatingObject(icon)) && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !
                m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
                m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                m_scrollAdd.X = 0;
                m_scrollAdd.Y = 0;
                m_blupiAir = false;
                m_blupiHelico = true;
                m_blupiRealRotation = 0;
                m_blupiVitesseX = 0.0;
                if (m_blupiCloud || m_blupiHide)
                {
                    m_blupiCloud = false;
                    m_blupiHide = false;
                    m_jauges[1].SetHide(true);
                }
            }
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && m_moveObject[icon].type == ObjectType::ObjectType46 && !
                m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
                m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                m_scrollAdd.X = 0;
                m_scrollAdd.Y = 0;
                m_blupiAir = false;
                m_blupiOver = true;
                m_blupiVitesseX = 0.0;
                if (m_blupiCloud || m_blupiHide)
                {
                    m_blupiCloud = false;
                    m_blupiHide = false;
                    m_jauges[1].SetHide(true);
                }
            }
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && m_moveObject[icon].type == ObjectType::ObjectType19 && !
                m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
                m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                m_scrollAdd.X = 0;
                m_scrollAdd.Y = 0;
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiAir = false;
                m_blupiJeep = true;
                m_blupiVitesseX = 0.0;
                if (m_blupiCloud || m_blupiHide)
                {
                    m_blupiCloud = false;
                    m_blupiHide = false;
                    m_jauges[1].SetHide(true);
                }
            }
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && m_moveObject[icon].type == ObjectType::ObjectType28 && !
                m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
                m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                m_scrollAdd.X = 0;
                m_scrollAdd.Y = 0;
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiAir = false;
                m_blupiTank = true;
                m_blupiVitesseX = 0.0;
                if (Config::FPS != Fps::Fps20)
                {
                    m_blupiVitesseY = 0.0;
                    m_blupiSubPixelY = 0.0;
                }
                if (m_blupiCloud || m_blupiHide)
                {
                    m_blupiCloud = false;
                    m_blupiHide = false;
                    m_jauges[1].SetHide(true);
                }
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType29 && m_blupiFocus && m_blupiBullet < 10)
            {
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                tinyPoint2.X = 570;
                tinyPoint2.Y = 430;
                VoyageInit(m_pixmap->HotSpotToHud(celSwitch), tinyPoint2, 177, PixmapChannel::Element);
                m_blupiBullet += 10;
                if (m_blupiBullet > 10)
                {
                    m_blupiBullet = 10;
                }
            }
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && m_moveObject[icon].type == ObjectType::ObjectType24 && !
                m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
                m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                m_scrollAdd.X = 0;
                m_scrollAdd.Y = 0;
                m_blupiAction = BlupiAction::TakeSkate;
                m_blupiPhase = 0;
                m_blupiPos.Y = m_moveObject[icon].posCurrent.Y / 64 * 64 + BLUPIOFFY;
                m_blupiFocus = false;
                m_blupiAir = false;
                m_blupiSkate = true;
                m_blupiVitesseX = 0.0;
                if (m_blupiCloud || m_blupiHide)
                {
                    m_blupiCloud = false;
                    m_blupiHide = false;
                    m_jauges[1].SetHide(true);
                }
            }
            if ((m_moveObject[icon].type == ObjectType::ObjectType3 || m_moveObject[icon].type == ObjectType::ObjectType16 || m_moveObject[icon].type == ObjectType::ObjectType96 ||
                m_moveObject[icon].type == ObjectType::ObjectType97) && m_blupiBalloon && m_blupiPosHelico.X == -1)
            {
                m_blupiBalloon = false;
                m_blupiAir = true;
                m_blupiTimeShield = 0;
                m_jauges[1].SetHide(true);
                m_decorAction = DecorAction::None;
                celSwitch.X = m_blupiPos.X - 34;
                celSwitch.Y = m_blupiPos.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType91, 0);
                PlaySound(SoundChannel::SoundChannel41, m_blupiPos);
                m_blupiPos.Y += 4;
                m_blupiVitesseY = 0.0;
                m_blupiPosHelico = m_blupiPos;
            }
            else if ((m_moveObject[icon].type == ObjectType::ObjectType2 || m_moveObject[icon].type == ObjectType::ObjectType3 || m_moveObject[icon].type == ObjectType::ObjectType96 ||
                    m_moveObject[icon].type == ObjectType::ObjectType97 || m_moveObject[icon].type == ObjectType::ObjectType16 || m_moveObject[icon].type == ObjectType::ObjectType4 ||
                    m_moveObject[icon].type == ObjectType::ObjectType17 || m_moveObject[icon].type == ObjectType::ObjectType20) && !m_blupiShield && !m_blupiHide &&
                !
                m_bSuperBlupi && m_blupiPosHelico.X == -1)
            {
                if (!m_blupiJeep && !m_blupiTank && !m_blupiSkate && (m_blupiFocus || m_blupiAction == BlupiAction::Air ||
                    m_blupiAction == BlupiAction::JumpAie))
                {
                    if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase)
                    {
                        m_blupiAir = true;
                    }
                    BlupiDead(BlupiAction::Clear1, BlupiAction::Clear2);
                }
                if (m_moveObject[icon].type == ObjectType::ObjectType17 || m_moveObject[icon].type == ObjectType::ObjectType20)
                {
                    celSwitch = m_moveObject[icon].posCurrent;
                    ObjectDelete(celSwitch, m_moveObject[icon].type);
                    celSwitch.X -= 34;
                    celSwitch.Y -= 34;
                    ObjectStart(celSwitch, ObjectType::ObjectType10, 0);
                    m_decorAction = DecorAction::BigShake;
                    m_decorPhase = 0;
                }
                else
                {
                    celSwitch = m_moveObject[icon].posCurrent;
                    ObjectDelete(celSwitch, m_moveObject[icon].type);
                    celSwitch.X -= 34;
                    celSwitch.Y -= 34;
                    ObjectStart(celSwitch, ObjectType::ObjectType8, 0);
                    m_decorAction = DecorAction::SmallShake;
                    m_decorPhase = 0;
                }
                if (!m_blupiJeep && !m_blupiTank)
                {
                    StopSound(SoundChannel::SoundChannel16);
                    StopSound(SoundChannel::SoundChannel18);
                    StopSound(SoundChannel::SoundChannel29);
                    StopSound(SoundChannel::SoundChannel31);
                }
                PlaySound(SoundChannel::SoundChannel10, m_moveObject[icon].posCurrent);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType44 && m_blupiFocus && !m_blupiBalloon && !m_blupiShield && !m_blupiHide && !
                m_bSuperBlupi)
            {
                ByeByeHelico();
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiVitesseX = 0.0;
                m_blupiVitesseY = 0.0;
                m_blupiBalloon = true;
                m_blupiEcrase = false;
                m_blupiAir = false;
                m_blupiHelico = false;
                m_blupiOver = false;
                m_blupiJeep = false;
                m_blupiTank = false;
                m_blupiSkate = false;
                m_blupiNage = false;
                m_blupiSurf = false;
                m_blupiSuspend = false;
                m_blupiJumpAie = false;
                m_blupiShield = false;
                m_blupiPower = false;
                m_blupiCloud = false;
                m_blupiHide = false;
                m_blupiTimeShield = 100;
                m_blupiPosMagic = m_blupiPos;
                m_jauges[1].SetHide(false);
                if (!m_blupiJeep && !m_blupiTank)
                {
                    StopSound(SoundChannel::SoundChannel16);
                    StopSound(SoundChannel::SoundChannel18);
                    StopSound(SoundChannel::SoundChannel29);
                    StopSound(SoundChannel::SoundChannel31);
                }
                PlaySound(SoundChannel::SoundChannel40, m_moveObject[icon].posCurrent);
                celSwitch.X = m_blupiPos.X - 34;
                celSwitch.Y = m_blupiPos.Y - 34;
                ObjectStart(celSwitch, ObjectType::ObjectType90, 0);
                m_decorAction = DecorAction::ElectricShake;
                m_decorPhase = 0;
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType54 && m_moveObject[icon].step != 2 && m_moveObject[icon].step != 4 &&
                m_blupiFocus && !m_blupiBalloon && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
            {
                ByeByeHelico();
                celSwitch.X = m_blupiPos.X;
                celSwitch.Y = (m_blupiPos.Y + 64 - 10) / 64 * 64 + 4;
                ObjectStart(celSwitch, ObjectType::ObjectType53, 0);
                m_blupiAction = BlupiAction::Glu;
                m_blupiPhase = 0;
                m_blupiSuspend = false;
                m_blupiJumpAie = false;
                m_blupiFocus = false;
                m_blupiRestart = true;
                if (flag2)
                {
                    m_blupiAir = true;
                }
                if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase || m_blupiJeep || m_blupiTank ||
                    m_blupiSkate)
                {
                    m_blupiHelico = false;
                    m_blupiOver = false;
                    m_blupiBalloon = false;
                    m_blupiEcrase = false;
                    m_blupiJeep = false;
                    m_blupiTank = false;
                    m_blupiSkate = false;
                    celSwitch = m_moveObject[icon].posCurrent;
                    celSwitch.X -= 34;
                    celSwitch.Y -= 34;
                    ObjectStart(celSwitch, ObjectType::ObjectType10, 0);
                    StopSound(SoundChannel::SoundChannel16);
                    StopSound(SoundChannel::SoundChannel18);
                    StopSound(SoundChannel::SoundChannel29);
                    StopSound(SoundChannel::SoundChannel31);
                    PlaySound(SoundChannel::SoundChannel10, m_moveObject[icon].posCurrent);
                    m_decorAction = DecorAction::SmallShake;
                    m_decorPhase = 0;
                }
                else
                {
                    PlaySound(SoundChannel::SoundChannel51, m_moveObject[icon].posCurrent);
                }
                m_blupiCloud = false;
                m_blupiHide = false;
                m_jauges[1].SetHide(true);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType23 && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiAction != BlupiAction::Win
                && m_blupiAction != BlupiAction::Bye && m_blupiAction != BlupiAction::Clear1 && m_blupiAction != BlupiAction::Clear2 && m_blupiAction != BlupiAction::Clear3 &&
                m_blupiAction != BlupiAction::Clear4 && m_blupiAction != BlupiAction::Clear5 && m_blupiAction != BlupiAction::Clear6 && m_blupiAction != BlupiAction::Clear7 &&
                m_blupiAction != BlupiAction::Clear8 && m_blupiAction != BlupiAction::Glu && m_blupiAction != BlupiAction::Electro && m_blupiAction != BlupiAction::Hide)
            {
                ByeByeHelico();
                celSwitch = m_moveObject[icon].posCurrent;
                ObjectDelete(celSwitch, m_moveObject[icon].type);
                m_blupiAction = BlupiAction::Glu;
                m_blupiPhase = 0;
                m_blupiSuspend = false;
                m_blupiJumpAie = false;
                m_blupiFocus = false;
                m_blupiRestart = true;
                if (flag2)
                {
                    m_blupiAir = true;
                }
                if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase || m_blupiJeep || m_blupiTank ||
                    m_blupiSkate)
                {
                    m_blupiHelico = false;
                    m_blupiOver = false;
                    m_blupiBalloon = false;
                    m_blupiEcrase = false;
                    m_blupiJeep = false;
                    m_blupiTank = false;
                    m_blupiSkate = false;
                }
                StartSploutchGlu(m_moveObject[icon].posCurrent);
                m_blupiCloud = false;
                m_blupiHide = false;
                m_jauges[1].SetHide(true);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType5)
            {
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                TinyPoint end2;
                end2.X = 430;
                end2.Y = 430;
                VoyageInit(m_pixmap->HotSpotToHud(celSwitch), end2, 6, PixmapChannel::Element);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 10);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -10);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType49 && (m_voyageIcon != 215 || m_voyageChannel != PixmapChannel::Element) && (m_blupiCle & DoorKeyFlags::Key1) ==
                DoorKeyFlags::None)
            {
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                TinyPoint end3;
                end3.X = 520;
                end3.Y = 418;
                VoyageInit(m_pixmap->HotSpotToHud(celSwitch), end3, 215, PixmapChannel::Element);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 10);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -10);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType50 && (m_voyageIcon != 222 || m_voyageChannel != PixmapChannel::Element) && (m_blupiCle & DoorKeyFlags::Key2) ==
                DoorKeyFlags::None)
            {
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                TinyPoint end4;
                end4.X = 530;
                end4.Y = 418;
                VoyageInit(m_pixmap->HotSpotToHud(celSwitch), end4, 222, PixmapChannel::Element);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 10);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -10);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType51 && (m_voyageIcon != 229 || m_voyageChannel != PixmapChannel::Element) && (m_blupiCle & DoorKeyFlags::Key3) ==
                DoorKeyFlags::None)
            {
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                TinyPoint end5;
                end5.X = 540;
                end5.Y = 418;
                VoyageInit(m_pixmap->HotSpotToHud(celSwitch), end5, 229, PixmapChannel::Element);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 60);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, 10);
                ObjectStart(m_moveObject[icon].posCurrent, ObjectType::ObjectType39, -10);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType6 && m_nbVies < MAX_EGG_COUNT && m_blupiFocus)
            {
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                VoyageInit(m_pixmap->HotSpotToHud(celSwitch), VoyageGetPosVie(m_nbVies + 1), 21, PixmapChannel::Element);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType25 && !m_blupiShield && !m_blupiHide && !m_blupiPower && m_blupiFocus)
            {
                PlaySound(SoundChannel::SoundChannel42, m_moveObject[icon].posCurrent);
                m_blupiShield = true;
                m_blupiPower = false;
                m_blupiCloud = false;
                m_blupiHide = false;
                m_blupiTimeShield = 100;
                m_blupiPosMagic = m_blupiPos;
                m_jauges[1].SetHide(false);
            }
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && m_moveObject[icon].type == ObjectType::ObjectType26 && !
                m_blupiShield && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep &&
                !m_blupiTank && !m_blupiSkate && m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                m_sucettePos = m_moveObject[icon].posCurrent;
                m_sucetteType = m_moveObject[icon].type;
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                m_blupiAction = BlupiAction::Sucette;
                m_blupiPhase = 0;
                m_blupiCloud = false;
                m_blupiHide = false;
                m_blupiFocus = false;
                PlaySound(SoundChannel::SoundChannel50, end);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType40 && !m_blupiHide && m_blupiFocus)
            {
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                m_blupiInvert = true;
                m_blupiTimeShield = 100;
                m_blupiPosMagic = m_blupiPos;
                m_jauges[1].SetHide(false);
                PlaySound(SoundChannel::SoundChannel66, end);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, -60);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, 60);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, 10);
                ObjectStart(m_blupiPos, ObjectType::ObjectType41, -10);
            }
            if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && m_moveObject[icon].type == ObjectType::ObjectType30 && !
                m_blupiShield && !m_blupiCloud && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase &&
                !m_blupiJeep && !m_blupiTank && !m_blupiSkate && m_blupiFocus)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                m_sucettePos = m_moveObject[icon].posCurrent;
                m_sucetteType = m_moveObject[icon].type;
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                m_blupiAction = BlupiAction::Drink;
                m_blupiPhase = 0;
                m_blupiShield = false;
                m_blupiPower = false;
                m_blupiJumpAie = false;
                m_blupiFocus = false;
                PlaySound(SoundChannel::SoundChannel57, end);
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType31 && !m_blupiShield && !m_blupiHide && !m_blupiPower && !m_blupiCloud && !
                m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
                m_blupiSkate && m_blupiFocus)
            {
                m_blupiAction = BlupiAction::Charge;
                m_blupiPhase = 0;
                m_blupiShield = false;
                m_blupiPower = false;
                m_blupiJumpAie = false;
                m_blupiFocus = false;
                m_blupiCloud = true;
                m_blupiTimeShield = 100;
                PlaySound(SoundChannel::SoundChannel58, end);
                if (m_blupiHide)
                {
                    m_blupiHide = false;
                    m_jauges[1].SetHide(true);
                }
            }
            if (m_moveObject[icon].type >= ObjectType::ObjectType200 && m_moveObject[icon].type <= ObjectType::ObjectType203 && m_blupiFocus)
            {
                if (m_moveObject[icon].type == ObjectType::ObjectType200)
                {
                    if (m_blupiPerso < 5 && getButtonPressedProperty() == Def::ButtonGlyph::PlayAction)
                    {
                        setButtonPressedProperty(Def::ButtonGlyph::None);
                        ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                        celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                        celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                        tinyPoint2.X = 0;
                        tinyPoint2.Y = 438;
                        VoyageInit(m_pixmap->HotSpotToHud(celSwitch), tinyPoint2, 108, PixmapChannel::Button);
                    }
                }
                else if (!m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
                {
                    ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                    BlupiDead(BlupiAction::Clear1, BlupiAction::Clear2);
                    celSwitch = m_moveObject[icon].posCurrent;
                    celSwitch.X -= 34;
                    celSwitch.Y -= 34;
                    ObjectStart(celSwitch, ObjectType::ObjectType10, 0);
                    PlaySound(SoundChannel::SoundChannel10, m_moveObject[icon].posCurrent);
                    m_decorAction = DecorAction::SmallShake;
                    m_decorPhase = 0;
                }
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType55 && m_blupiFocus && m_blupiDynamite == 0 && (m_voyageIcon != 252 ||
                m_voyageChannel != PixmapChannel::Element) && getButtonPressedProperty() == Def::ButtonGlyph::PlayAction)
            {
                setButtonPressedProperty(Def::ButtonGlyph::None);
                ObjectDelete(m_moveObject[icon].posCurrent, m_moveObject[icon].type);
                celSwitch.X = m_moveObject[icon].posCurrent.X - m_posDecor.X;
                celSwitch.Y = m_moveObject[icon].posCurrent.Y - m_posDecor.Y;
                tinyPoint2.X = 505;
                tinyPoint2.Y = 414;
                VoyageInit(m_pixmap->HotSpotToHud(celSwitch), tinyPoint2, 252, PixmapChannel::Element);
                m_blupiAction = BlupiAction::TakeDynamite;
                m_blupiPhase = 0;
                m_blupiFocus = false;
            }
            if (m_moveObject[icon].type == ObjectType::ObjectType12 && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase &&
                !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
                m_blupiFocus && m_blupiAction == BlupiAction::March)
            {
                end = m_moveObject[icon].posCurrent;
                if (m_blupiDir == Direction::Left && m_blupiPos.X > end.X)
                {
                    end.X = m_blupiPos.X - 59;
                    PlaySound(SoundChannel::SoundChannel38, end);
                    m_blupiActionOuf = BlupiAction::Ouf1b;
                    m_blupiTimeOuf = 0;
                    m_blupiAction = BlupiAction::Push;
                    m_blupiPhase = 0;
                }
                if (m_blupiDir == Direction::Right && m_blupiPos.X < end.X)
                {
                    end.X = m_blupiPos.X + 55;
                    PlaySound(SoundChannel::SoundChannel38, end);
                    m_blupiActionOuf = BlupiAction::Ouf1b;
                    m_blupiTimeOuf = 0;
                    m_blupiAction = BlupiAction::Push;
                    m_blupiPhase = 0;
                }
                if (!TestPushCaisse(icon, end, false))
                {
                    m_blupiPos.X = blupiPos.X;
                }
            }
            if ((m_moveObject[icon].type == ObjectType::ObjectType7 || m_moveObject[icon].type == ObjectType::ObjectType21) && m_blupiFocus)
            {
                if (m_goalPhase == 0)
                {
                    if (m_nbTresor >= m_totalTresor)
                    {
                        if (m_moveObject[icon].type == ObjectType::ObjectType21)
                        {
                            m_bFoundCle = true;
                        }
                        ByeByeHelico();
                        StopSound(SoundChannel::SoundChannel16);
                        StopSound(SoundChannel::SoundChannel18);
                        StopSound(SoundChannel::SoundChannel29);
                        StopSound(SoundChannel::SoundChannel31);
                        PlaySound(SoundChannel::SoundChannel14, m_moveObject[icon].posCurrent);
                        m_blupiAction = BlupiAction::Win;
                        m_blupiPhase = 0;
                        m_blupiFocus = false;
                        m_blupiFront = true;
                        m_blupiPos.Y = m_moveObject[icon].posCurrent.Y;
                    }
                    else
                    {
                        PlaySound(SoundChannel::SoundChannel13, m_moveObject[icon].posCurrent);
                    }
                    m_goalPhase = 50;
                }
                else
                {
                    m_goalPhase--;
                }
            }
            else
            {
                m_goalPhase = 0;
            }
        }
        else
        {
            m_goalPhase = 0;
        }
        if (m_blupiAction == BlupiAction::Push && m_blupiFocus)
        {
            icon = CaisseInFront();
            if (icon != -1)
            {
                end = m_moveObject[icon].posCurrent;
                if (m_blupiDir == Direction::Left)
                {
                    end.X = m_blupiPos.X - 59;
                }
                else
                {
                    end.X = m_blupiPos.X + 55;
                }
                if (!TestPushCaisse(icon, end, false))
                {
                    m_blupiPos.X = blupiPos.X;
                }
            }
            else
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
        }
        if (m_blupiAction == BlupiAction::Pop && m_blupiFocus)
        {
            icon = CaisseInFront();
            if (icon != -1)
            {
                end = m_moveObject[icon].posCurrent;
                if (m_blupiDir == Direction::Left)
                {
                    end.X = m_blupiPos.X - 59;
                }
                else
                {
                    end.X = m_blupiPos.X + 55;
                }
                if (!TestPushCaisse(icon, end, true))
                {
                    m_blupiAction = BlupiAction::Stop;
                    m_blupiPhase = 0;
                }
            }
            else
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
            }
        }
        if (!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !
            m_blupiSkate && !m_blupiNage && !m_blupiSurf && m_blupiFocus)
        {
            if (m_blupiActionOuf == BlupiAction::Ouf1a && m_blupiAction == BlupiAction::Stop)
            {
                if (m_blupiTimeOuf > 50)
                {
                    m_blupiAction = m_blupiActionOuf;
                    m_blupiPhase = 0;
                    PlaySound(SoundChannel::SoundChannel46, m_blupiPos);
                }
                m_blupiActionOuf = BlupiAction::None;
            }
            if (m_blupiActionOuf == BlupiAction::Ouf1b && m_blupiAction == BlupiAction::Stop)
            {
                if (m_blupiTimeOuf > Config::ScaleTime(50))
                {
                    m_blupiAction = m_blupiActionOuf;
                    m_blupiPhase = 0;
                }
                m_blupiActionOuf = BlupiAction::None;
            }
            if (m_blupiAction == BlupiAction::Ouf1b && m_blupiPhase == Config::ScaleTime(4))
            {
                PlaySound(SoundChannel::SoundChannel46, m_blupiPos);
            }
            if (m_blupiActionOuf == BlupiAction::Ouf5 && m_blupiAction == BlupiAction::Stop)
            {
                if (m_blupiTimeOuf > Config::ScaleTime(10) && m_blupiTimeOuf < Config::ScaleTime(50))
                {
                    m_blupiAction = m_blupiActionOuf;
                    m_blupiPhase = 0;
                }
                m_blupiActionOuf = BlupiAction::None;
            }
            if (m_blupiActionOuf == BlupiAction::Ouf3 && m_blupiAction == BlupiAction::Stop)
            {
                if (m_blupiTimeOuf > Config::ScaleTime(60))
                {
                    m_blupiAction = m_blupiActionOuf;
                    m_blupiPhase = 0;
                    PlaySound(SoundChannel::SoundChannel48, m_blupiPos);
                }
                m_blupiActionOuf = BlupiAction::None;
            }
            if (m_blupiActionOuf == BlupiAction::Ouf4 && m_blupiAction == BlupiAction::Stop)
            {
                if (m_blupiTimeOuf < Config::ScaleTime(10))
                {
                    m_blupiAction = m_blupiActionOuf;
                    m_blupiPhase = 0;
                    PlaySound(SoundChannel::SoundChannel49, m_blupiPos);
                }
                m_blupiActionOuf = BlupiAction::None;
            }
            if ((m_blupiActionOuf == BlupiAction::Mockery || m_blupiActionOuf == BlupiAction::Mockeryi || m_blupiActionOuf == BlupiAction::Mockeryp) && m_blupiAction == BlupiAction::Stop)
            {
                if (m_blupiTimeOuf < Config::ScaleTime(20))
                {
                    m_blupiAction = m_blupiActionOuf;
                    m_blupiPhase = 0;
                }
                m_blupiActionOuf = BlupiAction::None;
            }
        }
        if (m_blupiAction == BlupiAction::March && m_blupiActionOuf != BlupiAction::Mockery && m_blupiActionOuf != BlupiAction::Mockeryi && m_blupiActionOuf != BlupiAction::Mockeryp && !
            m_blupiSurf && !m_blupiNage)
        {
            m_blupiActionOuf = BlupiAction::None;
        }
        if (m_blupiActionOuf != BlupiAction::None)
        {
            m_blupiTimeOuf++;
        }
        if (m_blupiTimeMockery > 0)
        {
            m_blupiTimeMockery--;
        }
        if (m_blupiAction == BlupiAction::TakeDynamite && m_blupiPhase == Config::ScaleTime(18))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        if (m_blupiAction == BlupiAction::PutDynamite && m_blupiPhase == Config::ScaleTime(26))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        if (m_blupiPos.X - 30 > m_blupiPosHelico.X || m_blupiPos.X + 30 < m_blupiPosHelico.X || m_blupiPos.Y - 30 >
            m_blupiPosHelico.Y || m_blupiPos.Y + 30 < m_blupiPosHelico.Y)
        {
            m_blupiPosHelico.X = -1;
        }
        if (m_blupiTimeFire > 0)
        {
            m_blupiTimeFire--;
        }
        if (m_blupiAction == BlupiAction::Teleporte && m_blupiPhase == Config::ScaleTime(128))
        {
            TinyPoint newpos;
            if (SearchTeleporte(m_blupiPos, newpos))
            {
                m_blupiPos = newpos;
                ObjectStart(m_blupiPos, ObjectType::ObjectType27, 20);
                ObjectStart(m_blupiPos, ObjectType::ObjectType27, -20);
            }
            m_blupiFocus = true;
            m_blupiPosHelico = m_blupiPos;
        }
        if (m_blupiAction == BlupiAction::Switch && m_blupiPhase == Config::ScaleTime(10))
        {
            m_blupiAction = BlupiAction::Stop;
            m_blupiPhase = 0;
            m_blupiFocus = true;
        }
        if (getButtonPressedProperty() == Def::ButtonGlyph::PlayAction && m_blupiAction == BlupiAction::Stop)
        {
            m_blupiAction = BlupiAction::Non;
            m_blupiPhase = 0;
            PlaySound(SoundChannel::SoundChannel27, m_blupiPos);
        }
        if ((m_blupiAction == BlupiAction::Clear1 && m_blupiPhase == Config::ScaleTime(70)) || (m_blupiAction == BlupiAction::Clear2 && m_blupiPhase == Config::ScaleTime(100)) || (
            m_blupiAction == BlupiAction::Clear3 && m_blupiPhase == Config::ScaleTime(70)) || (m_blupiAction == BlupiAction::Clear4 && m_blupiPhase == Config::ScaleTime(110)) || (m_blupiAction == BlupiAction::Clear5 && m_blupiPhase == Config::ScaleTime(90)) || (m_blupiAction == BlupiAction::Clear6 && m_blupiPhase == Config::ScaleTime(90)) || (m_blupiAction == BlupiAction::Clear7 &&
            m_blupiPhase == Config::ScaleTime(90)) || (m_blupiAction == BlupiAction::Clear8 && m_blupiPhase == Config::ScaleTime(90)) || (m_blupiAction == BlupiAction::Drown && m_blupiPhase
            == Config::ScaleTime(90)) || (m_blupiAction == BlupiAction::Glu && m_blupiPhase == Config::ScaleTime(100)) || (m_blupiAction == BlupiAction::Electro && m_blupiPhase == Config::ScaleTime(90)))
        {
            if (m_nbVies > 0)
            {
                m_blupiAction = BlupiAction::Hide;
                m_blupiIcon = -1;
                m_blupiPhase = 0;
                if (m_blupiRestart)
                {
                    m_blupiPos = m_blupiValidPos;
                }
                TinyPoint posDecor = GetPosDecor(m_blupiPos);
                celSwitch.X = m_blupiPos.X - posDecor.X - 30;
                celSwitch.Y = m_blupiPos.Y - posDecor.Y;
                VoyageInit(VoyageGetPosVie(m_nbVies), m_pixmap->HotSpotToHud(celSwitch), 48, PixmapChannel::Blupi);
            }
            else
            {
                m_nbVies = -1;
                m_term = -1;
                DoorsLost();
            }
            m_blupiFront = false;
        }
        num2 = ((m_dimDecor.Y != 0) ? 6400 : 480);
        if (m_blupiPos.Y >= num2 + 1 && m_blupiPos.Y <= num2 + 40)
        {
            PlaySound(SoundChannel::SoundChannel8, m_blupiPos);
        }
        if (m_blupiPos.Y > num2 + 1000)
        {
            m_term = -1;
            DoorsLost();
        }
        if (m_blupiAction == BlupiAction::Win && m_blupiPhase == Config::ScaleTime(40))
        {
            if (m_bPrivate)
            {
                m_term = 1;
            }
            else if (m_mission == 1)
            {
                m_term = 199;
            }
            else if (m_mission == 199)
            {
                m_term = -2;
            }
            else if (m_bFoundCle)
            {
                OpenGoldsWin();
                m_term = 1;
            }
            else
            {
                OpenDoorsWin();
                m_term = m_mission / 10 * 10;
            }
        }
        if (m_blupiAction == BlupiAction::Bye && m_blupiPhase == Config::ScaleTime(30))
        {
            icon = IsWorld(m_blupiPos);
            if (icon != -1)
            {
                if (m_mission == 1)
                {
                    m_term = icon * 10;
                }
                else if (icon == 199)
                {
                    m_term = 1;
                }
                else
                {
                    m_term = m_mission / 10 * 10 + icon;
                }
            }
        }
        if (blupiAction == BlupiAction::Mockery && m_blupiAction != BlupiAction::Mockery)
        {
            StopSound(SoundChannel::SoundChannel65);
        }
        if (blupiAction == BlupiAction::Mockeryi && m_blupiAction != BlupiAction::Mockeryi)
        {
            StopSound(SoundChannel::SoundChannel65);
        }
        if (blupiAction == BlupiAction::Mockeryp && m_blupiAction != BlupiAction::Mockeryp)
        {
            StopSound(SoundChannel::SoundChannel47);
        }
        if (m_blupiFocus && !m_blupiAir && (!m_blupiHelico || BlupiIsGround()) && (!m_blupiOver || BlupiIsGround()) && !
            m_blupiBalloon && !m_blupiEcrase && !m_blupiShield && !m_blupiHide && !bVertigoLeft && !bVertigoRight &&
            m_blupiTransport == -1 && !IsLave(m_blupiPos) && !IsPiege(m_blupiPos) && !IsGoutte(m_blupiPos, true) && !
            IsScie(m_blupiPos) && !IsBridge(m_blupiPos, celSwitch) && IsTeleporte(m_blupiPos) == -1 && !
            IsBlitz(m_blupiPos, true) && !IsTemp(m_blupiPos) && !IsBalleTraj(m_blupiPos) && !IsMoveTraj(m_blupiPos))
        {
            if (m_blupiFifoNb > 0)
            {
                m_blupiValidPos = m_blupiFifoPos[0];
            }
            BlupiAddFifo(m_blupiPos);
        }
        end.X = m_blupiPos.X + 30 + m_scrollAdd.X;
        end.Y = m_blupiPos.Y + 30 + m_scrollAdd.Y;
        int num27 = std::abs(m_scrollPoint.X - end.X);
        int num28 = std::abs(m_scrollPoint.Y - end.Y);
        num3 = SCROLL_SPEED;
        if (num27 > SCROLL_MARGX * 2)
        {
            num3 += (num27 - SCROLL_MARGX * 2) / 4;
        }
        if (num28 > SCROLL_MARGY * 2)
        {
            num3 += (num28 - SCROLL_MARGY * 2) / 4;
        }
        if (m_scrollPoint.X < end.X)
        {
            m_scrollPoint.X += num3;
            if (m_scrollPoint.X >= end.X)
            {
                m_scrollPoint.X = end.X;
            }
        }
        if (m_scrollPoint.X > end.X)
        {
            m_scrollPoint.X -= num3;
            if (m_scrollPoint.X <= end.X)
            {
                m_scrollPoint.X = end.X;
            }
        }
        if (m_scrollPoint.Y < end.Y)
        {
            m_scrollPoint.Y += num3;
            if (m_scrollPoint.Y >= end.Y)
            {
                m_scrollPoint.Y = end.Y;
            }
        }
        if (m_scrollPoint.Y > end.Y)
        {
            m_scrollPoint.Y -= num3;
            if (m_scrollPoint.Y <= end.Y)
            {
                m_scrollPoint.Y = end.Y;
            }
        }
        if (m_blupiAction != BlupiAction::Clear2 && m_blupiAction != BlupiAction::Clear3)
        {
            m_posDecor = GetPosDecor(m_scrollPoint);
        }
        if (m_time % 4 == 0)
        {
            PosSound(m_blupiPos);
        }
        VoyageStep();
        m_blupiLastSpeedX = m_blupiSpeedX;
        m_blupiLastSpeedY = m_blupiSpeedY;
        m_lastKeyPress = m_keyPress;
    }

    /**
     * @note Clears every vehicle/mode/bonus flag so death always returns Blupi to a plain
     *       state, hides the gauges, and stops the looping motor sounds. When action2 is
     *       supplied, the played death animation is chosen randomly between the two. Certain
     *       "clear" death variants additionally launch a Voyage arc (Clear2/Clear3, e.g. the
     *       angel/balloon ascent) or spawn debris objects (Clear4) and play a matching sound.
     * @note Spawns helicopter debris up front via ByeByeHelico() regardless of cause, which is
     *       a no-op when Blupi was not in helicopter mode.
     */
    void Decor::BlupiDead(BlupiAction action1, System::Nullable<BlupiAction> action2)
    {
        ByeByeHelico();
        if (!action2.getHasValueProperty())
        {
            m_blupiAction = action1;
        }
        else
        {
            m_blupiAction = ((m_random.get()->Next() % 2 == 0) ? action1 : action2.getValueProperty());
        }
        m_blupiPhase = 0;
        m_blupiFocus = false;
        m_blupiHelico = false;
        m_blupiOver = false;
        m_blupiJeep = false;
        m_blupiTank = false;
        m_blupiSkate = false;
        m_blupiNage = false;
        m_blupiSurf = false;
        m_blupiSuspend = false;
        m_blupiJumpAie = false;
        m_blupiShield = false;
        m_blupiPower = false;
        m_blupiCloud = false;
        m_blupiHide = false;
#ifdef MODERN
        m_blupiGhost = false;
#endif
        m_blupiInvert = false;
        m_blupiBalloon = false;
        m_blupiEcrase = false;
        m_blupiRestart = false;
        m_blupiActionOuf = BlupiAction::None;
        m_jauges[0].SetHide(true);
        m_jauges[1].SetHide(true);
        StopSound(SoundChannel::SoundChannel16);
        StopSound(SoundChannel::SoundChannel18);
        StopSound(SoundChannel::SoundChannel29);
        StopSound(SoundChannel::SoundChannel31);
        TinyPoint pos;
        TinyPoint pos2;
        if (m_blupiAction == BlupiAction::Clear2)
        {
            pos.X = m_blupiPos.X - m_posDecor.X;
            pos.Y = m_blupiPos.Y - m_posDecor.Y;
            pos2.X = m_blupiPos.X - m_posDecor.X;
            pos2.Y = m_blupiPos.Y - m_posDecor.Y - 300;
            VoyageInit(m_pixmap->HotSpotToHud(pos), m_pixmap->HotSpotToHud(pos2), 230, PixmapChannel::Element);
            PlaySound(SoundChannel::SoundChannel74, m_blupiPos);
        }
        if (m_blupiAction == BlupiAction::Clear3)
        {
            pos.X = m_blupiPos.X - m_posDecor.X;
            pos.Y = m_blupiPos.Y - m_posDecor.Y;
            pos2.X = m_blupiPos.X - m_posDecor.X;
            pos2.Y = m_blupiPos.Y - m_posDecor.Y - 2000;
            VoyageInit(m_pixmap->HotSpotToHud(pos), m_pixmap->HotSpotToHud(pos2), 40, PixmapChannel::Element);
            PlaySound(SoundChannel::SoundChannel74, m_blupiPos);
        }
        if (m_blupiAction == BlupiAction::Clear4)
        {
            ObjectStart(m_blupiPos, ObjectType::ObjectType41, -70);
            ObjectStart(m_blupiPos, ObjectType::ObjectType41, 20);
            ObjectStart(m_blupiPos, ObjectType::ObjectType41, -20);
            PlaySound(SoundChannel::SoundChannel75, m_blupiPos);
        }
    }

    /**
     * @note Centres the viewport on @p pos by subtracting half the draw width/height, then
     *       clamps to the level extents [0, 6400 - drawSize]. A zero m_dimDecor.X/Y axis
     *       means that axis is not scrollable, so the corresponding result is pinned to 0
     *       (single-screen levels).
     */
    TinyPoint Decor::GetPosDecor(TinyPoint pos)
    {
        TinyPoint result;
        if (m_dimDecor.X == 0)
        {
            result.X = 0;
        }
        else
        {
            result.X = pos.X - m_drawBounds.getWidthProperty() / 2;
            result.X = std::max(result.X, 0);
            result.X = std::min(result.X, 6400 - m_drawBounds.getWidthProperty());
        }
        if (m_dimDecor.Y == 0)
        {
            result.Y = 0;
        }
        else
        {
            result.Y = pos.Y - m_drawBounds.getHeightProperty() / 2;
            result.Y = std::max(result.Y, 0);
            result.Y = std::min(result.Y, 6400 - m_drawBounds.getHeightProperty());
        }
        return result;
    }

    /**
     * @note Appends to the rope-trail FIFO but de-duplicates: a position equal to the most
     *       recent entry is ignored so the trail only records actual movement. Once the
     *       10-slot buffer is full it shifts everything down by one and writes the newest
     *       sample at the end (oldest sample discarded).
     */
    void Decor::BlupiAddFifo(TinyPoint pos)
    {
        if (m_blupiFifoNb < 10)
        {
            if (m_blupiFifoNb <= 0 || pos.X != m_blupiFifoPos[m_blupiFifoNb - 1].X || pos.Y != m_blupiFifoPos[
                m_blupiFifoNb - 1].Y)
            {
                m_blupiFifoPos[m_blupiFifoNb] = pos;
                m_blupiFifoNb++;
            }
        }
        else if (pos.X != m_blupiFifoPos[9].X || pos.Y != m_blupiFifoPos[9].Y)
        {
            for (int i = 0; i < 9; i++)
            {
                m_blupiFifoPos[i] = m_blupiFifoPos[i + 1];
            }
            m_blupiFifoPos[9] = pos;
        }
    }

    /**
     * @note One-argument overload: forwards with bCaisse = true so crates count as solid.
     */
    bool Decor::DecorDetect(TinyRect rect)
    {
        return DecorDetect(rect, true);
    }

    /**
     * @note Sub-tile collision. Each 64x64 tile is divided into a 4x4 grid of 16px
     *       "quart" cells; Tables::table_decor_quart[icon*16 + ...] is a per-icon bitmap
     *       saying which of those 16 sub-cells are solid. The query rectangle is converted
     *       to a 16px sub-cell range and every overlapped sub-cell is tested; the first solid
     *       sub-cell that actually intersects @p rect (Misc::IntersectRect) returns true and
     *       records its tile icon in m_detectIcon (which callers read to identify the surface).
     * @note Out-of-bounds left/top and the right wall are treated as solid. The bottom wall is
     *       solid only for free-floating modes (helico/over/balloon/ecrase/nage/surf). A few
     *       icons are conditionally passable: icon 214 is non-solid in helico/over modes, and
     *       icon 324 (a timed platform) is non-solid during part of its blink cycle.
     */
    bool Decor::DecorDetect(TinyRect rect, bool bCaisse)
    {
        m_detectIcon = -1;
        if (rect.Left < 0 || rect.Top < 0)
        {
            // Treat positions outside the left/top of the world as solid boundaries.
            return true;
        }
        int num = ((m_dimDecor.X != 0) ? 6400 : 640);
        if (rect.Right > num)
        {
            return true;
        }
        if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase || m_blupiNage || m_blupiSurf)
        {
            num = ((m_dimDecor.Y != 0) ? 6400 : 480);
            if (rect.Bottom > num)
            {
                return true;
            }
        }
        int num2 = rect.Left / 16;
        int num3 = (rect.Right + 16 - 1) / 16;
        int num4 = rect.Top / 16;
        int num5 = (rect.Bottom + 16 - 1) / 16;
        TinyRect src = TinyRect();
        TinyRect dst;
        for (int i = num4; i <= num5; i++)
        {
            for (int j = num2; j <= num3; j++)
            {
                int num6 = j / 4;
                int num7 = i / 4;
                if (num6 < 0 || num6 >= 100 || num7 < 0 || num7 >= 100)
                {
                    continue;
                }
                int icon = m_decor[num6][num7].icon;
                if (icon < 0 || icon >= MAXQUART || (m_blupiHelico && icon == 214) || (m_blupiOver && icon == 214) || (
                    icon == 324 && m_time / 4 % 20 >= 18))
                {
                    continue;
                }
                num6 = j % 4;
                num7 = i % 4;
                if (Tables::table_decor_quart[icon * 16 + num7 * 4 + num6] != 0)
                {
                    src.Left = j * 16;
                    src.Right = src.Left + 16;
                    src.Top = i * 16;
                    src.Bottom = src.Top + 16;
                    if (Misc::IntersectRect(dst, src, rect))
                    {
                        m_detectIcon = icon;
                        return true;
                    }
                }
            }
        }
        if (!bCaisse)
        {
            return false;
        }
        for (int k = 0; k < m_nbRankCaisse; k++)
        {
            int num8 = m_rankCaisse[k];
            src.Left = m_moveObject[num8].posCurrent.X;
            src.Right = m_moveObject[num8].posCurrent.X + 64;
            src.Top = m_moveObject[num8].posCurrent.Y;
            src.Bottom = m_moveObject[num8].posCurrent.Y + 64;
            if (Misc::IntersectRect(dst, src, rect))
            {
                m_detectIcon = m_moveObject[num8].icon;
                return true;
            }
        }
        return false;
    }

    /**
     * @note Bresenham-style march. It steps one pixel at a time along the dominant axis
     *       (whichever of |dx|,|dy| is larger) and derives the minor-axis offset by linear
     *       interpolation, calling DecorDetect() on @p rect translated to each step. On the
     *       first collision it rewinds @p end to the last clear position and returns false;
     *       if the whole path is clear @p end is unchanged and it returns true. The four inner
     *       loops are the four sign combinations of the dominant direction.
     */
    bool Decor::TestPath(const TinyRect& rect, const TinyPoint& start, TinyPoint& end)
    {
        int num = std::abs(end.X - start.X);
        int num2 = std::abs(end.Y - start.Y);
        TinyPoint tinyPoint = start;
        TinyRect rect2 = TinyRect();
        if (num > num2)
        {
            if (end.X > start.X)
            {
                for (int i = 0; i <= num; i++)
                {
                    int num3 = i * (end.Y - start.Y) / num;
                    rect2.Left = rect.Left + i;
                    rect2.Right = rect.Right + i;
                    rect2.Top = rect.Top + num3;
                    rect2.Bottom = rect.Bottom + num3;
                    if (DecorDetect(rect2))
                    {
                        end = tinyPoint;
                        return false;
                    }
                    tinyPoint.X = start.X + i;
                    tinyPoint.Y = start.Y + num3;
                }
            }
            if (end.X < start.X)
            {
                for (int i = 0; i >= -num; i--)
                {
                    int num3 = i * (start.Y - end.Y) / num;
                    rect2.Left = rect.Left + i;
                    rect2.Right = rect.Right + i;
                    rect2.Top = rect.Top + num3;
                    rect2.Bottom = rect.Bottom + num3;
                    if (DecorDetect(rect2))
                    {
                        end = tinyPoint;
                        return false;
                    }
                    tinyPoint.X = start.X + i;
                    tinyPoint.Y = start.Y + num3;
                }
            }
        }
        else
        {
            if (end.Y > start.Y)
            {
                for (int num3 = 0; num3 <= num2; num3++)
                {
                    int i = num3 * (end.X - start.X) / num2;
                    rect2.Left = rect.Left + i;
                    rect2.Right = rect.Right + i;
                    rect2.Top = rect.Top + num3;
                    rect2.Bottom = rect.Bottom + num3;
                    if (DecorDetect(rect2))
                    {
                        end = tinyPoint;
                        return false;
                    }
                    tinyPoint.X = start.X + i;
                    tinyPoint.Y = start.Y + num3;
                }
            }
            if (end.Y < start.Y)
            {
                for (int num3 = 0; num3 >= -num2; num3--)
                {
                    int i = num3 * (start.X - end.X) / num2;
                    rect2.Left = rect.Left + i;
                    rect2.Right = rect.Right + i;
                    rect2.Top = rect.Top + num3;
                    rect2.Bottom = rect.Bottom + num3;
                    if (DecorDetect(rect2))
                    {
                        end = tinyPoint;
                        return false;
                    }
                    tinyPoint.X = start.X + i;
                    tinyPoint.Y = start.Y + num3;
                }
            }
        }
        return true;
    }

    /**
     * @note Despite the name, this emits the vehicle exhaust/smoke (pollution) puff effect
     *       behind whichever vehicle Blupi is driving (helico/over/jeep/tank). The dense
     *       m_time%N / m_blupiPhase%N tests are hand-tuned emission schedules so puffs appear
     *       at irregular, organic intervals that differ between idling and moving. tinyPoint
     *       is the per-vehicle nozzle offset and @c num the puff lifetime; when @c flag is set
     *       a smoke object is spawned at the computed offset behind Blupi.
     */
    void Decor::MoveObjectPollution()
    {
        bool flag = false;
        TinyPoint blupiPos = m_blupiPos;
        TinyPoint tinyPoint;
        tinyPoint.X = 0;
        tinyPoint.Y = 0;
        int num = 20;
        if (m_blupiAction == BlupiAction::Turn)
        {
            return;
        }
        if (m_blupiHelico)
        {
            if (m_blupiVitesseY < -5.0)
            {
                if (m_time % 20 != 0 && m_time % 20 != 2 && m_time % 20 != 5 && m_time % 20 != 8 && m_time % 20 != 10 &&
                    m_time % 20 != 11 && m_time % 20 != 16 && m_time % 20 != 18)
                {
                    return;
                }
            }
            else if (m_blupiVitesseX == 0.0)
            {
                if (m_time % 50 != 0 && m_time % 50 != 12 && m_time % 50 != 30)
                {
                    return;
                }
            }
            else if (m_time % 20 != 0 && m_time % 20 != 3 && m_time % 20 != 5 && m_time % 20 != 11 && m_time % 20 != 15)
            {
                return;
            }
            tinyPoint.X = 22;
            flag = true;
        }
        if (m_blupiOver)
        {
            if (m_blupiSpeedY < 0.0 || ((unsigned int)m_keyPress & ToRaw(KeyPressFlags::Jump)) != 0)
            {
                if (m_time % 20 != 0 && m_time % 20 != 2 && m_time % 20 != 5 && m_time % 20 != 8 && m_time % 20 != 11 &&
                    m_time % 20 != 13 && m_time % 20 != 14 && m_time % 20 != 18)
                {
                    return;
                }
                num = 58;
                tinyPoint.X = m_random.get()->Next(-10, 10);
                tinyPoint.Y = 22;
            }
            else
            {
                if (m_time % 50 != 0 && m_time % 50 != 12 && m_time % 50 != 30)
                {
                    return;
                }
                num = 20;
                tinyPoint.X = 30;
            }
            flag = true;
        }
        if (m_blupiJeep)
        {
            if (m_blupiVitesseX == 0.0)
            {
                if (m_blupiPhase % 50 != 0 && m_blupiPhase % 50 != 12 && m_blupiPhase % 50 != 20 && m_blupiPhase % 50 !=
                    35)
                {
                    return;
                }
            }
            else if (m_blupiPhase % 20 != 0 && m_blupiPhase % 20 != 3 && m_blupiPhase % 20 != 5 && m_blupiPhase % 20 !=
                11 && m_blupiPhase % 20 != 15)
            {
                return;
            }
            tinyPoint.X = 32;
            flag = true;
        }
        if (m_blupiTank)
        {
            if (m_blupiVitesseX == 0.0)
            {
                if (m_blupiPhase % 50 != 0 && m_blupiPhase % 50 != 15 && m_blupiPhase % 50 != 28)
                {
                    return;
                }
            }
            else if (m_blupiPhase % 20 != 0 && m_blupiPhase % 20 != 4 && m_blupiPhase % 20 != 12)
            {
                return;
            }
            tinyPoint.X = 35;
            flag = true;
        }
        if (!flag)
        {
            return;
        }
        if (m_blupiDir == Direction::Right)
        {
            blupiPos.X -= tinyPoint.X - 5;
            if (num < 50)
            {
                num = -num;
            }
        }
        else
        {
            blupiPos.X += tinyPoint.X;
        }
        blupiPos.Y += tinyPoint.Y;
        ObjectStart(blupiPos, ObjectType::ObjectType36, num);
    }

    void Decor::MoveObjectPlouf(TinyPoint pos)
    {
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType14)
            {
                return;
            }
        }
        pos.Y -= 45;
        PlaySound(SoundChannel::SoundChannel23, pos);
        ObjectStart(pos, ObjectType::ObjectType14, 0);
    }

    void Decor::MoveObjectTiplouf(TinyPoint pos)
    {
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType35)
            {
                return;
            }
        }
        if (m_blupiDir == Direction::Right)
        {
            pos.X += 5;
        }
        else
        {
            pos.X -= 5;
        }
        pos.Y -= 45;
        PlaySound(SoundChannel::SoundChannel64, pos);
        ObjectStart(pos, ObjectType::ObjectType35, 0);
    }

    /**
     * @note Spawns a bubble that rises through the water column. It first walks upward tile by
     *       tile counting consecutive water tiles (icons 91/92) to find how far the bubble can
     *       float (num), then creates an ObjectType15 whose posEnd is that many tiles above the
     *       start, with a rise speed scaled to the distance. No bubble is spawned if there is
     *       no clear water column above (num <= 0).
     */
    void Decor::MoveObjectBlup(TinyPoint pos)
    {
        PlaySound(SoundChannel::SoundChannel24, pos);
        pos.Y -= 20;
        int num = 0;
        TinyPoint tinyPoint = pos;
        while (tinyPoint.Y > 0)
        {
            int icon = m_decor[(tinyPoint.X + 16) / 64][tinyPoint.Y / 64].icon;
            if (icon != 91 && icon != 92)
            {
                break;
            }
            num++;
            tinyPoint.Y -= 64;
        }
        num--;
        if (num > 0)
        {
            int num2 = MoveObjectFree();
            if (num2 != -1)
            {
                m_moveObject[num2].type = ObjectType::ObjectType15;
                m_moveObject[num2].phase = 0;
                m_moveObject[num2].posCurrent.X = pos.X;
                m_moveObject[num2].posCurrent.Y = pos.Y;
                m_moveObject[num2].posStart = m_moveObject[num2].posCurrent;
                m_moveObject[num2].posEnd.X = pos.X;
                m_moveObject[num2].posEnd.Y = pos.Y - num * 64;
                m_moveObject[num2].timeStopStart = 0;
                m_moveObject[num2].stepAdvance = Config::ScaleTime(num * 10);
                m_moveObject[num2].step = 2;
                m_moveObject[num2].time = 0;
                MoveObjectStepIcon(num2);
            }
        }
    }

    /**
     * @note Samples the tile under the centre of @p pos (offset by +30,+30) and maps its icon
     *       to a 1-based world number via several contiguous icon ranges (the hub level uses
     *       different icon banks for the world-entry signs). Special icons map to the bonus
     *       world 9 and world 199. Returns -1 when the tile is not a world marker or @p pos is
     *       outside the map.
     */
    int Decor::IsWorld(TinyPoint pos)
    {
        pos.X += 30;
        pos.Y += 30;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return -1;
        }
        int icon = m_decor[pos.X / 64][pos.Y / 64].icon;
        if (icon >= 158 && icon <= 165)
        {
            return icon - 158 + 1;
        }
        if (icon >= 166 && icon <= 173)
        {
            return icon - 166 + 1;
        }
        switch (icon)
        {
        case 309:
        case 310:
            return 9;
        case 411:
        case 412:
        case 413:
        case 414:
        case 415:
            return icon - 411 + 10;
        default:
            if (icon >= 416 && icon <= 420)
            {
                return icon - 416 + 10;
            }
            if (icon >= 174 && icon <= 181)
            {
                return icon - 174 + 1;
            }
            if (icon == 184)
            {
                return 199;
            }
            return -1;
        }
    }

    /**
     * @note Flips the switch tile itself (open icon 384 vs closed 385) and plays the matching
     *       click sound, then scans a horizontal window of 41 cells centred on the switch
     *       (cel.X-20 .. cel.X+20) and toggles every linked saw/blade tile it finds between its
     *       active (379) and inactive (378) icon. This is how a single switch enables or
     *       disables a whole row of linked hazards.
     */
    void Decor::ActiveSwitch(bool bState, TinyPoint cel)
    {
        TinyPoint pos;
        pos.X = cel.X * 64;
        pos.Y = cel.Y * 64;
        ModifDecor(pos, bState ? 384 : 385);
        PlaySound(bState ? SoundChannel::SoundChannel77 : SoundChannel::SoundChannel76, pos);
        cel.X -= 20;
        for (int i = 0; i < 41; i++)
        {
            if (cel.X >= 0 && cel.X < 100 && m_decor[cel.X][cel.Y].icon == (bState ? 379 : 378))
            {
                pos.X = cel.X * 64;
                pos.Y = cel.Y * 64;
                ModifDecor(pos, bState ? 378 : 379);
            }
            cel.X++;
        }
    }

    /**
     * @note Returns 0 unless Blupi's grab point (offset +30,+22) sits in the upper band of a
     *       bar/rail tile (icon 138 or 202) — the band check (pos.Y % 64 > 44) rejects the
     *       lower part of the tile so he only catches the bar near the top. The return value
     *       distinguishes a "hangable" bar (1) from one that is effectively floor because the
     *       cell below is solid or Blupi already has ground contact (2).
     */
    int Decor::GetTypeBarre(TinyPoint pos)
    {
        TinyPoint pos2 = pos;
        pos.X += 30;
        pos.Y += 22;
        if (pos.Y % 64 > 44)
        {
            return 0;
        }
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return 0;
        }
        int icon = m_decor[pos.X / 64][pos.Y / 64].icon;
        if (icon != 138 && icon != 202)
        {
            return 0;
        }
        if (pos.Y >= 6336)
        {
            return 1;
        }
        icon = m_decor[pos.X / 64][pos.Y / 64 + 1].icon;
        if (!IsPassIcon(icon))
        {
            return 2;
        }
        TinyRect rect = BlupiRect(pos2);
        rect.Top = pos2.Y + 60 - 2;
        rect.Bottom = pos2.Y + 60 - 1;
        if (DecorDetect(rect, true))
        {
            return 2;
        }
        return 1;
    }

    bool Decor::IsLave(TinyPoint pos)
    {
        pos.X += 30;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        return m_decor[pos.X / 64][pos.Y / 64].icon == 68;
    }

    bool Decor::IsPiege(TinyPoint pos)
    {
        pos.X += 30;
        pos.Y += 60;
        if (pos.X % 64 < 15 || pos.X % 64 > 49)
        {
            return false;
        }
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        return m_decor[pos.X / 64][pos.Y / 64].icon == 373;
    }

    bool Decor::IsGoutte(TinyPoint pos, bool bAlways)
    {
        pos.X += 30;
        if (pos.X % 64 < 15 || pos.X % 64 > 49)
        {
            return false;
        }
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        int icon = m_decor[pos.X / 64][pos.Y / 64].icon;
        if (bAlways)
        {
            if (icon != 404)
            {
                return icon == 410;
            }
            return true;
        }
        return icon == 404;
    }

    bool Decor::IsScie(TinyPoint pos)
    {
        pos.X += 30;
        if (pos.X % 64 < 4 || pos.X % 64 > 60)
        {
            return false;
        }
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        return m_decor[pos.X / 64][pos.Y / 64].icon == 378;
    }

    bool Decor::IsSwitch(TinyPoint pos, TinyPoint& celSwitch)
    {
        pos.X += 30;
        if (pos.X % 64 < 4 || pos.X % 64 > 60)
        {
            return false;
        }
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        celSwitch.X = pos.X / 64;
        celSwitch.Y = pos.Y / 64;
        if (m_decor[pos.X / 64][pos.Y / 64].icon != 384)
        {
            return m_decor[pos.X / 64][pos.Y / 64].icon == 385;
        }
        return true;
    }

    bool Decor::IsEcraseur(TinyPoint pos)
    {
        if (m_time / 3 % 10 > 2)
        {
            return false;
        }
        pos.X += 30;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        return m_decor[pos.X / 64][pos.Y / 64].icon == 317;
    }

    bool Decor::IsBlitz(TinyPoint pos, bool bAlways)
    {
        pos.X += 30;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        TinyPoint tinyPoint;
        tinyPoint.X = pos.X / 64;
        tinyPoint.Y = pos.Y / 64;
        if (m_decor[tinyPoint.X][tinyPoint.Y].icon != 305)
        {
            return false;
        }
        if (bAlways)
        {
            return true;
        }
        return BlitzActif(tinyPoint.X, tinyPoint.Y);
    }

    bool Decor::IsRessort(TinyPoint pos)
    {
        pos.X += 30;
        pos.Y += 60;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        return m_decor[pos.X / 64][pos.Y / 64].icon == 211;
    }

    bool Decor::IsTemp(TinyPoint pos)
    {
        pos.X += 30;
        pos.Y += 60;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        return m_decor[pos.X / 64][pos.Y / 64].icon == 324;
    }

    bool Decor::IsBridge(TinyPoint pos, TinyPoint& celBridge)
    {
        pos.X += 30;
        pos.Y += 60;
        if (pos.X >= 0 && pos.X < 6400 && pos.Y >= 0 && pos.Y < 6400 && m_decor[pos.X / 64][pos.Y / 64].icon == 364)
        {
            celBridge.X = pos.X / 64;
            celBridge.Y = pos.Y / 64;
            return true;
        }
        pos.Y -= 60;
        if (pos.X >= 0 && pos.X < 6400 && pos.Y >= 0 && pos.Y < 6400 && m_decor[pos.X / 64][pos.Y / 64].icon == 364)
        {
            celBridge.X = pos.X / 64;
            celBridge.Y = pos.Y / 64;
            return true;
        }
        return false;
    }

    /**
     * @note Probes two cells: Blupi's own column and one tile ahead in his facing direction
     *       (step sign chosen from m_blupiDir), so he can open a door he is walking into. A
     *       door is recognised by an icon in the 334..336 range; the icon value (which encodes
     *       the door colour/lock) is returned, or -1 if neither probe hits a door.
     */
    int Decor::IsDoor(TinyPoint pos, TinyPoint& celPorte)
    {
        int num = ((m_blupiDir != Direction::Left) ? 60 : (-60));
        pos.X += 30;
        for (int i = 0; i < 2; i++)
        {
            if (pos.X >= 0 && pos.X < 6400 && pos.Y >= 0 && pos.Y < 6400 && m_decor[pos.X / 64][pos.Y / 64].icon >= 334
                && m_decor[pos.X / 64][pos.Y / 64].icon <= 336)
            {
                celPorte.X = pos.X / 64;
                celPorte.Y = pos.Y / 64;
                return m_decor[pos.X / 64][pos.Y / 64].icon;
            }
            pos.X += num;
        }
        return -1;
    }

    int Decor::IsTeleporte(TinyPoint pos)
    {
        if (pos.X % 64 > 6)
        {
            return -1;
        }
        pos.X += 30;
        pos.Y -= 60;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return -1;
        }
        if (m_decor[pos.X / 64][pos.Y / 64].icon >= 330 && m_decor[pos.X / 64][pos.Y / 64].icon <= 333)
        {
            return m_decor[pos.X / 64][pos.Y / 64].icon;
        }
        return -1;
    }

    /**
     * @note Pairing is implicit in the tile icon: the entry teleporter's icon (330..333,
     *       returned by IsTeleporte) IS the pair id. This linearly scans the whole 100x100 map
     *       for the first cell sharing that same icon and far enough from @p pos (>40px in any
     *       axis, to skip the entry cell itself), and returns its position as the exit. There
     *       is no separate teleporter index table.
     * @warning If a third tile shares the same teleporter icon, the first match in row-major
     *          order wins; level data must keep teleporter icons in matched pairs.
     */
    bool Decor::SearchTeleporte(TinyPoint pos, TinyPoint& newpos)
    {
        int num = IsTeleporte(pos);
        if (num == -1)
        {
            return false;
        }
        for (int i = 0; i < 100; i++)
        {
            for (int j = 0; j < 100; j++)
            {
                if (num == m_decor[i][j].icon)
                {
                    newpos.X = i * 64;
                    newpos.Y = j * 64 + 60;
                    if (newpos.X < pos.X - 40 || newpos.X > pos.X + 40 || newpos.Y < pos.Y - 40 || newpos.Y > pos.Y +
                        40)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Decor::IsNormalJump(TinyPoint pos)
    {
        pos.X += 32;
        pos.Y -= 32;
        if (m_blupiDir == Direction::Left)
        {
            pos.X -= 15;
        }
        else
        {
            pos.X += 15;
        }
        for (int i = 0; i < 2; i++)
        {
            int num = pos.X / 64;
            int num2 = pos.Y / 64;
            if (num2 < 0)
            {
                return false;
            }
            int icon = m_decor[num][num2].icon;
            if (!IsPassIcon(icon))
            {
                return false;
            }
            pos.Y -= 64;
        }
        return true;
    }

    bool Decor::IsSurfWater(TinyPoint pos)
    {
        if (pos.Y % 64 < 64 - BLUPISURF)
        {
            return false;
        }
        int icon = m_decor[(pos.X + 30) / 64][pos.Y / 64].icon;
        int icon2 = m_decor[(pos.X + 30) / 64][(pos.Y + BLUPISURF) / 64].icon;
        if (icon != 92 && icon2 == 92)
        {
            return true;
        }
        return false;
    }

    bool Decor::IsDeepWater(TinyPoint pos)
    {
        int num = (pos.X + 30) / 64;
        int num2 = pos.Y / 64;
        if (num < 0 || num >= 100 || num2 < 0 || num2 >= 100)
        {
            return false;
        }
        int icon = m_decor[num][num2].icon;
        if (icon != 91)
        {
            return icon == 92;
        }
        return true;
    }

    bool Decor::IsOutWater(TinyPoint pos)
    {
        int icon = m_decor[(pos.X + 30) / 64][(pos.Y + 30) / 64].icon;
        if (icon == 91 || icon == 92)
        {
            return false;
        }
        return IsPassIcon(icon);
    }

    bool Decor::IsPassIcon(int icon)
    {
        if (icon == 324 && m_time / 4 % 20 >= 18)
        {
            return true;
        }
        if (icon >= 0 && icon < MAXQUART)
        {
            for (int i = 0; i < 16; i++)
            {
                if (Tables::table_decor_quart[icon * 16 + i] != 0)
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool Decor::IsBlocIcon(int icon)
    {
        if (icon < 0 || icon >= MAXQUART)
        {
            return false;
        }
        if (icon == 324 && m_time / 4 % 20 < 18)
        {
            return true;
        }
        for (int i = 0; i < 16; i++)
        {
            if (Tables::table_decor_quart[icon * 16 + i] == 0)
            {
                return false;
            }
        }
        return true;
    }

    void Decor::FlushBalleTraj()
    {
        for (int i = 0; i < 1300; i++)
        {
            m_balleTraj[i] = 0;
        }
    }

    /**
     * @note The 100x100 tile occupancy grid is bit-packed into the flat m_balleTraj array:
     *       each row of 100 tiles uses 13 ints (8 bits each => 104 bit-slots), so the word
     *       index is y*13 + x/8 and the bit within the word is x&7. One set bit means "a bullet
     *       passes through this tile".
     * @warning @p pos here is in TILE coordinates already; IsBalleTraj() takes PIXEL
     *          coordinates and converts. Do not pass pixel coordinates to this setter.
     */
    void Decor::SetBalleTraj(TinyPoint pos)
    {
        if (pos.X >= 0 && pos.X < 100 && pos.Y >= 0 && pos.Y < 100)
        {
            int num = pos.Y * 13;
            num += pos.X / 8;
            int num2 = pos.X & 7;
            m_balleTraj[num] |= 1 << num2;
        }
    }

    bool Decor::IsBalleTraj(TinyPoint pos)
    {
        pos.X = (pos.X + 32) / 64;
        pos.Y = (pos.Y + 32) / 64;
        if (pos.X < 0 || pos.X >= 100 || pos.Y < 0 || pos.Y >= 100)
        {
            return false;
        }
        int num = pos.Y * 13;
        num += pos.X / 8;
        int num2 = pos.X & 7;
        return (m_balleTraj[num] & (1 << num2)) != 0;
    }

    void Decor::FlushMoveTraj()
    {
        for (int i = 0; i < 1300; i++)
        {
            m_moveTraj[i] = 0;
        }
    }

    /**
     * @note Same bit-packed layout as SetBalleTraj() (word = y*13 + x/8, bit = x&7), but for
     *       the moving-object occupancy grid. @p pos is in TILE coordinates.
     */
    void Decor::SetMoveTraj(TinyPoint pos)
    {
        if (pos.X >= 0 && pos.X < 100 && pos.Y >= 0 && pos.Y < 100)
        {
            int num = pos.Y * 13;
            num += pos.X / 8;
            int num2 = pos.X & 7;
            m_moveTraj[num] |= 1 << num2;
        }
    }

    bool Decor::IsMoveTraj(TinyPoint pos)
    {
        pos.X = (pos.X + 32) / 64;
        pos.Y = (pos.Y + 32) / 64;
        if (pos.X < 0 || pos.X >= 100 || pos.Y < 0 || pos.Y >= 100)
        {
            return false;
        }
        int num = pos.Y * 13;
        num += pos.X / 8;
        int num2 = pos.X & 7;
        return (m_moveTraj[num] & (1 << num2)) != 0;
    }

    /**
     * @note Marches in tile steps along @p dir from @p pos until it leaves the map or hits a
     *       blocking tile, accumulating 64 pixels per clear tile. Projectile types (36/39/41/
     *       42/93) short-circuit to a fixed 500px range. While scanning a bullet path
     *       (ObjectType23) it also records each traversed tile into the balle-trajectory grid
     *       via SetBalleTraj(). A few types get a small trailing adjustment (one tile or 10px)
     *       so the object stops just short of the wall.
     */
    int Decor::SearchDistRight(TinyPoint pos, TinyPoint dir, ObjectType type)
    {
        int num = 0;
        if (type == ObjectType::ObjectType36 || type == ObjectType::ObjectType39 || type == ObjectType::ObjectType41 || type == ObjectType::ObjectType42 || type == ObjectType::ObjectType93)
        {
            return 500;
        }
        pos.X = (pos.X + 32) / 64;
        pos.Y = (pos.Y + 32) / 64;
        while (pos.X >= 0 && pos.X < 100 && pos.Y >= 0 && pos.Y < 100 && !IsBlocIcon(m_decor[pos.X][pos.Y].icon))
        {
            if (type == ObjectType::ObjectType23)
            {
                SetBalleTraj(pos);
            }
            num += 64;
            pos.X += dir.X;
            pos.Y += dir.Y;
        }
        if ((type == ObjectType::ObjectType34 || type == ObjectType::ObjectType38) && num >= 64)
        {
            num -= 64;
        }
        if (type == ObjectType::ObjectType23 && num >= 10)
        {
            num -= 10;
        }
        return num;
    }

    /**
     * @note Detects a fan tile (icons 126..137) under Blupi and, for the directional fan
     *       heads (126 left, 129 right, 132 up, 135 down), decides whether Blupi is in the
     *       fan's active band (flag) based on his sub-tile position. When triggered it
     *       additionally CONSUMES the fan: it walks the fan's air-column tile by tile in the
     *       blow direction and clears every matching air tile (ModifDecor(..,-1)). So calling
     *       this for an active fan permanently removes that fan's visual column from the map.
     * @warning Side-effecting: a true return mutates the tile map. This is not a pure query.
     */
    bool Decor::IsVentillo(TinyPoint pos)
    {
        int num = 0;
        bool flag = false;
        TinyPoint tinyPoint;
        pos.X += 30;
        pos.Y += 30;
        if (pos.X < 0 || pos.X >= 6400 || pos.Y < 0 || pos.Y >= 6400)
        {
            return false;
        }
        int icon = m_decor[pos.X / 64][pos.Y / 64].icon;
        switch (icon)
        {
        default:
            return false;
        case 126:
            if (pos.X % 64 <= 16)
            {
                flag = true;
            }
            tinyPoint.X = -64;
            tinyPoint.Y = 0;
            num = 110;
            break;
        case 127:
        case 128:
        case 129:
        case 130:
        case 131:
        case 132:
        case 133:
        case 134:
        case 135:
        case 136:
        case 137:
            break;
        }
        if (icon == 129)
        {
            if (pos.X % 64 >= 48)
            {
                flag = true;
            }
            tinyPoint.X = 64;
            tinyPoint.Y = 0;
            num = 114;
        }
        if (icon == 132)
        {
            if (pos.Y % 64 <= 32)
            {
                flag = true;
            }
            tinyPoint.X = 0;
            tinyPoint.Y = -64;
            num = 118;
        }
        if (icon == 135)
        {
            if (pos.Y % 64 >= 48)
            {
                flag = true;
            }
            tinyPoint.X = 0;
            tinyPoint.Y = 64;
            num = 122;
        }
        if (!flag)
        {
            return false;
        }
        ModifDecor(pos, -1);
        do
        {
            pos.X += tinyPoint.X;
            pos.Y += tinyPoint.Y;
            if (num != m_decor[pos.X / 64][pos.Y / 64].icon)
            {
                break;
            }
            ModifDecor(pos, -1);
        }
        while (pos.X >= 0 && pos.X < 6400 && pos.Y >= 0 && pos.Y < 6400);
        return true;
    }

    /**
     * @note Intentionally empty in this port. The original cloud-net stop behaviour was
     *       elided; the function is kept so its call sites need not be conditionalised. @p rank
     *       is unused.
     */
    void Decor::NetStopCloud(int rank)
    {
    }

    void Decor::StartSploutchGlu(TinyPoint pos)
    {
        TinyPoint pos2;
        pos2.X = pos.X;
        pos2.Y = pos.Y;
        ObjectStart(pos2, ObjectType::ObjectType98, 0);
        pos2.X = pos.X + 15;
        pos2.Y = pos.Y + 20;
        ObjectStart(pos2, ObjectType::ObjectType99, 0);
        pos2.X = pos.X - 20;
        pos2.Y = pos.Y + 18;
        ObjectStart(pos2, ObjectType::ObjectType99, 0);
        pos2.X = pos.X + 23;
        pos2.Y = pos.Y - 18;
        ObjectStart(pos2, ObjectType::ObjectType99, 0);
        pos2.X = pos.X - 15;
        pos2.Y = pos.Y - 18;
        ObjectStart(pos2, ObjectType::ObjectType99, 0);
        pos2.X = pos.X + 32;
        pos2.Y = pos.Y + 10;
        ObjectStart(pos2, ObjectType::ObjectType100, 0);
        pos2.X = pos.X - 28;
        pos2.Y = pos.Y + 15;
        ObjectStart(pos2, ObjectType::ObjectType100, 0);
        StopSound(SoundChannel::SoundChannel16);
        StopSound(SoundChannel::SoundChannel18);
        StopSound(SoundChannel::SoundChannel29);
        StopSound(SoundChannel::SoundChannel31);
        PlaySound(SoundChannel::SoundChannel51, pos);
    }

    /**
     * @note The @p speed argument encodes both direction and magnitude with a +/-50 bias:
     *       |speed|>50 means vertical motion (down if >50, up if <-50, magnitude = |speed|-50);
     *       otherwise horizontal (right if >0, left if <0). The travel distance is found by
     *       ray-casting to the next wall via SearchDistRight(), which sets posEnd and a
     *       step-advance proportional to speed*distance so the object reaches the wall in a
     *       consistent time. A bullet (ObjectType23) with zero clear distance is immediately
     *       cancelled (slot returned to ObjectType0).
     * @note Stationary effects (speed == 0) skip all the ray-cast logic and just sit at @p pos.
     *       MoveObjectPriority() is called so the new object gets a sensible draw order.
     */
    int Decor::ObjectStart(TinyPoint pos, ObjectType type, int speed)
    {
        int num = MoveObjectFree();
        if (num == -1)
        {
            return -1;
        }
        m_moveObject[num].type = type;
        m_moveObject[num].phase = 0;
        m_moveObject[num].posCurrent = pos;
        m_moveObject[num].posStart = pos;
        m_moveObject[num].posEnd = pos;
        MoveObjectStepIcon(num);
        if (speed != 0)
        {
            TinyPoint tinyPoint = pos;
            int num2 = speed;
            int num3 = 0;
            TinyPoint dir;
            if (num2 > 50)
            {
                num2 -= 50;
                dir.X = 0;
                dir.Y = 1;
                num3 = SearchDistRight(tinyPoint, dir, type);
                tinyPoint.Y += num3;
            }
            else if (num2 < -50)
            {
                num2 += 50;
                dir.X = 0;
                dir.Y = -1;
                num3 = SearchDistRight(tinyPoint, dir, type);
                tinyPoint.Y -= num3;
            }
            else if (num2 > 0)
            {
                dir.X = 1;
                dir.Y = 0;
                num3 = SearchDistRight(tinyPoint, dir, type);
                tinyPoint.X += num3;
            }
            else if (num2 < 0)
            {
                dir.X = -1;
                dir.Y = 0;
                num3 = SearchDistRight(tinyPoint, dir, type);
                tinyPoint.X -= num3;
            }
            if (num3 == 0)
            {
                if (type == ObjectType::ObjectType23)
                {
                    m_moveObject[num].type = ObjectType::ObjectType0;
                    return num;
                }
            }
            else
            {
                m_moveObject[num].posEnd = tinyPoint;
                m_moveObject[num].timeStopStart = 0;
                m_moveObject[num].stepAdvance = Config::ScaleTime(std::abs(num2 * num3 / 64));
                m_moveObject[num].step = 2;
                m_moveObject[num].time = 0;
            }
        }
        MoveObjectPriority(num);
        return num;
    }

    /**
     * @note For "physical" object types (enemies/crates in the listed set) the deletion also
     *       spawns a spinning ByeByeObject debris fragment from the object's current sprite, with
     *       a per-type spin/animation speed, so destruction is visible. Freeing the slot is just
     *       setting its type back to ObjectType0; the pool slot is reused later by MoveObjectFree().
     */
    bool Decor::ObjectDelete(TinyPoint pos, ObjectType type)
    {
        int num = MoveObjectSearch(pos, type);
        if (num == -1)
        {
            return false;
        }
        if (
            m_moveObject[num].type == ObjectType::ObjectType4 ||
            m_moveObject[num].type == ObjectType::ObjectType12 ||
            m_moveObject[num].type == ObjectType::ObjectType16 ||
            m_moveObject[num].type == ObjectType::ObjectType17 ||
            m_moveObject[num].type == ObjectType::ObjectType20 ||
            m_moveObject[num].type == ObjectType::ObjectType40 ||
            m_moveObject[num].type == ObjectType::ObjectType96 ||
            m_moveObject[num].type == ObjectType::ObjectType97)
        {
            int num2 = 17;
            double animationSpeed = 1.0;
            if (m_moveObject[num].type == ObjectType::ObjectType4)
            {
                num2 = 7;
            }
            if (m_moveObject[num].type == ObjectType::ObjectType17 || m_moveObject[num].type == ObjectType::ObjectType20)
            {
                num2 = 33;
            }
            if (m_moveObject[num].type == ObjectType::ObjectType40)
            {
                animationSpeed = 0.5;
            }
            ByeByeAdd(m_moveObject[num].channel, m_moveObject[num].icon, m_moveObject[num].posCurrent, num2,
                      animationSpeed);
        }
        m_moveObject[num].type = ObjectType::ObjectType0;
        return true;
    }

    /**
     * @note Writes @p icon (game-space @p pos is divided by 64 to a tile index). As a special
     *       case, clearing a fan tile (icons 126..137 -> -1) spawns a spinning debris fragment
     *       so the fan visibly breaks apart rather than simply vanishing.
     * @warning Does not bounds-check the tile index; @p pos must lie within the 100x100 map.
     */
    void Decor::ModifDecor(TinyPoint pos, int icon)
    {
        int icon2 = m_decor[pos.X / 64][pos.Y / 64].icon;
        if (icon == -1 && icon2 >= 126 && icon2 <= 137)
        {
            ByeByeAdd(PixmapChannel::Object, icon2, pos, 17.0, 1.0);
        }
        m_decor[pos.X / 64][pos.Y / 64].icon = icon;
    }

    /**
     * @note Per-frame driver over the whole object pool. It first clears m_blupiVector and
     *       m_blupiTransport so lift objects can re-assert Blupi's carry link this frame, then
     *       for each active slot calls MoveObjectStepLine() (linear motion) followed by
     *       MoveObjectStepIcon() (animation + per-type AI/collision). After stepping certain
     *       enemy types (4/32/33) it runs the contact reactions inline: if the enemy reaches a
     *       follower NPC or electrocutes Blupi it spawns an explosion (ObjectType8/38), kicks
     *       the screen shake, plays a sound and deletes the participants.
     */
    void Decor::MoveObjectStep()
    {
        m_blupiVector.X = 0;
        m_blupiVector.Y = 0;
        m_blupiTransport = -1;
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType0)
            {
                continue;
            }
            MoveObjectStepLine(i);
            MoveObjectStepIcon(i);
            if (m_moveObject[i].type == ObjectType::ObjectType4 || m_moveObject[i].type == ObjectType::ObjectType33 || m_moveObject[i].type == ObjectType::ObjectType32)
            {
                int num = MovePersoDetect(m_moveObject[i].posCurrent);
                if (num != -1)
                {
                    TinyPoint posCurrent = m_moveObject[i].posCurrent;
                    posCurrent.X -= 34;
                    posCurrent.Y -= 34;
                    ObjectStart(posCurrent, ObjectType::ObjectType8, 0);
                    PlaySound(SoundChannel::SoundChannel10, m_moveObject[i].posCurrent);
                    m_decorAction = DecorAction::SmallShake;
                    m_decorPhase = 0;
                    posCurrent = m_moveObject[i].posCurrent;
                    posCurrent.X += 2;
                    posCurrent.Y += BLUPIOFFY;
                    ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
                    ObjectStart(posCurrent, ObjectType::ObjectType37, 0);
                    ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
                }
                if (BlupiElectro(m_moveObject[i].posCurrent))
                {
                    TinyPoint posCurrent = m_moveObject[i].posCurrent;
                    posCurrent.X += 2;
                    posCurrent.Y += BLUPIOFFY;
                    ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
                    ObjectStart(posCurrent, ObjectType::ObjectType38, 55);
                    PlaySound(SoundChannel::SoundChannel59, posCurrent);
                }
            }
        }
    }

    /**
     * @note Implements the table-driven linear path as a 4-phase cycle stored in step:
     *       1 = dwell at start for timeStopStart frames, 2 = interpolate posStart->posEnd over
     *       stepAdvance frames, 3 = dwell at end for timeStopEnd frames, 4 = interpolate back
     *       posEnd->posStart over stepRecede frames, then loop to 1. Position is computed by
     *       linear interpolation on the frame counter (time), not by per-frame accumulation,
     *       so it cannot drift. One-shot objects (bubbles 15, bullets 23) self-destruct when
     *       they reach posEnd instead of receding.
     * @note Lift objects (1/47/48) special-case Blupi: if his feet strip overlaps the lift top
     *       (flag), this records the lift's displacement into m_blupiVector and sets
     *       m_blupiTransport = i so BlupiStep() carries him; types 47/48 add a constant sideways
     *       conveyor nudge. The homing type 97 instead walks one pixel/frame toward Blupi and
     *       explodes (spawns ObjectType9) when its path is blocked.
     * @note Every object also stamps its tile into the move-trajectory occupancy grid
     *       (SetMoveTraj) so other systems can test for object presence.
     */
    void Decor::MoveObjectStepLine(int i)
    {
        TinyPoint tinyPoint;
        bool flag = false;
        TinyRect tinyRect = TinyRect();
        if ((m_moveObject[i].type == ObjectType::ObjectType1 || m_moveObject[i].type == ObjectType::ObjectType47 || m_moveObject[i].type == ObjectType::ObjectType48) && !m_blupiSuspend)
        {
            TinyRect src = TinyRect();
            src.Left = m_blupiPos.X + 20;
            src.Right = m_blupiPos.X + 60 - 20;
            src.Top = m_blupiPos.Y + 60 - 2;
            src.Bottom = m_blupiPos.Y + 60 - 1;
            tinyRect.Left = m_moveObject[i].posCurrent.X;
            tinyRect.Right = m_moveObject[i].posCurrent.X + 64;
            tinyRect.Top = m_moveObject[i].posCurrent.Y;
            tinyRect.Bottom = m_moveObject[i].posCurrent.Y + 16;
            TinyRect dst;
            flag = Misc::IntersectRect(dst, tinyRect, src);
            tinyPoint = m_moveObject[i].posCurrent;
        }
        TinyPoint end;
        if (m_blupiFocus && !m_blupiHide && m_moveObject[i].type == ObjectType::ObjectType97)
        {
            end = m_moveObject[i].posCurrent;
            if (end.X < m_blupiPos.X)
            {
                end.X++;
            }
            if (end.X > m_blupiPos.X)
            {
                end.X--;
            }
            if (end.Y < m_blupiPos.Y)
            {
                end.Y++;
            }
            if (end.Y > m_blupiPos.Y)
            {
                end.Y--;
            }
            tinyRect.Left = end.X + 10;
            tinyRect.Right = end.X + 60 - 10;
            tinyRect.Top = end.Y + 10;
            tinyRect.Bottom = end.Y + 60 - 10;
            if (TestPath(tinyRect, m_moveObject[i].posCurrent, end))
            {
                m_moveObject[i].posCurrent = end;
                m_moveObject[i].posStart = end;
                m_moveObject[i].posEnd = end;
            }
            else
            {
                ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
                end.X -= 34;
                end.Y -= 34;
                ObjectStart(end, ObjectType::ObjectType9, 0);
                PlaySound(SoundChannel::SoundChannel10, end);
                m_decorAction = DecorAction::SmallShake;
                m_decorPhase = 0;
            }
        }
        if (m_moveObject[i].posStart.X != m_moveObject[i].posEnd.X || m_moveObject[i].posStart.Y != m_moveObject[i].
            posEnd.Y)
        {
            if (m_moveObject[i].step == 1)
            {
                if (m_moveObject[i].time < m_moveObject[i].timeStopStart)
                {
                    m_moveObject[i].time++;
                }
                else
                {
                    m_moveObject[i].step = 2;
                    m_moveObject[i].time = 0;
                }
            }
            else if (m_moveObject[i].step == 2)
            {
                if (m_moveObject[i].posCurrent.X != m_moveObject[i].posEnd.X || m_moveObject[i].posCurrent.Y !=
                    m_moveObject[i].posEnd.Y)
                {
                    m_moveObject[i].time++;
                    if (m_moveObject[i].stepAdvance != 0)
                    {
                        m_moveObject[i].posCurrent.X = (m_moveObject[i].posEnd.X - m_moveObject[i].posStart.X) *
                            m_moveObject[i].time / m_moveObject[i].stepAdvance + m_moveObject[i].posStart.X;
                        m_moveObject[i].posCurrent.Y = (m_moveObject[i].posEnd.Y - m_moveObject[i].posStart.Y) *
                            m_moveObject[i].time / m_moveObject[i].stepAdvance + m_moveObject[i].posStart.Y;
                    }
                }
                else if (m_moveObject[i].type == ObjectType::ObjectType15 || m_moveObject[i].type == ObjectType::ObjectType23)
                {
                    m_moveObject[i].type = ObjectType::ObjectType0;
                }
                else if (m_moveObject[i].type == ObjectType::ObjectType34)
                {
                    m_moveObject[i].posStart = m_moveObject[i].posCurrent;
                    m_moveObject[i].posEnd = m_moveObject[i].posCurrent;
                    m_moveObject[i].step = 3;
                    m_moveObject[i].time = 0;
                }
                else
                {
                    m_moveObject[i].step = 3;
                    m_moveObject[i].time = 0;
                }
            }
            else if (m_moveObject[i].step == 3)
            {
                if (m_moveObject[i].time < m_moveObject[i].timeStopEnd)
                {
                    m_moveObject[i].time++;
                }
                else
                {
                    m_moveObject[i].step = 4;
                    m_moveObject[i].time = 0;
                }
            }
            else if (m_moveObject[i].step == 4)
            {
                if (m_moveObject[i].posCurrent.X != m_moveObject[i].posStart.X || m_moveObject[i].posCurrent.Y !=
                    m_moveObject[i].posStart.Y)
                {
                    m_moveObject[i].time++;
                    if (m_moveObject[i].stepRecede != 0)
                    {
                        m_moveObject[i].posCurrent.X = (m_moveObject[i].posStart.X - m_moveObject[i].posEnd.X) *
                            m_moveObject[i].time / m_moveObject[i].stepRecede + m_moveObject[i].posEnd.X;
                        m_moveObject[i].posCurrent.Y = (m_moveObject[i].posStart.Y - m_moveObject[i].posEnd.Y) *
                            m_moveObject[i].time / m_moveObject[i].stepRecede + m_moveObject[i].posEnd.Y;
                    }
                }
                else
                {
                    m_moveObject[i].step = 1;
                    m_moveObject[i].time = 0;
                }
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType22 && m_moveObject[i].step == 3)
        {
            m_moveObject[i].type = ObjectType::ObjectType0;
        }
        end = m_moveObject[i].posCurrent;
        if (m_moveObject[i].type == ObjectType::ObjectType1 || m_moveObject[i].type == ObjectType::ObjectType47 || m_moveObject[i].type == ObjectType::ObjectType48)
        {
            end.Y -= 64;
        }
        end.X = (end.X + 32) / 64;
        end.Y = (end.Y + 32) / 64;
        SetMoveTraj(end);
        if (flag)
        {
            m_blupiVector.X = m_moveObject[i].posCurrent.X - tinyPoint.X;
            m_blupiVector.Y = m_moveObject[i].posCurrent.Y - (m_blupiPos.Y + 60 - BLUPIFLOOR);
            if (m_moveObject[i].type == ObjectType::ObjectType47)
            {
                m_blupiVector.X += 2;
            }
            if (m_moveObject[i].type == ObjectType::ObjectType48)
            {
                m_blupiVector.X -= 2;
            }
            if (m_blupiTimeNoAsc == 0)
            {
                m_blupiTransport = i;
            }
        }
    }

    /**
     * @note The per-type animation + AI dispatcher and the heart of object behaviour. It is a
     *       long flat if-chain keyed on ObjectType. Two patterns appear:
     *       - Table-driven animation: icon = baseIcon + phase/ScaleDiv(N) % frameCount (or an
     *         indirection through a Tables::table_* array), simply cycling the sprite. The phase
     *         counter is advanced at the end of the function.
     *       - Hand-coded AI/lifetime: bespoke logic for specific types, e.g. dynamite
     *         (ObjectType56) fires a scripted sequence of DynamiteStart() blasts at fixed phase
     *         ticks and self-destructs after phase 70; collectibles award score and despawn on
     *         Blupi contact; enemies test collision and kill/explode. These branches mutate
     *         game state (score, doors, m_decorAction shake, sounds) and may convert the object
     *         to ObjectType0 (free) or to an explosion type.
     * @note Channel is reassigned per branch because one pool slot can show sprites from
     *       different sheets depending on type. ScaleDiv/ScaleTime keep the original 20 FPS
     *       cadence at higher frame rates.
     */
    void Decor::MoveObjectStepIcon(int i)
    {
        if (m_moveObject[i].type == ObjectType::ObjectType47)
        {
            m_moveObject[i].icon = Tables::table_chenille[m_moveObject[i].phase / Config::ScaleDiv(1) % 6];
        }
        if (m_moveObject[i].type == ObjectType::ObjectType48)
        {
            m_moveObject[i].icon = Tables::table_chenillei[m_moveObject[i].phase / Config::ScaleDiv(1) % 6];
        }
        if (m_moveObject[i].type == ObjectType::ObjectType2)
        {
            m_moveObject[i].icon = 12 + m_moveObject[i].phase / Config::ScaleDiv(2) % 9;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType3)
        {
            m_moveObject[i].icon = 48 + m_moveObject[i].phase / Config::ScaleDiv(2) % 9;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType16)
        {
            m_moveObject[i].icon = 69 + m_moveObject[i].phase / Config::ScaleDiv(1) % 9;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType96)
        {
            m_moveObject[i].icon = Tables::table_follow1[m_moveObject[i].phase / Config::ScaleDiv(1) % 26];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType97)
        {
            m_moveObject[i].icon = Tables::table_follow2[m_moveObject[i].phase / Config::ScaleDiv(1) % 5];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType200)
        {
            m_moveObject[i].icon = 257 + m_moveObject[i].phase / Config::ScaleDiv(1) % 6;
            m_moveObject[i].channel = PixmapChannel::Blupi;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType201)
        {
            m_moveObject[i].icon = 257 + m_moveObject[i].phase / Config::ScaleDiv(1) % 6;
            m_moveObject[i].channel = PixmapChannel::Blupi1_11;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType202)
        {
            m_moveObject[i].icon = 257 + m_moveObject[i].phase / Config::ScaleDiv(1) % 6;
            m_moveObject[i].channel = PixmapChannel::Blupi1_12;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType203)
        {
            m_moveObject[i].icon = 257 + m_moveObject[i].phase / Config::ScaleDiv(1) % 6;
            m_moveObject[i].channel = PixmapChannel::Blupi1_13;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType55)
        {
            m_moveObject[i].icon = 252;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType56)
        {
            m_moveObject[i].icon = Tables::table_dynamitef[m_moveObject[i].phase / Config::ScaleDiv(1) % 100];
            m_moveObject[i].channel = PixmapChannel::Element;
            if (m_moveObject[i].phase == Config::ScaleTime(50))
            {
                DynamiteStart(i, 0, 0);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(53))
            {
                DynamiteStart(i, -100, 8);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(55))
            {
                DynamiteStart(i, 80, 10);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(56))
            {
                DynamiteStart(i, -15, -100);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(59))
            {
                DynamiteStart(i, 20, 70);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(62))
            {
                DynamiteStart(i, 30, -50);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(64))
            {
                DynamiteStart(i, -40, 30);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(67))
            {
                DynamiteStart(i, -180, 10);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(69))
            {
                DynamiteStart(i, 200, -10);
            }
            if (m_moveObject[i].phase >= Config::ScaleTime(70))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType5)
        {
            if (m_moveObject[i].phase / Config::ScaleDiv(3) % 22 < 11)
            {
                m_moveObject[i].icon = m_moveObject[i].phase / Config::ScaleDiv(3) % 11;
            }
            else
            {
                m_moveObject[i].icon = 11 - m_moveObject[i].phase / Config::ScaleDiv(3) % 11;
            }
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType6)
        {
            m_moveObject[i].icon = 21 + m_moveObject[i].phase / Config::ScaleDiv(4) % 8;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType7)
        {
            m_moveObject[i].icon = 29 + m_moveObject[i].phase / Config::ScaleDiv(3) % 8;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType21)
        {
            m_moveObject[i].icon = Tables::table_cle[m_moveObject[i].phase / Config::ScaleDiv(3) % 12];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType49)
        {
            m_moveObject[i].icon = Tables::table_cle1[m_moveObject[i].phase / Config::ScaleDiv(3) % 12];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType50)
        {
            m_moveObject[i].icon = Tables::table_cle2[m_moveObject[i].phase / Config::ScaleDiv(3) % 12];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType51)
        {
            m_moveObject[i].icon = Tables::table_cle3[m_moveObject[i].phase / Config::ScaleDiv(3) % 12];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType24)
        {
            m_moveObject[i].icon = Tables::table_skate[m_moveObject[i].phase / Config::ScaleDiv(1) % 34];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType25)
        {
            m_moveObject[i].icon = Tables::table_shield[m_moveObject[i].phase / Config::ScaleDiv(2) % 16];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType26)
        {
            m_moveObject[i].icon = Tables::table_power[m_moveObject[i].phase / Config::ScaleDiv(2) % 8];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType40)
        {
            m_moveObject[i].icon = Tables::table_invert[m_moveObject[i].phase / Config::ScaleDiv(2) % 20];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType31)
        {
            m_moveObject[i].icon = Tables::table_charge[m_moveObject[i].phase / Config::ScaleDiv(2) % 6];
            m_moveObject[i].channel = PixmapChannel::Object;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType27)
        {
            m_moveObject[i].icon = Tables::table_magictrack[m_moveObject[i].phase / Config::ScaleDiv(1) % 24];
            m_moveObject[i].channel = PixmapChannel::Element;
            if (m_moveObject[i].phase >= Config::ScaleTime(24))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType57)
        {
            m_moveObject[i].icon = Tables::table_shieldtrack[m_moveObject[i].phase / Config::ScaleDiv(1) % 20];
            m_moveObject[i].channel = PixmapChannel::Element;
            if (m_moveObject[i].phase >= Config::ScaleTime(20))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType39)
        {
            m_moveObject[i].icon = Tables::table_tresortrack[m_moveObject[i].phase / Config::ScaleDiv(1) % 11];
            m_moveObject[i].channel = PixmapChannel::Element;
            if (m_moveObject[i].phase >= Config::ScaleTime(11))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType58 && m_moveObject[i].phase >= Config::ScaleTime(20))
        {
            m_moveObject[i].type = ObjectType::ObjectType0;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType8)
        {
            if (m_moveObject[i].phase / Config::ScaleDiv(1) >= Tables::table_explo1Length)
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo1[m_moveObject[i].phase / Config::ScaleDiv(1)];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType9)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(20))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo2[(m_moveObject[i].phase / Config::ScaleDiv(1)) % 20];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType10)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(20))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo3[m_moveObject[i].phase / Config::ScaleDiv(1) % 20];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType11)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(9))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo4[m_moveObject[i].phase / Config::ScaleDiv(1) % 9];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType90)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(12))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo5[m_moveObject[i].phase / Config::ScaleDiv(1) % 12];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType91)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(6))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo6[m_moveObject[i].phase / Config::ScaleDiv(1) % 6];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType92)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(128))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo7[m_moveObject[i].phase / Config::ScaleDiv(1) % 128];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType93)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(5))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_explo8[m_moveObject[i].phase / Config::ScaleDiv(1) % 5];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType98)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(10))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_sploutch1[m_moveObject[i].phase / Config::ScaleDiv(1) % 10];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType99)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(13))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_sploutch2[m_moveObject[i].phase / Config::ScaleDiv(1) % 13];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType100)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(18))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_sploutch3[m_moveObject[i].phase / Config::ScaleDiv(1) % 18];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType53)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(90))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_tentacule[m_moveObject[i].phase / Config::ScaleDiv(2) % 45];
                m_moveObject[i].channel = PixmapChannel::Explosion;
            }
        }
        TinyPoint pos;
        if (m_moveObject[i].type == ObjectType::ObjectType52)
        {
            if (m_moveObject[i].phase == 0)
            {
                PlaySound(SoundChannel::SoundChannel72, m_moveObject[i].posStart);
            }
            if (m_moveObject[i].phase == Config::ScaleTime(137))
            {
                PlaySound(SoundChannel::SoundChannel73, m_moveObject[i].posStart);
            }
            if (m_moveObject[i].phase >= Config::ScaleTime(157))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_bridge[m_moveObject[i].phase / Config::ScaleDiv(1) % 157];
                m_moveObject[i].channel = PixmapChannel::Object;
                pos.X = m_moveObject[i].posStart.X / 64;
                pos.Y = m_moveObject[i].posStart.Y / 64;
                m_decor[pos.X][pos.Y].icon = m_moveObject[i].icon;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType36)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(16))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_pollution[m_moveObject[i].phase / Config::ScaleDiv(2) % 8];
                m_moveObject[i].channel = PixmapChannel::Element;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType41)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(16))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_invertstart[m_moveObject[i].phase / Config::ScaleDiv(2) % 8];
                m_moveObject[i].channel = PixmapChannel::Element;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType42)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(16))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_invertstop[m_moveObject[i].phase / Config::ScaleDiv(2) % 8];
                m_moveObject[i].channel = PixmapChannel::Element;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType14)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(14))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_plouf[m_moveObject[i].phase / Config::ScaleDiv(2) % 7];
                m_moveObject[i].channel = PixmapChannel::Object;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType35)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(6))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_tiplouf[m_moveObject[i].phase / Config::ScaleDiv(2) % 7];
                m_moveObject[i].channel = PixmapChannel::Object;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType15)
        {
            m_moveObject[i].icon = Tables::table_blup[m_moveObject[i].phase / Config::ScaleDiv(2) % 20];
            m_moveObject[i].channel = PixmapChannel::Object;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType4)
        {
            if (m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 22];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 22];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            else
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 22];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 22];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_bulldozer_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType17)
        {
            if (m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_poisson_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 48];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_poisson_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 48];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_poisson_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_poisson_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            else
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_poisson_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 48];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_poisson_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 48];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_poisson_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_poisson_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType20)
        {
            if (m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 10];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 10];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            else
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 10];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 10];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_oiseau_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType44)
        {
            if (m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_guepe_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 5];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_guepe_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 5];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_guepe_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 6];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_guepe_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 6];
                }
            }
            else
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_guepe_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 5];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_guepe_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 5];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_guepe_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 6];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_guepe_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 6];
                }
            }
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType54)
        {
            if (m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_creature_turn2[(m_moveObject[i].time / Config::ScaleDiv(1)) % 152];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_creature_turn2[(m_moveObject[i].time / Config::ScaleDiv(1)) % 152];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_creature_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_creature_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            else
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_creature_turn2[(m_moveObject[i].time / Config::ScaleDiv(1)) % 152];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_creature_turn2[(m_moveObject[i].time / Config::ScaleDiv(1)) % 152];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_creature_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_creature_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType32)
        {
            if (m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_blupih_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 26];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_blupih_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 26];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_blupih_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_blupih_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            else
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_blupih_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 26];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_blupih_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 26];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_blupih_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_blupih_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            if ((m_moveObject[i].step == 1 || m_moveObject[i].step == 3) && m_moveObject[i].time == Config::ScaleTime(21))
            {
                pos.X = m_moveObject[i].posCurrent.X;
                pos.Y = m_moveObject[i].posCurrent.Y + 40;
                if (ObjectStart(pos, ObjectType::ObjectType23, 55) != -1)
                {
                    PlaySound(SoundChannel::SoundChannel52, pos);
                }
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType33)
        {
            if (m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_blupit_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 24];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_blupit_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 24];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_blupit_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_blupit_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            else
            {
                if (m_moveObject[i].step == 1)
                {
                    m_moveObject[i].icon = Tables::table_blupit_turn2r[(m_moveObject[i].time / Config::ScaleDiv(1)) % 24];
                }
                if (m_moveObject[i].step == 3)
                {
                    m_moveObject[i].icon = Tables::table_blupit_turn2l[(m_moveObject[i].time / Config::ScaleDiv(1)) % 24];
                }
                if (m_moveObject[i].step == 2)
                {
                    m_moveObject[i].icon = Tables::table_blupit_right[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
                if (m_moveObject[i].step == 4)
                {
                    m_moveObject[i].icon = Tables::table_blupit_left[(m_moveObject[i].time / Config::ScaleDiv(1)) % 8];
                }
            }
            if ((m_moveObject[i].step == 1 || m_moveObject[i].step == 3) && m_moveObject[i].time == Config::ScaleTime(3))
            {
                int speed;
                if ((m_moveObject[i].posStart.X < m_moveObject[i].posEnd.X && m_moveObject[i].step == 1) || (
                    m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X && m_moveObject[i].step == 3))
                {
                    pos.X = m_moveObject[i].posCurrent.X - 30;
                    pos.Y = m_moveObject[i].posCurrent.Y + BLUPIOFFY;
                    speed = -5;
                }
                else
                {
                    pos.X = m_moveObject[i].posCurrent.X + 30;
                    pos.Y = m_moveObject[i].posCurrent.Y + BLUPIOFFY;
                    speed = 5;
                }
                if (ObjectStart(pos, ObjectType::ObjectType23, speed) != -1)
                {
                    PlaySound(SoundChannel::SoundChannel52, pos);
                }
            }
            if ((m_moveObject[i].step == 1 || m_moveObject[i].step == 3) && m_moveObject[i].time == Config::ScaleTime(21))
            {
                int speed;
                if ((m_moveObject[i].posStart.X < m_moveObject[i].posEnd.X && m_moveObject[i].step == 1) || (
                    m_moveObject[i].posStart.X > m_moveObject[i].posEnd.X && m_moveObject[i].step == 3))
                {
                    pos.X = m_moveObject[i].posCurrent.X + 30;
                    pos.Y = m_moveObject[i].posCurrent.Y + BLUPIOFFY;
                    speed = 5;
                }
                else
                {
                    pos.X = m_moveObject[i].posCurrent.X - 30;
                    pos.Y = m_moveObject[i].posCurrent.Y + BLUPIOFFY;
                    speed = -5;
                }
                if (ObjectStart(pos, ObjectType::ObjectType23, speed) != -1)
                {
                    PlaySound(SoundChannel::SoundChannel52, pos);
                }
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType34)
        {
            m_moveObject[i].icon = Tables::table_glu[m_moveObject[i].phase / Config::ScaleDiv(1) % 25];
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType37)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(70))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_clear[m_moveObject[i].phase / Config::ScaleDiv(1) % 70];
                m_moveObject[i].channel = PixmapChannel::Element;
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType38)
        {
            if (m_moveObject[i].phase >= Config::ScaleTime(90))
            {
                m_moveObject[i].type = ObjectType::ObjectType0;
            }
            else
            {
                m_moveObject[i].icon = Tables::table_electro[m_moveObject[i].phase / Config::ScaleDiv(1) % 90];
                if (m_moveObject[i].phase < Config::ScaleTime(30))
                {
                    m_moveObject[i].channel = PixmapChannel::Blupi1_12;
                }
                else
                {
                    m_moveObject[i].channel = PixmapChannel::Element;
                }
            }
        }
        if (m_moveObject[i].type == ObjectType::ObjectType13)
        {
            m_moveObject[i].icon = 68;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType46)
        {
            m_moveObject[i].icon = 208;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType19)
        {
            m_moveObject[i].icon = 89;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType28)
        {
            m_moveObject[i].icon = 167;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType23)
        {
            m_moveObject[i].icon = 176;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType29)
        {
            m_moveObject[i].icon = 177;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        if (m_moveObject[i].type == ObjectType::ObjectType30)
        {
            m_moveObject[i].icon = 178;
            m_moveObject[i].channel = PixmapChannel::Element;
        }
        m_moveObject[i].phase++;
        if (m_moveObject[i].phase > 32700 * Config::ScaleDiv(1))
        {
            m_moveObject[i].phase = 0;
        }
    }

    /**
     * @note One blast of a dynamite chain. It spawns an explosion sprite (ObjectType8) at the
     *       (dx,dy) offset, and only the central blast (dx==dy==0) also plays the boom sound and
     *       triggers the screen shake. It then clears destructible hazard tiles (saws 378/379,
     *       drips 404/410) in the 2x2-tile blast area, destroys every enemy/crate/object whose
     *       box overlaps the 128x128 blast rect (crates are exploded as a linked group via
     *       SearchLinkCaisse so a whole stack goes at once), and finally kills Blupi
     *       (BlupiAction::Clear1) if he is inside the blast and not shielded/super/hidden.
     */
    void Decor::DynamiteStart(int i, int dx, int dy)
    {
        TinyPoint posStart = m_moveObject[i].posStart;
        posStart.X -= 34;
        posStart.Y -= 34;
        posStart.X += dx;
        posStart.Y -= dy;
        ObjectStart(posStart, ObjectType::ObjectType8, 0);
        if (dx == 0 && dy == 0)
        {
            PlaySound(SoundChannel::SoundChannel10, posStart);
            m_decorAction = DecorAction::SmallShake;
            m_decorPhase = 0;
        }
        TinyRect src = TinyRect();
        src.Left = posStart.X;
        src.Right = posStart.X + 128;
        src.Top = posStart.Y;
        src.Bottom = posStart.Y + 128;
        TinyPoint tinyPoint;
        tinyPoint.Y = posStart.Y / 64;
        TinyPoint pos;
        for (int j = 0; j < 2; j++)
        {
            tinyPoint.X = posStart.X / 64;
            for (int k = 0; k < 2; k++)
            {
                if (tinyPoint.X >= 0 && tinyPoint.X < 100 && tinyPoint.Y >= 0 && tinyPoint.Y < 100)
                {
                    int icon = m_decor[tinyPoint.X][tinyPoint.Y].icon;
                    if (icon == 378 || icon == 379 || icon == 404 || icon == 410)
                    {
                        pos.X = tinyPoint.X * 64;
                        pos.Y = tinyPoint.Y * 64;
                        ModifDecor(pos, -1);
                    }
                }
                tinyPoint.X++;
            }
            tinyPoint.Y++;
        }
        TinyRect src2 = TinyRect();
        for (i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (
                m_moveObject[i].type == ObjectType::ObjectType2 ||
                m_moveObject[i].type == ObjectType::ObjectType3 ||
                m_moveObject[i].type == ObjectType::ObjectType96 ||
                m_moveObject[i].type == ObjectType::ObjectType97 ||
                m_moveObject[i].type == ObjectType::ObjectType4 ||
                m_moveObject[i].type == ObjectType::ObjectType6 ||
                m_moveObject[i].type == ObjectType::ObjectType12 ||
                m_moveObject[i].type == ObjectType::ObjectType13 ||
                m_moveObject[i].type == ObjectType::ObjectType16 ||
                m_moveObject[i].type == ObjectType::ObjectType17 ||
                m_moveObject[i].type == ObjectType::ObjectType18 ||
                m_moveObject[i].type == ObjectType::ObjectType19 ||
                m_moveObject[i].type == ObjectType::ObjectType20 ||
                m_moveObject[i].type == ObjectType::ObjectType24 ||
                m_moveObject[i].type == ObjectType::ObjectType25||
                m_moveObject[i].type == ObjectType::ObjectType26 ||
                m_moveObject[i].type == ObjectType::ObjectType28 ||
                m_moveObject[i].type == ObjectType::ObjectType30 ||
                m_moveObject[i].type == ObjectType::ObjectType32 ||
                m_moveObject[i].type == ObjectType::ObjectType33 ||
                m_moveObject[i].type == ObjectType::ObjectType34 ||
                m_moveObject[i].type == ObjectType::ObjectType40 ||
                m_moveObject[i].type == ObjectType::ObjectType44 ||
                m_moveObject[i].type == ObjectType::ObjectType46 ||
                m_moveObject[i].type == ObjectType::ObjectType52 ||
                m_moveObject[i].type == ObjectType::ObjectType54 ||
                m_moveObject[i].type == ObjectType::ObjectType200 ||
                m_moveObject[i].type == ObjectType::ObjectType201 ||
                m_moveObject[i].type == ObjectType::ObjectType202 ||
                m_moveObject[i].type == ObjectType::ObjectType203
                )
            {
                src2.Left = m_moveObject[i].posCurrent.X;
                src2.Right = m_moveObject[i].posCurrent.X + 60;
                src2.Top = m_moveObject[i].posCurrent.Y;
                src2.Bottom = m_moveObject[i].posCurrent.Y + 60;
                TinyRect dst;
                if (Misc::IntersectRect(dst, src2, src))
                {
                    if (m_moveObject[i].type == ObjectType::ObjectType12)
                    {
                        SearchLinkCaisse(i, true);
                        for (int l = 0; l < m_nbLinkCaisse; l++)
                        {
                            PixmapChannel channel = m_moveObject[m_linkCaisse[l]].channel;
                            int icon2 = m_moveObject[m_linkCaisse[l]].icon;
                            TinyPoint posCurrent = m_moveObject[m_linkCaisse[l]].posCurrent;
                            double num = m_random.get()->Next(7, 23);
                            if (m_random.get()->Next(0, 100) % 2 == 0)
                            {
                                num = 0.0 - num;
                            }
                            ByeByeAdd(channel, icon2, posCurrent, num, 1.0);
                            m_moveObject[m_linkCaisse[l]].type = ObjectType::ObjectType0;
                        }
                        ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
                        UpdateCaisse();
                    }
                    else
                    {
                        ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
                    }
                }
            }
        }
        if (m_blupiFocus && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiPos.X > posStart.X - 30 &&
            m_blupiPos.X < posStart.X + 30 + 64 && m_blupiPos.Y > posStart.Y - 30 && m_blupiPos.Y < posStart.Y + 30 +
            64)
        {
            BlupiDead(BlupiAction::Clear1, {});
            m_blupiAir = true;
        }
    }

    /**
     * @note Returns -1 immediately during the no-lift cooldown (m_blupiTimeNoAsc). It tests
     *       only the thin top strip of each lift object (1/47/48) against Blupi's box. To avoid
     *       tunnelling when Blupi falls fast, the query box is swept in 30px increments along
     *       his vertical movement (oldpos->newpos) and each intermediate position is tested, so
     *       a high-speed descent still lands on the lift instead of passing through it.
     */
    int Decor::AscenseurDetect(TinyRect rect, TinyPoint oldpos, TinyPoint newpos)
    {
        if (m_blupiTimeNoAsc != 0)
        {
            return -1;
        }
        int num = newpos.Y - oldpos.Y;
        int num2 = ((num >= 0) ? 30 : (-30));
        num = std::abs(num);
        TinyRect src = TinyRect();
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (
                m_moveObject[i].type != ObjectType::ObjectType1 &&
                m_moveObject[i].type != ObjectType::ObjectType47 &&
                m_moveObject[i].type != ObjectType::ObjectType48)
            {
                continue;
            }
            src.Left = m_moveObject[i].posCurrent.X;
            src.Right = m_moveObject[i].posCurrent.X + 64;
            src.Top = m_moveObject[i].posCurrent.Y;
            src.Bottom = m_moveObject[i].posCurrent.Y + 16;
            TinyRect dst;
            if (num < 30)
            {
                if (Misc::IntersectRect(dst, src, rect))
                {
                    return i;
                }
                continue;
            }
            TinyRect src2 = rect;
            src2.Top -= num / 30 * num2;
            src2.Bottom -= num / 30 * num2;
            for (int j = 0; j <= num / 30; j++)
            {
                if (Misc::IntersectRect(dst, src, src2))
                {
                    return i;
                }
                src2.Top += num2;
                src2.Bottom += num2;
            }
        }
        return -1;
    }

    /**
     * @note Reports vertigo when Blupi's box hangs off the left or right edge of lift @p i so
     *       the appropriate teetering animation can play. For the wide multi-segment lifts
     *       (AscenseurShift true) it swaps the side and starts a short no-lift cooldown
     *       (m_blupiTimeNoAsc = 10) so Blupi slides off the edge rather than balancing forever.
     */
    void Decor::AscenseurVertigo(int i, bool& bVertigoLeft, bool& bVertigoRight)
    {
        bVertigoLeft = false;
        bVertigoRight = false;
        if (m_blupiPos.X + 20 + 4 < m_moveObject[i].posCurrent.X)
        {
            bVertigoLeft = true;
        }
        if (m_blupiPos.X + 60 - 20 - 4 > m_moveObject[i].posCurrent.X + 64)
        {
            bVertigoRight = true;
        }
        if (AscenseurShift(i))
        {
            if (bVertigoLeft)
            {
                bVertigoLeft = false;
                bVertigoRight = true;
                m_blupiTimeNoAsc = 10;
            }
            else if (bVertigoRight)
            {
                bVertigoRight = false;
                bVertigoLeft = true;
                m_blupiTimeNoAsc = 10;
            }
        }
    }

    /**
     * @note "Shiftable" simply means the lift sprite is one of the wide horizontal-platform
     *       icons (311..316); those lifts let Blupi slide off the ends, narrow lifts do not.
     */
    bool Decor::AscenseurShift(int i)
    {
        if (i == -1)
        {
            return false;
        }
        if (m_moveObject[i].icon >= 311)
        {
            return m_moveObject[i].icon <= 316;
        }
        return false;
    }

    /**
     * @note The loop counter reuses (and immediately overwrites) the @p i parameter, so the
     *       passed index is ignored and EVERY object in the pool is rewound to its start state
     *       (posCurrent=posStart, step=1, time=0, phase=0). In practice this re-synchronises all
     *       lifts to a common cycle origin by resetting the whole pool's motion timers.
     * @warning @p i is not used as a selector despite its name; do not assume per-object scope.
     */
    void Decor::AscenseurSynchro(int i)
    {
        for (i = 0; i < MAXMOVEOBJECT; i++)
        {
            m_moveObject[i].posCurrent = m_moveObject[i].posStart;
            m_moveObject[i].step = 1;
            m_moveObject[i].time = 0;
            m_moveObject[i].phase = 0;
        }
    }

    void Decor::UpdateCaisse()
    {
        m_nbRankCaisse = 0;
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType12)
            {
                m_rankCaisse[m_nbRankCaisse++] = i;
            }
        }
    }

    /**
     * @note Two-pass group push. It first collects the linked crate group (SearchLinkCaisse),
     *       then tests every member with TestPushOneCaisse against the desired horizontal move;
     *       if any member is obstructed the whole push fails and nothing moves. Only when all
     *       members are clear does the second pass commit the move to each crate's
     *       posCurrent/posStart/posEnd, so a crate stack always moves atomically.
     */
    bool Decor::TestPushCaisse(int i, TinyPoint pos, bool bPop)
    {
        TinyPoint move;
        move.X = pos.X - m_moveObject[i].posCurrent.X;
        move.Y = 0;
        SearchLinkCaisse(i, bPop);
        int y = m_moveObject[i].posCurrent.Y;
        for (int j = 0; j < m_nbLinkCaisse; j++)
        {
            i = m_linkCaisse[j];
            if (!TestPushOneCaisse(i, move, y))
            {
                return false;
            }
        }
        for (int j = 0; j < m_nbLinkCaisse; j++)
        {
            i = m_linkCaisse[j];
            m_moveObject[i].posCurrent.X += move.X;
            m_moveObject[i].posStart.X += move.X;
            m_moveObject[i].posEnd.X += move.X;
        }
        return true;
    }

    /**
     * @note Tests one crate's destination. It first rejects the move if the shifted crate box
     *       would collide with solid geometry (DecorDetect with bCaisse=false, so other crates
     *       are ignored — they are handled as a group elsewhere). For a crate on the floor row
     *       (@p b is the reference Y), it additionally requires support under BOTH its lower
     *       corners so a pushed ground crate cannot be shoved out over a gap; crates above the
     *       floor row skip the support test.
     */
    bool Decor::TestPushOneCaisse(int i, TinyPoint move, int b)
    {
        TinyRect rect = TinyRect();
        int num = (rect.Left = m_moveObject[i].posCurrent.X + move.X);
        rect.Right = num + 64;
        rect.Top = m_moveObject[i].posCurrent.Y;
        rect.Bottom = m_moveObject[i].posCurrent.Y + 64;
        if (DecorDetect(rect, false))
        {
            return false;
        }
        if (m_moveObject[i].posCurrent.Y != b)
        {
            return true;
        }
        rect.Left = num;
        rect.Right = num + 20;
        rect.Top = m_moveObject[i].posCurrent.Y + 64;
        rect.Bottom = m_moveObject[i].posCurrent.Y + 64 + 2;
        if (!DecorDetect(rect))
        {
            return false;
        }
        rect.Left = num + 64 - 20;
        rect.Right = num + 64;
        rect.Top = m_moveObject[i].posCurrent.Y + 64;
        rect.Bottom = m_moveObject[i].posCurrent.Y + 64 + 2;
        if (!DecorDetect(rect))
        {
            return false;
        }
        return true;
    }

    /**
     * @note Flood-fills the set of crates that must move together starting from @p rank. It
     *       repeatedly scans the crate list, growing m_linkCaisse with any crate whose
     *       (1px-inflated) box touches an already-linked crate, until a full pass adds nothing
     *       (the @c flag fixed-point). Only crates at or above the seed's row are considered, so
     *       you push a stack but not the floor it rests on. @p bPop additionally restricts the
     *       link to crates within +/-32px horizontally of the seed (used when a stack pops/falls
     *       rather than being pushed sideways).
     */
    void Decor::SearchLinkCaisse(int rank, bool bPop)
    {
        m_nbLinkCaisse = 0;
        AddLinkCaisse(rank);
        TinyPoint posCurrent = m_moveObject[rank].posCurrent;
        bool flag;
        TinyRect src = TinyRect();
        TinyRect src2 = TinyRect();
        do
        {
            flag = false;
            for (int i = 0; i < m_nbLinkCaisse; i++)
            {
                int num = m_linkCaisse[i];
                if (m_moveObject[num].posCurrent.Y > posCurrent.Y || (bPop && (m_moveObject[num].posCurrent.X <
                    posCurrent.X - 32 || m_moveObject[num].posCurrent.X > posCurrent.X + 32)))
                {
                    continue;
                }
                src.Left = m_moveObject[num].posCurrent.X - 1;
                src.Top = m_moveObject[num].posCurrent.Y - 1;
                src.Right = src.Left + 64 + 1;
                src.Bottom = src.Top + 64 + 1;
                for (int j = 0; j < m_nbRankCaisse; j++)
                {
                    int num2 = m_rankCaisse[j];
                    if (num2 != num && m_moveObject[num2].posCurrent.Y <= posCurrent.Y && (!bPop || (m_moveObject[num2].
                        posCurrent.X >= posCurrent.X - 32 && m_moveObject[num2].posCurrent.X <= posCurrent.X + 32)))
                    {
                        src2.Left = m_moveObject[num2].posCurrent.X - 1;
                        src2.Top = m_moveObject[num2].posCurrent.Y - 1;
                        src2.Right = src2.Left + 64 + 1;
                        src2.Bottom = src2.Top + 64 + 1;
                        TinyRect dst;
                        if (Misc::IntersectRect(dst, src2, src) && AddLinkCaisse(num2))
                        {
                            flag = true;
                        }
                    }
                }
            }
        }
        while (flag);
    }

    bool Decor::AddLinkCaisse(int rank)
    {
        for (int i = 0; i < m_nbLinkCaisse; i++)
        {
            if (m_linkCaisse[i] == rank)
            {
                return false;
            }
        }
        m_linkCaisse[m_nbLinkCaisse] = rank;
        m_nbLinkCaisse++;
        return true;
    }

    int Decor::CaisseInFront()
    {
        TinyPoint tinyPoint;
        if (m_blupiDir == Direction::Left)
        {
            tinyPoint.X = m_blupiPos.X + 16 - 32;
            tinyPoint.Y = m_blupiPos.Y;
        }
        else
        {
            tinyPoint.X = m_blupiPos.X + 60 - 16 + 32;
            tinyPoint.Y = m_blupiPos.Y;
        }
        for (int i = 0; i < m_nbRankCaisse; i++)
        {
            int num = m_rankCaisse[i];
            if (tinyPoint.X > m_moveObject[num].posCurrent.X && tinyPoint.X < m_moveObject[num].posCurrent.X + 64 &&
                tinyPoint.Y > m_moveObject[num].posCurrent.Y && tinyPoint.Y < m_moveObject[num].posCurrent.Y + 64)
            {
                return num;
            }
        }
        return -1;
    }

    /**
     * @note Scales the allowed push distance by load and state: heavier stacks (more linked
     *       crates) move slower, the Power bonus doubles the distance, and during the first
     *       ScaleTime(20) frames of the push the speed ramps up from near zero so pushing
     *       starts gently. The result is clamped to at least 1px so a push always makes progress.
     */
    int Decor::CaisseGetMove(int max)
    {
        max -= (m_nbLinkCaisse - 1) / 2;
        if (max < 1)
        {
            max = 1;
        }
        if (m_blupiPower)
        {
            max *= 2;
        }
        if (m_blupiPhase < Config::ScaleTime(20))
        {
            max = max * m_blupiPhase / Config::ScaleTime(20);
            if (max == 0)
            {
                max++;
            }
        }
        return max;
    }

    /**
     * @note Decides whether a nearby enemy should taunt Blupi (and which taunt icon to use).
     *       It returns 0 (no taunt) while a taunt is on cooldown. When Blupi is airborne over
     *       lava/spikes (icons 68/317 in the two cells below) it returns icon 64. Otherwise it
     *       looks for a taunting enemy type overlapping Blupi's box (extended downward when he is
     *       in the air) and returns a taunt icon (63/64/83) chosen from the enemy's side relative
     *       to Blupi's facing direction; a thrown-object enemy (type 2) approaching head-on
     *       returns 0 so no taunt plays.
     */
    int Decor::MockeryDetect(TinyPoint pos)
    {
        if (m_blupiTimeMockery > 0)
        {
            return 0;
        }
        if (m_blupiAir)
        {
            TinyPoint tinyPoint;
            tinyPoint.X = pos.X + 30;
            tinyPoint.Y = pos.Y + 30 + 64;
            if (tinyPoint.X >= 0 && tinyPoint.X < 6400 && tinyPoint.Y >= 0 && tinyPoint.Y < 6400)
            {
                int icon = m_decor[tinyPoint.X / 64][tinyPoint.Y / 64].icon;
                if (icon == 68 || icon == 317)
                {
                    return 64;
                }
            }
            tinyPoint.Y += 64;
            if (tinyPoint.X >= 0 && tinyPoint.X < 6400 && tinyPoint.Y >= 0 && tinyPoint.Y < 6400)
            {
                int icon = m_decor[tinyPoint.X / 64][tinyPoint.Y / 64].icon;
                if (icon == 68 || icon == 317)
                {
                    return 64;
                }
            }
        }
        TinyRect src = TinyRect();
        src.Left = pos.X;
        src.Right = pos.X + 60;
        src.Top = pos.Y + 11;
        src.Bottom = pos.Y + 60;
        if (m_blupiAir)
        {
            src.Bottom += 90;
        }
        TinyRect src2 = TinyRect();
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type != ObjectType::ObjectType2 && m_moveObject[i].type != ObjectType::ObjectType16 && m_moveObject[i].type != ObjectType::ObjectType96 && m_moveObject[i]
                .type != ObjectType::ObjectType97 && m_moveObject[i].type != ObjectType::ObjectType4 && m_moveObject[i].type != ObjectType::ObjectType20 && m_moveObject[i].type != ObjectType::ObjectType44 &&
                m_moveObject[i].type != ObjectType::ObjectType54 && m_moveObject[i].type != ObjectType::ObjectType23 && m_moveObject[i].type != ObjectType::ObjectType32 && m_moveObject[
                    i].type != ObjectType::ObjectType33)
            {
                continue;
            }
            src2.Left = m_moveObject[i].posCurrent.X;
            src2.Right = m_moveObject[i].posCurrent.X + 60;
            src2.Top = m_moveObject[i].posCurrent.Y + 36;
            src2.Bottom = m_moveObject[i].posCurrent.Y + 60;
            TinyRect dst;
            if (!Misc::IntersectRect(dst, src2, src))
            {
                continue;
            }
            if (m_moveObject[i].type == ObjectType::ObjectType54)
            {
                return 83;
            }
            if (m_blupiDir == Direction::Right)
            {
                if (pos.X < src2.Left)
                {
                    if (m_moveObject[i].type == ObjectType::ObjectType2)
                    {
                        return 0;
                    }
                    return 63;
                }
                return 64;
            }
            if (pos.X < src2.Left)
            {
                return 64;
            }
            if (m_moveObject[i].type == ObjectType::ObjectType2)
            {
                return 0;
            }
            return 63;
        }
        return 0;
    }

    /**
     * @note Only meaningful while the Cloud bonus is active (returns false otherwise, and
     *       always false in ghost mode): the cloud's lightning arcs to a nearby enemy. It tests
     *       the enemy box at @p pos against Blupi's box inflated by 40px on all sides, i.e. the
     *       arc reach. A true result tells the caller to electrocute/destroy that enemy.
     */
    bool Decor::BlupiElectro(TinyPoint pos)
    {
#ifdef MODERN
        if (m_blupiGhost)
        {
            return false;
        }
#endif
        if (!m_blupiCloud)
        {
            return false;
        }
        TinyRect src = TinyRect();
        src.Left = pos.X + 16;
        src.Right = pos.X + 60 - 16;
        src.Top = pos.Y + 11;
        src.Bottom = pos.Y + 60 - 2;
        TinyRect src2 = TinyRect();
        src2.Left = m_blupiPos.X - 16 - 40;
        src2.Right = m_blupiPos.X + 60 + 16 + 40;
        src2.Top = m_blupiPos.Y + 11 - 40;
        src2.Bottom = m_blupiPos.Y + 60 - 2 + 40;
        TinyRect dst;
        if (Misc::IntersectRect(dst, src, src2))
        {
            return true;
        }
        return false;
    }

    /**
     * @note Wakes dormant follower objects. Any idle follower (ObjectType96) whose 100px-padded
     *       detection box overlaps Blupi is promoted to the active homing follower (ObjectType97,
     *       handled by MoveObjectStepLine) and plays its alert sound. Hiding (or ghost mode)
     *       suppresses detection so followers stay asleep.
     */
    void Decor::MoveObjectFollow(TinyPoint pos)
    {
#ifdef MODERN
        if (m_blupiGhost)
        {
            return;
        }
#endif
        if (m_blupiHide)
        {
            return;
        }
        TinyRect src = BlupiRect(pos);
        src.Left = pos.X + 16;
        src.Right = pos.X + 60 - 16;
        TinyRect src2 = TinyRect();
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType96)
            {
                src2.Left = m_moveObject[i].posCurrent.X - 100;
                src2.Right = m_moveObject[i].posCurrent.X + 60 + 100;
                src2.Top = m_moveObject[i].posCurrent.Y - 100;
                src2.Bottom = m_moveObject[i].posCurrent.Y + 60 + 100;
                TinyRect dst;
                if (Misc::IntersectRect(dst, src2, src))
                {
                    m_moveObject[i].type = ObjectType::ObjectType97;
                    PlaySound(SoundChannel::SoundChannel92, m_moveObject[i].posCurrent);
                }
            }
        }
    }

    /**
     * @note The general "what is Blupi touching?" scan used for damage/pickup. It skips a set
     *       of non-interactive/effect types (and skips crates while pushing). The contact box of
     *       each candidate is type-tuned (e.g. type 3 hazards only hit from above unless Blupi is
     *       ducking, crates 12 widen the box on the approach side, bullets 23 use a tight core).
     *       It returns the first object overlapping Blupi's tight box with bNear=true; a thrown
     *       enemy (type 2) within the larger anticipation box (src2) returns with bNear=false so
     *       the caller can react before contact.
     */
    int Decor::MoveObjectDetect(TinyPoint pos, bool& bNear)
    {
#ifdef MODERN
        if (m_blupiGhost)
        {
            bNear = false;
            return -1;
        }
#endif
        TinyRect src = BlupiRect(pos);
        src.Left = pos.X + 16;
        src.Right = pos.X + 60 - 16;
        TinyRect src2 = TinyRect();
        src2.Left = src.Left - 20;
        src2.Right = src.Right + 20;
        src2.Top = src.Top - 40;
        src2.Bottom = src.Bottom + 30;
        TinyRect src3 = TinyRect();
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType0 || m_moveObject[i].type == ObjectType::ObjectType27 || m_moveObject[i].type == ObjectType::ObjectType57 || m_moveObject[i]
                .type == ObjectType::ObjectType39 || m_moveObject[i].type == ObjectType::ObjectType58 || m_moveObject[i].type == ObjectType::ObjectType34 || m_moveObject[i].type == ObjectType::ObjectType37 ||
                m_moveObject[i].type == ObjectType::ObjectType38 || ((m_blupiAction == BlupiAction::Push || m_blupiAction == BlupiAction::Pop) && m_moveObject[i].type ==
                    ObjectType::ObjectType12))
            {
                continue;
            }
            src3.Left = m_moveObject[i].posCurrent.X + 16;
            src3.Right = m_moveObject[i].posCurrent.X + 60 - 16;
            src3.Top = m_moveObject[i].posCurrent.Y + 36;
            src3.Bottom = m_moveObject[i].posCurrent.Y + 60;
            if (m_moveObject[i].type == ObjectType::ObjectType3)
            {
                if (m_blupiAction == BlupiAction::Down)
                {
                    continue;
                }
                src3.Top = m_moveObject[i].posCurrent.Y;
                src3.Bottom = m_moveObject[i].posCurrent.Y + 60 - 36;
            }
            if (m_moveObject[i].type == ObjectType::ObjectType12)
            {
                src3.Left = m_moveObject[i].posCurrent.X - 16;
                src3.Right = m_moveObject[i].posCurrent.X + 64 + 16;
                src3.Top = m_moveObject[i].posCurrent.Y;
                src3.Bottom = m_moveObject[i].posCurrent.Y + 64;
                if (m_blupiDir == Direction::Left)
                {
                    src3.Left += 20;
                }
                else
                {
                    src3.Right -= 20;
                }
            }
            if (m_moveObject[i].type == ObjectType::ObjectType17 || m_moveObject[i].type == ObjectType::ObjectType20 || m_moveObject[i].type == ObjectType::ObjectType44 || m_moveObject[
                i].type == ObjectType::ObjectType54)
            {
                src3.Top = m_moveObject[i].posCurrent.Y + 16;
                src3.Bottom = m_moveObject[i].posCurrent.Y + 60 - 16;
            }
            if (m_moveObject[i].type == ObjectType::ObjectType23)
            {
                src3.Left = m_moveObject[i].posCurrent.X + 24;
                src3.Right = m_moveObject[i].posCurrent.X + 64 - 24;
                src3.Top = m_moveObject[i].posCurrent.Y + 10;
                src3.Bottom = m_moveObject[i].posCurrent.Y + 60 - 32;
            }
            TinyRect dst;
            if (Misc::IntersectRect(dst, src3, src))
            {
                bNear = true;
                return i;
            }
            if (m_moveObject[i].type == ObjectType::ObjectType2 && Misc::IntersectRect(dst, src3, src2))
            {
                bNear = false;
                return i;
            }
        }
        bNear = false;
        return -1;
    }

    int Decor::MoveAscenseurDetect(TinyPoint pos, int height)
    {
        if (m_blupiTimeNoAsc != 0)
        {
            return -1;
        }
        TinyRect src = TinyRect();
        src.Left = pos.X + 12;
        src.Right = pos.X + 60 - 12;
        src.Top = pos.Y + 60 - 2;
        src.Bottom = pos.Y + 60 + height - 1;
        TinyRect src2 = TinyRect();
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType1 || m_moveObject[i].type == ObjectType::ObjectType47 || m_moveObject[i].type == ObjectType::ObjectType48)
            {
                src2.Left = m_moveObject[i].posCurrent.X;
                src2.Right = m_moveObject[i].posCurrent.X + 64;
                src2.Top = m_moveObject[i].posCurrent.Y;
                src2.Bottom = m_moveObject[i].posCurrent.Y + 16;
                TinyRect dst;
                if (Misc::IntersectRect(dst, src2, src))
                {
                    return i;
                }
            }
        }
        return -1;
    }

    int Decor::MoveChargeDetect(TinyPoint pos)
    {
#ifdef MODERN
        if (m_blupiGhost)
        {
            return -1;
        }
#endif
        TinyRect src = TinyRect();
        src.Left = pos.X + 16;
        src.Right = pos.X + 60 - 16;
        src.Top = pos.Y + 11;
        src.Bottom = pos.Y + 60 - 2;
        TinyRect src2 = TinyRect();
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType31)
            {
                src2.Left = m_moveObject[i].posCurrent.X - 10;
                src2.Right = m_moveObject[i].posCurrent.X + 60 + 10;
                src2.Top = m_moveObject[i].posCurrent.Y + 36;
                src2.Bottom = m_moveObject[i].posCurrent.Y + 60;
                TinyRect dst;
                if (Misc::IntersectRect(dst, src2, src))
                {
                    return i;
                }
            }
        }
        return -1;
    }

    int Decor::MovePersoDetect(TinyPoint pos)
    {
#ifdef MODERN
        if (m_blupiGhost)
        {
            return -1;
        }
#endif
        TinyRect src = TinyRect();
        src.Left = pos.X + 16;
        src.Right = pos.X + 60 - 16;
        src.Top = pos.Y + 11;
        src.Bottom = pos.Y + 60 - 2;
        TinyRect src2 = TinyRect();
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type >= ObjectType::ObjectType200 && m_moveObject[i].type <= ObjectType::ObjectType203)
            {
                src2.Left = m_moveObject[i].posCurrent.X + 16;
                src2.Right = m_moveObject[i].posCurrent.X + 60 - 16;
                src2.Top = m_moveObject[i].posCurrent.Y + 36;
                src2.Bottom = m_moveObject[i].posCurrent.Y + 60;
                TinyRect dst;
                if (Misc::IntersectRect(dst, src2, src))
                {
                    return i;
                }
            }
        }
        return -1;
    }

    int Decor::MoveObjectDelete(TinyPoint cel)
    {
        int result = -1;
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type != ObjectType::ObjectType0)
            {
                if (cel.X == m_moveObject[i].posStart.X / 64 && cel.Y == m_moveObject[i].posStart.Y / 64)
                {
                    result = ToRaw(m_moveObject[i].type);
                    m_moveObject[i].type = ObjectType::ObjectType0;
                }
                else if (cel.X == m_moveObject[i].posEnd.X / 64 && cel.Y == m_moveObject[i].posEnd.Y / 64)
                {
                    result =ToRaw(m_moveObject[i].type);
                    m_moveObject[i].type = ObjectType::ObjectType0;
                }
            }
        }
        return result;
    }

    int Decor::MoveObjectFree()
    {
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType0)
            {
                // The self-assignment is a harmless no-op carried over from the original port.
                m_moveObject[i].type = ObjectType::ObjectType0;
                return i;
            }
        }
        return -1;
    }

    /**
     * @note Returns the layer bucket, not a fine-grained order: small enemies/effects (2/3/96/97)
     *       draw behind (1), crates (12) in the middle (2), everything else in front (3).
     *       MoveObjectSort() uses this as the sort key.
     */
    int Decor::SortGetType(ObjectType type)
    {
        switch (type)
        {
        case ObjectType::ObjectType2:
        case ObjectType::ObjectType3:
        case ObjectType::ObjectType96:
        case ObjectType::ObjectType97:
            return 1;
        case ObjectType::ObjectType12:
            return 2;
        default:
            return 3;
        }
    }

    /**
     * @note Two stages: first it compacts the pool so all active objects occupy the leading
     *       slots (free slots pushed to the tail), then it bubble-sorts the active prefix by
     *       SortGetType() so draw order matches layer priority. Finally it rebuilds the crate
     *       index (UpdateCaisse) and clears the link set, because the compaction/sort has
     *       invalidated the old pool indices stored in m_rankCaisse/m_linkCaisse.
     * @warning Reorders the pool, so any index into m_moveObject[] held across a call to this
     *          function becomes stale.
     */
    void Decor::MoveObjectSort()
    {
        MoveObject dst;
        int num = 0;
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type != ObjectType::ObjectType0)
            {
                MoveObjectCopy(m_moveObject[num++], m_moveObject[i]);
            }
        }
        for (int i = num; i < MAXMOVEOBJECT; i++)
        {
            m_moveObject[i].type = ObjectType::ObjectType0;
        }
        if (num <= 1)
        {
            return;
        }
        bool flag;
        do
        {
            flag = false;
            for (int i = 0; i < num - 1; i++)
            {
                if (SortGetType(m_moveObject[i].type) > SortGetType(m_moveObject[i + 1].type))
                {
                    MoveObjectCopy(dst, m_moveObject[i]);
                    MoveObjectCopy(m_moveObject[i], m_moveObject[i + 1]);
                    MoveObjectCopy(m_moveObject[i + 1], dst);
                    flag = true;
                }
            }
        }
        while (flag);
        UpdateCaisse();
        m_nbLinkCaisse = 0;
    }

    /**
     * @note Only acts on bullets (ObjectType23): it swaps the bullet at @p i with the first
     *       non-bullet slot at or before it, pulling bullets toward the front of the pool so
     *       they are stepped/drawn ahead of the objects they may hit. No-op for non-bullets or
     *       the slot 0 case. Rebuilds the crate index if a crate was involved in the swap.
     */
    void Decor::MoveObjectPriority(int i)
    {
        MoveObject dst;
        if (i == 0 || m_moveObject[i].type != ObjectType::ObjectType23)
        {
            return;
        }
        for (int j = 0; j < MAXMOVEOBJECT; j++)
        {
            if (m_moveObject[j].type == ObjectType::ObjectType23)
            {
                continue;
            }
            if (j <= i)
            {
                MoveObjectCopy(dst, m_moveObject[i]);
                MoveObjectCopy(m_moveObject[i], m_moveObject[j]);
                MoveObjectCopy(m_moveObject[j], dst);
                if (m_moveObject[i].type == ObjectType::ObjectType12 || m_moveObject[j].type == ObjectType::ObjectType12)
                {
                    UpdateCaisse();
                }
            }
            break;
        }
    }

    int Decor::MoveObjectSearch(TinyPoint pos)
    {
        return MoveObjectSearch(pos, {});
    }

    /**
     * @note Exact-position lookup (not box overlap). For most objects it matches posCurrent
     *       exactly against @p pos. Moving bullets (ObjectType23 with a non-degenerate path) are
     *       special-cased: they match anywhere within +/-100px along their travel axis (with the
     *       cross axis exact), so a bullet in flight can still be found at its origin cell.
     */
    int Decor::MoveObjectSearch(TinyPoint pos, System::Nullable<ObjectType> type)
    {
        for (int i = 0; i < MAXMOVEOBJECT; i++)
        {
            if (m_moveObject[i].type == ObjectType::ObjectType0 || (type.getHasValueProperty() && m_moveObject[i].type != type.getValueProperty()))
            {
                continue;
            }
            if (m_moveObject[i].type == ObjectType::ObjectType23 && m_moveObject[i].posStart.X != m_moveObject[i].posEnd.X)
            {
                if (m_moveObject[i].posCurrent.X >= pos.X - 100 && m_moveObject[i].posCurrent.X <= pos.X + 100 &&
                    m_moveObject[i].posCurrent.Y == pos.Y)
                {
                    return i;
                }
            }
            else if (m_moveObject[i].type == ObjectType::ObjectType23 && m_moveObject[i].posStart.Y != m_moveObject[i].posEnd.Y)
            {
                if (m_moveObject[i].posCurrent.Y >= pos.Y - 100 && m_moveObject[i].posCurrent.Y <= pos.Y + 100 &&
                    m_moveObject[i].posCurrent.X == pos.X)
                {
                    return i;
                }
            }
            else if (m_moveObject[i].posCurrent.X == pos.X && m_moveObject[i].posCurrent.Y == pos.Y)
            {
                return i;
            }
        }
        return -1;
    }

    void Decor::ByeByeHelico()
    {
        if (m_blupiHelico)
        {
            ByeByeAdd(PixmapChannel::Element, 68, m_blupiPos, 7.0, 0.5);
        }
    }

    void Decor::ByeByeAdd(PixmapChannel channel, int icon, TinyPoint pos, double rotationSpeed, double animationSpeed)
    {
        ByeByeObject byeByeObject;
        byeByeObject.channel = channel;
        byeByeObject.icon = icon;
        byeByeObject.posX = pos.X;
        byeByeObject.posY = pos.Y;
        byeByeObject.rotation = 0.0;
        byeByeObject.phase = 0.0;
        byeByeObject.rotationSpeed = rotationSpeed;
        byeByeObject.animationSpeed = animationSpeed;
        ByeByeObject byeByeObject2 = byeByeObject;
        int num = m_random.get()->Next(0, 10);
        if (m_random.get()->Next(0, 1000) % 2 == 0)
        {
            byeByeObject2.speedX = num + 10;
        }
        else
        {
            byeByeObject2.speedX = -(num + 10);
        }
        byeByeObjects.push_back(byeByeObject2);
    }

    /**
     * @note Advances each debris fragment with a ballistic arc: phase ramps 0..10 as a rising
     *       impulse (posY decreases by (10-phase)^1.5) then beyond 10 it falls (posY increases),
     *       giving a pop-up-then-drop motion. Horizontal speed decays toward zero each frame and
     *       rotation accumulates. Fragments are culled once phase exceeds ScaleTime(30). All
     *       deltas are multiplied by Config::SPEED_SCALE to preserve the 20 FPS feel.
     */
    void Decor::ByeByeStep()
    {
        int num = 0;
        while (num < byeByeObjects.size())
        {
            ByeByeObject& byeByeObject = byeByeObjects[num];
            double scaledSpeed = byeByeObject.animationSpeed * Config::SPEED_SCALE;
            double num2 = 10.0 - byeByeObject.phase;
            if (num2 > 0.0)
            {
                byeByeObject.posY -= std::pow(num2, 1.5) * scaledSpeed;
            }
            if (num2 < 0.0)
            {
                byeByeObject.posY += std::pow(0.0 - num2, 1.5) * scaledSpeed;
            }
            byeByeObject.posX += byeByeObject.speedX * scaledSpeed;
            if (byeByeObject.speedX > 0.0)
            {
                byeByeObject.speedX -= scaledSpeed;
            }
            if (byeByeObject.speedX < 0.0)
            {
                byeByeObject.speedX += scaledSpeed;
            }
            byeByeObject.rotation += byeByeObject.rotationSpeed * Config::SPEED_SCALE;
            byeByeObject.phase += scaledSpeed;
            if (byeByeObject.channel == PixmapChannel::Element && byeByeObject.icon >= 187 && byeByeObject.icon <= 194)
            {
                byeByeObject.icon = Tables::table_invert[(int)byeByeObject.phase / 2 % 20];
            }
            if (byeByeObject.phase > Config::ScaleTime(30))
            {
                byeByeObjects.erase(byeByeObjects.begin() + num);
            }
            else
            {
                num++;
            }
        }
    }

    void Decor::ByeByeDraw(TinyPoint posDecor)
    {
        for (ByeByeObject byeByeObject : byeByeObjects)
        {
            TinyPoint tinyPoint;
            tinyPoint.X = m_drawBounds.Left + (int)byeByeObject.posX - posDecor.X;
            tinyPoint.Y = m_drawBounds.Top + (int)byeByeObject.posY - posDecor.Y;
            TinyPoint pos = tinyPoint;
            m_pixmap->QuickIcon(byeByeObject.channel, byeByeObject.icon, pos, 1.0, byeByeObject.rotation);
        }
    }

    TinyPoint Decor::VoyageGetPosVie(int nbVies)
    {
        TinyPoint result;
        result.X = 210 + 16 * nbVies;
        result.Y = 417;
        return result;
    }

    /**
     * @note A "voyage" is the little fly-to-HUD animation a collected item plays. Only one runs
     *       at a time, so if one is already active it is force-completed first (set phase=total
     *       and VoyageStep()) which applies its reward before the new one starts. The travel
     *       duration is proportional to the Manhattan distance, with several (icon,channel)
     *       cases overriding the duration and/or playing a pickup sound up front. The actual
     *       reward (life, treasure, key, dynamite, follower) is granted at completion in
     *       VoyageStep(), not here.
     */
    void Decor::VoyageInit(TinyPoint start, TinyPoint end, int icon, PixmapChannel channel)
    {
        if (m_voyageIcon != -1)
        {
            m_voyagePhase = m_voyageTotal;
            VoyageStep();
        }
        m_voyageStart = start;
        m_voyageEnd = end;
        m_voyageIcon = icon;
        m_voyageChannel = channel;
        int num = std::abs(end.X - start.X);
        int num2 = std::abs(end.Y - start.Y);
        m_voyagePhase = 0;
        m_voyageTotal = Config::ScaleTime((num + num2) / 10);
        if (m_voyageIcon == 48 && m_voyageChannel == PixmapChannel::Blupi)
        {
            m_voyageTotal = Config::ScaleTime(40);
            m_nbVies--;
            m_sound->PlayImage(SoundChannel::SoundChannel9, end, -1, false);
        }
        if (m_voyageIcon == 21 && m_voyageChannel == PixmapChannel::Element)
        {
            m_sound->PlayImage(SoundChannel::SoundChannel12, start, -1, false);
        }
        if (m_voyageIcon == 6 && m_voyageChannel == PixmapChannel::Element)
        {
            if (m_nbTresor == m_totalTresor - 1)
            {
                m_sound->PlayImage(SoundChannel::SoundChannel19, start, -1, false);
            }
            else
            {
                m_sound->PlayImage(SoundChannel::SoundChannel11, start, -1, false);
            }
        }
        if (m_voyageIcon == 215 && m_voyageChannel == PixmapChannel::Element)
        {
            m_sound->PlayImage(SoundChannel::SoundChannel11, start, -1, false);
        }
        if (m_voyageIcon == 222 && m_voyageChannel == PixmapChannel::Element)
        {
            m_sound->PlayImage(SoundChannel::SoundChannel11, start, -1, false);
        }
        if (m_voyageIcon == 229 && m_voyageChannel == PixmapChannel::Element)
        {
            m_sound->PlayImage(SoundChannel::SoundChannel11, start, -1, false);
        }
        if (m_voyageIcon == 108 && m_voyageChannel == PixmapChannel::Button)
        {
            m_sound->PlayImage(SoundChannel::SoundChannel60, start, -1, false);
        }
        if (m_voyageIcon == 252 && m_voyageChannel == PixmapChannel::Element)
        {
            m_sound->PlayImage(SoundChannel::SoundChannel60, start, -1, false);
        }
        if (m_voyageIcon == 177 && m_voyageChannel == PixmapChannel::Element)
        {
            m_sound->PlayImage(SoundChannel::SoundChannel54, start, -1, false);
        }
        if (m_voyageIcon == 230 && m_voyageChannel == PixmapChannel::Element)
        {
            m_voyageTotal = Config::ScaleTime(100);
        }
        if (m_voyageIcon == 40 && m_voyageChannel == PixmapChannel::Element)
        {
            m_voyageTotal = Config::ScaleTime(50);
        }
    }

    /**
     * @note While the arc is in flight it only animates (cycling icon 230..241). On the frame it
     *       completes it APPLIES the reward keyed on (icon,channel): grant a life (capped at
     *       MAX_EGG_COUNT), increment treasure (and re-evaluate treasure-gated doors), set a key
     *       bit, add a follower or dynamite, or hand control back to Blupi after the angel ascent;
     *       a pickup confirmation sound is played, and the voyage is cleared (icon = -1).
     * @note This is where collectible pickups actually mutate game state, so the reward fires
     *       exactly once, at arc end.
     */
    void Decor::VoyageStep()
    {
        if (m_voyageIcon == -1)
        {
            return;
        }
        if (m_voyagePhase < m_voyageTotal)
        {
            if (m_time % Config::ScaleTime(2) == 0 && m_voyageIcon >= 230 && m_voyageIcon <= 241 && m_voyageChannel == PixmapChannel::Element)
            {
                m_voyageIcon++;
                if (m_voyageIcon > 241)
                {
                    m_voyageIcon = 230;
                }
            }
        }
        else
        {
            if (m_voyageIcon == 48 && m_voyageChannel == PixmapChannel::Blupi)
            {
                m_blupiAction = BlupiAction::Stop;
                m_blupiPhase = 0;
                m_blupiFocus = true;
            }
            if (m_voyageIcon == 21 && m_voyageChannel == PixmapChannel::Element)
            {
                if (m_nbVies < MAX_EGG_COUNT)
                {
                    m_nbVies++;
                }
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            if (m_voyageIcon == 6 && m_voyageChannel == PixmapChannel::Element)
            {
                m_nbTresor++;
                OpenDoorsTresor();
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            if (m_voyageIcon == 215 && m_voyageChannel == PixmapChannel::Element)
            {
                m_blupiCle = m_blupiCle | DoorKeyFlags::Key1;
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            if (m_voyageIcon == 222 && m_voyageChannel == PixmapChannel::Element)
            {
                m_blupiCle = m_blupiCle | DoorKeyFlags::Key2;
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            if (m_voyageIcon == 229 && m_voyageChannel == PixmapChannel::Element)
            {
                m_blupiCle = m_blupiCle | DoorKeyFlags::Key3;
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            if (m_voyageIcon == 108 && m_voyageChannel == PixmapChannel::Button)
            {
                m_blupiPerso++;
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            if (m_voyageIcon == 252 && m_voyageChannel == PixmapChannel::Element)
            {
                m_blupiDynamite++;
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            if (m_voyageIcon == 177 && m_voyageChannel == PixmapChannel::Element)
            {
                m_sound->PlayImage(SoundChannel::SoundChannel3, m_voyageEnd, -1, false);
            }
            m_voyageIcon = -1;
        }
        m_voyagePhase++;
    }

    void Decor::VoyageDraw()
    {
        if (m_voyageIcon == -1 || m_voyageTotal == 0)
        {
            return;
        }
        int num = m_voyagePhase;
        if (m_voyageIcon == 40 && m_voyageChannel == PixmapChannel::Element)
        {
            num -= Config::ScaleTime(30);
            if (num < 0)
            {
                num = 0;
            }
        }
        TinyPoint pos;
        pos.X = m_voyageStart.X + (m_voyageEnd.X - m_voyageStart.X) * num / m_voyageTotal;
        pos.Y = m_voyageStart.Y + (m_voyageEnd.Y - m_voyageStart.Y) * num / m_voyageTotal;
        if (m_voyageIcon != 40 || m_voyageChannel != PixmapChannel::Element || num != 0)
        {
            m_pixmap->HudIcon(m_voyageChannel, m_voyageIcon, pos);
        }
        if (m_voyageIcon == 40 && m_voyageChannel == PixmapChannel::Element)
        {
            int array[7] = {-8, -6, -4, 0, 4, 6, 8};
            pos.X -= 34;
            pos.X += m_posDecor.X;
            pos.Y += m_posDecor.Y;
            int num2 = array[m_random.get()->Next(0, 6)];
            int num3 = m_random.get()->Next(-10, 10);
            if (num == 0)
            {
                num2 /= 2;
                num3 *= 4;
            }
            pos.Y += num3;
            ObjectStart(pos, ObjectType::ObjectType93, num2);
        }
    }

    bool Decor::IsFloatingObject(int i)
    {
        TinyPoint posCurrent = m_moveObject[i].posCurrent;
        int num = (posCurrent.X + 32) / 64;
        int num2 = posCurrent.Y / 64 + 1;
        int icon = m_decor[num][num2].icon;
        return IsPassIcon(icon);
    }

    /**
     * @note Auto-tiling neighbour predicate: answers "does the cell at (x+dx, y+dy) form a
     *       border edge for the block at (x,y) coming from direction (dx,dy)?" Off-map cells
     *       count as a border (true). The large nested switch classifies hundreds of tile icons
     *       into "same material" / "different" decisions, with several icons (water 92, lava 68,
     *       spikes 317, slopes, etc.) treated as a border only from specific directions so edges
     *       and slopes connect correctly. Used by AdaptMidBorder() to build its neighbour mask.
     */
    bool Decor::IsRightBorder(int x, int y, int dx, int dy)
    {
        int num = ((m_dimDecor.X != 0) ? 100 : 10);
        int num2 = ((m_dimDecor.Y != 0) ? 100 : 8);
        if (x < 0 || x >= num || y < 0 || y >= num2)
        {
            return true;
        }
        int icon = m_decor[x + dx][y + dy].icon;
        if (icon < 386 || icon > 397)
        {
            switch (icon)
            {
            case 245:
            case 400:
                break;
            case 250:
            case 251:
            case 252:
            case 253:
            case 254:
            case 255:
            case 256:
            case 257:
            case 258:
            case 259:
            case 260:
                icon = m_decor[x][y].icon;
                if (icon >= 250)
                {
                    return icon <= 260;
                }
                return false;
            default:
                icon = m_decor[x][y].icon;
                switch (icon)
                {
                case -1:
                    return false;
                case 32:
                case 33:
                case 34:
                    return false;
                case 68:
                case 92:
                    if (dy == -1)
                    {
                        return false;
                    }
                    return true;
                case 317:
                    if (dy == 1)
                    {
                        return false;
                    }
                    return true;
                case 19:
                case 21:
                case 25:
                case 26:
                case 28:
                    if (dy == 1)
                    {
                        return false;
                    }
                    return true;
                case 27:
                    if (dx != 0)
                    {
                        return false;
                    }
                    return true;
                case 45:
                case 46:
                case 47:
                    if (dy == 1)
                    {
                        return false;
                    }
                    return true;
                default:
                    switch (icon)
                    {
                    case 15:
                    case 16:
                    case 75:
                    case 89:
                    case 90:
                    case 155:
                        if (dx == -1 || dy == 1)
                        {
                            return false;
                        }
                        return true;
                    case 17:
                    case 18:
                    case 74:
                    case 87:
                    case 88:
                    case 154:
                        if (dx == 1 || dy == 1)
                        {
                            return false;
                        }
                        return true;
                    case 76:
                    case 77:
                    case 199:
                    case 200:
                        if (dx != 0)
                        {
                            return false;
                        }
                        return true;
                    case 110:
                    case 111:
                    case 112:
                    case 113:
                    case 114:
                    case 115:
                    case 116:
                    case 117:
                    case 118:
                    case 119:
                    case 120:
                    case 121:
                    case 122:
                    case 123:
                    case 124:
                    case 125:
                        return false;
                    default:
                        switch (icon)
                        {
                        case 126:
                            if (dx == 1)
                            {
                                return true;
                            }
                            return false;
                        case 129:
                            if (dx == -1)
                            {
                                return true;
                            }
                            return false;
                        case 132:
                            if (dy == 1)
                            {
                                return true;
                            }
                            return false;
                        case 135:
                            if (dy == -1)
                            {
                                return true;
                            }
                            return false;
                        case 139:
                        case 140:
                        case 141:
                        case 142:
                        case 143:
                            return false;
                        default:
                            switch (icon)
                            {
                            case 138:
                                if (dy == -1)
                                {
                                    return true;
                                }
                                return false;
                            case 202:
                                return false;
                            case 107:
                            case 108:
                            case 109:
                            case 157:
                                if (dx != 0)
                                {
                                    return true;
                                }
                                return false;
                            default:
                                switch (icon)
                                {
                                case 309:
                                case 310:
                                case 410:
                                case 411:
                                case 412:
                                case 413:
                                case 414:
                                case 415:
                                case 416:
                                case 417:
                                case 418:
                                case 419:
                                case 420:
                                    break;
                                default:
                                    switch (icon)
                                    {
                                    case 182:
                                    case 183:
                                        if (dy != 0)
                                        {
                                            return true;
                                        }
                                        return false;
                                    case 334:
                                    case 335:
                                    case 336:
                                        if (dy != 0)
                                        {
                                            return true;
                                        }
                                        return false;
                                    default:
                                        if (icon >= 250 && icon <= 260)
                                        {
                                            return false;
                                        }
                                        if (icon >= 264 && icon <= 282)
                                        {
                                            return false;
                                        }
                                        switch (icon)
                                        {
                                        case 378:
                                            return false;
                                        case 404:
                                        case 410:
                                            return false;
                                        case 421:
                                        case 422:
                                        case 423:
                                        case 424:
                                        case 425:
                                        case 426:
                                        case 427:
                                        case 428:
                                        case 429:
                                        case 430:
                                        case 431:
                                        case 432:
                                        case 433:
                                        case 434:
                                        case 435:
                                        case 436:
                                        case 437:
                                        case 438:
                                        case 439:
                                        case 440:
                                            if (dy != 0)
                                            {
                                                return true;
                                            }
                                            return false;
                                        default:
                                            return true;
                                        }
                                    }
                                }
                                break;
                            case 158:
                            case 159:
                            case 160:
                            case 161:
                            case 162:
                            case 163:
                            case 164:
                            case 165:
                            case 166:
                            case 167:
                            case 168:
                            case 169:
                            case 170:
                            case 171:
                            case 172:
                            case 173:
                            case 174:
                            case 175:
                            case 176:
                            case 177:
                            case 178:
                            case 179:
                            case 180:
                            case 181:
                                break;
                            }
                            if (dy == 1)
                            {
                                return true;
                            }
                            return false;
                        }
                    }
                }
            }
        }
        icon = m_decor[x][y].icon;
        if ((icon < 386 || icon > 397) && icon != 400)
        {
            return icon == 245;
        }
        return true;
    }

    bool Decor::IsFromage(int x, int y)
    {
        if (x < 0 || x >= 100 || y < 0 || y >= 100)
        {
            return false;
        }
        int icon = m_decor[x][y].icon;
        if (icon >= 246 && icon <= 249)
        {
            return true;
        }
        if (icon == 339)
        {
            return true;
        }
        return false;
    }

    bool Decor::IsGrotte(int x, int y)
    {
        if (x < 0 || x >= 100 || y < 0 || y >= 100)
        {
            return false;
        }
        switch (m_decor[x][y].icon)
        {
        case 284:
        case 301:
            return true;
        case 337:
            return true;
        default:
            return false;
        }
    }

    /**
     * @note Auto-tiling for the generic terrain blocks. It builds a 4-bit neighbour mask (bit0
     *       below, bit1 above, bit2 right, bit3 left) by clearing a bit for each side that is NOT
     *       a matching border (IsRightBorder), then collapses the current icon to its block's
     *       base variant, finds that base in Tables::table_adapt_decor and reads the icon for the
     *       computed mask from the same 16-wide table row. A few result icons are randomly
     *       swapped for cosmetic variants so identical terrain does not look perfectly uniform.
     */
    void Decor::AdaptMidBorder(int x, int y)
    {
        if (x < 0 || x >= 100 || y < 0 || y >= 100)
        {
            return;
        }
        int num = 15;
        if (!IsRightBorder(x, y + 1, 0, -1))
        {
            num &= -2;
        }
        if (!IsRightBorder(x, y - 1, 0, 1))
        {
            num &= -3;
        }
        if (!IsRightBorder(x + 1, y, -1, 0))
        {
            num &= -5;
        }
        if (!IsRightBorder(x - 1, y, 1, 0))
        {
            num &= -9;
        }
        int num2 = m_decor[x][y].icon;
        if (num2 == 156)
        {
            num2 = 35;
        }
        if (num2 == 252 || num2 == 253)
        {
            num2 = 251;
        }
        if (num2 == 255)
        {
            num2 = 254;
        }
        if (num2 == 362)
        {
            num2 = 347;
        }
        if (num2 == 363)
        {
            num2 = 348;
        }
        if (num2 >= 341 && num2 <= 346)
        {
            num2 = 341;
        }
        for (int i = 0; i < 144; i++)
        {
            if (num2 == Tables::table_adapt_decor[i])
            {
                num2 = Tables::table_adapt_decor[i / 16 * 16 + num];
                if (num2 == 35 && m_random.get()->Next() % 2 == 0)
                {
                    num2 = 156;
                }
                if (num2 == 251)
                {
                    num2 = m_random.get()->Next(251, 253);
                }
                if (num2 == 254 && m_random.get()->Next() % 2 == 0)
                {
                    num2 = 255;
                }
                if (num2 == 347 && m_random.get()->Next() % 2 == 0)
                {
                    num2 = 362;
                }
                if (num2 == 348 && m_random.get()->Next() % 2 == 0)
                {
                    num2 = 363;
                }
                if (num2 == 341)
                {
                    num2 = m_random.get()->Next(341, 346);
                }
                m_decor[x][y].icon = num2;
                return;
            }
        }
        switch (m_decor[x][y].icon)
        {
        case -1:
        case 264:
        case 265:
        case 266:
        case 267:
        case 268:
        case 269:
        case 270:
        case 271:
        case 272:
        case 273:
        case 274:
        case 275:
        case 276:
        case 277:
        case 278:
        case 279:
        case 280:
        case 281:
        case 282:
            num = 15;
            if (!IsFromage(x, y + 1))
            {
                num &= -2;
            }
            if (!IsFromage(x, y - 1))
            {
                num &= -3;
            }
            if (!IsFromage(x + 1, y))
            {
                num &= -5;
            }
            if (!IsFromage(x - 1, y))
            {
                num &= -9;
            }
            num2 = Tables::table_adapt_fromage[num];
            if (num2 == 268 && m_random.get()->Next() % 2 == 0)
            {
                num2 = 279;
            }
            if (num2 == 269 && m_random.get()->Next() % 2 == 0)
            {
                num2 = 280;
            }
            if (num2 == 264 && m_random.get()->Next() % 2 == 0)
            {
                num2 = 281;
            }
            if (num2 == 265 && m_random.get()->Next() % 2 == 0)
            {
                num2 = 282;
            }
            m_decor[x][y].icon = num2;
            break;
        }
        switch (m_decor[x][y].icon)
        {
        case -1:
        case 285:
        case 286:
        case 287:
        case 288:
        case 289:
        case 290:
        case 291:
        case 292:
        case 293:
        case 294:
        case 295:
        case 296:
        case 297:
        case 298:
        case 299:
        case 300:
        case 302:
        case 303:
            num = 15;
            if (!IsGrotte(x, y + 1))
            {
                num &= -2;
            }
            if (!IsGrotte(x, y - 1))
            {
                num &= -3;
            }
            if (!IsGrotte(x + 1, y))
            {
                num &= -5;
            }
            if (!IsGrotte(x - 1, y))
            {
                num &= -9;
            }
            num2 = Tables::table_adapt_fromage[num + 16];
            if (num2 == 289 && m_random.get()->Next() % 2 == 0)
            {
                num2 = 300;
            }
            if (num2 == 285 && m_random.get()->Next() % 2 == 0)
            {
                num2 = 302;
            }
            if (num2 == 286 && m_random.get()->Next() % 2 == 0)
            {
                num2 = 303;
            }
            m_decor[x][y].icon = num2;
            break;
        }
    }

    /**
     * @note Re-runs the auto-tiler on the edited cell and its four orthogonal neighbours so all
     *       borders stay consistent after a tile change. It then maintains two side effects: any
     *       moving object on a newly-solid cell is removed (MoveObjectDelete), and the blitz
     *       emitter column (icon 304) is extended/retracted by writing/clearing up to 4 cells of
     *       lightning body (icon 305) below it depending on whether the emitter is present.
     */
    void Decor::AdaptBorder(TinyPoint cel)
    {
        AdaptMidBorder(cel.X, cel.Y);
        AdaptMidBorder(cel.X + 1, cel.Y);
        AdaptMidBorder(cel.X - 1, cel.Y);
        AdaptMidBorder(cel.X, cel.Y + 1);
        AdaptMidBorder(cel.X, cel.Y - 1);
        int icon = m_decor[cel.X][cel.Y].icon;
        if (icon != -1 && !IsPassIcon(icon))
        {
            MoveObjectDelete(cel);
        }
        icon = m_decor[cel.X][cel.Y].icon;
        if (icon == 304)
        {
            for (int i = 0; i < 4; i++)
            {
                cel.Y++;
                if (cel.Y >= 100)
                {
                    break;
                }
                icon = m_decor[cel.X][cel.Y].icon;
                if (icon != -1)
                {
                    break;
                }
                m_decor[cel.X][cel.Y].icon = 305;
            }
        }
        if (icon != -1)
        {
            return;
        }
        for (int i = 0; i < 4; i++)
        {
            cel.Y++;
            if (cel.Y >= 100)
            {
                break;
            }
            icon = m_decor[cel.X][cel.Y].icon;
            if (icon != 305)
            {
                break;
            }
            m_decor[cel.X][cel.Y].icon = -1;
        }
    }

    void Decor::CurrentDelete()
    {
        Worlds::DeleteCurrentGame();
    }

    /**
     * @note Serialises the entire live simulation field-by-field into the Worlds key/value
     *       writer (a "DescFile" section plus per-object/per-gauge sections), then flushes the
     *       built string to the current-game slot. It is the exact inverse of CurrentRead();
     *       any field added here must be read back there. Active moving objects are written as
     *       sparse records carrying their own pool index.
     */
    bool Decor::CurrentWrite()
    {
        Worlds::WriteClear();
        Worlds::WriteSection("DescFile");
        Worlds::WriteIntField("_version_", 1);
        Worlds::WritePointField("_posDecor_", m_posDecor);
        Worlds::WritePointField("_dimDecor_", m_dimDecor);
        Worlds::WriteIntField("_term_", m_term);
        Worlds::WriteIntField("_music_", m_music);
        Worlds::WriteIntField("_region_", m_region);
        Worlds::WriteIntField("_time_", m_time);
        Worlds::WritePointField("_blupiPos_", m_blupiPos);
        Worlds::WritePointField("_blupiValidPos_", m_blupiValidPos);
        Worlds::WriteIntField("_blupiAction_", ToRaw(m_blupiAction));
        Worlds::WriteIntField("_blupiDir_", ToRaw(m_blupiDir));
        Worlds::WriteIntField("_blupiPhase_", m_blupiPhase);
        Worlds::WriteDoubleField("_blupiVitesseX_", m_blupiVitesseX);
        Worlds::WriteDoubleField("_blupiVitesseY_", m_blupiVitesseY);
        Worlds::WriteIntField("_blupiIcon_", m_blupiIcon);
        Worlds::WriteIntField("_blupiSec_", ToRaw(m_blupiSec));
        Worlds::WriteIntField("_blupiChannel_", ToRaw(m_blupiChannel));
        Worlds::WritePointField("_blupiVector_", m_blupiVector);
        Worlds::WriteIntField("_blupiTransport_", m_blupiTransport);
        Worlds::WriteBoolField("_blupiFocus_", m_blupiFocus);
        Worlds::WriteBoolField("_blupiAir_", m_blupiAir);
        Worlds::WriteBoolField("_blupiHelico_", m_blupiHelico);
        Worlds::WriteBoolField("_blupiOver_", m_blupiOver);
        Worlds::WriteBoolField("_blupiJeep_", m_blupiJeep);
        Worlds::WriteBoolField("_blupiTank_", m_blupiTank);
        Worlds::WriteBoolField("_blupiSkate_", m_blupiSkate);
        Worlds::WriteBoolField("_blupiNage_", m_blupiNage);
        Worlds::WriteBoolField("_blupiSurf_", m_blupiSurf);
        Worlds::WriteBoolField("_blupiVent_", m_blupiVent);
        Worlds::WriteBoolField("_blupiSuspend_", m_blupiSuspend);
        Worlds::WriteBoolField("_blupiJumpAie_", m_blupiJumpAie);
        Worlds::WriteBoolField("_blupiShield_", m_blupiShield);
        Worlds::WriteBoolField("_blupiPower_", m_blupiPower);
        Worlds::WriteBoolField("_blupiCloud_", m_blupiCloud);
        Worlds::WriteBoolField("_blupiHide_", m_blupiHide);
        Worlds::WriteBoolField("_blupiInvert_", m_blupiInvert);
        Worlds::WriteBoolField("_blupiBalloon_", m_blupiBalloon);
        Worlds::WriteBoolField("_blupiEcrase_", m_blupiEcrase);
        Worlds::WriteBoolField("_blupiMotorHigh_", m_blupiMotorHigh);
        Worlds::WritePointField("_blupiPosHelico_", m_blupiPosHelico);
        Worlds::WritePointField("_blupiPosMagic_", m_blupiPosMagic);
        Worlds::WriteBoolField("_blupiRestart_", m_blupiRestart);
        Worlds::WriteBoolField("_blupiFront_", m_blupiFront);
        Worlds::WriteIntField("_blupiBullet_", m_blupiBullet);
        Worlds::WriteIntField("_blupiCle_", ToRaw(m_blupiCle));
        Worlds::WriteIntField("_blupiPerso_", m_blupiPerso);
        Worlds::WriteIntField("_blupiDynamite_", m_blupiDynamite);
        Worlds::WriteIntField("_blupiNoBarre_", m_blupiNoBarre);
        Worlds::WriteIntField("_blupiTimeShield_", m_blupiTimeShield);
        Worlds::WriteIntField("_blupiTimeFire_", m_blupiTimeFire);
        Worlds::WriteIntField("_blupiTimeNoAsc_", m_blupiTimeNoAsc);
        Worlds::WriteIntField("_blupiTimeOuf_", m_blupiTimeOuf);
        Worlds::WriteIntField("_blupiActionOuf_", ToRaw(m_blupiActionOuf));
        Worlds::WriteIntField("_blupiFifoNb_", m_blupiFifoNb);
        Worlds::WritePointField("_blupiStartPos_", m_blupiStartPos);
        Worlds::WriteIntField("_blupiStartDir_", ToRaw(m_blupiStartDir));
        Worlds::WriteIntField("_blupiLevel_", m_blupiLevel);
        Worlds::WriteBoolField("_bFoundCle_", m_bFoundCle);
        Worlds::WriteBoolField("_bPrivate_", m_bPrivate);
        Worlds::WriteBoolField("_bCheatDoors_", m_bCheatDoors);
        Worlds::WriteBoolField("_bSuperBlupi_", m_bSuperBlupi);
        Worlds::WriteBoolField("_bDrawSecret_", m_bDrawSecret);
        Worlds::WriteIntField("_mission_", m_mission);
        Worlds::WriteIntField("_nbVies_", m_nbVies);
        Worlds::WriteIntField("_nbTresor_", m_nbTresor);
        Worlds::WriteIntField("_totalTresor_", m_totalTresor);
        Worlds::WriteIntField("_goalPhase_", m_goalPhase);
        Worlds::WritePointField("_scrollPoint_", m_scrollPoint);
        Worlds::WritePointField("_scrollAdd_", m_scrollAdd);
        Worlds::WriteIntField("_voyageIcon_", m_voyageIcon);
        Worlds::WriteIntField("_voyageChannel_", ToRaw(m_voyageChannel));
        Worlds::WriteIntField("_voyagePhase_", m_voyagePhase);
        Worlds::WriteIntField("_voyageTotal_", m_voyageTotal);
        Worlds::WritePointField("_voyageStart_", m_voyageStart);
        Worlds::WritePointField("_voyageEnd_", m_voyageEnd);
        Worlds::WriteIntField("_decorAction_", ToRaw(m_decorAction));
        Worlds::WriteIntField("_decorPhase_", m_decorPhase);
        Worlds::WriteIntField("_nbRankCaisse_", m_nbRankCaisse);
        Worlds::WriteIntField("_nbLinkCaisse_", m_nbLinkCaisse);
        Worlds::WritePointField("_sucettePos_", m_sucettePos);
        Worlds::WriteIntField("_sucetteType_", ToRaw(m_sucetteType));
        Worlds::WriteIntArrayField("_RankCaisse_", m_rankCaisse, m_rankCaisseLength);
        Worlds::WriteIntArrayField("_LinkCaisse_", m_linkCaisse, m_linkCaisseLength);
        Worlds::WriteIntArrayField("_BalleTraj_", m_balleTraj, m_balleTrajLength);
        Worlds::WriteIntArrayField("_MoveTraj_", m_moveTraj, m_moveTrajLength);
        Worlds::WriteIntArrayField("_Doors_", m_doors, m_doorsLength);
        Worlds::WriteEndSection();
        Worlds::WriteSection("Decor");
        Worlds::WriteEndSection();
        for (int i = 0; i < 100; i++)
        {
            int array[100];
            for (int j = 0; j < 100; j++)
            {
                array[j] = m_decor[i][j].icon;
            }
            Worlds::WriteDecorField(array, 100);
        }
        Worlds::WriteSection("BigDecor");
        Worlds::WriteEndSection();
        for (int k = 0; k < 100; k++)
        {
            int array2[100];
            for (int l = 0; l < 100; l++)
            {
                array2[l] = m_bigDecor[k][l].icon;
            }
            Worlds::WriteDecorField(array2, 100);
        }
        for (int m = 0; m < MAXMOVEOBJECT; m++)
        {
            if (m_moveObject[m].type != ObjectType::ObjectType0)
            {
                Worlds::WriteSection("MoveObject");
                Worlds::WriteIntField("index", m);
                Worlds::WriteIntField("type", ToRaw(m_moveObject[m].type));
                Worlds::WriteIntField("stepAdvance", m_moveObject[m].stepAdvance);
                Worlds::WriteIntField("stepRecede", m_moveObject[m].stepRecede);
                Worlds::WriteIntField("timeStopStart", m_moveObject[m].timeStopStart);
                Worlds::WriteIntField("timeStopEnd", m_moveObject[m].timeStopEnd);
                Worlds::WritePointField("posStart", m_moveObject[m].posStart);
                Worlds::WritePointField("posEnd", m_moveObject[m].posEnd);
                Worlds::WritePointField("posCurrent", m_moveObject[m].posCurrent);
                Worlds::WriteIntField("step", m_moveObject[m].step);
                Worlds::WriteIntField("time", m_moveObject[m].time);
                Worlds::WriteIntField("phase", m_moveObject[m].phase);
                Worlds::WriteIntField("channel", ToRaw(m_moveObject[m].channel));
                Worlds::WriteIntField("icon", m_moveObject[m].icon);
                Worlds::WriteEndSection();
            }
        }
        for (int n = 0; n < 2; n++)
        {
            Worlds::WriteSection("Jauge");
            Worlds::WriteBoolField("hide", m_jauges[n].GetHide());
            Worlds::WriteIntField("mode", ToRaw(m_jauges[n].GetMode()));
            Worlds::WriteIntField("level", m_jauges[n].GetLevel());
            Worlds::WriteEndSection();
        }
        auto text = Worlds::GetWriteString();
        Worlds::WriteCurrentGame(text);
        return true;
    }

    /**
     * @note Field-by-field deserialisation of the quick-save into live state. Two non-obvious
     *       points: (1) it starts from a clean InitDecor() then overwrites every field, so any
     *       state not persisted falls back to the InitDecor default; (2) moving objects are
     *       stored sparsely as records carrying their own "index" into m_moveObject[], and the
     *       loop stops at the first record with type 0. All time-based MoveObject fields are run
     *       through Config::ScaleTime() on load so a save made at one FPS replays correctly at
     *       another.
     */
    bool Decor::CurrentRead()
    {
        const System::Nullable<string>& text = Worlds::ReadCurrentGame();
        if (!text.getHasValueProperty() || text.getValueProperty().empty())
        {
            return false;
        }
        InitDecor();
        auto linesVector = System::String::Split(text.getValueProperty(), '\n');
        string* lines = linesVector.data();
        int linesLength = linesVector.size();
        Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_version_");
        m_posDecor = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_posDecor_");
        m_dimDecor = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_dimDecor_");
        m_term = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_term_");
        m_music = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_music_");
        m_region = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_region_");
        m_time = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_time_");
        m_blupiPos = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_blupiPos_");
        m_blupiValidPos = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_blupiValidPos_");
        m_blupiAction = ToBlupiAction(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiAction_"));
        m_blupiDir = ToDirection(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiDir_"));
        m_blupiPhase = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiPhase_");
        m_blupiVitesseX = Worlds::GetDoubleField(lines, linesLength, "DescFile", 0, "_blupiVitesseX_");
        m_blupiVitesseY = Worlds::GetDoubleField(lines, linesLength, "DescFile", 0, "_blupiVitesseY_");
        m_blupiIcon = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiIcon_");
        m_blupiSec = ToSecretPower(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiSec_"));
        m_blupiChannel = ToPixmapChannel(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiChannel_"));
        m_blupiVector = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_blupiVector_");
        m_blupiTransport = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiTransport_");
        m_blupiFocus = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiFocus_");
        m_blupiAir = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiAir_");
        m_blupiHelico = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiHelico_");
        m_blupiOver = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiOver_");
        m_blupiJeep = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiJeep_");
        m_blupiTank = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiTank_");
        m_blupiSkate = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiSkate_");
        m_blupiNage = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiNage_");
        m_blupiSurf = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiSurf_");
        m_blupiVent = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiVent_");
        m_blupiSuspend = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiSuspend_");
        m_blupiJumpAie = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiJumpAie_");
        m_blupiShield = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiShield_");
        m_blupiPower = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiPower_");
        m_blupiCloud = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiCloud_");
        m_blupiHide = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiHide_");
        m_blupiInvert = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiInvert_");
        m_blupiBalloon = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiBalloon_");
        m_blupiEcrase = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiEcrase_");
        m_blupiMotorHigh = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiMotorHigh_");
        m_blupiPosHelico = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_blupiPosHelico_");
        m_blupiPosMagic = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_blupiPosMagic_");
        m_blupiRestart = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiRestart_");
        m_blupiFront = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_blupiFront_");
        m_blupiBullet = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiBullet_");
        m_blupiCle = ToDoorKeyFlags(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiCle_"));
        m_blupiPerso = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiPerso_");
        m_blupiDynamite = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiDynamite_");
        m_blupiNoBarre = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiNoBarre_");
        m_blupiTimeShield = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiTimeShield_");
        m_blupiTimeFire = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiTimeFire_");
        m_blupiTimeNoAsc = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiTimeNoAsc_");
        m_blupiTimeOuf = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiTimeOuf_");
        m_blupiActionOuf = ToBlupiAction(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiActionOuf_"));
        m_blupiFifoNb = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiFifoNb_");
        m_blupiStartPos = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_blupiStartPos_");
        m_blupiStartDir = ToDirection(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiStartDir_"));
        m_blupiLevel = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_blupiLevel_");
        m_bFoundCle = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_bFoundCle_");
        m_bPrivate = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_bPrivate_");
        m_bCheatDoors = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_bCheatDoors_");
        m_bSuperBlupi = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_bSuperBlupi_");
        m_bDrawSecret = Worlds::GetBoolField(lines, linesLength, "DescFile", 0, "_bDrawSecret_");
        m_mission = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_mission_");
        m_nbVies = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_nbVies_");
        m_nbTresor = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_nbTresor_");
        m_totalTresor = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_totalTresor_");
        m_goalPhase = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_goalPhase_");
        m_scrollPoint = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_scrollPoint_");
        m_scrollAdd = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_scrollAdd_");
        m_voyageIcon = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_voyageIcon_");
        m_voyageChannel = ToPixmapChannel(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_voyageChannel_"));
        m_voyagePhase = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_voyagePhase_");
        m_voyageTotal = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_voyageTotal_");
        m_voyageStart = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_voyageStart_");
        m_voyageEnd = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_voyageEnd_");
        m_decorAction = ToDecorAction(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_decorAction_"));
        m_decorPhase = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_decorPhase_");
        m_nbRankCaisse = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_nbRankCaisse_");
        m_nbLinkCaisse = Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_nbLinkCaisse_");
        m_sucettePos = Worlds::GetPointField(lines, linesLength, "DescFile", 0, "_sucettePos_");
        m_sucetteType = ToObjectType(Worlds::GetIntField(lines, linesLength, "DescFile", 0, "_sucetteType_"));
        Worlds::GetIntArrayField(lines, linesLength, "DescFile", 0, "_RankCaisse_", m_rankCaisse, m_rankCaisseLength);
        Worlds::GetIntArrayField(lines, linesLength, "DescFile", 0, "_LinkCaisse_", m_linkCaisse, m_linkCaisseLength);
        Worlds::GetIntArrayField(lines, linesLength, "DescFile", 0, "_BalleTraj_", m_balleTraj, m_balleTrajLength);
        Worlds::GetIntArrayField(lines, linesLength, "DescFile", 0, "_MoveTraj_", m_moveTraj, m_moveTrajLength);
        Worlds::GetIntArrayField(lines, linesLength, "DescFile", 0, "_Doors_", m_doors, m_doorsLength);
        for (int i = 0; i < 100; i++)
        {
            for (int j = 0; j < 100; j++)
            {
                auto decorField = Worlds::GetDecorField(lines, linesLength, "Decor", j, i);
                m_decor[j][i].icon = decorField.GetValueOrDefault(-1);
            }
        }
        for (int k = 0; k < 100; k++)
        {
            for (int l = 0; l < 100; l++)
            {
                auto decorField = Worlds::GetDecorField(lines, linesLength, "BigDecor", l, k);
                m_bigDecor[l][k].icon = decorField.GetValueOrDefault(-1);
            }
        }
        for (int m = 0; m < MAXMOVEOBJECT; m++)
        {
            m_moveObject[m].type = ObjectType::ObjectType0;
        }
        for (int n = 0; n < MAXMOVEOBJECT; n++)
        {
            int intField = Worlds::GetIntField(lines, linesLength, "MoveObject", n, "type");
            if (intField == 0)
            {
                break;
            }
            int intField2 = Worlds::GetIntField(lines, linesLength, "MoveObject", n, "index");
            m_moveObject[intField2].type = ToObjectType(intField);
            m_moveObject[intField2].stepAdvance = Config::ScaleTime(Worlds::GetIntField(lines, linesLength, "MoveObject", n,
                                                                      "stepAdvance"));
            m_moveObject[intField2].stepRecede = Config::ScaleTime(Worlds::GetIntField(lines, linesLength, "MoveObject", n, "stepRecede"));
            m_moveObject[intField2].timeStopStart = Config::ScaleTime(Worlds::GetIntField(lines, linesLength, "MoveObject", n,
                                                                        "timeStopStart"));
            m_moveObject[intField2].timeStopEnd = Config::ScaleTime(Worlds::GetIntField(lines, linesLength, "MoveObject", n,
                                                                      "timeStopEnd"));
            m_moveObject[intField2].posStart = Worlds::GetPointField(lines, linesLength, "MoveObject", n, "posStart");
            m_moveObject[intField2].posEnd = Worlds::GetPointField(lines, linesLength, "MoveObject", n, "posEnd");
            m_moveObject[intField2].posCurrent = Worlds::GetPointField(lines, linesLength, "MoveObject", n,
                                                                       "posCurrent");
            m_moveObject[intField2].step = Worlds::GetIntField(lines, linesLength, "MoveObject", n, "step");
            m_moveObject[intField2].time = Worlds::GetIntField(lines, linesLength, "MoveObject", n, "time");
            m_moveObject[intField2].phase = Worlds::GetIntField(lines, linesLength, "MoveObject", n, "phase");
            m_moveObject[intField2].channel = ToPixmapChannel(Worlds::GetIntField(lines, linesLength, "MoveObject", n, "channel"));
            m_moveObject[intField2].icon = Worlds::GetIntField(lines, linesLength, "MoveObject", n, "icon");
        }
        for (int num = 0; num < 2; num++)
        {
            m_jauges[num].SetHide(Worlds::GetBoolField(lines, linesLength, "Jauge", num, "hide"));
            m_jauges[num].SetMode(Worlds::GetIntField(lines, linesLength, "Jauge", num, "mode"));
            m_jauges[num].SetLevel(Worlds::GetIntField(lines, linesLength, "Jauge", num, "level"));
        }
        return true;
    }

    /**
     * @note Loads a designed level (as opposed to a quick-save). Tile icon 0 from the file is
     *       normalised to -1 (empty) since the editor uses 0 for "no tile". Moving objects are
     *       packed densely (slot n = record n) and the loop stops at the first type-0 record.
     *       Time-based object fields are run through Config::ScaleTime(); some types get fixed
     *       overrides on load (e.g. ObjectType54 gets a fixed 152-frame dwell at both ends).
     * @note Only Blupi's START position/direction are read here (blupiPos field), not a live
     *       position — PlayPrepare() later places Blupi at the start.
     */
    bool Decor::Read(int gamer, int rank, bool bUser)
    {
        InitDecor();
        auto worldOpt = Worlds::ReadWorld(gamer, rank);
        if (!worldOpt.getHasValueProperty() || worldOpt.getValueProperty().empty())
        {
            return false;
        }

        auto arrayVector = worldOpt.getValueProperty();
        auto vectorSize = arrayVector.size();
        string* array = arrayVector.data();
        if (arrayVector.empty())
        {
            return false;
        }
        m_posDecor = Worlds::GetPointField(array, vectorSize, "DescFile", 0, "posDecor");
        m_dimDecor = Worlds::GetPointField(array, vectorSize, "DescFile", 0, "dimDecor");
        m_music = Worlds::GetIntField(array, vectorSize, "DescFile", 0, "music");
        m_region = Worlds::GetIntField(array, vectorSize, "DescFile", 0, "region");
        m_blupiStartPos = Worlds::GetPointField(array, vectorSize, "DescFile", 0, "blupiPos");
        m_blupiStartDir = ToDirection(Worlds::GetIntField(array, vectorSize, "DescFile", 0, "blupiDir"));
        for (int i = 0; i < 100; i++)
        {
            for (int j = 0; j < 100; j++)
            {
                int decorField = Worlds::GetDecorField(array, vectorSize, "Decor", j, i).GetValueOrDefault(-1);
                m_decor[j][i].icon = decorField != 0 ? decorField : -1;
            }
        }
        for (int k = 0; k < 100; k++)
        {
            for (int l = 0; l < 100; l++)
            {
                int decorField = Worlds::GetDecorField(array, vectorSize, "BigDecor", l, k).GetValueOrDefault(-1);
                m_bigDecor[l][k].icon = decorField != 0 ? decorField : -1;
            }
        }
        for (int m = 0; m < MAXMOVEOBJECT; m++)
        {
            m_moveObject[m].type = ObjectType::ObjectType0;
        }
        for (int n = 0; n < MAXMOVEOBJECT; n++)
        {
            int intField = Worlds::GetIntField(array, vectorSize, "MoveObject", n, "type");
            if (intField == 0)
            {
                break;
            }
            m_moveObject[n].type = ToObjectType(intField);
            m_moveObject[n].stepAdvance = Config::ScaleTime(Worlds::GetIntField(array, vectorSize, "MoveObject", n, "stepAdvance"));
            m_moveObject[n].stepRecede = Config::ScaleTime(Worlds::GetIntField(array, vectorSize, "MoveObject", n, "stepRecede"));
            m_moveObject[n].timeStopStart = Config::ScaleTime(Worlds::GetIntField(array, vectorSize, "MoveObject", n, "timeStopStart"));
            m_moveObject[n].timeStopEnd = Config::ScaleTime(Worlds::GetIntField(array, vectorSize, "MoveObject", n, "timeStopEnd"));
            m_moveObject[n].posStart = Worlds::GetPointField(array, vectorSize, "MoveObject", n, "posStart");
            m_moveObject[n].posEnd = Worlds::GetPointField(array, vectorSize, "MoveObject", n, "posEnd");
            m_moveObject[n].posCurrent = Worlds::GetPointField(array, vectorSize, "MoveObject", n, "posCurrent");
            m_moveObject[n].step = Worlds::GetIntField(array, vectorSize, "MoveObject", n, "step");
            m_moveObject[n].time = Worlds::GetIntField(array, vectorSize, "MoveObject", n, "time");
            m_moveObject[n].phase = Worlds::GetIntField(array, vectorSize, "MoveObject", n, "phase");
            m_moveObject[n].channel = ToPixmapChannel(Worlds::GetIntField(array, vectorSize, "MoveObject", n, "channel"));
            m_moveObject[n].icon = Worlds::GetIntField(array, vectorSize, "MoveObject", n, "icon");
            if (m_moveObject[n].type == ObjectType::ObjectType54)
            {
                m_moveObject[n].timeStopStart = Config::ScaleTime(152);
                m_moveObject[n].timeStopEnd = Config::ScaleTime(152);
            }
        }
        return true;
    }

    /**
     * @note Stub in this port: level deletion is handled elsewhere, so this always reports
     *       success without touching storage. Parameters are unused.
     */
    bool Decor::Delete(int gamer, int rank, bool bUser)
    {
        return true;
    }

    /**
     * @note Stub in this port: always reports "no file". Parameters are unused.
     */
    bool Decor::FileExist(int gamer, int rank, bool bUser)
    {
        return false;
    }

    /**
     * @note Scans the map for the world's terminal marker icon (looked up in
     *       Tables::world_terminal) and places Blupi on whichever adjacent cell is passable,
     *       facing toward the marker. Used to drop Blupi next to the world he last completed
     *       when entering the hub.
     */
    bool Decor::SearchWorld(int world, TinyPoint& blupi, Direction& dir)
    {
        if (world < 0 || world > 12)
        {
            return false;
        }
        int num = Tables::world_terminal[world * 2];
        int num2 = Tables::world_terminal[world * 2 + 1];
        for (int i = 0; i < 100; i++)
        {
            for (int j = 0; j < 100; j++)
            {
                int icon = m_decor[i][j].icon;
                if (icon == num || icon == num2)
                {
                    if (IsPassIcon(m_decor[i - 1][j].icon))
                    {
                        blupi.X = (i - 1) * 64 + 2;
                        blupi.Y = j * 64 + BLUPIOFFY;
                        dir = Direction::Right;
                        return true;
                    }
                    if (IsPassIcon(m_decor[i + 1][j].icon))
                    {
                        blupi.X = (i + 1) * 64 + 2;
                        blupi.Y = j * 64 + BLUPIOFFY;
                        dir = Direction::Left;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /**
     * @note Finds world-door number @p n (door sign icons 174..181 map to door 1..8) and locates
     *       the actual door tile (icon 182) within two cells to either side, returning both the
     *       door cell and the cell just beyond it where Blupi should stand. The search prefers
     *       the nearer cell on each side.
     */
    bool Decor::SearchDoor(int n, TinyPoint& cel, TinyPoint& blupi)
    {
        for (int i = 0; i < 100; i++)
        {
            for (int j = 0; j < 100; j++)
            {
                int icon = m_decor[i][j].icon;
                if (icon >= 174 && icon <= 181 && icon - 174 + 1 == n)
                {
                    if (i > 0 && m_decor[i - 1][j].icon == 182)
                    {
                        cel.X = i - 1;
                        cel.Y = j;
                        blupi.X = (i - 2) * 64 + 2;
                        blupi.Y = j * 64 + BLUPIOFFY;
                        return true;
                    }
                    if (i > 1 && m_decor[i - 2][j].icon == 182)
                    {
                        cel.X = i - 2;
                        cel.Y = j;
                        blupi.X = (i - 3) * 64 + 2;
                        blupi.Y = j * 64 + BLUPIOFFY;
                        return true;
                    }
                    if (i < 99 && m_decor[i + 1][j].icon == 182)
                    {
                        cel.X = i + 1;
                        cel.Y = j;
                        blupi.X = (i + 2) * 64 + 2;
                        blupi.Y = j * 64 + BLUPIOFFY;
                        return true;
                    }
                    if (i < 98 && m_decor[i + 2][j].icon == 182)
                    {
                        cel.X = i + 2;
                        cel.Y = j;
                        blupi.X = (i + 3) * 64 + 2;
                        blupi.Y = j * 64 + BLUPIOFFY;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Decor::SearchGold(int n, TinyPoint& cel)
    {
        for (int num = 99; num >= 0; num--)
        {
            for (int num2 = 99; num2 >= 0; num2--)
            {
                if (m_decor[num2][num].icon == 183)
                {
                    cel.X = num2;
                    cel.Y = num;
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @note Only acts on the hub level (mission 1): it moves Blupi's start position to stand
     *       beside the world he most recently completed (SearchWorld(@p lastWorld)) so re-entering
     *       the hub places him where he left off. No effect on regular gameplay levels.
     */
    void Decor::MainSwitchInitialize(int lastWorld)
    {
        if (m_mission == 1)
        {
            TinyPoint blupi;
            Direction dir = Direction::None;
            if (SearchWorld(lastWorld, blupi, dir))
            {
                m_blupiStartPos = blupi;
                m_blupiStartDir = dir;
            }
        }
    }

    /**
     * @note Two distinct behaviours selected by mission. Private (user) levels return early —
     *       all gates are treated as open. On the hub (mission 1) it reveals progress: collected
     *       golds animate up (ObjectType22), and world-entry/bonus tiles are swapped to their
     *       "unlocked" icon variant when the matching door flag (or the open-doors cheat) is set.
     *       On a world's entry screen (mission % 10 == 0) it opens the doors to sub-levels the
     *       player has reached and snaps Blupi's start position/direction beside the last open
     *       door.
     * @note Door indexing is by convention: m_doors[180 + worldOffset] holds gold/world flags on
     *       the hub, while m_doors[mission + i] holds the per-sublevel door flags on world screens.
     */
    void Decor::AdaptDoors(bool bPrivate)
    {
        TinyPoint cel;
        TinyPoint blupi;
        m_bPrivate = bPrivate;
        if (m_bPrivate)
        {
            return;
        }
        if (m_mission == 1)
        {
            for (int i = 0; i < 20; i++)
            {
                if (SearchGold(i, cel) && (m_doors[180 + i] == 1 || m_bCheatDoors))
                {
                    m_decor[cel.X][cel.Y].icon = -1;
                    int num = MoveObjectFree();
                    m_moveObject[num].type = ObjectType::ObjectType22;
                    m_moveObject[num].stepAdvance = Config::ScaleTime(50);
                    m_moveObject[num].stepRecede = Config::ScaleTime(1);
                    m_moveObject[num].timeStopStart = 0;
                    m_moveObject[num].timeStopEnd = 0;
                    m_moveObject[num].posStart.X = 64 * cel.X;
                    m_moveObject[num].posStart.Y = 64 * cel.Y;
                    m_moveObject[num].posEnd.X = 64 * cel.X;
                    m_moveObject[num].posEnd.Y = 64 * (cel.Y - 1);
                    m_moveObject[num].posCurrent = m_moveObject[num].posStart;
                    m_moveObject[num].step = 1;
                    m_moveObject[num].time = 0;
                    m_moveObject[num].phase = 0;
                    m_moveObject[num].channel = PixmapChannel::Object;
                    m_moveObject[num].icon = 183;
                    PlaySound(SoundChannel::SoundChannel33, m_moveObject[num].posStart);
                }
            }
            for (int j = 0; j < 100; j++)
            {
                for (int k = 0; k < 100; k++)
                {
                    int icon = m_decor[j][k].icon;
                    if (icon >= 158 && icon <= 165 && (m_doors[180 + icon - 158 + 1] == 1 || m_bCheatDoors))
                    {
                        m_decor[j][k].icon += 8;
                    }
                    if (icon == 309 && (m_doors[189] == 1 || m_bCheatDoors))
                    {
                        m_decor[j][k].icon = 310;
                    }
                    if (icon >= 410 && icon <= 415 && (m_doors[180 + icon - 410 + 9] == 1 || m_bCheatDoors))
                    {
                        m_decor[j][k].icon += 5;
                    }
                }
            }
        }
        else
        {
            if (m_mission % 10 != 0)
            {
                return;
            }
            for (int i = 0; i < 10; i++)
            {
                if (SearchDoor(i, cel, blupi))
                {
                    const int doorIndex = m_mission + i;
                    const bool shouldOpen = m_doors[doorIndex] == 1 || m_bCheatDoors;
                    log::Debug("Decor::AdaptDoors mission=" + std::to_string(m_mission)
                        + " doorIndex=" + std::to_string(doorIndex)
                        + " state=" + std::to_string(m_doors[doorIndex])
                        + " cheat=" + std::to_string(m_bCheatDoors ? 1 : 0)
                        + " open=" + std::to_string(shouldOpen ? 1 : 0));

                    if (shouldOpen)
                    {
                        OpenDoor(cel);
                        m_blupiStartPos = blupi;
                        if (blupi.X < cel.X * 64)
                        {
                            m_blupiStartDir = Direction::Right;
                        }
                        else
                        {
                            m_blupiStartDir = Direction::Left;
                        }
                    }
                }
            }
        }
    }

#ifdef MODERN
    bool Decor::IsGhost()
    {
        return m_blupiGhost;
    }

    void Decor::SetCheatZoom(double factor)
    {
        m_cheatZoomFactor = factor;
    }
#endif

    /**
     * @note Treasure-gated doors use consecutive icons starting at 421 (door requiring 1
     *       treasure = 421, 2 = 422, ...). This opens every such door whose treasure requirement
     *       is now met (icon in 421 .. 421 + m_nbTresor - 1). Called whenever a treasure is
     *       collected.
     */
    void Decor::OpenDoorsTresor()
    {
        TinyPoint cel;
        for (int i = 0; i < 100; i++)
        {
            for (int j = 0; j < 100; j++)
            {
                int icon = m_decor[i][j].icon;
                if (icon >= 421 && icon <= 421 + m_nbTresor - 1)
                {
                    cel.X = i;
                    cel.Y = j;
                    OpenDoor(cel);
                }
            }
        }
    }

    /**
     * @note Opening is animated, not instant: the door tile is cleared from the map and a
     *       transient ObjectType22 is spawned that slides the old door sprite one tile upward
     *       (posStart -> posEnd one cell up) over ~50 frames before it self-destructs, with the
     *       door-open sound. So after this call the cell is already passable while the visual
     *       lift-away plays out.
     */
    void Decor::OpenDoor(TinyPoint cel)
    {
        int icon = m_decor[cel.X][cel.Y].icon;
        m_decor[cel.X][cel.Y].icon = -1;
        int num = MoveObjectFree();
        m_moveObject[num].type = ObjectType::ObjectType22;
        m_moveObject[num].stepAdvance = Config::ScaleTime(50);
        m_moveObject[num].stepRecede = Config::ScaleTime(1);
        m_moveObject[num].timeStopStart = 0;
        m_moveObject[num].timeStopEnd = 0;
        m_moveObject[num].posStart.X = 64 * cel.X;
        m_moveObject[num].posStart.Y = 64 * cel.Y;
        m_moveObject[num].posEnd.X = 64 * cel.X;
        m_moveObject[num].posEnd.Y = 64 * (cel.Y - 1);
        m_moveObject[num].posCurrent = m_moveObject[num].posStart;
        m_moveObject[num].step = 1;
        m_moveObject[num].time = 0;
        m_moveObject[num].phase = 0;
        m_moveObject[num].channel = PixmapChannel::Object;
        m_moveObject[num].icon = icon;
        PlaySound(SoundChannel::SoundChannel33, m_moveObject[num].posStart);
    }

    /**
     * @note Records the win by setting the door flag for the NEXT sublevel (m_doors[mission+1]),
     *       which is what AdaptDoors() later reads to unlock progression. This only flips the
     *       persistent flag; it does not animate any door in the current level.
     */
    void Decor::OpenDoorsWin()
    {
        m_doors[m_mission + 1] = 1;
        log::Debug("Decor::OpenDoorsWin mission=" + std::to_string(m_mission)
            + " unlockedDoor=" + std::to_string(m_mission + 1));
    }

    /**
     * @note Marks the whole world (index mission/10) as cleared by setting its gold flag at the
     *       conventional offset m_doors[180 + worldIndex]; AdaptDoors() reads this on the hub to
     *       reveal the world's collected gold.
     */
    void Decor::OpenGoldsWin()
    {
        m_doors[180 + m_mission / 10] = 1;
    }

    /**
     * @note Despite the name it does not touch m_doors: losing simply restores the life count to
     *       the default 3 (the original game's reset-on-fail behaviour for this code path).
     */
    void Decor::DoorsLost()
    {
        m_nbVies = 3;
    }
}
