# Fullscreen diagnosis — 2026-09-28

This is the historical diagnosis before repair. The owner subsequently authorized CNA fixes;
see [FULLSCREEN_FIXES.md](FULLSCREEN_FIXES.md) for the implementation and new verification.

Vulkan and Canvas have defects in CNA's conversion from logical game coordinates to the
physical presentation surface. DirectX9 has a separate device-loss recovery problem, with
the exact back-buffer reference holder still unconfirmed. This investigation changed no
game, CNA or SharpRuntime source code and applied no workaround.

The tested artifacts are the existing builds recorded in [BUILD_RENDERERS.md](BUILD_RENDERERS.md),
built against CNA `3d5742e84c21`. During diagnosis the CNA checkout advanced independently to
`a62c40b09e86`; the intervening commits change networking and its documentation/tests, leaving
the rendering and runtime code cited below unchanged. SharpRuntime is `007280bd1cc7`.

All graphical probes used CNA's private Weston/Xwayland runner. Wine used a separate prefix,
and Chrome used a separate profile. No live desktop, user browser profile or normal Wine
prefix was used.

## Vulkan: physical letterbox rectangle used as logical projection dimensions

The game supplies coordinates for its logical 800 × 480 surface. CNA's default presentation
mode is Letterbox. `GraphicsDevice::UpdateViewportFromWindow` keeps the public viewport logical,
but passes the physical presentation rectangle to the renderer's `SetViewport`.

Relevant CNA code, relative to `../../libcna/cna/`:

- `modules/graphics/src/Xna/GraphicsDevice.cpp:4553`: obtains the physical default viewport;
  line 4601 passes it to the renderer while retaining the logical dimensions in the public viewport.
- `modules/renderers/vulkan/include/CNA/Internal/Renderers/Vulkan/VulkanRenderer.hpp:3322`:
  `GetDefaultViewportRect` delegates to `GetPresentedRectEXT`.
- `modules/renderers/vulkan/src/VulkanRenderer.cpp:16429`: `GetViewportSize` reports logical dimensions.
- The same file at line 16546 computes the physical Letterbox rectangle, and line 22201
  stores it as an active viewport.
- **The same file at lines 14717–14733 misclassifies that default rectangle as a custom viewport
  whenever it differs from the full physical target. It replaces the logical sprite projection
  dimensions with the physical viewport width/height.**

For a 1600 × 900 window, the correct physical presentation rectangle is
`(x=50, y=0, width=1500, height=900)`, with scale 1.875. Sprite projection should still use
800 × 480. Instead, it uses 1500 × 900. Dividing by those dimensions and then mapping into
that same viewport cancels the enlargement: sprites retain their original pixel sizes.

This was reproduced by resizing a fresh Vulkan window from 800 × 480 to 1600 × 900.
The window origin was `(560,300)`; the nonblack image bounds were `(610,300)` to `(1410,780)`
(exclusive upper bounds), exactly 800 × 480 with the predicted 50-pixel letterbox offset.
The image should have grown to 1500 × 900.

The private display's native F11 test switched its output mode to 800 × 480, so it did not
reproduce the user's native-resolution fullscreen dimensions. The larger-window probe
confirms the scaling defect independently; the user's fullscreen observation matches it.

Evidence:

- [Before resize](build/logs/fullscreen-diagnosis/vulkan-wide-windowed.png)
- [After resize](build/logs/fullscreen-diagnosis/vulkan-wide-resized.png)
- [Measured window dimensions](build/logs/fullscreen-diagnosis/vulkan-wide-results.json)
- [Runtime log](build/logs/fullscreen-diagnosis/vulkan-wide-runtime.log)

## Canvas: enlarged backing buffer without presentation scaling

The actual page Fullscreen button activates DOM fullscreen and expands the canvas backing
buffer from 800 × 480 to 1920 × 1024. The game's logical viewport remains 800 × 480, as intended,
but CNA never supplies the corresponding scale and letterbox offset to Canvas2D rendering.

Relevant CNA code, relative to `../../libcna/cna/`:

- `modules/renderers/canvas/src/CanvasRenderer.cpp:153`: returns the logical viewport size.
- Lines 158 and 164 only store virtual resolution and presentation mode; line 169 updates
  the surface description without applying a drawing transform.
