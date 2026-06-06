# Doxygen Documentation Plan — mobile-eggbert

**Goal:** Every `.hpp` and `.cpp` file in the project must have complete, accurate English Doxygen documentation.
`.cpp` files only receive Doxygen when the implementation contains non-trivial logic not already documented in the corresponding `.hpp` (e.g. complex algorithms, non-obvious side effects, internal helpers without a header counterpart).

The project is a C++ port of the *Speedy Blupi* game (originally an XNA/Windows Phone title written in C#), using the CNA (C-to-Native Application) framework that maps XNA idioms to C++ equivalents.

---

## Legend

| Status | Meaning |
|---|---|
| `NONE` | No Doxygen comments at all |
| `MINIMAL` | A few comments, mostly noise |
| `PARTIAL` | Some classes/methods documented, many gaps |
| `GOOD` | Most public API documented, minor gaps remain |
| `EXCELLENT` | Fully documented, only fine-tuning needed |

---

## Header Files (.hpp)

### 1. `include/WindowsPhoneSpeedyBlupi/ConfigDef.hpp`
**Status:** EXCELLENT  
**Description:** Compile-time enumerations for resolution scale (`ResolutionScale`) and target frame-rate (`Fps`), plus constants for the default resolution and FPS. Used by `Config.hpp` to parameterise the legacy/modern runtime modes.  
**Work needed:** Verify every enumerator has a doc-comment; add a `@file` and `@brief` block if missing.

---

### 2. `include/WindowsPhoneSpeedyBlupi/Config.hpp`
**Status:** EXCELLENT  
**Description:** Central compile-time configuration struct. Selects between LEGACY mode (pixel-perfect recreation of the original Windows Phone behaviour) and MODERN mode (extended resolution, higher FPS, debug overlays). Provides `ScaleTime`, `ScaleDiv`, and `ScaleAsset` helpers that translate original 30 FPS timing values to the configured FPS.  
**Work needed:** Confirm all template/constexpr helpers carry `@brief`; check that LEGACY vs MODERN conditional blocks are clearly explained.

---

### 3. `include/WindowsPhoneSpeedyBlupi/Decor.hpp`
**Status:** EXCELLENT  
**Description:** The heart of the gameplay simulation. Owns the 100×100 tile map, the Blupi player state, all moving objects, doors, switches, teleporters, lifts, and hazards. Drives collision detection, animation sequencing, viewport scrolling, particle effects, and the per-frame update tick.  
**Work needed:** Very large file (~1 200 lines); audit that every public method has a `@param`/`@return`; add `@note` tags for any magic-number fields; verify `@file` block accuracy.

---

### 4. `include/WindowsPhoneSpeedyBlupi/decor/DecorAction.hpp`
**Status:** PARTIAL  
**Description:** Enum `DecorAction` listing the tile animation actions that can be applied to background tiles: `None`, `SmallShake`, `BigShake`, `ElectricShake`. Also provides stream-compatible comparison and conversion operators.  
**Work needed:** Add `@file` + `@brief` block; add a doc-comment to every enumerator explaining when the game engine triggers that action.

---

### 5. `include/WindowsPhoneSpeedyBlupi/decor/DoorKeyFlags.hpp`
**Status:** PARTIAL  
**Description:** Bitmask enum `DoorKeyFlags` representing which key types a door requires or a player holds (`None`, `Key1`, `Key2`, `Key3`, `All`). Provides bitwise OR/AND/XOR operators.  
**Work needed:** Add `@file` + `@brief` block; document each flag value and each operator overload; explain how the flags integrate with the door-unlock logic in `Decor`.

---

### 6. `include/WindowsPhoneSpeedyBlupi/decor/ObjectType.hpp`
**Status:** GOOD  
**Description:** Enum `ObjectType` mapping integer IDs (0–203) to every possible moving object in the game: enemies, crates, projectiles, collectibles, lifts, and special effects. The IDs are inherited from the original game's numeric magic constants.  
**Work needed:** Replace the existing TODO comment with grouped doc-comments that describe each object category (enemies, pickups, hazards, etc.); add `@file`/`@brief`.

---

### 7. `include/WindowsPhoneSpeedyBlupi/Def.hpp`
**Status:** GOOD  
**Description:** Static utility class `Def` holding game-wide constants (screen dimensions, maximum channel counts, animation limits) and the `ButtonGlyph` enumeration for on-screen touch button icons.  
**Work needed:** Ensure every constant has a `@brief`; add `@note` where the value originates from the original Windows Phone screen spec.

---

### 8. `include/WindowsPhoneSpeedyBlupi/def/BlupiAction.hpp`
**Status:** EXCELLENT  
**Description:** Enum `BlupiAction` with 88 player animation/state entries (`Stop`, `March`, `Jump`, `Helico`, `Tank`, `Shield`, `Dead`, …). Each value maps to a sprite animation sequence in the sprite sheet.  
**Work needed:** Verify every enumerator has an individual doc-comment; add cross-references to `Tables.hpp` animation table where relevant.

---

### 9. `include/WindowsPhoneSpeedyBlupi/def/ContinueMission.hpp`
**Status:** GOOD  
**Description:** Enum `ContinueMission` for the three states of a "continue level" request flow: `None` (no request), `Pending` (player pressed continue), `Active` (loading is in progress).  
**Work needed:** Minor — add `@file` block; confirm state-transition semantics are described.

---

### 10. `include/WindowsPhoneSpeedyBlupi/def/Direction.hpp`
**Status:** GOOD  
**Description:** Enum `Direction` for the player's horizontal facing (`None`, `Left`, `Right`). Used throughout `Decor` and `BlupiAction` to determine sprite mirroring.  
**Work needed:** Add `@file` block; note that `None` is the initial/unset state.

---

### 11. `include/WindowsPhoneSpeedyBlupi/def/GameSpeed.hpp`
**Status:** GOOD  
**Description:** Enum `GameSpeed` with five multiplier levels (`Slow` … `Fastest`) and a helper `FKeyToGameSpeed()` that converts function-key presses (F1–F5) to the corresponding speed level.  
**Work needed:** Add `@file` block; document the multiplier ratios for each level; document `FKeyToGameSpeed` with `@param`/`@return`.

---

### 12. `include/WindowsPhoneSpeedyBlupi/def/KeyPressFlags.hpp`
**Status:** GOOD  
**Description:** Bitmask enum `KeyPressFlags` representing the three active input signals (`Jump`, `Fire`, `Down`). Aggregated by `InputPad` and consumed by `Decor`.  
**Work needed:** Add `@file` block; describe how each flag maps to physical keys and touch buttons.

---

### 13. `include/WindowsPhoneSpeedyBlupi/def/PixmapChannel.hpp`
**Status:** EXCELLENT  
**Description:** Enum `PixmapChannel` identifying each of the 17 sprite-sheet/texture-atlas slots used by the rendering system (Object, Blupi, Background, UI layers, etc.).  
**Work needed:** Confirm channel numbers match the actual asset loading order in `Pixmap`; verify all 17 values are documented.

---

### 14. `include/WindowsPhoneSpeedyBlupi/def/SecretPower.hpp`
**Status:** GOOD  
**Description:** Enum `SecretPower` for the five power-up bonus states (`None`, `Shield`, `Power`, `Cloud`, `Hide`) obtainable via cheat codes or pickups.  
**Work needed:** Add `@file` block; describe the in-game effect of each power.

---

### 15. `include/WindowsPhoneSpeedyBlupi/def/SoundChannel.hpp`
**Status:** GOOD  
**Description:** Enum `SoundChannel` mapping 93 symbolic names to audio asset file indices. Used by `Sound` and `Decor` to request specific sound effects.  
**Work needed:** Add `@file` block; group channels by category (footstep, jump, enemy, UI) with `@note` comments.

---

### 16. `include/WindowsPhoneSpeedyBlupi/def/Zoom.hpp`
**Status:** GOOD  
**Description:** Enum `Zoom` for the cheat-mode zoom levels (`Zoom100`, `Zoom50`, `Zoom25`, `Zoom12`), available only in MODERN mode.  
**Work needed:** Add `@file` block; note that this feature is disabled in LEGACY mode.

---

### 17. `include/WindowsPhoneSpeedyBlupi/Game1.hpp`
**Status:** EXCELLENT  
**Description:** Top-level game class inheriting XNA `Game`. Owns and wires together all major subsystems (`Pixmap`, `Sound`, `Decor`, `InputPad`, `GameData`). Drives the phase state machine (boot → logo → menu → gameplay → pause → game-over) and the XNA `Initialize` / `LoadContent` / `Update` / `Draw` lifecycle.  
**Work needed:** Verify every phase enum value and every subsystem pointer is documented; check that the state-machine transitions are described.

---

### 18. `include/WindowsPhoneSpeedyBlupi/GameData.hpp`
**Status:** GOOD  
**Description:** Manages persistent save data for up to 3 gamer slots. Stores door-open state, remaining lives, world progress, and settings (music/sound volume, accelerometer sensitivity). Serialises to a flat byte array that mirrors the original Windows Phone save format for compatibility.  
**Work needed:** Document every field; add `@note` on byte-layout compatibility constraint; document save-slot indexing.

---

### 19. `include/WindowsPhoneSpeedyBlupi/Helper.hpp`
**Status:** GOOD  
**Description:** Stateless utility providing `formatString()`, a lightweight printf-style formatter that replaces `{0}`, `{1}`, … placeholders with variadic string arguments.  
**Work needed:** Add `@file` block; document the placeholder syntax and behaviour for out-of-range indices.

---

### 20. `include/WindowsPhoneSpeedyBlupi/IGame1.hpp`
**Status:** GOOD  
**Description:** Abstract interface for `Game1`, allowing subsystems to call back into the top-level game object without a circular dependency. Exposes the graphics device, content manager, and game-mode query flags.  
**Work needed:** Document every virtual method; clarify the ownership semantics of returned pointers.

---

### 21. `include/WindowsPhoneSpeedyBlupi/InputPad.hpp`
**Status:** GOOD  
**Description:** Handles all player input sources: capacitive touch, keyboard, and accelerometer. Translates raw input into `KeyPressFlags` bitmasks. Also renders the on-screen virtual gamepad overlay and manages the in-game cheat-code menu.  
**Work needed:** Document the accelerometer dead-zone logic; document MODERN-only features (persistent cheats, debug overlay toggle).

---

### 22. `include/WindowsPhoneSpeedyBlupi/IPixmap.hpp`
**Status:** EXCELLENT  
**Description:** Pure-virtual interface for the sprite rendering subsystem. Declares all draw operations (sprite, filled rectangle, frame), the sprite-batch lifecycle (`BeginSpriteBatch` / `EndSpriteBatch`), texture-atlas loading, and viewport queries.  
**Work needed:** Minor audit only — confirm every method has `@param`/`@return`.

---

### 23. `include/WindowsPhoneSpeedyBlupi/ISound.hpp`
**Status:** EXCELLENT  
**Description:** Pure-virtual interface for the audio subsystem. Declares sound-effect playback with volume and balance parameters, looping audio, and master volume control.  
**Work needed:** Minor audit only.

---

### 24. `include/WindowsPhoneSpeedyBlupi/Jauge.hpp`
**Status:** EXCELLENT  
**Description:** Rectangular HUD gauge widget (124×22 px). Tracks a numeric level, a display mode (`Empty`, `Red`, `Blue`, `Yellow`), and visibility. Used for energy, time, and key indicators in the gameplay HUD.  
**Work needed:** Verify mode semantics (what `Red` vs `Blue` means gameplay-wise) are documented.

---

### 25. `include/WindowsPhoneSpeedyBlupi/Misc.hpp`
**Status:** EXCELLENT  
**Description:** Static utility class for 2D geometry: rectangle intersection/union, point rotation around an origin, and angle-to-direction conversion. Used throughout `Decor` and `Pixmap`.  
**Work needed:** Minor audit — confirm `@param`/`@return` on every method.

---

### 26. `include/WindowsPhoneSpeedyBlupi/MyResource.hpp`
**Status:** PARTIAL  
**Description:** Localised UI string resource table. Maps integer resource IDs to strings in French, English, and German. Lazy-initialised on first access.  
**Work needed:** Add `@file` + class `@brief`; document the language-selection logic; add brief `@brief` to every resource constant.

---

### 27. `include/WindowsPhoneSpeedyBlupi/Pixmap.hpp`
**Status:** GOOD  
**Description:** Concrete rendering implementation. Manages texture-atlas loading, the XNA `SpriteBatch`, viewport rectangle, and optional hotspot-zoom transforms. Implements `IPixmap`.  
**Work needed:** Document the zoom/hotspot maths; ensure every field has a `@brief`; document the interaction between `BeginSpriteBatch`/`EndSpriteBatch` and draw calls.

---

### 28. `include/WindowsPhoneSpeedyBlupi/Slider.hpp`
**Status:** GOOD  
**Description:** Horizontal UI slider widget. Used in the settings screen to adjust accelerometer sensitivity. Handles touch drag interaction and renders the track and thumb.  
**Work needed:** Add `@file` block; document value range and default; document touch-input handling.

---

### 29. `include/WindowsPhoneSpeedyBlupi/Sound.hpp`
**Status:** GOOD  
**Description:** Concrete audio subsystem implementation. Manages sound-effect asset loading, per-channel playback with volume and stereo-balance lookup tables, and looping support.  
**Work needed:** Document the volume/balance lookup table structure; document channel-conflict resolution (what happens when all channels are occupied).

---

### 30. `include/WindowsPhoneSpeedyBlupi/Tables.hpp`
**Status:** EXCELLENT  
**Description:** Static repository of all original game data tables: the 2 911-element Blupi animation sequence table, movement/velocity data, tile-adaptation maps, and cheat-code strings. These tables are a direct port from the original C# source.  
**Work needed:** Add cross-references between the tables and the `Decor`/`BlupiAction` consumers; confirm the column layout of the animation table is documented.

---

### 31. `include/WindowsPhoneSpeedyBlupi/Text.hpp`
**Status:** GOOD  
**Description:** Static utility class for bitmap text rendering. Maps character codes to glyph rectangles on the font sprite sheet, handles proportional character spacing, and supports centred, left-aligned, and slanted text styles.  
**Work needed:** Document the font sprite-sheet layout; clarify the slant transform used for italic-style text.

---

### 32. `include/WindowsPhoneSpeedyBlupi/TinyPoint.hpp`
**Status:** GOOD  
**Description:** Lightweight 2D integer point `(X, Y)`. Provides `ToString()` for serialisation. A dependency-free replacement for XNA's `Point` struct.  
**Work needed:** Add `@file` block; document operator overloads if present.

---

### 33. `include/WindowsPhoneSpeedyBlupi/TinyRect.hpp`
**Status:** GOOD  
**Description:** Integer rectangle struct with `(Left, Right, Top, Bottom)` storage order (note: non-standard order). Provides computed `Width` / `Height` properties and `ToString()`. A replacement for XNA's `Rectangle`.  
**Work needed:** Add `@file` block; prominently document the non-standard field order so readers do not confuse it with `(Left, Top, Right, Bottom)`.

---

### 34. `include/WindowsPhoneSpeedyBlupi/Worlds.hpp`
**Status:** GOOD  
**Description:** Static helpers for world and save-game file I/O. Reads and writes level data files, parsing typed fields (int, bool, double, `TinyPoint`, door state) from a line-based text format.  
**Work needed:** Document the file format grammar; add `@param`/`@return` to every parse helper; document error behaviour for malformed input.

---

## Source Files (.cpp)

> **Rule:** Only add or improve Doxygen in `.cpp` files for logic that is not already explained in the matching `.hpp`, i.e. non-trivial algorithms, internal helper functions/lambdas without a header declaration, or important implementation invariants.

---

### 35. `src/WindowsPhoneSpeedyBlupi/Program.cpp`
**Status:** NONE  
**Description:** Application entry point. Creates a `Game1` instance and calls `Run()`. Wraps execution in a top-level exception handler that logs unhandled exceptions before re-throwing.  
**Work needed:** Add `@file` block; add a `@brief` to the `main()` function explaining exception handling and logging strategy.

---

### 36. `src/WindowsPhoneSpeedyBlupi/Decor.cpp`
**Status:** PARTIAL  
**Description:** Implementation of the full gameplay simulation. Contains the most complex logic in the project: per-tile physics and collision, per-object AI state machines, player action transitions, lift/teleporter mechanics, and particle emission.  
**Work needed:** Add `@file` block. For each non-trivial private/internal function that has no declaration in `Decor.hpp`, add a short `@brief`. Document any algorithm that would surprise a reader (e.g. how tile-overlap collision resolves, how the teleporter pairing table is indexed).

---

### 37. `src/WindowsPhoneSpeedyBlupi/Game1.cpp`
**Status:** PARTIAL  
**Description:** Implements the XNA lifecycle (`Initialize`, `LoadContent`, `Update`, `Draw`) and the top-level phase state machine. Manages transitions between boot, logo display, main menu, gameplay, pause, and game-over screens.  
**Work needed:** Add `@file` block. Document the phase transition graph in a `@details` block on the class or in a file-level comment. Add `@brief` to any internal-only helper lambdas or static functions.

---

### 38. `src/WindowsPhoneSpeedyBlupi/Helper.cpp`
**Status:** MINIMAL  
**Description:** Implements `formatString()`. Iterates the format string, detects `{N}` tokens, and substitutes the Nth argument from the variadic list.  
**Work needed:** Add `@file` block. The algorithm is simple but note any edge cases (e.g. `{N}` where N ≥ argument count) in an `@note`.

---

### 39. `src/WindowsPhoneSpeedyBlupi/GameData.cpp`
**Status:** MINIMAL  
**Description:** Property accessors and (de)serialisation for the save-data byte array. Each property getter/setter computes a byte offset from the slot index and field layout.  
**Work needed:** Add `@file` block. Add one `@note` documenting the byte-array layout schema (field names, offsets, sizes) so future changes do not silently corrupt saves.

---

### 40. `src/WindowsPhoneSpeedyBlupi/InputPad.cpp`
**Status:** PARTIAL  
**Description:** Touch-region hit testing, keyboard scan-code mapping, accelerometer axis normalisation, and virtual-gamepad draw calls.  
**Work needed:** Add `@file` block. Document the accelerometer dead-zone and sensitivity curve. Document any non-obvious touch-region coordinate constants.

---

### 41. `src/WindowsPhoneSpeedyBlupi/Pixmap.cpp`
**Status:** PARTIAL  
**Description:** Loads PNG texture atlases via the CNA content pipeline, manages the XNA `SpriteBatch`, computes source/destination rectangles for every draw call, and applies the hotspot-zoom transform.  
**Work needed:** Add `@file` block. Document the zoom-transform maths (how viewport coordinates map to screen coordinates under zoom). Document any batching invariant (e.g. why `BeginSpriteBatch` must not be called while a batch is already open).

---

### 42. `src/WindowsPhoneSpeedyBlupi/Sound.cpp`
**Status:** PARTIAL  
**Description:** Loads `.wav` assets, manages per-sound playback state, applies per-channel volume from the lookup table, and routes balance (stereo panning) based on object X position.  
**Work needed:** Add `@file` block. Document the volume lookup table format. Document the panning formula if it is non-trivial.

---

### 43. `src/WindowsPhoneSpeedyBlupi/Jauge.cpp`
**Status:** MINIMAL  
**Description:** Draws the gauge widget by selecting the correct sprite from the HUD atlas based on level and mode. Includes redraw-dirty optimisation to skip redundant draw calls.  
**Work needed:** Add `@file` block. Note the redraw-dirty optimisation logic with a brief `@note` so it is not accidentally removed.

---

### 44. `src/WindowsPhoneSpeedyBlupi/Misc.cpp`
**Status:** NONE  
**Description:** Implements the static geometry helpers: rectangle intersection test, union expansion, and integer point rotation using a lookup table of sin/cos values.  
**Work needed:** Add `@file` block. Document the rotation lookup table — its range, resolution, and how indices map to angles in degrees.

---

### 45. `src/WindowsPhoneSpeedyBlupi/MyResource.cpp`
**Status:** MINIMAL  
**Description:** Defines the static string arrays for each locale and populates the resource map on first access via `std::call_once` or equivalent lazy initialisation.  
**Work needed:** Add `@file` block. Document the supported locales and how the active locale is selected.

---

### 46. `src/WindowsPhoneSpeedyBlupi/Slider.cpp`
**Status:** MINIMAL  
**Description:** Renders the slider track and thumb at the correct position, handles touch-drag to update the value, and clamps the value to `[min, max]`.  
**Work needed:** Add `@file` block. Document the drag-to-value mapping and the clamping behaviour.

---

### 47. `src/WindowsPhoneSpeedyBlupi/Tables.cpp`
**Status:** MINIMAL  
**Description:** Defines the large static data arrays declared in `Tables.hpp`. The most significant is the 2 911-row Blupi animation table.  
**Work needed:** Add `@file` block. Add a top-of-file comment describing each table's column layout (what each column index means) — this is the only place that explanation belongs, and it is currently missing.

---

### 48. `src/WindowsPhoneSpeedyBlupi/Text.cpp`
**Status:** MINIMAL  
**Description:** Implements character-to-glyph lookup and the draw loop that blits individual characters from the font sprite sheet with proportional spacing.  
**Work needed:** Add `@file` block. Document the glyph coordinate table structure; note the proportional-width override table if one exists.

---

### 49. `src/WindowsPhoneSpeedyBlupi/TinyRect.cpp`
**Status:** NONE  
**Description:** Implements the computed `Width` and `Height` properties of `TinyRect` (i.e. `Right - Left` and `Bottom - Top`).  
**Work needed:** Add `@file` block. The implementations are trivial; no method-level docs needed beyond what the header already has, but note the non-standard field order once more for readers who open the `.cpp` directly.

---

### 50. `src/WindowsPhoneSpeedyBlupi/Worlds.cpp`
**Status:** PARTIAL  
**Description:** Implements file open/close, line-by-line parsing, and typed field extraction for the world-data file format.  
**Work needed:** Add `@file` block. Document the file format grammar (line structure, key=value syntax, type suffixes) in a `@details` block. Document error/fallback behaviour for missing or malformed fields.

---

## Prioritised Order of Work

Files are ordered from highest impact / most documentation debt to lowest.

| Priority | File | Reason |
|---|---|---|
| 1 | `Decor.cpp` | Largest, most complex, most partial documentation |
| 2 | `Decor.hpp` | Core API, needs full audit of all ~1 200 lines |
| 3 | `Tables.cpp` / `Tables.hpp` | Column layout of animation tables completely undocumented |
| 4 | `Game1.cpp` / `Game1.hpp` | Phase state machine transitions not described |
| 5 | `Pixmap.cpp` | Zoom-transform maths undocumented |
| 6 | `Worlds.cpp` / `Worlds.hpp` | File format grammar missing |
| 7 | `GameData.cpp` / `GameData.hpp` | Byte-layout schema undocumented |
| 8 | `InputPad.cpp` / `InputPad.hpp` | Accelerometer curve undocumented |
| 9 | `Sound.cpp` / `Sound.hpp` | Volume table format undocumented |
| 10 | `decor/DecorAction.hpp` | Missing `@file`, per-enumerator docs |
| 11 | `decor/DoorKeyFlags.hpp` | Missing `@file`, operator docs |
| 12 | `decor/ObjectType.hpp` | Per-category grouping needed |
| 13 | `MyResource.hpp` / `MyResource.cpp` | Locale selection logic undocumented |
| 14 | `Misc.cpp` | Rotation table undocumented |
| 15 | `Text.cpp` / `Text.hpp` | Glyph table layout undocumented |
| 16 | `Jauge.cpp` | Dirty-flag optimisation note missing |
| 17 | `Slider.cpp` / `Slider.hpp` | Drag-to-value mapping undocumented |
| 18 | `Helper.cpp` / `Helper.hpp` | Edge-case note missing |
| 19 | `Program.cpp` | Entry-point and exception handler |
| 20 | `TinyRect.cpp` / `TinyRect.hpp` | Non-standard field order warning missing |
| 21 | All remaining GOOD/EXCELLENT files | Audit pass only — fill any gap found |

---

## Conventions to Follow

Use the richest Doxygen markup that is meaningful for the item being documented. Every applicable tag listed below should be used when it adds real value; omit it only when it would be redundant or trivially obvious.

### File block (every `.hpp` and `.cpp`)

```cpp
/**
 * @file   FileName.hpp
 * @brief  One-line summary of the file's purpose.
 * @details
 *   Extended description: what the file provides, its role in the
 *   architecture, and any design constraints worth knowing.
 * @author Original author / porter
 * @date   Year first created or ported
 */
```

### Class / struct

```cpp
/**
 * @class  ClassName
 * @brief  One-line summary.
 * @details
 *   Extended description: responsibilities, ownership model, lifecycle,
 *   and any invariants the caller must respect.
 * @note   Non-obvious constraint or quirk.
 * @warning Precondition that causes UB or data corruption if violated.
 * @see    RelatedClass, relatedFunction()
 */
```

### Method / function

```cpp
/**
 * @brief  One-line summary.
 * @details
 *   Longer description when the behaviour is non-trivial.
 * @param[in]  paramName  What this input represents; valid range if relevant.
 * @param[out] paramName  What is written; ownership / lifetime.
 * @param[in,out] paramName  Both read and written; contract.
 * @return Description of the return value and its valid range.
 * @retval 0   Specific meaning of a particular return value (use when applicable).
 * @throws std::runtime_error  When and why this exception is thrown.
 * @pre    Precondition that must hold before the call.
 * @post   Guaranteed state after a successful call.
 * @note   Non-obvious side-effect or implementation detail.
 * @warning Dangerous edge case or UB condition.
 * @see    RelatedFunction(), AnotherClass
 * @todo   Known gap or planned improvement (use sparingly).
 */
```

### Enum

```cpp
/**
 * @enum  EnumName
 * @brief One-line summary.
 * @details Extended description of when and how the enum is used.
 */
enum class EnumName {
    Value1, ///< @brief Brief meaning; when the engine sets this value.
    Value2, ///< @brief Brief meaning.
};
```

### Struct field / class member variable

```cpp
int m_field; ///< @brief What this field holds; valid range or units.
```

For fields with complex invariants, use a full block comment instead of a trailing `///<`.

### Operator overload

Document every non-obvious operator the same way as a regular function, with `@param` and `@return`.

### Template parameter

```cpp
/**
 * @tparam T  Constraint on T (e.g. must satisfy CopyConstructible).
 */
```

---

### General rules

- Do **not** duplicate in `.cpp` what is already documented in the matching `.hpp`.
- `@param[in]` / `@param[out]` / `@param[in,out]` — always use the directional qualifier.
- `@return` vs `@retval` — use `@return` for a general description; add `@retval` lines for specific sentinel values (`-1`, `nullptr`, `false`).
- `@throws` — document every exception type the function can propagate, including those from callees if they are part of the contract.
- `@pre` / `@post` — use for non-trivial contracts; skip when the precondition is obvious from parameter types.
- `@note` — non-obvious invariants, implementation quirks, portability constraints.
- `@warning` — preconditions whose violation causes UB, data corruption, or security issues.
- `@see` — cross-reference closely related classes and functions.
- `@todo` — known gaps acceptable only during the porting phase; must not remain in finished files.
- Write in plain English; avoid abbreviations; use third-person present tense for `@brief` ("Returns …", "Loads …", "Computes …").
