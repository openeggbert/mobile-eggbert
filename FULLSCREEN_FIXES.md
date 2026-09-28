# Fullscreen fixes — 2026-09-28

The subsequent DirectX9 motion/background report exposed another reset and projection path;
see [DIRECTX9_FULLSCREEN_FOLLOWUP.md](DIRECTX9_FULLSCREEN_FOLLOWUP.md) for its repair and tests.

The authorized follow-up repairs CNA's Vulkan/Canvas scaling and DirectX9 device-loss
recovery. CNA commit: `6f6100cd35df`, based on `a62c40b09e86`.
No Mobile Eggbert or SharpRuntime source was changed during this repair.

Vulkan now retains the logical SpriteBatch projection separately from the physical
presentation viewport. Canvas applies presentation scaling, centered letterboxing,
viewport clipping and matching input-coordinate transforms. Custom viewports, user
transforms, deferred batches and offscreen render targets are covered by pixel checks.

DirectX9 checks and recovers the device before entering Game.Draw. Update continues while
Draw waits for restoration. A failed fullscreen Reset is retried against current drawable
dimensions, and native reset invalidates the cached blend/depth states so they are reapplied.

## Built artifacts

- [Linux Vulkan](build/linux-vulkan/WindowsPhoneSpeedyBlupi)
- [Web Canvas](build/web-canvas/WindowsPhoneSpeedyBlupi.html), with matching JS/WASM/data files
- [Windows DirectX9](build/windows-directx9/WindowsPhoneSpeedyBlupi.exe)

These three variants were rebuilt with the repair. The other nineteen variants retain the
earlier build verification in [BUILD_RENDERERS.md](BUILD_RENDERERS.md).

## Verification

All graphical tests used CNA's private Weston/Xwayland runner. Chrome used a dedicated
profile and Wine a dedicated prefix.

- Vulkan and browser Canvas: **11 presentation assertions each**, including custom viewport
  clipping, Immediate transforms, independent deferred projections and RenderTarget2D isolation.
- Canvas host unit tests: **19/19 pass**, including Letterbox/input and Stretch/HiDPI cases.
- Existing Vulkan presentation-mode and 3D viewport-subregion probes: both pass.
- DirectX9 recovery probe: Update continues while lost Draw is skipped; explicit restoration
  resumes drawing. A deliberately held backbuffer makes fullscreen Reset fail; releasing it
  restores drawing. Native AlphaBlend/depth settings are checked after reset.
- Actual Vulkan game at 1600 x 900: correctly enlarged **1500 x 900** image with 50-pixel side bars.
- Actual Canvas page Fullscreen at 1920 x 1024: correctly centered **1707 x 1024** image,
  with Resize canvas both checked and unchecked; exiting restores 800 x 480.
- Actual DirectX9 F11 under Wine: process remains alive and image fills the private fullscreen
  surface. The private Vulkan F11 display changes to 800 x 480; larger-size scaling was
  verified independently by resizing and pixel probes.

The Wine backbuffer warning still appears; its reference owner remains unconfirmed. The
recovery defects are repaired and tested without claiming the warning itself was eliminated.
Web F11 is excluded by the game's existing InputPad code; this repair covers the page's
Fullscreen button. OpenGLES rendering and the earlier alpha-content discrepancy are deferred.

## Evidence

- [CNA task and implementation details](../../libcna/cna/plans/plan_fullscreen_regressions.md)
- [Native regression results](build/logs/fullscreen-fixed/native-final-runner.log)
- [Canvas browser pixel checks](build/logs/fullscreen-fixed/canvas-probe-final-runner.log)
- [DirectX9 deterministic recovery](build/logs/fullscreen-fixed/directx9-frame-recovery-final.log)
- [Vulkan resized image](build/logs/fullscreen-fixed/vulkan-wide-resized.png)
- [Canvas fullscreen image](build/logs/fullscreen-fixed/canvas-fullscreen-resize.png)
- [DirectX9 fullscreen image](build/logs/fullscreen-fixed/directx9-fullscreen.png)

Build logs are `linux-vulkan-final-build.log`, `canvas-final-build.log` and
`directx9-final-build.log` in `build/logs/fullscreen-fixed/`. Full CNA-root CTest was not run;
the probes use the consumer's toolchains and freshly built CNA libraries.