- **`modules/renderers/canvas/src/CanvasSpriteBatchRenderer.cpp:233` calls `ctx.setTransform`
  with the SpriteBatch user matrix alone.** `SetTransformMatrix` at line 424 stores that
  matrix, which is identity for these original game drawing calls. No physical presentation
  scale or offset is composed with it.

Browser measurements confirmed this with the page's Resize canvas option both unchecked and
checked: the backing buffer and CSS rectangle become 1920 × 1024 at `(0,0)`, the context
transform stays identity, and nontransparent pixels remain confined to `(0,0)` through
`(799,479)` inclusive. The screenshot reproduces the reported small image at the upper left
and black remainder. Exiting fullscreen restores the ordinary 800 × 480 canvas.

F11 behavior is separate: `src/WindowsPhoneSpeedyBlupi/InputPad.cpp:439` explicitly excludes
the Web platform from the game's F11 toggle. Browser-window fullscreen was also checked
through Chrome's window API; it leaves the page's fixed-size canvas unchanged and does not
activate DOM fullscreen on the canvas. The page button is the tested path for canvas fullscreen.

Evidence:

- [Windowed](build/logs/fullscreen-diagnosis/canvas-windowed.png)
- [Fullscreen, Resize canvas unchecked](build/logs/fullscreen-diagnosis/canvas-fullscreen-no-resize.png)
- [Fullscreen, Resize canvas checked](build/logs/fullscreen-diagnosis/canvas-fullscreen-resize.png)
- [Browser measurements and console events](build/logs/fullscreen-diagnosis/web-results.json)

## DirectX9: reset warning reproduced; device-loss recovery cannot safely reach the next Present

The private Wine test reproduced the same graphics warning immediately after F11:

```text
err:d3d:wined3d_swapchain_resize_buffers Something's still holding back buffer 0 (...).
```

The process remained alive during the seven-second observation after F11, including refocusing
the window. The user's fatal `DeviceLostException` was therefore not reproduced in this
environment. No specific owner of the outstanding back-buffer reference has been identified.

Source analysis nevertheless reveals a concrete recovery defect in CNA. Paths below are
relative to `../../libcna/cna/`:

1. `modules/renderers/directx9/src/DirectX9Renderer.cpp:459` resets the device on a surface or
   presentation change; line 536 requests this after changing fullscreen state. It releases
   registered default-pool resources and the cached depth surface before `Reset` at line 496.
2. `DirectX9Renderer::Present` at line 660 sets `deviceLost_` when native Present returns
   `D3DERR_DEVICELOST`. Recovery is attempted by `PollDeviceLost` at line 615 on a later Present.
3. `modules/runtime/src/GraphicsDeviceManager.cpp:374` allows Draw when
   `CanBeginDrawEXT()` returns true. DirectX9 does not override this method, so it inherits
   the unconditional true at
   `modules/graphics/include/CNA/Internal/Renderers/Common/IGraphicsRenderer.hpp:3854`.
4. The next drawing operation calls `EnsureRenderReadyEXT` (`DirectX9Renderer.cpp:170`),
   which calls `ThrowIfDeviceLost` at line 626 and throws `DeviceLostException`.
5. `modules/runtime/src/Game.cpp:743` calls Draw before EndDraw/Present, without a device-loss
   catch. The exception escapes `Game::Run` instead of reaching the recovery poll.

This is a source-confirmed failure path consistent with the user's fatal log. It does not
prove which native call first loses the device in that run, or who holds the back buffer.
Resolving the reference holder requires further native/Wine tracing; it should not be
described as a confirmed leak in a particular CNA object.

Evidence:

- [Wine runtime log](build/logs/fullscreen-diagnosis/directx9-runtime.log)
- [Windowed](build/logs/fullscreen-diagnosis/directx9-windowed.png)
- [After F11](build/logs/fullscreen-diagnosis/directx9-fullscreen.png)
- [Window geometry and process status](build/logs/fullscreen-diagnosis/native-results.json)

## Scope of the result

The confirmed scaling defects belong to CNA's renderer presentation code. The DirectX9
loss/recovery path also belongs to CNA; native reset ownership remains unresolved. The
game's restored original SpriteBatch sequences do not need a renderer-specific workaround
for these findings. No SharpRuntime defect was found during this investigation.

Successful compilation of all 22 variants remains a build result; it does not establish
correct fullscreen behavior. This report records diagnosis only, in accordance with the
instruction to report CNA/SharpRuntime bugs without fixing those repositories.
