# Plan: Vendor CNA + SharpRuntime, prune, strip backends, downgrade to SDL 2 / C++98

**2026-07-21 update**: target changed from SDL 1.2 to **SDL 2** (user decision) — see the revised
Phase 4 below. All original SDL-1.2-specific analysis is kept struck through/annotated rather than
deleted, so the reasoning trail stays visible.

Branch: `claude/mobile-eggbert-refactor-pz57pn` (this repo only — `cna` and `sharp-runtime`
are read from their own GitHub repos as source, but no changes are pushed back to them).

## Current state (baseline, verified 2026-07-21)

- `mobile-eggbert` currently does **not** contain `cna` or `sharp-runtime` at all — no
  submodules, no vendored copies. `CMakeLists.txt:86-88` hardcodes
  `CNA_GRAPHICS_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}/../cna`, i.e. it expects `cna` (and
  transitively `sharp-runtime`) to exist as a **sibling checkout** next to `mobile-eggbert`,
  provided by whatever clones the dev environment. This sibling-dir assumption has to be
  replaced by an in-repo path once we vendor.
- `mobile-eggbert` itself: 50 `.hpp`/`.cpp` files, `CMAKE_CXX_STANDARD 23`.
- `cna` (`develop`, `ac3aaaeb`): 940 `.hpp`/`.cpp` files, ~46 MB checkout, `CMAKE_CXX_STANDARD 23`,
  license **Ms-PL** (Microsoft Public License — different from mobile-eggbert's MIT; needs its
  own notice preserved for the vendored subtree, see Open Question 3). Vendors its own copies of
  SDL3, SDL3_image, SDL3_mixer, cgltf, enet, stb, googletest, miniz, nlohmann, tinyxml2 under
  `third_party/` and `vendor/`.
- `sharp-runtime` (`develop`, `d0aeecf3`): 1572 `.hpp`/`.cpp` files, `CMAKE_CXX_STANDARD 23`,
  license MIT.
- `cna` currently defines **14 graphics backends**
  (`CMakeLists.txt:51`): `SDL_RENDERER EASYGL BGFX VULKAN WEBGPU HEADLESS SOFTWARE D3D11 D3D12
  CANVAS ASCII DX3 D3D9 SDL_GPU`, each with its own dir under
  `src/CNA/Internal/Backends/<Name>` and `include/CNA/Internal/Backends/<Name>`. Only
  `SdlRenderer` survives this task; the other 13 (including `Headless`, currently used for
  CI/tests — see Open Question 2) are removed.
- `mobile-eggbert` directly includes 47 distinct headers from `CNA/`, `Microsoft/`, `System/`,
  `SharpRuntime/` — that's the seed set for the transitive used-API analysis in Phase 2, not the
  full set (each of those headers pulls in more).
- ~~**Architecture mismatch for Phase 4**: `cna`'s `SdlRenderer` backend is built on **SDL3**'s
  `SDL_Renderer`/`SDL_Texture` GPU-accelerated 2D API. **SDL 1.2 has no renderer/texture API at
  all** — it only offers `SDL_Surface` + `SDL_BlitSurface` software blitting, plus a completely
  different event/window/audio/joystick API (no gamepad API, no touch input, no hi-DPI, single
  window only). Phase 4 is therefore not a search-and-replace of SDL3 calls — it is a rewrite of
  the backend's rendering strategy on top of a much smaller, older API.~~ **Superseded**: target
  is now SDL 2, not SDL 1.2 (see Phase 4 below) — SDL2 has `SDL_Renderer`/`SDL_Texture` (SDL3's
  2D API is a direct descendant of SDL2's, mostly renamed/reshuffled, not conceptually new),
  `SDL_GameController`/`SDL_Joystick`, and `SDL_TouchFingerEvent`, so the rewrite-from-scratch risk
  called out here for SDL 1.2 does not apply — Phase 4 against SDL2 is much closer to a real
  search-and-replace/API-diff exercise. Kept struck through rather than deleted so the earlier
  reasoning stays on record.

## Phases

### Phase 1 — Vendor cna + sharp-runtime as real subdirectories
- Copy `cna` (`develop` HEAD) and `sharp-runtime` (`develop` HEAD) into
  `mobile-eggbert/cna/` and `mobile-eggbert/sharp-runtime/` as plain committed files (no
  `.git`, no submodule gitlink — see Open Question 1 on whether to use `git subtree` to keep
  attribution history or a flat copy).
- Update `CMakeLists.txt` (`CNA_GRAPHICS_SOURCE_DIR`) and any other sibling-dir path
  assumptions (`android/`, `cmake/`, scripts) to point in-repo instead of `../cna`.
- Carry over required license/notice files (`cna`'s Ms-PL `LICENSE`, `THIRD_PARTY_NOTICES.md`)
  into the vendored subtree per Open Question 3.
- Build baseline (unchanged vendored code, `SDL_RENDERER` backend, default `CNA_GRAPHICS_BACKEND`)
  to confirm the copy + path rewiring didn't break anything, **before** any pruning starts.
- Commit + push.

### Phase 2 — Remove unused classes/methods from the vendored copies
- Start from the 47-header seed set of direct includes (see baseline above) and walk the
  transitive `#include` graph through the vendored `cna` and `sharp-runtime` to build the set of
  headers/TUs actually reachable from mobile-eggbert.
- Within reachable files, remove unused free functions/classes/methods (dead private helpers,
  unused overloads, e.g. XNA API surface mobile-eggbert never calls).
- Delete unreachable files entirely (whole classes/subsystems mobile-eggbert never touches —
  e.g. `Net`, `Media`, `GltfImport`, `Xnb` importer, etc., pending confirmation none of it is
  used — `Worlds`/`GameData` load raw level files, not glTF/XNB, needs verification).
- Iterative build-and-fix loop: remove a candidate, rebuild, restore if something broke
  (templates/macros/virtual dispatch can hide indirect uses that grep misses).
- Commit in reviewable batches (e.g. per subsystem/directory), not one giant diff.

### Phase 3 — Strip all graphics backends except SDL_RENDERER
- Delete the 13 backend dirs: `Ascii Bgfx Canvas D3D9 D3D11 D3D12 D3DCommon Dx3 EasyGL Headless
  SdlGpu Software Vulkan WebGPU` from both `src/CNA/Internal/Backends/` and
  `include/CNA/Internal/Backends/` in the vendored `cna` copy — confirm none of `SdlRenderer`
  depends on shared code that lived in `D3DCommon` or another sibling dir before deleting it.
  Also remove `dx9-spike/`, `plan_ascii.md`, `plan_canvas.md`, `plan_dx*.md`, `plan_sdlgpu.md`,
  `plan_webgpu.md`, `plan_software.md`, `plan_headless.md`, and other backend-specific docs
  referenced only by the removed backends.
- Simplify `cna/CMakeLists.txt`'s backend-selection logic (currently branches on 14 strings) and
  mobile-eggbert's own `CMakeLists.txt:51` (`MOBILE_EGGBERT_BACKENDS`) down to `SDL_RENDERER`
  only; drop the now-dead `CNA_BACKEND_*` boolean-flag alternate path.
- Remove vendored third-party deps that only existed for a deleted backend (e.g. `bgfx`,
  `enet`/`net` if unused elsewhere, WebGPU runtime lib copy step in mobile-eggbert's own
  `CMakeLists.txt:206-211`) — cross-check against Phase 2's reachability analysis first so we
  don't delete something Phase 2 already determined is still used elsewhere.
- Build + smoke-run with `SDL_RENDERER` to confirm nothing regressed.
- Commit + push.

### Phase 4 — Migrate SDL3 → SDL 2 (target changed from SDL 1.2, 2026-07-21 user decision)
- Replace vendored SDL3/SDL3_image/SDL3_mixer in `cna/third_party/` with SDL2 / SDL2_image /
  SDL2_mixer.
- Adapt `SdlGraphicsBackend` (and anything else under `src/CNA/Internal/Backends/SdlRenderer/`)
  from SDL3's `SDL_Renderer`/`SDL_Texture` calls to SDL2's — both APIs are the same
  renderer/texture model (SDL3's 2D API is SDL2's, evolved, not replaced), so this is expected to
  be close to a real API-diff/rename pass, not a rendering-strategy rewrite. Known SDL2-vs-SDL3
  API differences to work through as they're hit: `SDL_Init`/`SDL_CreateWindow`/
  `SDL_CreateRenderer` signature and flag differences, `SDL_RenderCopy` (SDL2) vs
  `SDL_RenderTexture` (SDL3), `int`-returning-`0`-on-success (SDL2) vs `bool`-returning-`true`-on-
  success (SDL3) for most calls, `SDL_bool` vs `bool`, `SDL_Keycode`/`SDL_Scancode` table
  differences, `SDL_GetTicks()` return type (`Uint32` in SDL2 vs `Uint64` in SDL3), pixel format
  enum renames.
- Rework window/event loop and keyboard/mouse state onto SDL2's event API (`SDL_PollEvent` shape
  differs from SDL3's). Joystick/gamepad input maps onto SDL2's `SDL_Joystick`/
  `SDL_GameController` (SDL3 renamed this to `SDL_Gamepad`, otherwise conceptually the same).
  Touch input (used by `InputPad` per CLAUDE.md) has a real SDL2 equivalent
  (`SDL_TouchFingerEvent`/`SDL_GetTouchFinger`) — Open Question 4 (below) is resolved by the
  SDL2 switch: no fallback strategy needed, this is a normal API adaptation.
- Rework audio playback (`Sound`/`ISound`) onto SDL2_mixer's API (mostly the same shape as
  SDL3_mixer, some function/constant renames).
- Build + smoke-run.
- Commit + push, likely in several sub-steps (window/event, then rendering, then input, then
  audio) rather than one commit, given the size.

### Phase 5 — Migrate C++23 → C++14 (target changed from C++98, 2026-07-22 user decision)

**Why the change**: Phase 4 pre-analysis (SDL3→SDL2) established the real constraint driving the
low-C++-standard requirement is VS2017's `v141_xp` toolset (the last MSVC toolset that can target
Windows XP SP2) — and that toolset's compiler supports up to C++17 language-wise, with only its
*library* (STL) having practical XP-runtime risk in a few headers (`std::filesystem`, some
`<thread>` sync primitives calling WinAPI functions absent on XP). C++98 was solving for a
constraint (pre-C++11 compiler) that doesn't actually exist for this project's real target
toolchain. C++14 keeps the actual reason for going low (XP/`v141_xp` compatibility) while avoiding
a much larger, riskier rewrite that would strip lambdas, `auto`, smart pointers, move semantics,
and `<thread>` — all of which C++14 keeps.

