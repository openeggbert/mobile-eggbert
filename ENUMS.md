# Magic Number → Enum Refactoring Plan

This document lists groups of magic integer constants identified in the codebase that could be
replaced with named `enum class` types to improve readability and type safety.

No code has been changed. This is an analysis-only document.

---

## 1. Tile Icon Types — HIGH PRIORITY

**Suggested name:** `TileIconType` (or `DecorIcon`)  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp` (~50+ locations),
`include/WindowsPhoneSpeedyBlupi/Decor.hpp`

These numeric icon indices identify tile/decor sprites on the game map. They appear in predicate
methods (`IsLave`, `IsPiege`, `IsScie`, `IsDoor`, `IsTeleporte`, `IsBridge`, `IsVentillo`,
`IsRessort`, `IsEcraseur`, `IsBlitz`, `IsSurfWater`, `IsDeepWater`, `IsOutWater`, `IsNormalJump`,
`IsTemp`, `IsBridge`) and throughout `Build`, `MoveStep`, and collision logic.

| Value(s) | Suggested name | Description |
|---|---|---|
| 68 | `Lava` | Lava tile |
| 91 | `WaterShallow` | Shallow water / surf zone |
| 92 | `WaterDeep` | Deep water zone |
| 107–109 | `NormalJumpA`–`NormalJumpC` | Normal jump spring tiles |
| 110 | `FanLeft` | Fan blowing left |
| 114 | `FanRight` | Fan blowing right |
| 118 | `FanUp` | Fan blowing upward |
| 122 | `FanDown` | Fan blowing downward |
| 126–137 | `BarRail*` | Bar / rope / rail tiles |
| 158–165 | `Door*` | Coloured door tiles (plain, red, green, blue variants) |
| 166–173 | `Teleporter*` | Teleporter pad tiles |
| 174–181 | `DoorPortal*` | Door / portal tile variants |
| 182–183 | `DoorAnchor*` | Door frame / anchor markers |
| 184 | `DoorExtra` | Additional door-related tile |
| 203 | `Mine` | Marine / nautical mine |
| 211 | `Spring` | Spring (ressort) tile |
| 214 | `HiddenTile` | Hidden / secret tile |
| 246–249 | `Bridge*` | Bridge tiles |
| 251–260 | `Collectible*` | Collectible / item tiles |
| 264–289 | `CollectibleVariant*` | Collectible variants |
| 304 | `BlitzAnchor` | Lightning conductor anchor |
| 305 | `BlitzHazard` | Active lightning hazard |
| 309 | `DoorSpecial` | Special door variant |
| 317 | `Crusher` | Crusher (écraseur) hazard |
| 324 | `Temperature` | Temperature control tile |
| 330–336 | `TeleporterPair*` | Teleporter pair markers |
| 341–363 | `HazardZone*` | Lava / hazard zone tiles |
| 364 | `BridgeEndpoint` | Bridge endpoint tile |
| 373 | `SpikeTrap` | Spike trap (piège) |
| 378 | `SawBlade` | Saw blade (scie) |
| 384–385 | `BridgeOpen` / `BridgeClosed` | Bridge state tiles |
| 404–407 | `DripActive*` | Active drip hazard frames (see also group 4) |
| 410–420 | `DoorVariant*` | Door / portal variants |
| 421+ | `Treasure*` | Treasure / gold collectibles |

---

## 2. Door State — MEDIUM PRIORITY

**Suggested name:** `DoorState`  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`  
**Locations:** `MemorizeDoors`, `InitializeDoors`, `OpenDoor`, `OpenDoorsWin`, `AdaptDoors`

```cpp
enum class DoorState : intcs {
    Closed = 0,
    Open   = 1,
};
```

---

## 3. Terrain Type (Sound Environment) — MEDIUM PRIORITY

**Suggested name:** `TerrainType`  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`  
**Location:** `SoundEnviron` method (~lines 1436–1498)

Groups of tile icon ranges are compared to select which footstep / ambient sound to play. The
ranges map to these terrain categories:

| Icon range(s) | Terrain |
|---|---|
| 32–34, 41–47, 139–147 | `Grass` |
| 1–28, 78–90, 250–260, 311–316, 324–329 | `Stone` |
| 284–303, 341–363 | `Lava` |
| 215–234 | `Water` |
| 246–249 | `Bridge` (wood) |
| 107–109 | `Spring` |
| 338 | `SpecialHazard` |

```cpp
enum class TerrainType {
    Grass,
    Stone,
    Lava,
    Water,
    Bridge,
    Spring,
    SpecialHazard,
};
```

---

## 4. Drip Hazard Frame Range — LOW PRIORITY

**Suggested name:** `DripHazardFrame`  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`  
**Location:** `Build` method (~lines 1044–1052)

```cpp
enum class DripHazardFrame : intcs {
    ActiveStart = 404,
    ActiveEnd   = 407,
};
```

---

## 5. Blitz (Lightning) Cycle Phases — LOW PRIORITY

**Suggested name:** `BlitzCycle`  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`  
**Location:** `BlitzActif` method (~lines 623–631)

```cpp
enum class BlitzCycle : intcs {
    Length          = 100, ///< Total cycle length in ticks.
    ActiveThreshold =  50, ///< Ticks below this value are in the active (danger) half.
};
```

Specific "on" frame indices within the cycle (0, 7, 18, 25, 33, 44) could also be named if the
pattern is understood; currently their meaning is unclear from context alone.

---

## 6. Shield Animation Thresholds — LOW PRIORITY

**Suggested name:** `ShieldAnimation`  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`  
**Location:** `Build` method (~lines 785–831)

```cpp
enum class ShieldAnimation : intcs {
    HighTimeThreshold     = 25, ///< Timer value above which shield renders differently.
    CycleDivisor          =  4, ///< Animation frame cycle divisor.
    FrameToggleThreshold  =  2, ///< Frame index below which shield sprite is visible.
};
```

---

## 7. Hazard Render Y-Offsets — LOW PRIORITY

**Suggested name:** `HazardRenderOffset`  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`  
**Location:** `Build` method (~lines 727–733)

Vertical pixel adjustments applied when drawing specific hazard tile types:

| Icon range | Offset | Hazard |
|---|---|---|
| 66–68 | −13 px | Marine mine tiles |
| 87–89 | −2 px | Other hazard variant |

```cpp
enum class HazardRenderOffset : intcs {
    Mine  = -13,
    Other =  -2,
};
```

---

## 8. Power Effect Icon Offsets — LOW PRIORITY

**Suggested name:** `PowerEffectIcon`  
**Files affected:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`  
**Location:** `Build` method (~lines 811–815)

```cpp
enum class PowerEffectIcon : intcs {
    BaseOffset  = 48, ///< Base sprite icon index for power-up overlays.
    CycleLength =  6, ///< Number of frames in the power-up animation cycle.
};
```

---

## Implementation Notes

- All new enums should live under `include/WindowsPhoneSpeedyBlupi/decor/` alongside the existing
  `DecorAction.hpp`, `DoorKeyFlags.hpp`, and `ObjectType.hpp`.
- Use `enum class` (scoped) with an explicit underlying type of `intcs` (the project's `int32_t`
  alias from SharpRuntime) to match existing conventions.
- `TileIconType` (group 1) is the highest-value change: one enum replaces ~50 scattered literals
  across `Decor.cpp` and would make the predicate methods (`IsLave`, `IsDoor`, etc.) self-
  documenting.
- Groups 6–8 are minor quality improvements; only worth doing alongside a larger refactor of the
  surrounding `Build` method.
