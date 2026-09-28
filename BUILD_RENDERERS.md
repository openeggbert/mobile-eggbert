# Renderer build verification — 2026-09-28

All 22 requested variants configured, compiled and linked successfully after restoring the
original game's per-sprite SpriteBatch Begin/Draw/End sequences. Menu and first-level rendering
were checked with SDL_RENDERER and VULKAN on a private Weston/Xwayland display against the
supplied old Linux executable. Subsequent fullscreen diagnosis and authorized CNA repairs
fixed VULKAN/web CANVAS scaling and DIRECTX9 loss/recovery handling. Those three variants were
rebuilt and checked at runtime; see [FULLSCREEN_FIXES.md](FULLSCREEN_FIXES.md) for the repair
revision, evidence and limits. The Wine back-buffer warning still appears, with its reference
owner unconfirmed. A further DirectX9 repair covers successful native resets and logical
sprite/background presentation during movement; see
[DIRECTX9_FULLSCREEN_FOLLOWUP.md](DIRECTX9_FULLSCREEN_FOLLOWUP.md) for the latest DirectX9
revision and build. Other browser and Wine renderers were not run.
The reported OpenGLES rendering problem remains deferred. The results below describe
configuration, compilation and linking, rather than complete runtime correctness.

## Dependency revisions

The dependencies are consumed from `../../libcna/`. The table records the original 22-variant
build verification, when the source checkouts were clean. The later three-variant fullscreen
rebuild uses the CNA repair revision recorded in [FULLSCREEN_FIXES.md](FULLSCREEN_FIXES.md).

| Repository | Commit |
|---|---|
| cna | `3d5742e84c21` |
| sharp-runtime | `007280bd1cc7` |
| easy-gl | `deda7a426c3c` |
| meta-gl | `20c8b2dc5bb8` |

CNA's DirectX 11/12 MinGW fixes were supplied by another agent in commit `d5cf852212fb`
(`DX-271`). The original build/restoration task made no CNA or SharpRuntime changes;
the separately authorized fullscreen repair changes CNA only.

Toolchains: CMake 3.31.6, Ninja, GCC 14.2 on Linux, MinGW-w64 GCC 14-win32 for Windows,
and Emscripten 6.0.9 for web. Linux and Windows builds use Debug; web builds use Release.

## Results and artifacts

| Platform | Renderer | Preset | Result | Entry file |
|---|---|---|---|---|
| Linux | SDL_RENDERER | `linux-sdl-renderer` | OK | `build/linux-sdl-renderer/WindowsPhoneSpeedyBlupi` |
| Linux | OPENGLES2 | `linux-opengles2` | OK | `build/linux-opengles2/WindowsPhoneSpeedyBlupi` |
| Linux | OPENGLES3 | `linux-opengles3` | OK | `build/linux-opengles3/WindowsPhoneSpeedyBlupi` |
| Linux | OPENGL33 | `linux-opengl33` | OK | `build/linux-opengl33/WindowsPhoneSpeedyBlupi` |
| Linux | VULKAN | `linux-vulkan` | OK | `build/linux-vulkan/WindowsPhoneSpeedyBlupi` |
| Linux | WEBGPU | `linux-webgpu` | OK | `build/linux-webgpu/WindowsPhoneSpeedyBlupi` |
| Linux | HEADLESS | `linux-headless` | OK | `build/linux-headless/WindowsPhoneSpeedyBlupi` |
| Linux | SOFTWARE | `linux-software` | OK | `build/linux-software/WindowsPhoneSpeedyBlupi` |
| Linux | STUB | `linux-stub` | OK | `build/linux-stub/WindowsPhoneSpeedyBlupi` |
| Linux | SDL_GPU | `linux-sdl-gpu` | OK | `build/linux-sdl-gpu/WindowsPhoneSpeedyBlupi` |
| Linux | OPENGL4 | `linux-opengl4` | OK | `build/linux-opengl4/WindowsPhoneSpeedyBlupi` |
| Linux | FNA3D | `linux-fna3d` | OK | `build/linux-fna3d/WindowsPhoneSpeedyBlupi` |
| Web | WEBGL1 | `web-webgl1` | OK | `build/web-webgl1/WindowsPhoneSpeedyBlupi.html` |
| Web | WEBGL2 | `web-webgl2` | OK | `build/web-webgl2/WindowsPhoneSpeedyBlupi.html` |
| Web | WEBGPU | `web-webgpu` | OK | `build/web-webgpu/WindowsPhoneSpeedyBlupi.html` |
| Web | CANVAS | `web-canvas` | OK | `build/web-canvas/WindowsPhoneSpeedyBlupi.html` |
| Web | HTML_DOM | `web-html-dom` | OK | `build/web-html-dom/WindowsPhoneSpeedyBlupi.html` |
| Web | SVG_DOM | `web-svg-dom` | OK | `build/web-svg-dom/WindowsPhoneSpeedyBlupi.html` |
| Windows / Wine | DIRECTX11 | `windows-directx11` | OK | `build/windows-directx11/WindowsPhoneSpeedyBlupi.exe` |
| Windows / Wine | DIRECTX12 | `windows-directx12` | OK | `build/windows-directx12/WindowsPhoneSpeedyBlupi.exe` |
| Windows / Wine | DIRECTX9 | `windows-directx9` | OK | `build/windows-directx9/WindowsPhoneSpeedyBlupi.exe` |
| Windows / Wine | GDI | `windows-gdi` | OK | `build/windows-gdi/WindowsPhoneSpeedyBlupi.exe` |

