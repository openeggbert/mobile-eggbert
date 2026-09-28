# DirectX9 fullscreen motion follow-up — 2026-09-28

Follow-up CNA task **FULLSCREEN-002**, commit `9bbba48d8c37`, based on
`6f6100cd35df`. Mobile Eggbert and SharpRuntime source were not changed.

The earlier reset/recovery repair did not cover ordinary successful native resize resets.
A new native-state probe confirmed that repeated AlphaBlend and DepthStencil.None assignments
were suppressed by GraphicsDevice's object-identity cache after Reset had reverted the device
to blending disabled and depth enabled. CNA now invalidates retained blend, depth and
rasterizer state after that reset, without inventing another public DeviceReset event.

DirectX9 also returned physical dimensions as the logical viewport and built SpriteBatch's
projection from the physical viewport. That disagreed with the game's fixed logical
background pieces. CNA now separates logical projection from physical presentation, applies
Letterbox placement and consistent input mapping, preserves custom viewports/render targets,
and keeps the D3D9 correction at half a physical pixel. Clipped Overscan sprite projection
is covered by the new tests.

## Verification

Before repair, two native-state assertions and seven shared presentation assertions failed.
After repair, five private-Wine test executables exit 0:

| Probe | Passing assertions |
|---|---:|
| Successful resize, retained state, input, multi-frame motion/background and Overscan | 25 |
| Shared SpriteBatch presentation/custom viewport/transform/render-target checks | 11 |
| Simulated loss and held-backbuffer fullscreen-reset recovery | 2 |
| Existing DirectX9 smoke checks, compiled with HiDef | 62 |
| Existing DirectX9 SpriteBatch checks, compiled with HiDef | 10 |

The older smoke/SpriteBatch fixtures select Reach despite calling GetBackBufferData, which
the current XNA boundary correctly rejects in Reach. Their temporary HiDef copies change
only that setup requirement; the original fixtures were not edited.

The actual game was also run through a temporary launcher using its existing game objects
and drawing code. The launcher sets the physical window size through GameWindow to test a
large surface: private Wine did not reliably forward external window-manager resizing to SDL.
The normal game's F11/movement run and this controlled presentation run both use a dedicated
Wine prefix on CNA's private Weston/Xwayland display. No test changes are built into the
delivered game executable.

Final large-surface measurements are recorded in
`build/logs/directx9-fullscreen-followup/game-large-motion-final-runner.log` and
`game-controlled-runtime.log`. During normal gameplay, the logical viewport stays **800 x 480**
and the physical rectangle becomes **(50,0,1500,900)** on a 1600 x 900 surface. Movement in
both directions is visually clean. The desktop screenshot crops part of this large window
because Wine returns it to `(560,300)`; full-target coverage is independently checked by
the pixel probes. F11 shows a complete **800 x 480** scene at `(0,10)` on the private
800 x 500 fullscreen surface, and movement remains clean there.
Wine's held-backbuffer warning may still occur; its reference owner remains unconfirmed.

## Artifacts and evidence

- [Rebuilt DirectX9 game](build/windows-directx9/WindowsPhoneSpeedyBlupi.exe)
- [Final five-probe results](build/logs/directx9-fullscreen-followup/probes-hidef-runner.log)
- [Successful-reset and motion checks](build/logs/directx9-fullscreen-followup/cna_test_directx9_resize_state.log)
- [Native-state failures before repair](build/logs/directx9-fullscreen-followup/resize-state-before.log)
- [Presentation failures before repair](build/logs/directx9-fullscreen-followup/presentation-before.log)
- [Movement on the large surface](build/logs/directx9-fullscreen-followup/controlled-resized-right.png)
- [Movement after F11](build/logs/directx9-fullscreen-followup/controlled-fullscreen-right.png)
- [CNA task details](../../libcna/cna/plans/plan_directx9_fullscreen_motion.md)

Build evidence is in `game-final-build.log`, `game-projection-build.log` and the individual
probe-build logs in `build/logs/directx9-fullscreen-followup/`. Only DirectX9 was rebuilt
in this follow-up; other renderer qualification remains as previously recorded.