**Scope, verified 2026-07-22** (grepped the full reachable tree — mobile-eggbert's own `include/`+
`src/`, plus `cna`/`sharp-runtime` post-Phase-4 — for C++17/20/23-only constructs, not estimated):
- **Zero occurrences** of: structured bindings (`auto [a, b] = ...`), `std::variant`, `std::span`,
  concepts/`requires`, spaceship operator (`<=>`), coroutines (`co_await`/`co_return`/`co_yield`),
  `std::format`, C++20 modules.
- **`std::ranges`** — 1 real file: `include/WindowsPhoneSpeedyBlupi/Def.hpp` uses
  `std::ranges::none_of` (C++20). (Grep also matched `sharp-runtime`'s own `CLAUDE.md`/`NEXT.md`
  prose mentioning `std::ranges` as a style rule — not code, not in scope.)
- **`if constexpr`** — 2 files (C++17).
- **`std::string_view`** — 4 files (C++17).
- **`std::optional`** — 13 files (C++17), the largest item: `SpriteBatch`, `SpriteFont`,
  `SdlInputBridge`, `InputManager`, `GamerServices/Guide` (`.hpp`+`.cpp` pairs) in `cna`; mobile-
  eggbert's own `Worlds`, `Decor`, `Pixmap` (`.hpp`+`.cpp` pairs). Each call site needs a real
  case-by-case decision (sentinel value, `bool`+out-param pair, or pointer), not a mechanical
  find/replace — this is the actual work of this phase, not busywork.
- No C++11-only constructs (`auto`, lambdas, range-based `for`, `nullptr`, `enum class`,
  `override`/`final`, smart pointers, move semantics, uniform initialization, variadic templates,
  `<thread>`/`<atomic>`/`<mutex>`, `<chrono>`) need touching at all — C++14 is a superset of C++11
  for all of these.
- `CMakeLists.txt`: `CMAKE_CXX_STANDARD 23` → `14` in mobile-eggbert and the vendored `cna`/
  `sharp-runtime` copies.
- Much smaller than the original C++98 plan implied — expected to be a single focused pass
  (`std::optional` call sites are the real work), not the largest phase by file-touch count.
  Build + smoke-run after the `std::optional` rewrite and again after the final standard-version
  flip; commit in reviewable batches (mechanical C++17 fixes, then `std::optional` rewrites, then
  the CMake standard-version change + final verification), not one giant diff.
- Commit + push throughout.

## Decisions (2026-07-21, confirmed by user)

1. **Vendoring method**: plain flat copy of `develop` HEAD, no `.git` history, no submodule.
2. **Backends kept**: `SDL_RENDERER` **and** `HEADLESS` **and** `SOFTWARE` survive Phase 3 (not
   just `SDL_RENDERER` alone). The other 11 (`Ascii Bgfx Canvas D3D9 D3D11 D3D12 D3DCommon Dx3
   EasyGL SdlGpu Vulkan WebGPU`) are still removed — recheck `D3DCommon` isn't a shared dep of
   the 3 kept backends before deleting it.
3. **Phasing**: proceed through Phases 1-3 continuously, commit+push throughout. **Stop before
   Phase 4** (SDL migration — target changed to SDL 2, see top of file) and check in before
   starting it — Phase 5 (C++98) follows the same stop, revisited once Phase 4's scope/approach
   is agreed.

## Phase 1 status: done (this commit)

- `cna` (develop `ac3aaaeb`) and `sharp-runtime` (develop `d0aeecf3`) copied into `mobile-eggbert/cna/`
  and `mobile-eggbert/sharp-runtime/` as plain files, no `.git`. Nested submodules
  `third_party/{SDL,SDL_image,SDL_mixer}` (pinned commits `cbe3fbe9`/`fcb9d0b1`/`3075d3ed`) were
  populated with real source too, same flat-copy treatment — `vendor/googletest` in both repos was
  left as an empty placeholder since it's test-only and neither repo's `CMakeLists.txt` wires it
  into the main build.
- `CMakeLists.txt:86` (`CNA_GRAPHICS_SOURCE_DIR`) now points at the in-repo `cna/` instead of
  `../cna`. `cna/CMakeLists.txt`'s own `add_subdirectory(../sharp-runtime ...)` needed **no
  change** — `cna` and `sharp-runtime` are still siblings, just both now one level inside
  `mobile-eggbert/` instead of one level above it, so the relative path still resolves correctly.
- Fixed `android/app/build.gradle`'s `../../../cna/...` → `../../cna/...` (one less `..`, since
  `cna` moved from a mobile-eggbert sibling to a mobile-eggbert child).
- Updated `ANDROID.md`, `README.md`, `WINDOWS.md` path/submodule instructions that were pointing
  at the old sibling-checkout model (some of this text was already stale before this change — no
  `.gitmodules` exists in this repo, so the "init submodules" instructions in those docs predate
  even the sibling-checkout setup).
- `.gitignore`: added `.sdl-prebuilt-*/` (cna builds SDL3 once into
  `cna/.sdl-prebuilt-<platform>/`, a local build cache, not meant to be committed).