Native build directories include `Content/` and `worlds/`. Each web entry file has matching
`.js`, `.wasm` and `.data` files; serve the whole directory over HTTP.
Windows directories also include SDL3, SDL3_mixer, libcurl and MinGW runtime DLLs.
Native Linux builds resolve their shared libraries through the configured dependency prefixes;
keep these dependencies available when running the binaries.

## Rebuilding

The project's `CMakePresets.json` contains configure and build presets for all variants.
The existing build caches retain the dependency locations used for this verification:

```bash
cmake --preset linux-vulkan
cmake --build --preset linux-vulkan --parallel 4

cmake --preset windows-directx11
cmake --build --preset windows-directx11 --parallel 4
```

For web builds, activate the Emscripten SDK first and set `EMSDK`:

```bash
export EMSDK=/home/robertvokac/emsdk
source "$EMSDK/emsdk_env.sh"
cmake --preset web-webgl2
cmake --build --preset web-webgl2 --parallel 4
```

The configure logs record the complete dependency overrides used for each build. This workspace
reused installed SDL packages with `CNA_USE_SYSTEM_SDL=ON`. Windows dependencies are installed in
`build/dependencies/windows/`, including libcurl 8.14.1 with Schannel TLS and nlohmann JSON 3.11.3.
The native WebGPU build uses `/home/robertvokac/deps/wgpu-native-v29.0.1.1`.
FNA3D uses the private copy in `build/dependencies/FNA3D/`.

## Original rendering restored

`Pixmap::DrawBackground`, `DrawPart` and `DrawIcon` now use separate
`Begin(BackToFront, AlphaBlend) / Draw / End` sequences, matching the original C# game.
The supplied `../mobile-eggbert-legacy/mobile-eggbert-core/Pixmap.cs` confirms these sequences
at lines 292–294, 356–358 and 491–493.
The frame-wide batch, its state flag and its build option were removed. The existing geometry
refresh on window resizes is retained through `IPixmap::UpdateGeometry` before each frame.

The menu buttons, complete text, player, terrain, level objects and life icons are visible in
the checked SDL_RENDERER and VULKAN runs. No artificial sprite depths, blend-state changes or
renderer-specific game workarounds were introduced. This restoration did not modify CNA or
SharpRuntime; the later fullscreen repair is documented separately above.

A separate alpha-rendering discrepancy remains relative to the old executable: translucent
menu panels and buttons are brighter, and sprite edges have more visible halos. This was
observed in both checked renderers. CNA's loose-PNG load path retains straight-alpha pixels,
while `AlphaBlend` uses premultiplied-alpha blending; this observation does not justify changing
XNA's blend equation. Further investigation of content preparation is separate from restoring
the draw order. No correction for this discrepancy was applied.

Runtime screenshots and comparison details are in `build/logs/texture-diagnosis/RESTORATION.md`;
runner logs are `build/logs/restore-original-runtime.log` and
`build/logs/restore-original-vulkan-runtime.log`.

## Verification evidence

- All configure and build commands exited with code 0.
- Every cache selects the requested renderer.
- Linux executables have ELF headers and no unresolved `ldd` dependencies.
- Windows executables have PE headers; every imported non-system DLL is packaged beside the game.
- Native asset paths and sizes match the source `Content/` and `worlds/` trees.
- All six web packages contain HTML, JavaScript, WASM and data; WASM headers and JavaScript syntax pass.
- SDL_RENDERER and VULKAN display the restored menu and first-level sprites on a private display.
- `git diff --check` passes.

Logs are in `build/logs/<preset>-configure.log` and `build/logs/<preset>-build.log`.
Machine-readable results are in `build/logs/matrix-final-results.json`,
`build/logs/artifact-verification.json` and `build/logs/dependency-revisions.json`.
