# Plan: Vendor CNA + SharpRuntime, prune, strip backends, downgrade to SDL 1.2 / C++98

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
- **Architecture mismatch for Phase 4**: `cna`'s `SdlRenderer` backend is built on **SDL3**'s
  `SDL_Renderer`/`SDL_Texture` GPU-accelerated 2D API. **SDL 1.2 has no renderer/texture API at
  all** — it only offers `SDL_Surface` + `SDL_BlitSurface` software blitting, plus a completely
  different event/window/audio/joystick API (no gamepad API, no touch input, no hi-DPI, single
  window only). Phase 4 is therefore not a search-and-replace of SDL3 calls — it is a rewrite of
  the backend's rendering strategy on top of a much smaller, older API. Flagging this now so
  scope is clear before work starts.

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

### Phase 4 — Migrate SDL3 → SDL 1.2
- Replace vendored SDL3/SDL3_image/SDL3_mixer in `cna/third_party/` with SDL 1.2 /
  SDL_image 1.2 / SDL_mixer 1.2.
- Rewrite `SdlGraphicsBackend` (and anything else under
  `src/CNA/Internal/Backends/SdlRenderer/`) from the `SDL_Renderer`/`SDL_Texture` model to
  `SDL_Surface`/`SDL_BlitSurface` software rendering — this is the architecturally invasive part
  called out above, not a mechanical API swap.
- Rework window/event loop, keyboard/mouse state, and joystick input (SDL 1.2 predates the
  gamepad API) to SDL 1.2 equivalents; touch input (used by `InputPad` per CLAUDE.md) has no
  SDL 1.2 equivalent and needs a fallback strategy — Open Question 4.
- Rework audio playback (`Sound`/`ISound`) onto SDL_mixer 1.2's API.
- Build + smoke-run.
- Commit + push, likely in several sub-steps (window/event, then rendering, then input, then
  audio) rather than one commit, given the size.

### Phase 5 — Migrate C++23 → C++98
- Applies to mobile-eggbert's own 50 files **and** whatever remains of the vendored `cna`/
  `sharp-runtime` after Phases 2-3.
- Known C++98-incompatible constructs to hunt for and replace throughout: `auto`, lambdas,
  range-based `for`, `nullptr` → `NULL`/`0`, `enum class` → plain `enum` (namespaced to avoid
  collisions), `override`/`final`, `constexpr` → `const`/macros, smart pointers
  (`unique_ptr`/`shared_ptr`) → manual `new`/`delete` or a pre-C++11 owning-pointer helper,
  move semantics/rvalue refs, variadic templates, `static_assert`, uniform initialization
  (`{}`), `<unordered_map>`/`<unordered_set>` → `<map>`/`<set>` (TR1 not guaranteed), `<thread>`/
  `<atomic>`/`<mutex>` → SDL 1.2 threading primitives, `<chrono>` → SDL/`time.h`, trailing
  return types, `constexpr if`, structured bindings, string formatting beyond `iostream`/
  `sprintf`.
- `CMakeLists.txt`: `CMAKE_CXX_STANDARD 23` → `98` in mobile-eggbert and the vendored `cna`/
  `sharp-runtime` copies.
- This is expected to be the largest phase by file-touch count; do it in small,
  buildable-at-every-step commits (e.g. one subsystem/directory at a time), not a single sweep.
- Build + smoke-run after each sub-step.
- Commit + push throughout.

## Decisions (2026-07-21, confirmed by user)

1. **Vendoring method**: plain flat copy of `develop` HEAD, no `.git` history, no submodule.
2. **Backends kept**: `SDL_RENDERER` **and** `HEADLESS` **and** `SOFTWARE` survive Phase 3 (not
   just `SDL_RENDERER` alone). The other 11 (`Ascii Bgfx Canvas D3D9 D3D11 D3D12 D3DCommon Dx3
   EasyGL SdlGpu Vulkan WebGPU`) are still removed — recheck `D3DCommon` isn't a shared dep of
   the 3 kept backends before deleting it.
3. **Phasing**: proceed through Phases 1-3 continuously, commit+push throughout. **Stop before
   Phase 4** (SDL 1.2 migration) and check in before starting it — Phase 5 (C++98) follows the
   same stop, revisited once Phase 4's scope/approach is agreed.

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

## Remaining open questions (not yet answered, relevant to Phase 1/2, low-risk defaults applied unless told otherwise)

3. **Ms-PL attribution for vendored `cna`**: default plan applied — `cna`'s `LICENSE` (Ms-PL) and
   `THIRD_PARTY_NOTICES.md` were kept as-is inside the vendored `cna/` subdirectory (satisfies
   Ms-PL's redistribution terms for that code, doesn't relicense mobile-eggbert's own MIT code).
6. ~~Scope check for Phase 2~~ — resolved empirically by Phase 2's reachability analysis: `Net`
   confirmed entirely unused and deleted; `Xnb` confirmed entirely unused and deleted; `Media`
   mostly unused and deleted (video/picture/playlist library — `Sound`/`ISound` audio playback,
   which mobile-eggbert does use, lives in a different subsystem and was untouched).
   `GltfImport` — not called out separately above because it turned out to already be
   `cna`-internal-only wiring with no direct mobile-eggbert include; not specifically verified
   file-by-file beyond what the reachability script decided, flagging here in case it matters.
4. **Touch input on SDL 1.2** — deferred to the Phase 4 check-in.
