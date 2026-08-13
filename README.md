# Mobile Eggbert

Mobile Eggbert is a modified version of Speedy Blupi, originally developed for Windows Phone and released in 2013. The
project underwent the following transformations:

- decompiled by the ILSpy to the C# source code
- migrated from XNA 4.0 to Monogame
- migrated from C# to C++
- migrated from Monogame to CNA

CNA is XNA-like wrapper around the SDL 3 (cross-platform software development library).

The C++ source code was created using the following Git commit of the Git repository mobile-eggbert-core:

https://github.com/openeggbert/mobile-eggbert-core/commit/1cbc13415b768085b7f5c97fbf35a773d7f14a8e

## Development

### Dependency checkouts

Mobile Eggbert consumes the current CNA checkout next to this repository. CNA in turn consumes
sharp-runtime from the same parent directory:

```text
openeggbert/
├── mobile-eggbert/
├── cna/
└── sharp-runtime/
```

Initialise CNA's vendored dependencies after cloning it:

```bash
git -C ../cna submodule update --init
```

Use `-DMOBILE_EGGBERT_CNA_ROOT=/path/to/cna` and
`-DCNA_SHARP_RUNTIME_ROOT=/path/to/sharp-runtime` for a different layout.

### Linux native build

```bash
cmake -S . -B build-linux \
  -DCNA_GRAPHICS_RENDERER=SDL_RENDERER
cmake --build build-linux --target WindowsPhoneSpeedyBlupi
```

### Windows native build

```powershell
cmake -S . -B build-windows \
  -DCNA_GRAPHICS_RENDERER=SDL_RENDERER
cmake --build build-windows --target WindowsPhoneSpeedyBlupi
```

### Windows cross-build from Linux (MinGW-w64)

**Important: Always use a clean build directory when switching toolchains (e.g., `rm -rf build-windows`).**

1. Ensure you have `mingw-w64` installed (e.g., `sudo apt install mingw-w64`).
2. Run the build:
```bash
# Ensure you are in mobile-eggbert directory
rm -rf build-windows
cmake -S . -B build-windows \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64.cmake \
  -DCNA_GRAPHICS_RENDERER=SDL_RENDERER
cmake --build build-windows --target WindowsPhoneSpeedyBlupi
```

CNA builds its vendored SDL3, SDL3_image and SDL3_mixer dependencies for the selected toolchain.

### Direct3D 11 / Direct3D 12 (Windows renderers, runnable on Linux via Wine/Proton)

CNA has full Direct3D 11 and Direct3D 12 renderers. They are Windows-only, but you can build **and
run** them from Linux — the D3D11 build even runs under plain Wine. Both were verified end to end
(the game builds, starts, creates a real GPU device and swapchain, and presents frames).

Build either one exactly like the cross-build above, just swapping the renderer:

```bash
# Direct3D 11
cmake -S . -B build-d3d11 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=../cna/cmake/toolchains/mingw-w64.cmake \
  -DCNA_GRAPHICS_RENDERER=DIRECTX11 -DCMAKE_BUILD_TYPE=Release -DCNA_BUILD_TESTS=OFF
cmake --build build-d3d11 --target WindowsPhoneSpeedyBlupi

# Direct3D 12 (same, with D3D12)
cmake -S . -B build-d3d12 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=../cna/cmake/toolchains/mingw-w64.cmake \
  -DCNA_GRAPHICS_RENDERER=DIRECTX12 -DCMAKE_BUILD_TYPE=Release -DCNA_BUILD_TESTS=OFF
cmake --build build-d3d12 --target WindowsPhoneSpeedyBlupi
```

Mobile Eggbert disables CNA's own tests and examples by default. Set
`-DMOBILE_EGGBERT_BUILD_CNA_TESTS=ON` only when working on CNA itself.

#### Running D3D11 on Linux — plain Wine + DXVK

D3D11 needs nothing special beyond a Wine prefix with DXVK installed:

```bash
cd build-d3d11
WINEPREFIX=~/.wine-cna-d3d11 wine ./WindowsPhoneSpeedyBlupi.exe
```

#### Running D3D12 on Linux — **must** go through Proton, not plain Wine

Under plain system Wine, a D3D12 game **crashes on startup** with a null-pointer page fault:

```
vkd3d_instance_get_vk_instance(instance=0000000000000000)
  ← inside Wine's OWN dxgi.dll (dlls/dxgi/swapchain.c)
```

**This is not a bug in the game or in CNA.** It is a DLL-pairing mismatch in the environment: a
distro's system `dxgi.dll` cannot hand a D3D12 command queue to vkd3d-proton's separately-installed
`d3d12.dll` — those two only work as a matched pair, which is what Proton ships. (Full analysis:
`cna/plan_dx.md`, tasks `DX-100`/`DX-102`.) **On real Windows this does not happen at all**,
since there is only one DXGI, Microsoft's own.

