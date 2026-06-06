# Mobile Eggbert — Claude Code Guidelines

## Project Overview

**Mobile Eggbert** is a C++ port of *Speedy Blupi*, originally a Windows Phone XNA game (2013).
The port path was: original C# → decompiled with ILSpy → migrated to MonoGame → migrated to C++
→ migrated from MonoGame to **CNA**.

**CNA** is an XNA 4.0-compatible C++ framework built on SDL3. It lives at:
```
/rv/data/development/github.com/openeggbert/cna
```

**SharpRuntime** is the C++ reimplementation of .NET base-class types (System.Math, System.String,
EventHandler, IDisposable, primitive type aliases, etc.). It lives at:
```
/rv/data/development/github.com/openeggbert/sharp-runtime
```

For CNA-specific coding rules (namespaces, XNA API compliance, C# property conventions, type
aliases, events, visibility mapping, etc.) see:
```
/rv/data/development/github.com/openeggbert/cna/CLAUDE.md
```

---

## SharpRuntime Dependency Rule

**If something needed by this project or by CNA does not yet exist in sharp-runtime, it must be
added to sharp-runtime — not worked around in-place.**

This applies to:
- .NET primitive type aliases (`bytecs`, `intcs`, `String`, etc.)
- `System.*` classes or interfaces (`IDisposable`, `Math`, `Random`, `EventHandler<T>`, …)
- Any BCL behaviour required for correct XNA API compliance

Add a minimal, correctly-named stub or full implementation in the appropriate
`include/System/` or `src/System/` path inside the sharp-runtime repo. Never use a raw C++ type
as a substitute on the XNA API surface.

---

## Build

Default debug build:
```bash
cmake --build cmake-build-debug --target WindowsPhoneSpeedyBlupi
```

Vulkan build:
```bash
cmake --build cmake-build-vulkan --target WindowsPhoneSpeedyBlupi
```

Submodules must be initialised before first build:
```bash
git submodule update --init --recursive
```

---

## Source Layout

```
include/WindowsPhoneSpeedyBlupi/   — all .hpp headers
  decor/                           — DecorAction, DoorKeyFlags, ObjectType
  def/                             — BlupiAction, Direction, GameSpeed, …
src/WindowsPhoneSpeedyBlupi/       — all .cpp implementations
```

Key classes:

| File | Role |
|---|---|
| `Game1` | Top-level XNA game; phase state machine; owns all subsystems |
| `Decor` | Full gameplay simulation (tiles, Blupi, objects, collision, AI) |
| `Pixmap` / `IPixmap` | Sprite rendering (SDL3 SpriteBatch wrapper) |
| `Sound` / `ISound` | Audio playback |
| `InputPad` | Touch / keyboard / accelerometer input |
| `GameData` | Save-data serialisation (flat byte array, 3 gamer slots) |
| `Tables` | Static game-data tables (animation sequences, movement, etc.) |
| `Worlds` | Level and save-game file I/O |

---

## Doxygen Documentation

Every `.hpp` and `.cpp` file must have complete Doxygen documentation.
Full conventions (tags, style, per-item rules) are in:
```
DOXYGEN_DOCUMENTATION_PLAN.md
```

Summary:
- Every file: `@file` + `@brief` + `@details`
- Every class: `@class`, `@brief`, `@details`, `@note`, `@warning`, `@see`
- Every method: `@brief`, `@param[in/out]` (always directional), `@return`/`@retval`, `@throws`, `@pre`, `@post`, `@note`, `@warning`
- Every member variable: trailing `///<` with `@brief`
- `.cpp` files: only document what is **not** already in the `.hpp` (algorithms, internal helpers, non-obvious invariants)

---

## Code Rules

- Do not change XNA API names or signatures — match CNA / FNA exactly.
- Do not add features or abstractions beyond what the task requires.
- Do not add comments that explain *what* the code does — only *why* when non-obvious.
- `TinyRect` has a **non-standard field order**: `Left, Right, Top, Bottom` (not `Left, Top, Right, Bottom`).
- `Config.hpp` controls LEGACY vs MODERN mode — changes there affect timing, resolution, and feature flags globally.
