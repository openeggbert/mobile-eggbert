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

## Open questions (answer before Phase 1 starts)

1. **Vendoring method**: plain flat copy of `cna`/`sharp-runtime` `develop` HEAD (no history), or
   `git subtree add` (keeps upstream history/attribution, still a normal in-repo copy, still no
   submodule)?
2. **Headless backend**: task says keep only `SDL_RENDERER` — that also removes `Headless`,
   which looks like it's meant for CI/automated testing without a display. OK to delete it too
   (no headless CI test path afterward), or should it survive as a second exception?
3. **Ms-PL attribution for vendored `cna`**: keep `cna`'s `LICENSE` (Ms-PL) and
   `THIRD_PARTY_NOTICES.md` inside the vendored `cna/` subdirectory as-is (satisfies Ms-PL's
   redistribution terms for that code, doesn't relicense mobile-eggbert's own MIT code), or
   different handling in mind?
4. **Touch input on SDL 1.2**: `InputPad` (per project CLAUDE.md) handles touch/keyboard/
   accelerometer. SDL 1.2 has no touch API. Fall back to mouse-as-touch emulation only, or is
   touch out of scope for the SDL 1.2 target platform(s)?
5. **Phasing/commit granularity**: proceed through Phases 1→5 continuously on this one branch
   as instructed, commit+push after each phase (and sub-step within Phases 4-5)? Confirming
   before Phase 1 since Phases 4-5 are large enough that a mid-course correction after lots of
   commits would be costly.
6. **Scope check for Phase 2**: safe to assume `Net`, `Media`, `GltfImport`, and `Xnb` importer
   subsystems in `cna` are entirely unused by mobile-eggbert (per `Worlds`/`GameData` loading
   raw level/save files, not glTF or XNB) and can be deleted outright, or should any be kept
   pending a closer look during Phase 2 itself?