So run D3D12 through a properly Proton-managed launch, using the helper script in cna:

```bash
cd build-d3d12
bash ../../cna/scripts/run-proton-vkd3d.sh "$(pwd)/WindowsPhoneSpeedyBlupi.exe"
```

It needs a local Steam install with "Proton - Experimental"; it bootstraps its own dedicated prefix
on first use and never touches your personal `~/.wine`.

Verified working: a real vkd3d-proton D3D12 device initialises against the GPU and
`dxgi_vk_swap_chain_init` creates a real 800x480 swapchain (the game's own resolution), which then
presents frames — zero crashes, zero exceptions.

### Web / Emscripten build

#### Prerequisites

1. Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html):
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```
2. Ensure all submodules are initialised:
   ```bash
   git submodule update --init --recursive
   ```

#### Configure

```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S . -B cmake-build-web -DCMAKE_BUILD_TYPE=Debug
```

#### Build

```bash
cmake --build cmake-build-web -j
```

#### Run

```bash
emrun cmake-build-web/WindowsPhoneSpeedyBlupi.html
```

#### Generated files

| File | Description |
|------|-------------|
| `WindowsPhoneSpeedyBlupi.html` | Main entry point — open in browser |
| `WindowsPhoneSpeedyBlupi.js`   | Emscripten JS glue |
| `WindowsPhoneSpeedyBlupi.wasm` | WebAssembly binary |
| `WindowsPhoneSpeedyBlupi.data` | Preloaded asset bundle |

#### Virtual filesystem layout

| Path | Source directory | Notes |
|------|-----------------|-------|
| `/Content/backgrounds` | `Content/backgrounds/` | Read-only; preloaded |
| `/Content/icons` | `Content/icons/` | Read-only; preloaded |
| `/Content/sounds` | `Content/sounds/` | Read-only; preloaded |
| `/worlds` | `worlds/` | Read-only; preloaded |
| `/save` | IndexedDB (IDBFS) | Writable; persists `SpeedyBlupi` save file |

#### Notes

- Save data (`SpeedyBlupi`) is stored in `/save/.cna_isolated_storage/SpeedyBlupi`
  backed by the browser's IndexedDB. It is flushed to IndexedDB on every write
  and on page unload.
- Audio uses SDL_mixer; the browser may require a user gesture before audio
  starts. If no sound is heard, click the canvas once.
- CPU usage is bounded — the game uses `emscripten_set_main_loop` (backed by
  `requestAnimationFrame`) instead of a busy loop.
- **Game speed**: the Web build uses a fixed-timestep accumulator in
  `CNA/Game.cpp` to match native desktop timing. The browser calls the RAF
  callback at ~60 Hz; real inter-frame wall-clock time is measured and
  accumulated, and `Update()` fires only when one full `TargetElapsedTime`
  slice has accumulated. This ensures gameplay speed is identical to
  Linux/Windows regardless of the browser's actual RAF cadence. A 250 ms
  spike cap prevents runaway catch-up after the tab is backgrounded.

### Compile-time renderer selection

Pick exactly one renderer at configure time with `-DCNA_GRAPHICS_RENDERER=<name>`. Mobile Eggbert
delegates selection and validation to CNA, so all 46 CNA renderer identities are available:

```text
SDL_RENDERER, OPENGLES2, OPENGLES3, OPENGL33, WEBGL1, WEBGL2, BGFX, VULKAN,
WEBGPU, MAGNUM, HEADLESS, SOFTWARE, STUB, DIRECTX11, DIRECTX12, DIRECT2D,
CANVAS, HTML_DOM, SKIA, BLEND2D, FREEDIRECT, DIRECTX9, DIRECTX1, DIRECTX2,
DIRECTX3, DIRECTX5, DIRECTX6, DIRECTX7, DIRECTX8, DIRECTX10, SDL_GPU,
OPENGLES1, OPENGL4, OPENGL1, OPENGL2, WICKED, SOKOL, DILIGENT, GLIDE, GDI,
LLGL, METAL, FNA3D, SVG_DOM, OPENVG, PORTABLEGL
```

The renderer is compiled into the executable; it is not selected at runtime. CNA enforces each
renderer's platform and toolchain requirements—for example, the DirectX renderers require a
Windows target, the DOM renderers require Emscripten, Metal requires macOS and Glide requires a
32-bit Windows target. Some renderers also require their documented sibling or system dependency.
CNA's defaults are `OPENGLES3` on Linux, `WEBGL2` on Emscripten and `SDL_RENDERER` elsewhere.

Mobile Eggbert links only `CNA::Runtime`, `CNA::Devices` and `CNA::GamerServices` directly, plus
their required transitive module closure and the selected renderer. It does not link CNA's
umbrella, Net or CNAEXT targets. sharp-runtime is likewise limited to CNA's required component
closure plus `IO.IsolatedStorage` for save games.

## Progress