- **Baseline build result**: configured and built successfully — 522 of 526 translation units
  compile clean, including all 50 of mobile-eggbert's own files and the `SDL_RENDERER` backend.
  The remaining 4 failures are confined entirely to `cna`'s `Internal/Xnb/*` content-pipeline
  subsystem (`PrimitiveContentTypeReaders.cpp`, `DecimalDateTimeContentTypeReaders.cpp`,
  `SpriteFontContentTypeReader.cpp`, `XnbBuiltInReaders.cpp`) — `ContentReader` is missing
  `ReadDecimal()`/`ReadChar()` methods that `include/CNA/Internal/Xnb/*.hpp` call. Verified this
  is a **pre-existing bug already on `cna`'s own `develop` HEAD**, not something the copy/path
  rewiring introduced (grepped `ContentReader.hpp` in the vendored copy — those methods are
  genuinely absent). Left as-is rather than patched here, since `Xnb` is exactly the kind of
  subsystem Phase 2's reachability analysis is expected to delete outright (mobile-eggbert's
  `Worlds`/`GameData` load raw level/save files, not compiled XNB content) — if Phase 2 confirms
  that, the broken subsystem is removed rather than fixed. If Phase 2 finds `Xnb` **is** reachable
  after all, this bug will need fixing at that point instead.
- Container build dependencies needed and not preinstalled: `libxcursor-dev libxi-dev
  libxinerama-dev libxrandr-dev libxss-dev libxfixes-dev libwayland-dev libdecor-0-dev
  libxtst-dev libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev libgbm-dev libudev-dev
  libasound2-dev libpipewire-0.3-dev libavcodec-dev libavformat-dev libavutil-dev
  libswresample-dev` (the last four are `cna`'s own documented `VideoPlayer`/FFmpeg requirement
  from `cna/CLAUDE.md`, also a Phase 2 pruning candidate if mobile-eggbert never uses
  `VideoPlayer`).

## Phase 2 status: done (this commit)

- Wrote a file-reachability script (`#include`-graph BFS from mobile-eggbert's own 50 source
  files, through `cna/{include,src}` and `sharp-runtime/{include,src}`, treating a reached
  header's same-path `.cpp` as reachable too, matching this project's strict 1-`.hpp`-1-`.cpp`
  convention). `Internal/Backends/**` was excluded from the analysis entirely and left completely
  untouched — backend selection is wired through CMake target selection, not the `#include`
  graph, so it's out of scope for a reachability cut and is Phase 3's job instead.
- Of 2133 candidate `.hpp`/`.cpp` files outside `Backends/`, **1467 were unreachable** and
  deleted; 666 remain. Spot-checked before deleting: core types (`Vector2`, `Rectangle`, `Color`,
  `Texture2D`, `SpriteBatch`, `Game`, `GameTime`, `SharpRuntimeHelper`, `Prop`) all correctly
  reachable; `Accelerometer` (used by `InputPad`) reachable while `Compass`/`Gyroscope`/`Motion`
  (not used) correctly unreachable; zero reachable file references `System::Collections` at all
  (mobile-eggbert/cna's reachable subset uses `std::` containers directly, confirmed by grep, not
  a script bug).
- Deleted subsystems this resolved as fully unused, confirming Phase 1's open question 6:
  `Microsoft::Xna::Framework::Net` / `CNA::Internal::Net` (never linked — `CNA_Net` wasn't in
  mobile-eggbert's `target_link_libraries` to begin with), `CNA::Internal::Xnb` (the XNB
  content-pipeline reader classes — this also removes the pre-existing upstream build failure
  noted in Phase 1, since the broken files are gone rather than fixed), most of `CNA::Internal::
  Media` (video/picture/playlist library — `VideoDecoder`/`VideoPlayer` unused), most of
  `CNA::Devices`/`Microsoft::Devices` (camera, clipboard, file dialog, message box, system tray,
  URL launcher, power/display/locale/system info — none used), and all of `sharp-runtime`'s
  `System::Net`, `System::Xml`, `System::Collections`, most of `System::IO`/`Threading`/`Text`/
  `Security`/`Globalization`/`Diagnostics`/`Buffers`/`Runtime`/`Numerics`/`ComponentModel`.
- `cna/cmake/CnaLibrary.cmake`: removed the `CNA_Net` `add_library` block (its source dir is now
  empty — `add_library` with zero sources is a hard CMake error) and decoupled `CNA_GamerServices`
  from the same `CNA_ENABLE_NET` flag's Net half (`GamerServices` has no dependency on Net/ENet;
  it was only ever bundled with Net by the flag's naming, not by an actual code dependency).
  Updated the option's help text accordingly (`cna/CMakeLists.txt`).
- Also deleted, as pure dev-only tooling never built or used by mobile-eggbert:
  `cna/tests/` (401 files), `sharp-runtime/tests/` (376 files), and the now-pointless empty
  `vendor/googletest` submodule placeholders in both (neither repo's main build wires googletest
  in when `CNA_BUILD_TESTS`/`SHARP_RUNTIME_BUILD_TESTS` are off, and mobile-eggbert's documented
  build commands never passed `-DCNA_BUILD_TESTS=ON`). Flipped `cna/CMakeLists.txt`'s
  `CNA_BUILD_TESTS` option default from `ON` to `OFF` to match — it already defaulted to `ON`
  before this change while `vendor/googletest` was never populated in mobile-eggbert's checkout
  either, so a plain default build was already latently broken on this point pre-Phase-2; this
  makes the default consistent with what actually ships now instead of just leaving it broken.
- **Verified via full configure+build with zero manual CMake overrides** (the exact command
  from `README.md`'s "Linux native build" section, i.e. no `-DCNA_BUILD_TESTS=OFF` needed
  anymore): 225 translation units (down from 526 pre-Phase-2), **zero build failures**, binary
  links successfully. Smoke-ran it for 5s with `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy` (no
  real display in this container) — no crash, no missing-symbol/dynamic-link errors.
- `third_party/`/`vendor/` third-party library trees (SDL/SDL_image/SDL_mixer/cgltf/stb/enet in
  `cna`, miniz/nlohmann/tinyxml2 in `sharp-runtime`) were **deliberately left untouched** in this
  phase — they're not cna/sharp-runtime's own classes/methods, and pruning them (e.g. `enet` is
  now plausibly dead weight since `Net`/`ENet` usage was just deleted) is a reasonable follow-up
  but was kept out of this phase to keep it reviewable and scoped to the literal "unused classes
  and methods" ask.
- `cna` size: 144M → still 144M (third_party untouched, as above — the 1467-file deletion was
  entirely within `include/`+`src/`, a small fraction of the checkout's bytes vs. vendored SDL
  source). `sharp-runtime`: 15M → 7M (its vendor libs are tiny single-file deps, so the
  `include/`+`src/` cut shows up directly in checkout size).

## Phase 3 status: done (this commit)

- Deleted the 11 non-kept backend directories from both `cna/src/CNA/Internal/Backends/` and
  `cna/include/CNA/Internal/Backends/`: `Ascii Bgfx Canvas D3D9 D3D11 D3D12 D3DCommon Dx3 EasyGL
  SdlGpu Vulkan WebGPU`. Verified first that none of the 3 kept backends (`SdlRenderer Headless
  Software`) include anything from `D3DCommon` or any other removed backend — the only
  cross-backend dependency any of them has is the shared `Backends/Common/` interface
  (`IGraphicsBackend` etc., relative-included as `../Common/...`), which was left untouched.
  8 files remain under `Backends/` include+src combined (`Common` + the 3 kept backends' own
  `SdlGraphicsBackend`/`HeadlessGraphicsBackend`/`SoftwareGraphicsBackend` .hpp/.cpp pairs).
