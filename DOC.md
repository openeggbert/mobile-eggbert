# Mobile Eggbert — Technical Documentation

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [History and Relationship to Other Projects](#2-history-and-relationship-to-other-projects)
3. [Repository Structure](#3-repository-structure)
4. [Dependencies](#4-dependencies)
5. [Build System](#5-build-system)
6. [Compile-Time Configuration](#6-compile-time-configuration)
7. [Game Phase System](#7-game-phase-system)
8. [Image Channels (PixmapChannel)](#8-image-channels-pixmapchannel)
9. [Player Action States (BlupiAction)](#9-player-action-states-blupiaction)
10. [Moving Object Types (ObjectType)](#10-moving-object-types-objecttype)
11. [Sound Effects (SoundChannel)](#11-sound-effects-soundchannel)
12. [Key Input Flags (KeyPressFlags)](#12-key-input-flags-keypressflags)
13. [Security Power-Ups (SecretPower)](#13-security-power-ups-secretpower)
14. [Direction and Game Speed](#14-direction-and-game-speed)
15. [Camera Actions (DecorAction)](#15-camera-actions-decoraction)
16. [Door Key Flags (DoorKeyFlags)](#16-door-key-flags-doorkeyflags)
17. [UI Buttons (ButtonGlyph)](#17-ui-buttons-buttonglyph)
18. [Game Constants and Dimensions](#18-game-constants-and-dimensions)
19. [World and Data Structures](#19-world-and-data-structures)
20. [CDecor / Decor Class](#20-cdecor--decor-class)
21. [Game1 — Main Game Loop](#21-game1--main-game-loop)
22. [Save Data Format](#22-save-data-format)
23. [World File Format](#23-world-file-format)
24. [Game Asset Files](#24-game-asset-files)
25. [Cheat System](#25-cheat-system)
26. [Localization](#26-localization)
27. [Performance Notes](#27-performance-notes)
28. [Doxygen Documentation Plan](#28-doxygen-documentation-plan)
29. [Known Issues and TODO](#29-known-issues-and-todo)

---

## 1. Project Overview

**Mobile Eggbert** is a C++ port of *Speedy Blupi* — the Windows Phone XNA game released in 2013. It is a side-scrolling platform game in which the player controls Blupi through 100×100-tile levels, collecting treasures, using vehicles and power-ups, and reaching the goal exit.

The project targets multiple platforms:
- Linux (primary development target)
- Windows (MinGW cross-build or native)
- Web (Emscripten / WebAssembly)
- Android (NDK + Gradle)

**Primary language:** C++23

**Framework:** [CNA](../cna) — a custom C++ XNA-style wrapper built on top of SDL3

**License:** See `LICENSE`

**Executable name:** `WindowsPhoneSpeedyBlupi`

---

## 2. History and Relationship to Other Projects

The evolution of this codebase:

1. **Speedy Blupi (Windows Phone, 2013)** — original C# XNA 4.0 game. Source never publicly released.
2. **Decompilation via ILSpy** — the Windows Phone binary was decompiled to C# source. Symbol names were preserved, which assisted later reverse-engineering of Speedy Eggbert 2 / Free Eggbert.
3. **MonoGame migration** — decompiled C# was adapted to run on MonoGame.
4. **C++ rewrite** — complete rewrite from C# to C++ (starting from `mobile-eggbert-core` commit `1cbc134`).
5. **CNA migration** — graphics/audio backend replaced from MonoGame with CNA (SDL3-based XNA wrapper).

**Related projects in this repository collection:**

| Project | Role |
|---|---|
| `planetblupi/` | Original Planet Blupi C++ source (official open-source, isometric RTS) |
| `free-eggbert/` | Decompiled Speedy Eggbert 2 (DirectX 3 platform game) |
| `mobile-eggbert/` | This project — C++ port of the Windows Phone Speedy Blupi |
| `mobile-eggbert-legacy/` | Previous/legacy version of this port |
| `cna/` | CNA framework (C++ XNA API on SDL3) used by this project |
| `sharp-runtime/` | C++ implementations of .NET base-class types needed by the port |
| `free-direct/` | DirectDraw → SDL3 abstraction layer |
| `free-api/` | Win32 → cross-platform API abstraction layer |

Mobile Eggbert is **not** a fork of Planet Blupi or Free Eggbert — it is a separate, independent C++ port that originated from the Windows Phone version of the game.

---

## 3. Repository Structure

```
mobile-eggbert/
├── include/WindowsPhoneSpeedyBlupi/    # All header files
│   ├── ConfigDef.hpp                   # LEGACY vs MODERN mode selection
│   ├── Config.hpp                      # FPS, time scaling, resolution scaling
│   ├── Decor.hpp                       # Core game simulation class
│   ├── Def.hpp                         # Global constants (LXIMAGE, MAXCELX, etc.)
│   ├── Game1.hpp                       # Main XNA game class
│   ├── GameData.hpp                    # Save data serialization
│   ├── Helper.hpp                      # String formatting utilities
│   ├── IGame1.hpp                      # Interface for game instance
│   ├── InputPad.hpp                    # Touch/keyboard/accelerometer input
│   ├── IPixmap.hpp                     # Interface for sprite rendering
│   ├── ISound.hpp                      # Interface for audio playback
│   ├── Jauge.hpp                       # HUD gauge widget
│   ├── Misc.hpp                        # Geometry helpers
│   ├── MyResource.hpp                  # Localized UI strings (FR/EN/DE)
│   ├── Pixmap.hpp                      # CNA/SpriteBatch sprite renderer
│   ├── Slider.hpp                      # UI slider widget
│   ├── Sound.hpp                       # CNA audio playback
│   ├── Tables.hpp                      # Static animation/data tables
│   ├── Text.hpp                        # Bitmap font renderer
│   ├── TinyPoint.hpp                   # 2D integer point
│   ├── TinyRect.hpp                    # 2D integer rectangle
│   ├── Worlds.hpp                      # World file I/O
│   ├── def/                            # Enumeration headers
│   │   ├── BlupiAction.hpp             # 88 player animation states
│   │   ├── ContinueMission.hpp         # Checkpoint/resume states
│   │   ├── Direction.hpp               # None/Left/Right
│   │   ├── GameSpeed.hpp               # Slow/Normal/Fast/Faster/Fastest
│   │   ├── KeyPressFlags.hpp           # Jump/Fire/Down bitmask
│   │   ├── PixmapChannel.hpp           # 17 sprite-sheet channels
│   │   ├── SecretPower.hpp             # 5 active power-up states
│   │   ├── SoundChannel.hpp            # 93 sound effect slots
│   │   └── Zoom.hpp                    # 4 zoom levels (MODERN mode)
│   └── decor/                          # Decor-specific enum headers
│       ├── DecorAction.hpp             # Camera-shake action types
│       ├── DoorKeyFlags.hpp            # 3-key bitmask
│       └── ObjectType.hpp              # 203+ moving object types
├── src/WindowsPhoneSpeedyBlupi/        # Implementation files
│   ├── Program.cpp                     # Entry point
│   ├── Game1.cpp                       # XNA game loop, phase state machine
│   ├── Decor.cpp                       # ~9400+ lines — core gameplay simulation
│   ├── Pixmap.cpp                      # Sprite rendering via SpriteBatch
│   ├── Sound.cpp                       # Audio playback (volume/balance tables)
│   ├── InputPad.cpp                    # Touch, keyboard, accelerometer input
│   ├── GameData.cpp                    # Save-data serialization
│   ├── Tables.cpp                      # 2911-element animation table
│   ├── Helper.cpp                      # String formatting
│   ├── Jauge.cpp                       # HUD gauge rendering
│   ├── Misc.cpp                        # Geometry helpers
│   ├── MyResource.cpp                  # Localized strings
│   ├── Slider.cpp                      # UI slider widget
│   ├── Text.cpp                        # Bitmap font rendering
│   ├── TinyRect.cpp                    # Rectangle helpers
│   └── Worlds.cpp                      # World file parsing and I/O
├── Content/                            # Game assets (PNG + WAV)
│   ├── backgrounds/                    # 37 background PNGs
│   ├── backgrounds4x/                  # 4× upscaled backgrounds (35 files)
│   ├── icons/                          # 8 sprite-sheet PNGs
│   ├── icons4x/                        # 4× upscaled icons (8 files)
│   ├── sounds/                         # 92 WAV sound effects
│   └── Content.mgcb                    # MonoGame content manifest (metadata)
├── worlds/                             # 78 level text files
├── documentation/                      # Supplemental docs
│   └── Cheat System.md
├── android/                            # Android Gradle project
├── cmake/                              # CMake helper modules
├── cmake-build-debug/                  # Build output directory
├── Microsoft.Devices.Sensors/          # Sensor API stubs
├── Microsoft.Xna.Framework.GamerServices/ # XNA GamerServices stubs
├── Properties/                         # Project properties
├── CMakeLists.txt                      # Main build file (CMake 3.21+)
├── Doxyfile                            # Doxygen configuration
├── README.md                           # Build instructions
├── ANDROID.md                          # Android-specific build guide
├── WINDOWS.md                          # Windows MinGW DLL notes
├── CLAUDE.md                           # Development guidelines for AI
├── TODO.md                             # Outstanding issues
├── RAM.md                              # Memory/performance analysis
├── ENUMS.md                            # Magic-number refactoring analysis
└── DOXYGEN_DOCUMENTATION_PLAN.md       # Doxygen documentation status
```

---

## 4. Dependencies

### External frameworks

| Dependency | Purpose |
|---|---|
| **CNA** (`../cna`) | C++ XNA-style API wrapper built on SDL3 (graphics, audio, input, content pipeline) |
| **SharpRuntime** (`../sharp-runtime`) | C++ implementations of .NET base-class types (`String`, `List`, `Dictionary`, etc.) needed by the C# → C++ port |
| **SDL3** | Window management, rendering, input (via CNA) |
| **SDL3_image** | PNG loading (via CNA) |
| **SDL3_mixer** | Audio mixing (via CNA) |

### Build tools

| Tool | Purpose |
|---|---|
| CMake 3.21+ | Build system generator |
| C++23 compiler (GCC/Clang) | Main compilation |
| MinGW-w64 | Windows cross-compilation |
| Emscripten SDK | WebAssembly build |
| Android NDK 28.2.13676358 | Android build |
| Gradle | Android APK packaging |

### Runtime DLLs (Windows only)

Required alongside the executable on Windows (copied automatically by CMake):
- `libwinpthread-1.dll`
- `SDL3.dll`
- `SDL3_image.dll`
- `SDL3_mixer.dll`

Alternatively, the MinGW runtime is linked statically via `-static-libgcc -static-libstdc++`.

---

## 5. Build System

### Initialize submodules

```bash
git submodule update --init --recursive
```

### Linux (SDL_Renderer backend)

```bash
cmake -S . -B build-linux \
  -DCNA_BACKEND_SDL_RENDERER=ON \
  -DCNA_BACKEND_EASY_GL=OFF \
  -DCNA_BACKEND_BGFX=OFF
cmake --build build-linux --target WindowsPhoneSpeedyBlupi
```

### Available rendering backends (exactly one active)

| CMake flag | Description |
|---|---|
| `CNA_BACKEND_SDL_RENDERER=ON` | SDL_Renderer (default on Emscripten/Android) |
| `CNA_BACKEND_EASY_GL=ON` | easy-gl (desktop, lightweight OpenGL) |
| `CNA_BACKEND_BGFX=ON` | bgfx (desktop, multi-API) |
| Vulkan | Desktop, currently the default |

### Web / Emscripten

```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S . -B cmake-build-web -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-web
emrun cmake-build-web/WindowsPhoneSpeedyBlupi.html
```

Emscripten-specific settings (set by CMakeLists.txt):
- `ALLOW_MEMORY_GROWTH=1`
- `INITIAL_MEMORY=134MB`
- `Content/` and `worlds/` directories preloaded into the virtual filesystem
- Virtual FS mount points: `/Content/backgrounds`, `/Content/icons`, `/Content/sounds`, `/worlds`

### Android

```bash
cd android
./gradlew assembleDebug
adb install app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n org.openeggbert.speedyblupi/.SpeedyBlupiActivity
```

Android configuration:
- NDK version: 28.2.13676358
- Target API level: 35
- Assets packaged in APK, accessed via `AAssetManager`

### Windows (cross-compile from Linux via MinGW)

Target: `x86_64-w64-mingw32`. Executable and required DLLs are placed in `cmake-build-debug/`. See `WINDOWS.md` for details.

### Asset layout per platform

| Platform | Content directory |
|---|---|
| Linux / Windows | `Content/` copied next to the executable at build time |
| Emscripten | Preloaded into virtual FS at build time from `Content/` and `worlds/` |
| Android | Packaged as APK assets, accessed via AAssetManager |

---

## 6. Compile-Time Configuration

All configuration lives in `include/WindowsPhoneSpeedyBlupi/ConfigDef.hpp` and `Config.hpp`.

### Two operating modes (exactly one active)

#### LEGACY mode

Faithfully reproduces original Windows Phone behaviour:
- FPS: locked at 20
- `TIME_SCALE = 1.0`
- `SPEED_SCALE = 1.0`
- Touch buttons always visible
- No FPS or resolution scaling

#### MODERN mode (default)

Experimental extended mode:
- Configurable FPS: 20, 30, 60, 90, 120, or 144
- `TIME_SCALE = FPS / 20` (e.g., 3.0 at 60 FPS)
- `SPEED_SCALE = 20 / FPS` (e.g., 0.333 at 60 FPS)
- Touch buttons hidden on non-touchscreen platforms
- Helper functions: `ScaleTime()`, `ScaleDiv()`, `ScaleAsset()`

### Resolution scaling (MODERN mode, experimental)

| Enum | Scale | Resolution |
|---|---|---|
| `ScaleResolution1` | 1× | 640×480 (default) |
| `ScaleResolution2` | 2× | 1280×960 (future) |
| `ScaleResolution4` | 4× | 2560×1920 (future) |

When using higher resolution scales, asset loading automatically picks the corresponding `icons4x/` and `backgrounds4x/` directories.

---

## 7. Game Phase System

`Def::Phase` drives the top-level game mode. `Game1::SetPhase()` centralizes all side effects when transitioning (asset loading, InputPad reconfiguration, fade animations).

| Phase | Description |
|---|---|
| `None` | Initial / uninitialized |
| `First` | First frame after startup |
| `Wait` | Loading progress screen (artificial delay with gauge) |
| `Init` | Main menu / gamer selection |
| `Play` | Active gameplay |
| `Pause` | Pause overlay |
| `Lost` | Level failure screen |
| `Win` | Level completion screen |
| `Trial` | Demo/trial paywall screen |
| `MainSetup` | Settings accessed from main menu |
| `PlaySetup` | Settings accessed during gameplay |
| `Resume` | Checkpoint continuation prompt |
| `Ranking` | High-score screen |

Phase transitions use fade-out/fade-in animations. All 12 phases are handled in `Game1::Update()` and `Game1::Draw()`.

---

## 8. Image Channels (PixmapChannel)

`CPixmap` (via `IPixmap`) manages 17 named sprite-sheet surfaces. Defined in `include/WindowsPhoneSpeedyBlupi/def/PixmapChannel.hpp`:

| Enum | Value | Sprite sheet | File |
|---|---|---|---|
| `Object` | 1 | Foreground objects, pickups, enemies | `icons/object.png` |
| `Blupi` | 2 | Default player character | `icons/blupi.png` |
| `Background` | 3 | Level terrain background | `Content/backgrounds/decorNNN.png` |
| `Button` | 4 | On-screen UI buttons | `icons/button.png` |
| `Jauge` | 5 | HUD gauge bar | `icons/jauge.png` |
| `Text` | 6 | Bitmap font glyphs | `icons/text.png` |
| `Explosion` | 9 | Explosion and visual effects | `icons/explo.png` |
| `Element` | 10 | Collectibles and particle effects | `icons/element.png` |
| `Blupi1_11` | 11 | Player character variant 1 | `icons/blupi1.png` |
| `Blupi1_12` | 12 | Player character variant 2 | *(alternate blupi sheet)* |
| `Blupi1_13` | 13 | Player character variant 3 | *(alternate blupi sheet)* |
| `Pad` | 14 | Touch control pad overlay | *(UI pad)* |
| `SpeedyBlupiBackground` | 15 | Title screen (Speedy Blupi logo) | `backgrounds/speedyblupi.png` |
| `BlupiYoupieBackground` | 16 | Title screen (Blupi Youpie) | `backgrounds/blupiyoupie.png` |
| `GearBackground` | 17 | Settings/gear screen background | `backgrounds/gear.png` |

Channels 7 and 8 are reserved/unused. The total capacity is 17 channels (indices 1–17).

---

## 9. Player Action States (BlupiAction)

88 animation states for the player character. Defined in `include/WindowsPhoneSpeedyBlupi/def/BlupiAction.hpp`.

### Standing and walking
| State | Description |
|---|---|
| `Stop` | Standing still |
| `March` | Walking |
| `Turn` | Turning around |
| `StopMarch` | Stopping from a walk |

### Jumping and falling
| State | Description |
|---|---|
| `Jump` | Beginning of jump |
| `Air` | In the air (mid-jump) |
| `StopJump` | Landing from jump |
| `StopJumph` | Landing variant |
| `TurnAir` | Turning in mid-air |
| `Vertigo` | Edge vertigo ("whoops") |
| `Recede` | Backing away from edge |
| `Advance` | Walking away from edge |
| `Recedeq` | Quick recede |
| `Advanceq` | Quick advance |
| `JumpAie` | Headache upon landing |
| `Teleporte` | Teleporting |

### Helicopter
| State | Description |
|---|---|
| `StopHelico` | Hovering (helicopter stopped) |
| `MarchHelico` | Flying (helicopter in motion) |
| `TurnHelico` | Turning in helicopter |
| `HelicoGlu` | Glued while in helicopter |

### Swimming
| State | Description |
|---|---|
| `StopNage` | Stopped in deep water |
| `MarchNage` | Swimming in deep water |
| `TurnNage` | Turning in deep water |
| `StopSurf` | Stopped at water surface |
| `MarchSurf` | Swimming at water surface |
| `TurnSurf` | Turning at water surface |
| `Drown` | Drowning |

### Jeep
| State | Description |
|---|---|
| `StopJeep` | Jeep stopped |
| `MarchJeep` | Jeep moving |
| `TurnJeep` | Jeep turning |

### Crate pushing/pulling
| State | Description |
|---|---|
| `Push` | Pushing a crate |
| `StopPop` | Stopped while pulling crate |
| `Pop` | Pulling a crate |
| `Ouf3` | Stop moving crate ("gahh!") |

### Bar hanging
| State | Description |
|---|---|
| `StopSuspend` | Stopped on bars |
| `MarchSuspend` | Moving on bars |
| `TurnSuspend` | Turning on bars |
| `JumpSuspend` | Pulling up from bars |

### Skateboard
| State | Description |
|---|---|
| `StopSkate` | Skateboard stopped |
| `MarchSkate` | Skateboarding |
| `TurnSkate` | Turning on skateboard |
| `JumpSkate` | Jumping on skateboard |
| `AirSkate` | In air on skateboard |
| `TakeSkate` | Picking up skateboard |
| `DeposeSkate` | Dropping skateboard |

### Tank
| State | Description |
|---|---|
| `StopTank` | Tank stopped |
| `MarchTank` | Tank moving |
| `TurnTank` | Turning in tank |
| `FireTank` | Tank firing |

### Hovercraft
| State | Description |
|---|---|
| `StopOver` | Hovercraft stopped |
| `MarchOver` | Hovercraft moving |
| `TurnOver` | Hovercraft turning |

### Power-up effects
| State | Description |
|---|---|
| `Sucette` | Eating lollipop (power boost) |
| `Glu` | Glued (trapped) |
| `Drink` | Drinking |
| `Charge` | Charging up |
| `Electro` | Electrocuted |
| `Balloon` | Stung by wasp (swelling) |
| `Hide` | Invisible (cloud power) |
| `StopEcrase` | Stopped while flattened |
| `MarchEcrase` | Moving while flattened |
| `Switch` | Switch interaction |

### Reactions and expressions
| State | Description |
|---|---|
| `Win` | Victory animation |
| `Bye` | "au au" farewell |
| `Mockery` | Mockery taunt |
| `Mockeryi` | Mockery variant i |
| `Down` | Looking down |
| `Up` | Looking up |
| `Ouf1a` / `Ouf1b` | Relief 1a / 1b |
| `Ouf2` | Relief 2 |
| `Ouf4` | Relief 4 |
| `Ouf5` | Relief 5 |

### Utility
| State | Description |
|---|---|
| `Set` | Set state |
| `Clear1`–`Clear8` | Unused clear animation slots |

---

## 10. Moving Object Types (ObjectType)

Active moving game objects (enemies, items, vehicles, effects) tracked as `MoveObject` instances. Defined in `include/WindowsPhoneSpeedyBlupi/decor/ObjectType.hpp`. Up to 200 simultaneously active objects.

### Lifts and conveyors
| Type | Description |
|---|---|
| `Ascenseur` (1) | Standard lift |
| `AscenseurS` (47) | Conveyor belt moving right |
| `AscenseurSi` (48) | Conveyor belt moving left |

### Enemies
| Type | Description |
|---|---|
| `BombeDown` (2) | Floor bomb |
| `BombeUp` (3) | Hanging bomb |
| `BombeMove` (16) | Moving bomb |
| `BombeFollow1` (96) | Homing bomb (stopped) |
| `BombeFollow2` (97) | Homing bomb (moving) |
| `Bulldozer` (4) | Bulldozer |
| `Poisson` (17) | Fish (aquatic enemy) |
| `Oiseau` (20) | Bird (aerial enemy) |
| `Guepe` (44) | Wasp |
| `Tentacule` (53) | Slime tentacle effect |
| `Creature` (54) | Moving slime creature |

### Collectibles
| Type | Description |
|---|---|
| `Tresor` (5) | Treasure |
| `Egg` (6) | Egg / extra life |
| `Goal` (7) | Level exit marker |
| `Cle` (21) | Gold key |
| `Cle1` (49) | Red key |
| `Cle2` (50) | Green key |
| `Cle3` (51) | Blue key |

### Vehicles
| Type | Description |
|---|---|
| `Helico` (13) | Helicopter |
| `BlupiHelico` (32) | Player inside helicopter |
| `Jeep` (19) | Jeep |
| `Tank` (28) | Tank |
| `BlupiTank` (33) | Player inside tank |
| `Over` (46) | Hovercraft |

### Interactive objects
| Type | Description |
|---|---|
| `Caisse` (12) | Pushable/pullable crate |
| `Skate` (24) | Skateboard |
| `Door` (22) | Door |
| `Bridge` (52) | Fragile bridge |

### Power-ups
| Type | Description |
|---|---|
| `Shield` (25) | Shield power-up |
| `Power` (26) | Lollipop power-up |
| `Drink` (30) | Drinkable item |
| `Charge` (31) | Charging device |
| `Dynamite` (55) | Dynamite stick |
| `DynamiteF` (56) | Lit dynamite |

### Effects and particles
| Type | Description |
|---|---|
| `MagicTrack` (27) | Sparkle trail |
| `TresorTrack` (39) | Treasure sparkle trail |
| `ShieldTrack` (57) | Shield particle trail |
| `HideTrack` (58) | Invisibility particle trail |
| `Explo1`–`Explo4` (8–11) | Explosion effects 1–4 |
| `Explo5`–`Explo10` (90–95) | Explosion effects 5–10 |
| `Sploutch1`–`Sploutch3` (98–100) | Splat particle effects |
| `Pollution` (36) | Pollution/toxic zone |
| `Electro` (38) | Electrocuted player effect |
| `Clear` (37) | Burned player effect |

### Water effects
| Type | Description |
|---|---|
| `Plouf` (14) | Big water splash |
| `TiPlouf` (35) | Small water splash |
| `Blup` (15) | Underwater bubble |
| `Glu` (34) | Glue blob |

### Projectiles
| Type | Description |
|---|---|
| `Balle` (23) | Glue ball projectile |
| `Bullet` (29) | Tank bullet |

### Control-inversion effects
| Type | Description |
|---|---|
| `Invert` (40) | Controls inversion active |
| `InvertStart` (41) | Controls inversion starting |
| `InvertStop` (42) | Controls inversion ending |
| `InvertSpin` (43) | Controls inversion spin |

### Multiplayer personal bombs
| Type | Description |
|---|---|
| `BombePerso1` (200) | Player 1 bomb (yellow) |
| `BombePerso2` (201) | Player 2 bomb (orange) |
| `BombePerso3` (202) | Player 3 bomb (blue) |
| `BombePerso4` (203) | Player 4 bomb (green) |

---

## 11. Sound Effects (SoundChannel)

93 sound effect slots mapped to WAV files `Content/sounds/sound000.wav`–`sound091.wav`. Defined in `include/WindowsPhoneSpeedyBlupi/def/SoundChannel.hpp`.

`Sound0` is reserved. `Sound1`–`Sound92` correspond to WAV files `sound000.wav`–`sound091.wav` (0-based index offset by 1).

Notable sounds by category:

| Range | Category |
|---|---|
| Sound0 | Reserved |
| Sound1–Sound4 | Jump variants (low/medium/high, landing on stone) |
| Sound5–Sound8 | Turn, vertigo, look-down, fall |
| Sound9 | Respawn |
| Sound10–Sound14 | Explosion, treasure, egg, level lost, level won |
| Sound15–Sound18 | Helicopter start/high/stop/low |
| Sound19 | Last treasure collected |
| Sound20–Sound22 | Unused, look-up, low-jump |
| Sound23–Sound26 | Big splash, bubble, surface inhale, drown |
| Sound27 | Error / invalid action |
| Sound28–Sound31 | Jeep start/high/stop/low |
| Sound32–Sound35 | "Au au" bye, door creak, grab bar, pull up from bar |
| Sound36–Sound40 | Singe, patience, push crate, pull crate, headache "aie!" |
| Sound41 | Spring bounce |
| Sound42–Sound45 | Shield on/off, lollipop on/off |
| Sound46–Sound49 | Relief expressions (exit water, scared, crate-stop groan, shriek) |
| Sound50–Sound54 | Lollipop lick, glue, tank fire ok/fail, pick up glue |
| Sound55–Sound59 | Cloud start/stop, drink, charge, electro |
| Sound60–Sound63 | Personal bomb take/place, hide start/stop |
| Sound64–Sound67 | Small splash, mockery, controls invert start/stop |
| Sound68–Sound71 | Hovercraft stall, lightning, squish, teleport |
| Sound72–Sound77 | Bridge fall/appear, angel, saw, switch off/on |
| Sound78–Sound91 | Surface footstep pairs (wood, metal, cave, slime, plastic, cheese, grass) |
| Sound92 | Homing bomb beep |

---

## 12. Key Input Flags (KeyPressFlags)

Bitmask for active player input. Defined in `include/WindowsPhoneSpeedyBlupi/def/KeyPressFlags.hpp`.

| Flag | Bit | Description |
|---|---|---|
| `Jump` | bit 0 | Jump button |
| `Fire` | bit 1 | Fire/action button |
| `Down` | bit 2 | Look-down / down button |

Direction input (left/right) is separate, handled via `Direction` enum and the `InputPad` class.

---

## 13. Security Power-Ups (SecretPower)

Active special power states. Defined in `include/WindowsPhoneSpeedyBlupi/def/SecretPower.hpp`.

| Enum | Description |
|---|---|
| `None` | No power-up active |
| `Shield` | Shield active (damage protection, 100-tick timer) |
| `Power` | Lollipop active (strength boost) |
| `Cloud` | Cloud invisibility active |
| `Hide` | Hidden state active |

---

## 14. Direction and Game Speed

### Direction (`include/WindowsPhoneSpeedyBlupi/def/Direction.hpp`)

| Enum | Value | Description |
|---|---|---|
| `None` | 0 | Unset / initial |
| `Left` | 1 | Facing / moving left |
| `Right` | 2 | Facing / moving right |

### GameSpeed (`include/WindowsPhoneSpeedyBlupi/def/GameSpeed.hpp`)

Selectable game simulation speed in the settings menu:

| Enum | Multiplier | Description |
|---|---|---|
| `Slow` | 0.5× | Half speed |
| `Normal` | 1.0× | Original speed |
| `Fast` | 2.0× | Double speed |
| `Faster` | 3.0× | Triple speed |
| `Fastest` | 4.0× | Quadruple speed |

---

## 15. Camera Actions (DecorAction)

Controls camera shake in response to in-game events. Defined in `include/WindowsPhoneSpeedyBlupi/decor/DecorAction.hpp`.

| Enum | Value | Trigger |
|---|---|---|
| `None` | 0 | No shake |
| `SmallShake` | 1 | Minor impact (crate landing, bonus collection) |
| `BigShake` | 2 | Major impact (fan blade hit, large explosion) |
| `ElectricShake` | 5 | Electric contact |

---

## 16. Door Key Flags (DoorKeyFlags)

3-bit bitmask tracking which keys the player currently holds. Defined in `include/WindowsPhoneSpeedyBlupi/decor/DoorKeyFlags.hpp`.

| Enum | Value | Description |
|---|---|---|
| `None` | 0 | No keys |
| `Key1` | 1 << 0 = 1 | Red key |
| `Key2` | 1 << 1 = 2 | Green key |
| `Key3` | 1 << 2 = 4 | Blue key |
| `All` | 7 | All three keys |

---

## 17. UI Buttons (ButtonGlyph)

39 named UI buttons dispatched via `Game1`. Most buttons map to a `WM_BUTTON*`-equivalent callback:

### Main menu (Init phase)
| Glyph | Description |
|---|---|
| `InitGamerA/B/C` | Select gamer slot A, B, or C |
| `InitSetup` | Open settings from main menu |
| `InitPlay` | Start playing |
| `InitBuy` | Purchase / unlock full game |
| `InitRanking` | Show high scores |

### Win/lost screens
| Glyph | Description |
|---|---|
| `WinLostReturn` | Return to main menu |

### Trial paywall
| Glyph | Description |
|---|---|
| `TrialBuy` | Purchase full game |
| `TrialCancel` | Cancel / back |

### Settings
| Glyph | Description |
|---|---|
| `SetupSounds` | Toggle sounds |
| `SetupJump` | Toggle jump mode |
| `SetupZoom` | Toggle zoom level |
| `SetupAccel` | Toggle accelerometer |
| `SetupReset` | Reset all settings |
| `SetupReturn` | Close settings |

### Pause menu
| Glyph | Description |
|---|---|
| `PauseMenu` | Return to main menu from pause |
| `PauseBack` | Close pause overlay |
| `PauseSetup` | Open settings from pause |
| `PauseRestart` | Restart current level |
| `PauseContinue` | Continue / resume |

### In-game HUD
| Glyph | Description |
|---|---|
| `PlayPause` | Open pause menu |
| `PlayJump` | Jump (touch button) |
| `PlayAction` | Fire/action (touch button) |
| `PlayDown` | Look-down (touch button) |

### Resume/checkpoint
| Glyph | Description |
|---|---|
| `ResumeMenu` | Return to menu from resume screen |
| `ResumeContinue` | Continue from checkpoint |

### Ranking
| Glyph | Description |
|---|---|
| `RankingContinue` | Close ranking screen |

### Cheat unlock grid
| Glyph | Description |
|---|---|
| `Cheat11/12/21/22/31/32` | 6 buttons in 3×2 grid for entering the unlock gesture |
| `Cheat1`–`Cheat9` | 9 cheat action buttons (visible after unlock) |

---

## 18. Game Constants and Dimensions

Defined in `include/WindowsPhoneSpeedyBlupi/Def.hpp`.

### Viewport
| Constant | Value | Description |
|---|---|---|
| `LXIMAGE` | 640 | Game viewport width (pixels) |
| `LYIMAGE` | 480 | Game viewport height (pixels) |

### World grid
| Constant | Value | Description |
|---|---|---|
| `MAXCELX` | 100 | Max tile columns |
| `MAXCELY` | 100 | Max tile rows |

### Sprite dimensions
| Constant | Value | Description |
|---|---|---|
| `DIMOBJX` | 64 | Object sprite width (px) |
| `DIMOBJY` | 64 | Object sprite height (px) |
| `DIMBLUPIX` | 60 | Player sprite width (px) |
| `DIMBLUPIY` | 60 | Player sprite height (px) |

### Coordinate systems
- **Tile coordinates:** `[0, 100) × [0, 100)` integers
- **Game-space pixels:** tile_index × 64 (using `DIMOBJX`/`DIMOBJY`)
- **Screen-space:** game-space + scroll offset (managed by Pixmap via CNA)

---

## 19. World and Data Structures

### `Cellule` struct
The world is stored as two 100×100 grids of `Cellule` values:

```cpp
struct Cellule {
    short icon;  // sprite index into the background/decor sheet
};
```

- `m_decor[100][100]` — base tile layer (terrain, hazards, etc.)
- `m_bigDecor[100][100]` — secondary decorative layer

### `MoveObject` struct
Up to 200 simultaneously active moving objects (enemies, crates, projectiles, vehicles, effects):

| Field | Type | Description |
|---|---|---|
| `type` | ObjectType | Object type enum |
| `stepAdvance` | short | Steps when advancing |
| `stepRecede` | short | Steps when receding |
| `timeStopStart` | short | Time paused at start position |
| `timeStopEnd` | short | Time paused at end position |
| `posStart` | TinyPoint | Starting position |
| `posEnd` | TinyPoint | Target/end position |
| `posCurrent` | TinyPoint | Current position |
| `step` | short | Current animation step |
| `time` | short | Timer |
| `phase` | short | Current motion phase |
| `channel` | short | Sprite channel |
| `icon` | short | Sprite icon index |

### `ByeByeObject`
A vector of short-lived particle/fragment objects used for visual effects only. They expire automatically and never affect gameplay.

---

## 20. CDecor / Decor Class

`Decor` (`include/WindowsPhoneSpeedyBlupi/Decor.hpp`, `src/WindowsPhoneSpeedyBlupi/Decor.cpp`) is the ~9400+ line core of the gameplay simulation. It manages the world grid, all moving objects, the player character, physics, collision detection, and goal logic.

### Responsibilities split across Decor.cpp

The single `Decor.cpp` file handles everything:
- World initialization and reset (`InitDecor`, `InitGamer`)
- Per-frame update step (`MoveStep`)
- Player physics and input processing
- Vehicle logic (helicopter, jeep, tank, hovercraft, skateboard)
- Moving object AI (enemies, bombs, lifts, conveyors)
- Collision detection against tile icons (`IsLave()`, `IsDeepWater()`, `IsDoor()`, `IsBridge()`, etc.)
- Camera scrolling
- Rendering (`Build`, `DrawInfo`)
- Level load/save delegation to `Worlds`
- Goal/win/loss detection

### Player state fields (in Decor class)

| Field | Type | Description |
|---|---|---|
| `m_blupiX`, `m_blupiY` | int | Position in game pixels |
| `m_blupiAction` | BlupiAction | Current animation state |
| `m_blupiDir` | Direction | Facing direction |
| `m_blupiPhase` | int | Animation frame index (into Tables) |
| `m_blupiChannel` | PixmapChannel | Which sprite sheet variant to use |
| `m_blupiHelico` | bool | In helicopter |
| `m_blupiJeep` | bool | In jeep |
| `m_blupiTank` | bool | In tank |
| `m_blupiSkate` | bool | On skateboard |
| `m_blupiSurf` | bool | At water surface |
| `m_blupiNage` | bool | Swimming |
| `m_blupiOver` | bool | In hovercraft |
| `m_blupiInvert` | bool | Controls inverted |
| `m_blupiCloud` | bool | Cloud invisibility active |
| `m_blupiShield` | int | Shield timer (100 ticks when active) |
| `m_blupiPhantom` | int | Invisibility timer |
| `m_blupiCle` | DoorKeyFlags | Keys collected |
| `m_blupiBullet` | int | Tank ammo count |
| `m_blupiDynamite` | int | Dynamite count |

---

## 21. Game1 — Main Game Loop

`Game1` (inherits CNA's `Game` base class) owns all subsystems and drives the phase state machine:

### Subsystems

| Member | Type | Description |
|---|---|---|
| `graphics` | GraphicsDeviceManager | Display/window management |
| `pixmap` | `std::shared_ptr<IPixmap>` | Sprite rendering |
| `sound` | `std::shared_ptr<ISound>` | Audio playback |
| `decor` | `Decor` | Gameplay simulation |
| `inputPad` | `InputPad` | Touch/keyboard/accelerometer input |
| `gameData` | `GameData` | Persistent save data |
| `waitJauge` | `Jauge` | Loading progress gauge (Wait phase) |

### Lifecycle

1. `Initialize()` — platform/window setup
2. `LoadContent()` — asset loading via CNA content pipeline
3. `Update()` — per-frame logic: input, phase state machine, physics
4. `Draw()` — per-frame rendering with SpriteBatch

### Tables system

`Tables.cpp` contains a 2911-element static animation table mapping each `(BlupiAction, phase)` pair to a `(channel, icon, nextPhase)` tuple. This table drives all character animation without per-action if/else logic in `Decor.cpp`.

---

## 22. Save Data Format

`GameData` manages a flat 640-byte binary array written to the platform's persistent storage (IndexedDB on web, file system on desktop/Android).

**Layout:**

| Offset | Size | Content |
|---|---|---|
| 0–9 | 10 bytes | Header: version, selected gamer, settings (sound, jump, zoom, accel) |
| 10–219 | 210 bytes | Gamer slot A: lives, last world, 200-byte door state |
| 220–429 | 210 bytes | Gamer slot B |
| 430–639 | 210 bytes | Gamer slot C |

**Key accessors:**
- `getSelectedGamerProperty()` / `setSelectedGamerProperty()`
- `getNbViesProperty()` / `setNbViesProperty()`
- `getLastWorldProperty()` / `setLastWorldProperty()`
- Door states: per-level arrays tracking which doors have been opened

Serialized/deserialized via `Worlds::ReadGameData()` / `Worlds::WriteGameData()`.

---

## 23. World File Format

World levels are stored as **line-delimited text files** in `worlds/worldNNN.txt`.

**Numbering scheme:**

| Range | Chapter |
|---|---|
| `world001.txt` | Tutorial |
| `world010`–`world019` | Chapter 1 (levels 0–9) |
| `world020`–`world029` | Chapter 2 |
| `world030`–`world039` | Chapter 3 |
| ... | ... |
| `world090`–`world099` | Chapter 9 |
| `world100`–`world199` | Chapters 10–19 |
| `world199.txt` | Final bonus level |

**File structure:**
- First line: metadata section (`DescFile` fields: posDecor, dimDecor, world, music, region, blupiPos, blupiDir, name)
- Following lines: tile grid rows — 100 comma-separated icon indices per row × 100 rows
- `Decor` section: base tile map
- `BigDecor` section: secondary decorative layer
- `Doors` section: door state array (0=locked, 1=open, 1 is default and suppressed for compactness)

**Field format:**
- `name=value` pairs separated by spaces
- Type suffixes: none (int), `.0` (double), `True`/`False` (bool), `x;y` (TinyPoint), `,` (array separator)

---

## 24. Game Asset Files

### Content/backgrounds/ (37 PNG files)

**Level terrain backgrounds** (32 themes):
| File | Description |
|---|---|
| `decor000.png` | Ice / cavern theme |
| `decor001.png` | Forest / jungle theme |
| `decor002.png`–`decor031.png` | Additional themes (desert, city, space, etc.) |

**UI screens:**
| File | Description |
|---|---|
| `speedyblupi.png` | Speedy Blupi title screen |
| `blupiyoupie.png` | Blupi Youpie title screen |
| `init.png` | Main menu / gamer select background |
| `gear.png` | Settings screen background |
| `pause.png` | Pause screen overlay |
| `lost.png` | Level failure screen |
| `win.png` | Level completion screen |
| `trial.png` | Trial/paywall screen |
| `wait.png` | Loading screen |
| `setup.png` | Settings background |

**Content/backgrounds4x/**: 4× high-DPI upscaled versions of 35 of the above (used by ScaleResolution4).

### Content/icons/ (8 PNG files)

| File | Channel | Approx. size | Description |
|---|---|---|---|
| `blupi.png` | `Blupi` (2) | ~853 KB | Main player character (88 action sprites) |
| `blupi1.png` | `Blupi1_11` (11) | ~814 KB | Alternate player variant 1 |
| `button.png` | `Button` (4) | ~127 KB | On-screen UI buttons (40×40 px each) |
| `element.png` | `Element` (10) | ~536 KB | Collectibles, enemies, hazards |
| `explo.png` | `Explosion` (9) | ~910 KB | Explosions and visual effects |
| `object.png` | `Object` (1) | varies | Moving foreground objects |
| `jauge.png` | `Jauge` (5) | small | HUD gauge bar (124×22 px) |
| `text.png` | `Text` (6) | small | Bitmap font (32×32 px cells) |

**Content/icons4x/**: 4× upscaled versions (8 files).

### Content/sounds/ (92 WAV files)

`sound000.wav` through `sound091.wav`, ranging from ~3 KB to ~15 KB each. See [Section 11](#11-sound-effects-soundchannel) for the full mapping.

### worlds/ (78 level files)

`world001.txt` through `world199.txt`. See [Section 23](#23-world-file-format) for format details.

---

## 25. Cheat System

Documented in `documentation/Cheat System.md`.

### Unlock gesture

The cheat menu is hidden and unlocked by pressing the following 10-button sequence on the 3×2 cheat button grid:

```
Cheat12 → Cheat22 → Cheat32 → Cheat12 → Cheat11 →
Cheat21 → Cheat22 → Cheat21 → Cheat31 → Cheat32
```

### UI cheat buttons (Cheat1–Cheat9, mapped in Game1::CheatAction)

| Button | Cheat | Effect |
|---|---|---|
| Cheat1 | `OpenDoors` | Toggle all doors open/closed |
| Cheat2 | `SuperBlupi` | Toggle enhanced player abilities |
| Cheat3 | `ShowSecret` | Toggle display of hidden objects |
| Cheat4 | `LayEgg` | Set lives to 9 |
| Cheat5 | `Reset` | Reset all game data |
| Cheat6 | `TrialMode` | Toggle trial mode simulation |
| Cheat7 | `CleanAll` | Clear all objects from the map |
| Cheat8 | `AllTreasure` | Grant all treasures |
| Cheat9 | `EndGoal` | Instantly complete the level |

### Full cheat code list (Tables::CheatCodes, 22 total)

`BuildOfficialMissions`, `OpenDoors`, `CleanAll`, `SuperBlupi`, `LayEgg`, `KillEgg`, `Skate`, `Copter`, `Jeep`, `AllTreasure`, `EndGoal`, `ShowSecret`, `RoundShield`, `Lollipop`, `Bombs`, `BirdLime`, `Tank`, `PowerCharge`, `Drink`, `Overcraft`, `Dynamite`, `WeelKeys`

---

## 26. Localization

`MyResource.cpp` / `MyResource.hpp` contain localized UI strings in three languages:

| Language | Notes |
|---|---|
| French | Default (game's original language) |
| English | Full translation |
| German | Full translation |

Strings cover: level names, button labels, menu text, status messages, cheat code names.

---

## 27. Performance Notes

Four performance issues identified in `RAM.md`:

**Issue 1 — `ByeByeDraw` (`Decor.cpp:9252`)**
Object copying in the per-frame draw loop:
```cpp
// Current: copies 72-byte struct each iteration
for (ByeByeObject obj : byeByeObjects) { ... }
// Fix: use const reference
for (const ByeByeObject& obj : byeByeObjects) { ... }
```

**Issue 2 — `ByeByeAdd` (`Decor.cpp:9187–9206`)**
Double-copy before vector insertion. Fix: use `emplace_back` with direct field initialization.

**Issue 3 — `ByeByeStep` (`Decor.cpp:9241`)**
`vector::erase` in the middle of a vector is O(n) per removal during iteration. Fix: erase-remove idiom or deferred batch deletion.

**Issue 4 — `MoveObjectSort` (`Decor.cpp:9074–9108`)**
Manual bubble sort with manual struct copying: O(n²) with large constant factor. Fix: `std::sort` with a lambda comparator.

No unwanted heap allocations were found — `make_shared`/`make_unique` calls are all one-time initialization.

---

## 28. Doxygen Documentation Plan

`DOXYGEN_DOCUMENTATION_PLAN.md` tracks the Doxygen documentation status for all 50 source files.

### Priority levels

| Priority | Files |
|---|---|
| 1–4 (highest) | `Decor.cpp/hpp`, `Tables.cpp/hpp`, `Game1.cpp/hpp` — complex algorithms, animation table layout, phase state machine |
| 5–12 | `Pixmap`, `Worlds`, `GameData`, `InputPad`, `Sound`, decor/* headers |
| 13–21 | Remaining utilities: `MyResource`, `Misc`, `Text`, `Slider`, `Helper`, `TinyRect`, `TinyPoint`, `IPixmap`, `ISound`, `IGame1`, `ConfigDef`, `Config` |

### Conventions

- Every file: `@file`, `@brief`, `@details`
- Every class: `@class`, `@brief`, `@details`, `@note`, `@warning`, `@see`
- Every method: `@brief`, `@param[in/out]`, `@return`/`@retval`, `@throws`, `@pre`, `@post`, `@note`, `@warning`
- Member variables: trailing `///< @brief` inline comments
- `.cpp` files: document only non-trivial logic not already documented in the header

---

## 29. Known Issues and TODO

From `TODO.md`:

1. **Accelerometer visibility** — accelerometer setting button should only appear on hardware that has an accelerometer
2. **Sound system** — needs verification across all platforms
3. **Transparency** — some sprites have transparency handling issues
4. **Fullscreen mode** — currently malfunctions
5. **Web sound delay** — audio has noticeable startup delay in the Emscripten build
6. **Cheat name conflict** — `quick` and `quicklollypop` cheats clash; `quicklollypop` should be renamed

From `ENUMS.md`:
- Multiple places in the codebase still use raw integer magic numbers instead of the typed enums defined in `include/def/` — ongoing refactoring work.

From `DOXYGEN_DOCUMENTATION_PLAN.md`:
- Doxygen documentation is planned for all 50 source files but is not yet written.