- `cna/cmake/BackendSelection.cmake`: rewritten from a 14-backend selector (with Windows-only/
  Emscripten-only hard gates, sibling-repo checks for `easy-gl`/`free-direct`, BGFX FetchContent,
  WebGPU/SDL_GPU third-party wiring) down to the 3 kept backends only.
  `cna/cmake/BackendLibraries.cmake`: dropped the `D3DCommon` shared-core library, the ASCII/
  SdlRenderer-core sharing library, the D3D9 isolated-effect-library block, and the 11-branch
  `elseif` chain for backend-specific link libraries (only `SDL_RENDERER`'s `SDL3::SDL3` link
  remains — `HEADLESS`/`SOFTWARE` never needed backend-specific extra libs even before this).
  `mobile-eggbert/CMakeLists.txt`: `MOBILE_EGGBERT_BACKENDS` cut from 14 entries to 3; removed the
  now-permanently-dead `CNA_GRAPHICS_BACKEND STREQUAL "WEBGPU"` runtime-copy block.
- **Found and fixed a real latent break while doing this**: `cna/cmake/Examples.cmake` (~940
  lines of cna's own demo apps) had ~20 `add_executable`/`target_link_libraries(... CNA_Net ...)`
  calls for Net/GamerServices demo targets, all nested inside a single
  `if(CNA_GRAPHICS_BACKEND STREQUAL "EASYGL" OR ... "VULKAN")` block. Since mobile-eggbert always
  forces `CNA_GRAPHICS_BACKEND=SDL_RENDERER`, that condition was already false throughout Phases
  1-2, so Phase 2 deleting `CNA_Net`'s source never actually broke mobile-eggbert's own build —
  but it would have broken a plain `cmake --build --target all` (or building `cna` standalone
  with `-DCNA_GRAPHICS_BACKEND=EASYGL`, which no longer exists as a choice anyway after this
  phase's `BackendSelection.cmake` rewrite made that condition permanently unreachable). Rather
  than leave ~940 lines of now-permanently-dead CMake referencing deleted subsystems/backends,
  deleted `cna/examples/` (used by nothing in mobile-eggbert — cna's own demo apps),
  `cna/tools/` (dev tooling: reference-dump, avatar pipeline, gltf-to-cnj converter, etc.),
  `cna/main.cpp` (cna's own standalone entry point, never built by mobile-eggbert), and
  `cna/dx9-spike/`, plus the CMake files that wired them in:
  `cmake/Examples.cmake`, `cmake/Tests/*.cmake` (14 files — per-backend CTest registration for
  removed backends plus the kept ones, all dead now that `tests/` itself is gone since Phase 2),
  `cmake/Harnesses.cmake`, `cmake/ToolGltfToCnj.cmake`.
- Also removed, now genuinely dead weight rather than deferred cleanup: `third_party/enet` and
  `cmake/ThirdPartyENet.cmake` — the only consumer, `Microsoft::Xna::Framework::Net`, was already
  deleted in Phase 2, confirmed via `grep` that zero remaining source file mentions
  `enet`/`ENet` — and the now-unreferenced `cmake/ThirdPartyWebGPU.cmake` /
  `cmake/WebGPUNativeSmokeTest.cmake` (nothing includes them once `BackendSelection.cmake` no
  longer has a `WEBGPU` branch).
- Removed the planning/reference docs for the 11 deleted backends specifically (left everything
  else — `Net`/`Xnb`/`Media`/`GltfImport`-related docs, `plan_headless.md`, `plan_software.md` —
  untouched, since those subsystems still have live remaining code after Phase 2, unlike the 11
  removed backends which have zero source left at all): `plan_ascii.md plan_canvas.md plan_dx.md
  plan_dx3.md plan_dx9.md plan_sdlgpu.md plan_webgpu.md` and `docs/{ascii,canvas,dx3,webgpu}-
  backend.md` / `docs/easygl_bugs.md`. Updated `cna/CLAUDE.md`'s own backend-list/table and
  deleted its now-dangling "WebGPU Is Active" section (pointed at the just-deleted
  `docs/webgpu-backend.md`).
- Updated `README.md`: simplified the Linux/Windows native build commands (no more
  `-DCNA_BACKEND_EASY_GL=OFF -DCNA_BACKEND_BGFX=OFF`, which don't exist as options anymore),
  deleted the entire "Direct3D 11 / Direct3D 12" section (Wine/DXVK/Proton instructions for a
  now-removed backend), rewrote "Backend status" down to the 3 kept backends, removed a second
  stale `git submodule update --init --recursive` step from the Emscripten build instructions.
- **Verified via 3 separate configure+build passes**: `SDL_RENDERER` (mobile-eggbert's actual
  target, `WindowsPhoneSpeedyBlupi`) — 225 translation units, 0 failures, binary links; smoke-run
  under `SDL_VIDEODRIVER=dummy` produced real init logs (window/renderer/audio-mixer creation,
  not just silent non-crash) confirming the game actually runs end-to-end, not just links.
  `HEADLESS` and `SOFTWARE` (`cna`'s own `CNA`+backend library target, standalone) — both
  configure and build with 0 failures. Also verified a removed backend
  (`-DCNA_GRAPHICS_BACKEND=EASYGL`) now fails fast at configure time with a clear
  `CNA: Unknown graphics backend: EASYGL` error instead of silently doing something wrong.
- `cna` size: 144M → 115M. `sharp-runtime`: 7M → 2.9M (`vendor/googletest` placeholder was the
  bulk of what was left to remove there after Phase 2).

## Phase 2.5 status: done (this commit, 2026-07-21, user-requested deeper cleanup)

User asked, after reviewing Phase 1-3: verify the build still works, delete CNJ too, and switch
the Phase 4 target from SDL 1.2 to SDL 2 (the latter handled above).

**Round 1 — CNJ/XNB/glTF removal from `ContentManager`.** Investigated why CNJ/`Xnb` remnants
survived Phase 2 (see Open Question 6's correction above): `ContentManager.cpp` genuinely used
(loads `Texture2D`/`SoundEffect`), but internally supported many more content types/formats than
mobile-eggbert's actual `Content/` (`.png`/`.wav` only) ever needs. Dispatched a sub-agent to
strip it down to just `Texture2D`'s native-image path and `SoundEffect`'s native-audio path.
Deleted (27 files): the whole CNJ/glTF subsystem (`cnj.md`, `plan_cnj.md`, `CnjEnvelope.hpp`,
`CnjSourceFile.hpp`, `GltfImport/GltfImportCore.{hpp,cpp}`, `MorphTargetEXT.{hpp,cpp}`,
`Internal/Json.hpp`), the whole `.xnb` binary-format subsystem (`ContentReader.{hpp,cpp}`,
`ContentTypeReaderManager.{hpp,cpp}`, `ContentTypeReader.hpp`, `ContentManifestEntry.hpp`,
`Internal/Xnb/{XnbHeader,XnbTypeReaderTable,XnbTypeName,XnbReadLimits,XnbDecompression,
LzxDecoder}.*`), and the now-orphaned vendored `third_party/{cgltf,stb}`. Edited
`ContentManager.cpp` 2977→269 lines and `ContentManager.hpp` 568→265 lines down to only the
`Texture2D`/`SoundEffect` readers; updated `cna/cmake/CnaLibrary.cmake`/`UnitTests.cmake` to drop
the now-gone `cgltf`/`stb` include dirs.

**Round 2 — cascade cleanup.** Re-ran Phase 2's reachability script after Round 1 landed (exactly
the follow-up the user asked for: whole-file reachability alone couldn't see that `ContentManager`
pulling in `Model`/`Curve`/`SpriteFont`/etc. was itself the problem — once those readers were
gone, everything *they* pulled in became newly unreachable). Found 61 newly-dead files (7 already
excluded `Backends/`), including confirmation of the user's own **`BoundingBox` example**: it was
reachable only via `ModelMesh.hpp` → gone once `Model`'s reader was deleted (this round deleted
`BoundingBox`/`Ray`/`Plane`/`BoundingSphere`/`BoundingFrustum` too — verified `Plane` has one
other real consumer, `Matrix::CreateShadow`/`CreateReflection`, but nothing else in the cluster
does). Deleted 54 non-backend files: `Model`/`ModelMesh`/`ModelBone`/`ModelMeshPart`/collections,
`Curve`/`CurveKey`/collections, the 3D-only Effects (`AlphaTestEffect`, `DualTextureEffect`,
`EnvironmentMapEffect`, `PbrEffect`, `ShaderEffect`, `SkinnedEffect`, `SkinnedPbrEffect`,
`SkinnedModelEXT`), `AnimationPlayer`, `Video`/`VideoPlayer`/`VideoDecoder`, `BoundingBox`/`Ray`/
`Plane`/`BoundingSphere`/`BoundingFrustum`, and `sharp-runtime`'s `BinaryReader`/
`EndOfStreamException` (only used by the now-gone XNB reader). Looped the script again after
deleting these — **0 further non-backend files found (fixed point)**.

**sharp-runtime vendor cleanup** (user's other question, same session): confirmed via grep that
`miniz`/`nlohmann`/`tinyxml2` (System::IO::Compression/System::Xml's only consumers, both deleted
in Phase 2) and `zlib` (miniz's only reason for existing) had zero remaining references anywhere
in reachable `cna`/`sharp-runtime` code. Deleted `sharp-runtime/vendor/{miniz,nlohmann,tinyxml2}`
and the corresponding `add_library`/`find_package(ZLIB)`/`target_link_libraries` lines from
`sharp-runtime/CMakeLists.txt`.

**A real hazard hit and fixed during this phase**: dispatched the Round-1 sub-agent to edit
`ContentManager.cpp` while *also* directly editing/deleting files in the same `cna/`/
`sharp-runtime/` tree myself (the vendor cleanup above) — a genuine concurrency bug, not a tooling
glitch. The sub-agent ran its own verification builds against the same working tree; when it hit
files I had legitimately deleted (my sharp-runtime vendor cleanup, and later my own Round-2
cascade deletions racing against its final verification pass), it had no way to know those were
intentional and `git checkout --`'d them back from HEAD, silently undoing real work multiple
times (this is why the stop-hook fired on uncommitted changes several times before this was
resolved — the tree was genuinely mid-flight, not safe to commit). Fixed by waiting for the
sub-agent to fully finish (confirmed via its completion notification) before redoing the Round-2
cascade cleanup single-actor, with no other agent touching the same paths concurrently. Lesson
for later phases: never run a sub-agent against `cna/`/`sharp-runtime/` at the same time as direct
edits in this session against the same trees, even on "different" files — shared build state
(the `.sdl-prebuilt-*/` cache) and the sub-agent's own build-and-fix loop can both step on
unrelated concurrent changes.

**Final verification** (single clean pass, no concurrency): full configure+build of
`WindowsPhoneSpeedyBlupi` — 187 translation units (down from 212 after Round 1, 225 after Phase
3), 0 failures, links clean. Smoke-test output byte-for-byte consistent with the known-good
baseline (window/renderer/audio-mixer init logs, no new errors). `cna`: 115M → 113M (third_party
still dominates — SDL/SDL_image/SDL_mixer source, untouched). `sharp-runtime`: 2.9M → 1.3M.
Combined `.hpp`/`.cpp` count across both vendored copies: 940 (Phase 1 baseline) → 666 (Phase 2)
→ 459 (Phase 2.5, this commit).

**Scope note for the user**: this phase found and removed *whole classes* that were reachable
only as dead weight (Model, Curve, the 3D effects, BoundingBox, etc.) — but did **not** attempt
method-level pruning *within* classes that remain genuinely used. Concrete example found during
Round 2: `Matrix` is definitely used, but two of its ~15 static factory methods
(`Matrix::CreateShadow`, `Matrix::CreateReflection`) take a `Plane` parameter that mobile-eggbert
never actually calls — going further and trimming individual unused methods out of still-used
classes (`Matrix`, `Vector2`, `Color`, `GraphicsDevice`, etc., potentially across all 459
remaining files) is a materially larger, slower, and more error-prone undertaking than this
phase's whole-file/whole-class cuts, and changes what this vendored copy *is* (a hand-trimmed,
mobile-eggbert-specific stub library vs. a pruned-but-API-complete XNA framework fork). Flagged
back to the user rather than assumed — see chat for the actual question asked.

## Phase 2.6 status: done (2026-07-21) — method-level pruning series

The user answered Phase 2.5's scope question: yes, go further into method-level pruning within
still-used classes, not just whole-file/whole-class cuts. Executed as 5 sequential batches (one
subagent at a time, foreground, fully waited-for before the next started or before this session
touched `cna`/`sharp-runtime` itself — see the concurrency-hazard note below for why sequential-
only, not parallel), each: grep every reachable call site across mobile-eggbert + `cna` +
`sharp-runtime` for every public method/operator/constant in its assigned files, delete zero-call-
site ones, verify with a full rebuild (restore anything that turns out load-bearing), re-run the
file-level reachability script afterward to catch newly-orphaned whole files, loop until 0 new
findings, then commit+push as its own checkpoint.

**Batch 1 — core math/framework types** (Vector2/3/4, Matrix, Quaternion, Rectangle, Color,
MathHelper, etc.): confirmed the `Matrix::CreateShadow`/`CreateReflection` example from Phase
2.5 — removing those 2 (of ~15) factory methods cascaded `Plane`/`BoundingBox`/`BoundingSphere`/
`BoundingFrustum`/`Ray` out entirely. Trimmed `Quaternion` to almost nothing, most of Vector2/3/4's
uncommon XNA math helpers, `Rectangle`/`Color`/`MathHelper` down to what's actually called.

**Batch 2 — Graphics** (`cna`'s largest subsystem, 79 files): mobile-eggbert is `SpriteBatch`-
only — deleted the entire custom-Effect infrastructure (`BasicEffect`, `Effect`, `EffectParameter`/
`Pass`/`Technique`, `IEffectFog`/`Lights`/`Matrices`) and the entire 3D/advanced-buffer cluster
(`Texture3D`, `TextureCube`, `RenderTarget2D`/`Cube`, `VertexBuffer`/`IndexBuffer`, all
`VertexPositionXxx` formats) — 35 whole files gone. Pruned `GraphicsDevice` down to its
`SpriteBatch`-relevant surface (removed the entire non-SpriteBatch draw API, buffer/render-target
binding). Caught one false-positive (`VertexElement` looked dead by grep but `IGraphicsBackend.hpp`,
outside the batch, declares `SetVertexDeclaration(const std::vector<VertexElement>&)` — restored).

**Batch 3 — Input**: gamepad support turned out to be almost entirely unreachable (touchscreen
game); a large fraction of the rest was `*ForTests` infrastructure dead now that this pruned copy
has no `tests/` dir. `SdlGamepadBackend`'s interface shrunk from 25 virtual methods to 3. Caught
two name-collision false positives (`GamePadButtons::getAProperty` vs `Color::getAProperty`;
`GamePadState::getIsConnectedProperty` vs `TouchPanelCapabilities`' own) and one operator-syntax
near-miss (`TouchCollection::operator[]`, called via bracket syntax the grep missed).

**Batch 4 — Audio/Media/Sensors/Joysticks**: 3D positional audio, microphone input, and music/
video streaming all turned out entirely unreachable (mobile-eggbert only plays fire-and-forget
`SoundEffect`s and reads the accelerometer) — `AudioEmitter`/`AudioListener`/`SongCollection`/
`VisualizationData`/`VisualizationCapture`/`VisualizationFFT` deleted. Kept
`DynamicSoundEffectInstance` whole despite zero callers — `FrameworkDispatcher.cpp` (outside the
batch) names the type directly in a `std::vector` and calls methods on it, so it's real reachable
API surface even though never exercised by this specific game today.

**Batch 5 (a/b/c) — sharp-runtime's remaining ~130 files**: hit a real process failure here — the
first attempt at this batch spawned its own unsupervised sub-agents to parallelize research, then
reported completion before their results were checked/applied, leaving the tree in a half-edited
state (only `System::Random` had actually landed). Redone properly in 3 follow-up commits, each a
single agent working directly with no further delegation:
- **5a (`Random`)**: trimmed to `Next()`/`Next(int)`/`Next(int,int)` — the entire surface
  `Decor.cpp` calls.
- **5b (Text/Threading/DateTime/String)**: only `Encoding::UTF8()` is ever called anywhere —
  deleted `ASCIIEncoding`/`Latin1Encoding`/`UTF32Encoding`/`UTF7Encoding`/`UnicodeEncoding`
  entirely. `System::String` cut ~90% (712+893 lines → 62+102) — only 5 of ~35 method names (one
  overload each) are ever called anywhere in the reachable tree, the biggest single win of the
  whole series. Traced `DateTime`/`DateTimeOffset`/`TimeSpan`'s actual reachable call graph
  (rooted at `DateTimeOffset::getUtcNowProperty()`, called from the Sensors subsystem) rather than
  pruning each file in isolation.
- **5c (exceptions/Math/IO/IsolatedStorage/misc)**: `Math` cut from ~50 members to `PI`/`Sin`/
  `Cos`/`Min`/`Max` (confirmed `cna` itself never calls `Math::` at all — only mobile-eggbert's own
  `Misc.cpp`/`Pixmap.cpp`/`Slider.cpp`/`GameData.cpp` do). `Int128`/`Type`/`DivideByZeroException`
  deleted via cascade (each only reachable through another already-dead method). `IsolatedStorage*`
  trimmed to the narrow subset `Worlds.cpp`'s save-game persistence actually uses. Exception
  hierarchy trimmed to the ctor overloads verified against real `throw` sites, not just method
  presence.

**Concurrency hazard, confirmed and worked around twice**: dispatching a sub-agent to edit
`cna`/`sharp-runtime` while *this session* also directly edits/deletes files in the same trees is
unsafe — the sub-agent's own verification builds see the direct edits as unexplained missing
files and "fix" them with `git checkout --`, silently reverting real work (happened once between
Phase 2.5 and this phase, and the lesson — wait for full agent completion before touching the
same trees — held for the rest of the series). Separately, a sub-agent spawning *its own*
unsupervised sub-agents and reporting done before they finish is a second, independent failure
mode (batch 5's first attempt) — fixed by explicitly forbidding further delegation in every
subsequent prompt.

**Final numbers**: combined `cna`+`sharp-runtime` `.hpp`/`.cpp` file count: 940 (Phase 1) → 666
(Phase 2) → 459 (Phase 2.5) → 337 (end of this series) — combined line count across what remains:
~44,000. `sharp-runtime` checkout size: 15M (Phase 1) → 2.9M (Phase 2.5) → 780K (now). Translation
units compiled for `WindowsPhoneSpeedyBlupi`: 526 (pre-Phase-2) → 225 (end of Phase 3) → 187 (end
of Phase 2.5) → 142 (end of this series). Verified via a final from-scratch clean configure+build+
smoke-test after the last commit: 0 failures, output matches the known-good baseline throughout.

## Phase 4 pre-analysis: SDL3 → SDL2 feasibility (2026-07-21, requested by user before starting Phase 4)

Surveyed the actual SDL3 API surface remaining after Phases 1-3 and the Phase 2.6 method-pruning
series (i.e. against the current, already-heavily-pruned tree, not the original upstream `cna`) to
answer: is rewriting this to SDL2 realistic? **Verdict: yes, no hard blocker.** Every SDL3 API
actually used has either a direct SDL2 equivalent or a well-scoped manual reimplementation path —
nothing requires a capability SDL2 fundamentally lacks (unlike the original SDL 1.2 target, which
would have needed a full rendering-model rewrite — see the struck-through analysis near the top of
this file).

**Blast radius**: 31 of the 337 remaining `cna`/`sharp-runtime` files `#include <SDL3/...>`
directly (~9%) — grep-verified via `grep -rl "#include <SDL3" cna/include cna/src sharp-runtime`.
Three files carry most of the real complexity:
- `cna/src+include/CNA/Internal/Backends/SdlRenderer/SdlGraphicsBackend.{cpp,hpp}` (~1000 lines) —
  the rendering backend.
- `cna/src/CNA/Internal/Input/SdlInputBridge.cpp` (1430 lines) — keyboard/mouse/touch/gamepad via
  SDL events.
- `cna/src/CNA/Internal/Input/InputManager.cpp` (472 lines) — input orchestration.
- `cna/include+src/CNA/Internal/Audio/AudioMixer.{hpp,cpp}` — audio.

The other ~27 files (window creation, sensors/accelerometer, storage paths, logger, title
container) are thin, mechanical call sites.

**Transfers cleanly** (direct SDL2 equivalents, confirmed by inspecting actual call sites):
- The `SDL_Renderer`/`SDL_Texture` model itself — SDL2 has had this since 2.0.0, unlike SDL 1.2's
  surface-blit-only model. `SDL_RenderClear`/`SDL_RenderPresent`/`SDL_CreateTexture`/
  `SDL_UpdateTexture`/`SDL_SetTextureBlendMode` all have direct SDL2 counterparts.
- Float-based rects/points (`SDL_FRect`/`SDL_FPoint`) — SDL2 added float renderer variants
  (`SDL_RenderCopyF` etc.) in 2.0.10+, so this isn't SDL3-only.
- `SDL_sensor.h` (accelerometer) — SDL2 has had this since 2.0.9, same conceptual API
  (`SDL_SensorOpen`/`SDL_SensorGetData`).
- `SDL_CreateWindow` — SDL3 dropped the x/y position params SDL2 requires (typically
  `SDL_WINDOWPOS_UNDEFINED`); trivial signature change.
- Environment: `libsdl2-dev` (2.30.0+dfsg-1ubuntu3.1) is directly `apt`-installable in this
  container (confirmed via `apt-cache policy`) — no toolchain/environment blocker for Phase 4
  builds.

**Needs real reimplementation, not just renaming** (the actual engineering cost of this phase):
1. **`SDL_RenderTextureAffine`** — an SDL3-only function (arbitrary 3-corner affine texture
   mapping), used by `SdlGraphicsBackend.cpp` specifically for `SpriteBatch.Begin(transformMatrix)`
   (camera/world transforms — see the file's own Task 675 comment). SDL2 has no equivalent; the
   fix is `SDL_RenderGeometry` (available since SDL2 2.0.18) fed with manually-computed quad
   corners — a genuine small reimplementation of one function, not a rename.
2. **`SDL_SetRenderLogicalPresentation`** — SDL3 exposes 4 modes (`CnaPresentationMode::{Letterbox,
   Overscan,Stretch,NativeBackBuffer}`, all 4 confirmed in active use by
   `SdlGraphicsBackend.cpp`'s `toSdlPresentationMode`/`toSdlPresentationModeString`). SDL2's
   `SDL_RenderSetLogicalSize` only covers letterbox-style integer-scaled presentation — `Overscan`
   and `Stretch` need manual viewport math on top of it.
3. **`AudioMixer` — the single biggest item.** Confirmed via grep that this uses SDL3_mixer 3.x's
   **new `MIX_*` track-based API** (`MIX_CreateTrack`/`MIX_PlayTrack`/`MIX_PROP_PLAY_*` properties)
   — a ground-up redesign from classic SDL2_mixer's channel-based `Mix_*` API
   (`Mix_Chunk`/`Mix_PlayChannel`/`Mix_Volume`). This is not a rename job, it's a real (if
   well-precedented and low-risk — SDL2_mixer is mature and thoroughly documented) rewrite of the
   mixer backend onto a channel-allocation model.
4. **`bool` vs `int`/pointer return convention** — SDL3 mostly returns `bool` (`true`=success);
   SDL2 mostly returns `int` (`0`=success) or a pointer. Mechanical but touches essentially every
   SDL call site across all 31 files (call-site count in the hundreds) — tedious, not risky (the
   compiler catches most mismatches immediately), but real line-count work.
5. **Input event details** — `SDL_Gamepad` → `SDL_GameController` rename, event timestamp width
   (`Uint64` → `Uint32`), minor event-struct field differences. Volume-wise the largest surface
   (~1900 lines across `InputManager.cpp`/`SdlInputBridge.cpp`), but no capability gap — SDL2 fully
   supports gamepad/touch/keyboard/mouse.

**Recommended approach when Phase 4 actually starts** (not yet executed): same methodology as
every phase so far — vendor SDL2/SDL2_image/SDL2_mixer into `cna/third_party/`, migrate file by
file starting with the 3 hot files (`AudioMixer` first, since it's the most self-contained
rewrite; then `SdlGraphicsBackend`; then the input pair), full rebuild + smoke-test after each
file, commit per logical unit, not one giant diff.

## Phase 4 status: done (2026-07-21) — SDL3 → SDL2 migration

Executed per the pre-analysis's own recommended order (vendor swap → `AudioMixer` →
`SdlGraphicsBackend` → input pair → remaining thin call sites), as 4 commits. No files were
added or removed by this phase (confirmed via `git diff --stat` against the pre-Phase-4 commit) —
37 `cna`/`sharp-runtime` `.hpp`/`.cpp` files touched, all internal-implementation edits; the
combined file count (337) and translation-unit count for `WindowsPhoneSpeedyBlupi` (142) are
unchanged from the end of Phase 2.6.

**Vendor swap**: `cna/third_party/{SDL,SDL_image,SDL_mixer}` replaced with SDL2 (release-2.30.11),
SDL2_image (release-2.8.2), SDL2_mixer (release-2.8.0) — same flat-copy treatment Phase 1 used,
sourced from the shared `~/deps/` cache per the org-level `CLAUDE.md`'s no-re-cloning rule.
`cna/cmake/ThirdPartySDL.cmake` rewritten for SDL2's build/install layout (target names, library
filenames, `SDL2IMAGE_*`/`SDL2MIXER_*` option names in place of `SDLIMAGE_*`/`SDLMIXER_*`) and
every CMake file referencing SDL3 target names updated (root `CMakeLists.txt`, `cna/CMakeLists.txt`,
`cna/cmake/{BackendLibraries,CnaLibrary,UnitTests}.cmake`, `sharp-runtime/CMakeLists.txt`, the
mingw-w64 toolchain files, `CNA/Entrypoint.hpp`).

**AudioMixer** (the pre-analysis's own "most self-contained rewrite" call, done first): SDL3_mixer's
track-object model (`MIX_CreateTrack`/`MIX_Audio`, per-track cooked callbacks, `MIX_PROP_PLAY_*`
loop-region properties) has no SDL2_mixer equivalent — SDL2_mixer is channel-index based, with no
persistent per-sound object and no bounded loop-region support at all. Introduced a
`CNA::Internal::Audio::Track` abstraction backed by a dedicated channel, lazily attached only while
playing. Looping is implemented as single-lap playback + lazy poll-and-restart (`TrackPlaying()`)
rather than SDL2_mixer's own loop counter, specifically so `Stop(false)`'s "let this lap finish,
then stop" contract still works (SDL2_mixer's native loop counter has no way to express that once
started). Stereo pan uses a per-channel `Mix_RegisterEffect` crossfeed-matrix callback (the same
FNA-matching math as before); pitch uses a one-shot linear-interpolation resample of the decoded
chunk — a disclosed simplification vs. the pre-migration true realtime frequency-ratio glide, never
exercised by mobile-eggbert (which only ever sets pitch before `Play()`, confirmed via
`Sound.cpp`). Bounded loop regions (`LoopBegin`/`LoopLength`) are unsupported (`Mix_Chunk` has no
loop-point concept) — whole-chunk looping only; also never exercised (mobile-eggbert's `SoundEffect`
is always loaded from whole `.wav` files via the plain string constructor). `MediaPlayer`'s single
background-music slot mapped cleanly onto SDL2_mixer's purpose-built `Mix_Music` API — actually
*simpler* than the pre-migration per-track implementation. `DynamicSoundEffectInstance`'s continuous
PCM streaming (no SDL2_mixer equivalent to a live-audio-stream-bound track) reimplemented via a
silent looped placeholder chunk plus a `Mix_RegisterEffect` callback pulling from an
`SDL_AudioStream` ring buffer — not exercised by mobile-eggbert itself (Phase 2.6 kept the class only
because `FrameworkDispatcher.cpp` names it directly, not because this game calls it).

**SdlGraphicsBackend** (the rendering backend, ~830 lines, the largest single file): the most
extensive rework. `SDL_CreateRenderer`'s SDL3 `(window, name)` pair became SDL2's `(window, device
index, flags)`; vsync moved from a runtime `SDL_SetRenderVSync` call to a creation-time
`SDL_RENDERER_PRESENTVSYNC` flag (`SetSwapInterval()` is now a documented no-op after construction —
SDL2 has no runtime vsync toggle at all); `SDL_RenderTexture`/`RenderTextureRotated` became
`SDL_RenderCopyF`/`RenderCopyExF`; `SDL_RenderReadPixels` changed from "allocate and return a new
surface" to "write into a caller buffer" (actually simpler on SDL2 — no format-conversion step
needed). Deliberately does not request `SDL_RENDERER_ACCELERATED`, so headless/software-only video
drivers (`SDL_VIDEODRIVER=dummy`, this project's own smoke-test/CI driver) still get a working
renderer instead of a hard `SDL_CreateRenderer` failure. `ApplyBlendState()` falls back to
`SDL_BLENDMODE_BLEND` when a custom-composed blend mode is rejected — hit for real during this
phase's own smoke-testing: SDL2's software renderer (used under `SDL_VIDEODRIVER=dummy`) rejects
most `SDL_ComposeCustomBlendMode` factor/operation combinations outright ("That operation is not
supported"), unlike SDL2's opengl/vulkan renderers or the pre-migration SDL3 stack, which both
supported the full generality; the accelerated (real-display, OpenGL) path never needs the fallback.

Two items needed real reimplementation, exactly as the pre-analysis flagged in advance:
- `SDL_SetRenderLogicalPresentation`'s 4 modes (Letterbox/Overscan/Stretch/NativeBackBuffer) have no
  single SDL2 equivalent — `SDL_RenderSetLogicalSize` natively covers only Letterbox (uniform
  scale-to-fit, centered). Overscan (scale-to-fill, centered, cropping overflow) and Stretch
  (independent x/y scale, no cropping) are now applied manually via `SDL_RenderSetScale` +
  `SDL_RenderSetViewport` (a deliberately oversized/negative-origin viewport for Overscan, relying on
  SDL2 always clipping to the real render-target bounds regardless of viewport size). `ReadBackbuffer`'s
  own physical/logical-size invariant check now reads `SDL_RenderGetViewport` instead of the SDL3-only
  `SDL_GetRenderLogicalPresentationRect` — correct in all 4 modes since every mode now sets the
  viewport explicitly (or lets SDL2 auto-manage it for Letterbox).
- `SDL_RenderTextureAffine` (arbitrary 3-corner quad mapping, used for
  `SpriteBatch.Begin(transformMatrix)`) has no SDL2 equivalent — reimplemented via
  `SDL_RenderGeometry`, feeding it the same already-transformed screen-space quad corners directly
  as a two-triangle textured mesh with per-vertex UV/color (`SDL_RenderGeometry` ignores texture
  color/alpha mod, unlike the normal draw path, so color is passed per-vertex instead).

`GameWindow`/`GraphicsDeviceManager`/`GraphicsDevice`/`Game.cpp`: window creation needs explicit x/y
position arguments (`SDL_WINDOWPOS_UNDEFINED`); fullscreen takes a window-flags value
(`SDL_WINDOW_FULLSCREEN_DESKTOP`) instead of a bool; several setters (`SetWindowSize`,
`SetWindowTitle`, `SetWindowResizable`, `SetWindowBordered`, `MinimizeWindow`, `RestoreWindow`) now
return `void` instead of `bool`; the event loop's window sub-events are nested under one
`SDL_WINDOWEVENT` top-level type (`event.window.event == SDL_WINDOWEVENT_RESIZED` etc.), unlike
SDL3's separate top-level `SDL_EVENT_WINDOW_*` constants — see `Game::PollEvents()`.
`GraphicsAdapter`'s display enumeration moved from SDL3's opaque `SDL_DisplayID` array
(`SDL_GetDisplays`) to SDL2's plain zero-based display-index model (`SDL_GetNumVideoDisplays`/
`SDL_GetDisplayMode`). `Texture2D`/`ImageLoader`: `SDL_Surface` creation/conversion/blit function
renames (`SDL_CreateRGBSurfaceWithFormat(From)`, `SDL_FreeSurface`, `SDL_ConvertSurfaceFormat`,
`SDL_SoftStretchLinear` in place of `SDL_BlitSurfaceScaled(..., SDL_SCALEMODE_LINEAR)`) plus a custom
dynamic-memory `SDL_RWops` (SDL2 has no auto-growing memory write sink, unlike SDL3's
`SDL_IOFromDynamicMem`) for the `Stream`-overload `SaveAsPng`/`SaveAsJpeg` — never called by
mobile-eggbert itself, only the filename overloads are.

**Input pair + sensors**: `SdlGamepadBackend`/`SdlJoystickBackend`: `SDL_Gamepad` →
`SDL_GameController` throughout (SDL3 renamed the whole subsystem); `IsGamepad`/`OpenGamepad`/
`OpenJoystick` now take a device *index* rather than an instance id (SDL2's `SDL_IsGameController`/
`SDL_GameControllerOpen`/`SDL_JoystickOpen` are index-based) — the only real call site
(`SdlInputBridge`'s `CONTROLLERDEVICEADDED`/`JOYDEVICEADDED` handlers) already has a device index on
hand for exactly that event (SDL2 documents `.which` as a device index on ADDED, an instance id on
REMOVED), so no conversion was actually needed. Joystick battery query loses percentage granularity
(SDL2's `SDL_JoystickCurrentPowerLevel` returns a coarse enum only) — `*percent` is always -1,
matching XNA's own "not always available" contract. `SdlInputBridge.cpp` (1430 lines): event-type
constants, union member renames (`event.cdevice`/`caxis`/`cbutton`, not `gdevice`/`gaxis`/`gbutton`),
keyboard event fields moved under a nested `.keysym` (`event.key.keysym.sym`/`scancode`/`mod`),
touch event field `.fingerId` (not `.fingerID`), `SDL_RenderCoordinatesFromWindow` →
`SDL_RenderWindowToLogical`. Two SDL3-only event types (per-device mouse/keyboard added/removed, IME
candidate-list) have no SDL2 equivalent at all — the corresponding NOXNA/EXT events
(`MouseConnectedEXT` etc., `TextEditingCandidatesEXT`) simply never fire under this backend, a
disclosed capability gap, not exercised by mobile-eggbert itself. `SdlSensorSubsystem.hpp`: SDL2's
sensor API is device-index-based (`SDL_NumSensors`/`SDL_SensorOpen(int)`/
`SDL_SensorGetDeviceInstanceID`), unlike SDL3's array-of-instance-ids `SDL_GetSensors()`;
`SDL_EVENT_SENSOR_UPDATE` → `SDL_SENSORUPDATE`; the `SDL_EventFilter` trampoline's return type
changed from `bool` to `int` (a real, caught-by-the-file's-own-`static_assert` signature mismatch);
`SDL_AddEventWatch` returns `void` in SDL2 (no failure to detect beyond the pre-existing
force-failure test hook) and `SDL_RemoveEventWatch` is named `SDL_DelEventWatch`.
`SystemDeviceBackend.cpp`: SDL2 has no multi-mouse/multi-keyboard enumeration at all
(`SDL_GetMice`/`SDL_GetKeyboards` are SDL3-only) — `GetMice()`/`GetKeyboards()` now report one
synthetic default-device entry each; `GetTouchDevices()` keeps real per-device enumeration but loses
device names (SDL2 has none). `MouseCursor`: stock cursor enum renamed
(`SDL_SYSTEM_CURSOR_ARROW`, not `_DEFAULT`) and `SDL_DestroyCursor` → `SDL_FreeCursor`.
`TextInputEXT`: SDL2's text-input API lost its per-window/IME-type-hint-properties parameters
entirely (process-global `SDL_StartTextInput()`/`SDL_StopTextInput()`/`SDL_SetTextInputRect()`, no
properties system) — `StartTextInputWithTypeEXT`'s type hint is silently dropped, a disclosed
simplification not exercised by mobile-eggbert (no XNA/NOXNA text-entry UI calls it).

**Verification**: full clean configure+build (`cna/.sdl-prebuilt-*` deleted, `build/` reconfigured
from scratch) — 0 compile errors after fixing a first pass of build failures (mostly int/bool
return-convention mismatches the compiler caught immediately, a few real API-shape differences:
`SDL_AudioStream`'s forward-declared struct tag conflicting with SDL2's real
`typedef struct _SDL_AudioStream SDL_AudioStream`, `Mix_UnregisterEffects` not existing under that
name in SDL2_mixer (`Mix_UnregisterAllEffects` instead), `SDLK_A`..`SDLK_Z` needing lowercase SDL2
names). Smoke-tested twice: `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy` (this project's standing
headless verification method since Phase 2) ran the full test duration with zero errors/crashes,
software renderer, blend-mode fallback engaged as designed; a second run against the real X11
display (`DISPLAY=:0`) confirmed the accelerated OpenGL path also initializes cleanly with no
fallback needed. Both logs match the established init-sequence shape (window → renderer → logical
presentation → audio mixer) with no new warnings beyond the disclosed, intentional ones above.

**Commits**: vendor swap; `AudioMixer` rewrite; `SdlGraphicsBackend` + window/display layer rewrite;
input pair + sensors rewrite — 4 commits, each independently buildable in sequence, none pushed.

## Remaining open questions (not yet answered, relevant to Phase 1/2, low-risk defaults applied unless told otherwise)

3. **Ms-PL attribution for vendored `cna`**: default plan applied — `cna`'s `LICENSE` (Ms-PL) and
   `THIRD_PARTY_NOTICES.md` were kept as-is inside the vendored `cna/` subdirectory (satisfies
   Ms-PL's redistribution terms for that code, doesn't relicense mobile-eggbert's own MIT code).
6. ~~Scope check for Phase 2~~ — resolved empirically by Phase 2's reachability analysis: `Net`
   confirmed entirely unused and deleted; `Media` mostly unused and deleted (video/picture/
   playlist library — `Sound`/`ISound` audio playback, which mobile-eggbert does use, lives in a
   different subsystem and was untouched). **Correction (2026-07-21, see Phase 2.5 below)**:
   `Xnb` and `GltfImport` were *not* fully deleted by Phase 2 — small remnants stayed reachable
   because `ContentManager.cpp` (genuinely used — it's how `Texture2D`/`SoundEffect` get loaded)
   `#include`s them for its `.xnb`/`.cnj`-loading branches. File-level reachability isn't fine
   enough to see that those specific branches inside an otherwise-used file are themselves dead
   for this project's actual content (`Content/` has only `.png`/`.wav`, never `.xnb`/`.cnj`).
   The user caught this and asked for `.cnj` specifically to be removed too — see Phase 2.5.
4. **Touch input on SDL 1.2** — resolved: target is SDL2 now (see top of file), which has a real
   touch API, so no fallback strategy is needed here after all.
