#include "CNA/Internal/Backends/SdlRenderer/SdlGraphicsBackend.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>

namespace CNA::Internal::Backends::SdlRenderer
{
    using namespace Microsoft::Xna::Framework;
    using namespace Microsoft::Xna::Framework::Graphics;
    using namespace CNA::Internal::Backends;

    // --- SdlTextureBackend ---

    SdlTextureBackend::SdlTextureBackend(SDL_Renderer* renderer, const ImageData& data)
    {
        width = data.width;
        height = data.height;

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
        if (!texture)
        {
            throw std::runtime_error(std::string("Failed to create SDL texture: ") + SDL_GetError());
        }

        if (SDL_UpdateTexture(texture, nullptr, data.pixels.data(), width * 4) != 0)
        {
            SDL_DestroyTexture(texture);
            texture = nullptr;
            throw std::runtime_error(std::string("Failed to update SDL texture: ") + SDL_GetError());
        }
    }

    SdlTextureBackend::~SdlTextureBackend()
    {
        if (texture)
        {
            SDL_DestroyTexture(texture);
        }
    }

    void SdlTextureBackend::UpdatePixels(const uint8_t* rgba, int stride)
    {
        if (!texture || !rgba) return;
        SDL_UpdateTexture(texture, nullptr, rgba, stride);
    }

    void SdlTextureBackend::UpdatePixelsLevel(int level, const uint8_t*, int, int)
    {
        throw std::runtime_error(
            "SDL_Renderer does not support mip-level texture uploads (level " + std::to_string(level) +
            "): SDL_Renderer's 2D texture API has no native mip chain or per-level LOD sampling. "
            "Use Texture2D::SetData(level=0, ...) only, or generate mips via a mipMap-aware backend.");
    }

    // --- SdlSpriteBatchBackend ---

    SdlSpriteBatchBackend::SdlSpriteBatchBackend(SDL_Renderer* r) : renderer(r)
    {
    }

    void SdlSpriteBatchBackend::Begin()
    {
        // Task 695 finding: this previously hardcoded SDL_SetRenderDrawBlendMode(SDL_BLENDMODE_BLEND)
        // unconditionally, clobbering whatever SdlGraphicsBackend::ApplyBlendState had just set via
        // GraphicsDevice::setBlendStateProperty(blendState) -- called by SpriteBatch::Begin()
        // immediately before backend_->Begin() runs (see SpriteBatch.cpp). This was invisible while
        // ApplyBlendState's own mapping was still incomplete (BlendState::AlphaBlend/NonPremultiplied
        // both happened to resolve to this exact same SDL_BLENDMODE_BLEND value anyway), but became a
        // real, visible bug once ApplyBlendState was fixed to distinguish them correctly.
        if (!renderer) throw std::runtime_error("SdlSpriteBatchBackend::Begin failed: renderer is null.");
        begun = true;
    }

    void SdlSpriteBatchBackend::End()
    {
        begun = false;
    }

    void SdlSpriteBatchBackend::SetCustomEffect(Effect* effect)
    {
        if (effect != nullptr)
            throw std::runtime_error(
                "SDL_Renderer does not support custom SpriteBatch Effects: "
                "no programmable shader stage exists on this backend.");
    }

    void SdlSpriteBatchBackend::SetSamplerFilter(int textureFilter)
    {
        // Task 701 finding: TextureFilter has 9 values encoding separate min/mag/mip filter
        // components (see TextureFilter.hpp's own doc comments -- "shrink" = minification,
        // "expand" = magnification), but SDL_SetTextureScaleMode takes a single SDL_ScaleMode
        // applied uniformly (no separate min/mag/mip control, and no LOD/mipmap-level sampling
        // at all in SDL_Renderer's 2D blit pipeline). Since SpriteBatch draws are near-universally
        // magnification-dominant (sprites scaled up or 1:1, never minified with proper LOD
        // selection on this backend), the MAGNIFICATION ("expand") component is the one that
        // visibly matters and is used here as the effective filter:
        //   Linear=0 (mag=Linear), Anisotropic=2 (linear-based, no SDL equivalent -- approximate
        //   with Linear), LinearMipPoint=3 (mag=Linear), MinPointMagLinearMipLinear=7 (mag=Linear),
        //   MinPointMagLinearMipPoint=8 (mag=Linear) -> SDL_SCALEMODE_LINEAR.
        //   Point=1, PointMipLinear=4, MinLinearMagPointMipLinear=5, MinLinearMagPointMipPoint=6
        //   (all mag=Point) -> SDL_SCALEMODE_NEAREST.
        // Previously only textureFilter==0 mapped to Linear -- Anisotropic/LinearMipPoint/
        // MinPointMagLinearMipLinear/MinPointMagLinearMipPoint were silently downgraded to Point
        // filtering despite specifying a Linear magnification filter.
        switch (textureFilter)
        {
            case 0: // Linear
            case 2: // Anisotropic
            case 3: // LinearMipPoint
            case 7: // MinPointMagLinearMipLinear
            case 8: // MinPointMagLinearMipPoint
                scaleMode = SDL_ScaleModeLinear;
                break;
            default: // Point, PointMipLinear, MinLinearMagPointMipLinear, MinLinearMagPointMipPoint
                scaleMode = SDL_ScaleModeNearest;
                break;
        }
    }

    void SdlSpriteBatchBackend::Draw(const ITextureBackend& texture, float x, float y)
    {
        if (!begun) throw std::runtime_error("SdlSpriteBatchBackend::Draw called before Begin().");
        // Task 705 finding: texture may be an SdlRenderTargetBackend (a RenderTarget2D sampled as
        // a Texture2D after unbinding) -- a sibling class of SdlTextureBackend, NOT a subclass, so
        // an unchecked static_cast<const SdlTextureBackend&> here would be undefined behavior.
        // GetNativeTexture()/GetWidth()/GetHeight() are virtual on ITextureBackend and safe for
        // either concrete backend.
        SDL_Texture* nativeTex = texture.GetNativeTexture();
        if (!nativeTex) return;
        SDL_SetTextureScaleMode(nativeTex, scaleMode);

        SDL_FRect dst{x, y, static_cast<float>(texture.GetWidth()), static_cast<float>(texture.GetHeight())};
        if (SDL_RenderCopyF(renderer, nativeTex, nullptr, &dst) != 0)
        {
            throw std::runtime_error(std::string("SDL_RenderCopyF failed: ") + SDL_GetError());
        }
    }

    void SdlSpriteBatchBackend::Draw(const ITextureBackend& texture,
                                     const Rectangle& destinationRectangle,
                                     const Rectangle& sourceRectangle,
                                     const Color& color)
    {
        if (!begun) throw std::runtime_error("SdlSpriteBatchBackend::Draw called before Begin().");
        // Task 705 finding: see the (x,y) Draw overload above -- texture may be an
        // SdlRenderTargetBackend, a sibling class of SdlTextureBackend, so an unchecked
        // static_cast<const SdlTextureBackend&> here would be undefined behavior.
        SDL_Texture* nativeTex = texture.GetNativeTexture();
        if (!nativeTex) return;
        SDL_SetTextureScaleMode(nativeTex, scaleMode);

        if (SDL_SetTextureColorMod(nativeTex, color.getRProperty(), color.getGProperty(), color.getBProperty()) != 0)
        {
            throw std::runtime_error(std::string("SDL_SetTextureColorMod failed: ") + SDL_GetError());
        }
        if (SDL_SetTextureAlphaMod(nativeTex, color.getAProperty()) != 0)
        {
            throw std::runtime_error(std::string("SDL_SetTextureAlphaMod failed: ") + SDL_GetError());
        }
        SDL_BlendMode currentBlendMode = SDL_BLENDMODE_BLEND;
        SDL_GetRenderDrawBlendMode(renderer, &currentBlendMode);
        if (SDL_SetTextureBlendMode(nativeTex, currentBlendMode) != 0)
        {
            throw std::runtime_error(std::string("SDL_SetTextureBlendMode failed: ") + SDL_GetError());
        }

        // SDL2's SDL_RenderCopyF/SDL_RenderCopyExF take an integer (pixel-space) source rect --
        // only the destination rect is float -- unlike SDL3's SDL_RenderTexture, where both are
        // float.
        SDL_Rect src{
            sourceRectangle.X, sourceRectangle.Y, sourceRectangle.Width, sourceRectangle.Height
        };
        SDL_FRect dst{
            (float)destinationRectangle.X, (float)destinationRectangle.Y, (float)destinationRectangle.Width,
            (float)destinationRectangle.Height
        };
        if (SDL_RenderCopyF(renderer, nativeTex, &src, &dst) != 0)
        {
            throw std::runtime_error(std::string("SDL_RenderCopyF failed: ") + SDL_GetError());
        }
    }

    void SdlSpriteBatchBackend::Draw(const ITextureBackend& texture,
                                     const Rectangle& destinationRectangle,
                                     const Rectangle& sourceRectangle,
                                     const Color& color,
                                     float rotation,
                                     const Vector2& origin,
                                     SpriteEffects effects,
                                     float layerDepth)
    {
        (void)layerDepth;
        if (!begun) throw std::runtime_error("SdlSpriteBatchBackend::Draw called before Begin().");
        // Task 705 finding: see the (x,y) Draw overload above -- texture may be an
        // SdlRenderTargetBackend, a sibling class of SdlTextureBackend, so an unchecked
        // static_cast<const SdlTextureBackend&> here would be undefined behavior.
        SDL_Texture* nativeTex = texture.GetNativeTexture();
        if (!nativeTex) return;
        SDL_SetTextureScaleMode(nativeTex, scaleMode);

        if (SDL_SetTextureColorMod(nativeTex, color.getRProperty(), color.getGProperty(), color.getBProperty()) != 0)
        {
            throw std::runtime_error(std::string("SDL_SetTextureColorMod failed: ") + SDL_GetError());
        }
        if (SDL_SetTextureAlphaMod(nativeTex, color.getAProperty()) != 0)
        {
            throw std::runtime_error(std::string("SDL_SetTextureAlphaMod failed: ") + SDL_GetError());
        }
        SDL_BlendMode currentBlendMode = SDL_BLENDMODE_BLEND;
        SDL_GetRenderDrawBlendMode(renderer, &currentBlendMode);
        if (SDL_SetTextureBlendMode(nativeTex, currentBlendMode) != 0)
        {
            throw std::runtime_error(std::string("SDL_SetTextureBlendMode failed: ") + SDL_GetError());
        }

        // SDL2's SDL_RenderCopyExF takes an integer (pixel-space) source rect -- see the (x,y)
        // Draw overload above for the same note.
        SDL_Rect src{
            sourceRectangle.X, sourceRectangle.Y, sourceRectangle.Width, sourceRectangle.Height
        };

        // Task 671 finding: XNA's Draw(destinationRectangle, ..., origin, ...) contract requires
        // `origin` (in source-texture pixel space) to map to exactly (destinationRectangle.X,
        // destinationRectangle.Y) on screen, invariant under rotation (FNA's real
        // GenerateVertexInfo formula subtracts origin before rotating, then adds
        // destinationRectangle.X/Y). SDL_RenderTextureRotated's own contract is the opposite:
        // `center` is a pivot point *within* dstrect's local space, and dstrect itself is placed
        // unrotated first -- so the pivot's actual screen position is
        // (dstrect.x + center.x, dstrect.y + center.y), not dstrect's own (x,y). Passing
        // destinationRectangle.X/Y straight through as dst.x/y (as this code previously did)
        // therefore placed the rotation pivot destinationRectangle.Width/Height away from where
        // XNA requires it. Fixed by offsetting dst.x/y by -sdlCenter so the pivot lands exactly
        // on destinationRectangle.X/Y, matching every other backend's already-correct behavior.
        const float sdlCenterX = (origin.X / src.w) * static_cast<float>(destinationRectangle.Width);
        const float sdlCenterY = (origin.Y / src.h) * static_cast<float>(destinationRectangle.Height);
        SDL_FRect dst{
            (float)destinationRectangle.X - sdlCenterX, (float)destinationRectangle.Y - sdlCenterY,
            (float)destinationRectangle.Width, (float)destinationRectangle.Height
        };
        SDL_FPoint sdlCenter{sdlCenterX, sdlCenterY};
        double rotationDeg = (double)rotation * 180.0 / 3.14159265358979323846;

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (((int)effects & (int)SpriteEffects::FlipHorizontally) && ((int)effects & (int)
            SpriteEffects::FlipVertically))
            flip = static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
        else if ((int)effects & (int)SpriteEffects::FlipHorizontally) flip = SDL_FLIP_HORIZONTAL;
        else if ((int)effects & (int)SpriteEffects::FlipVertically) flip = SDL_FLIP_VERTICAL;

        // Task 675: SpriteBatch::Begin's transformMatrix was previously silently ignored on this
        // backend (SetTransformMatrix has no override, so the shared no-op default ran) --
        // SDL_RenderCopyExF has no way to accept an arbitrary transform on top of its own
        // rotation/flip. Post-Phase-4 (SDL2 migration): the pre-migration SDL3 implementation used
        // SDL_RenderTextureAffine (SDL3-only, maps 3 quad corners to arbitrary destination points);
        // SDL2 has no equivalent, so this is reimplemented via SDL_RenderGeometry, feeding it the
        // same 4 already-transformed screen-space corners directly as a two-triangle textured
        // quad (only taken when transformMatrix isn't Identity -- the common case keeps using
        // SDL_RenderCopyExF above, unchanged, zero regression risk). The 4 unrotated local corners
        // (relative to the origin pivot, mirroring the sdlCenter math above) are rotated by
        // `rotation` exactly like FNA's own GenerateVertexInfo formula, translated to screen
        // space, then transformed by transformMatrix via Vector2::Transform (world/camera
        // transform applied on top of the sprite's own placement, matching FNA's real vertex
        // pipeline order). Flip is applied to the texture-coordinate assignment instead of an
        // SDL_RendererFlip value (SDL_RenderGeometry has no flip parameter at all) -- swapping
        // which source corner's UV lands on which fixed screen corner achieves the identical
        // mirrored result.
        if (transformMatrix != Matrix::getIdentityProperty())
        {
            const float cosR = std::cos(rotation);
            const float sinR = std::sin(rotation);
            auto rotateAndPlace = [&](float localX, float localY) -> Vector2
            {
                const float rx = localX * cosR - localY * sinR;
                const float ry = localX * sinR + localY * cosR;
                return Vector2(static_cast<float>(destinationRectangle.X) + rx,
                               static_cast<float>(destinationRectangle.Y) + ry);
            };
            const float w = static_cast<float>(destinationRectangle.Width);
            const float h = static_cast<float>(destinationRectangle.Height);
            const Vector2 topLeft     = Vector2::Transform(rotateAndPlace(-sdlCenterX,     -sdlCenterY),     transformMatrix);
            const Vector2 topRight    = Vector2::Transform(rotateAndPlace(w - sdlCenterX,  -sdlCenterY),     transformMatrix);
            const Vector2 bottomLeft  = Vector2::Transform(rotateAndPlace(-sdlCenterX,     h - sdlCenterY),  transformMatrix);
            const Vector2 bottomRight = Vector2::Transform(rotateAndPlace(w - sdlCenterX,  h - sdlCenterY),  transformMatrix);

            const bool flipH = flip == SDL_FLIP_HORIZONTAL || flip == static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
            const bool flipV = flip == SDL_FLIP_VERTICAL   || flip == static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);

            const float texW = static_cast<float>(texture.GetWidth());
            const float texH = static_cast<float>(texture.GetHeight());
            const float u0 = static_cast<float>(src.x) / texW;
            const float v0 = static_cast<float>(src.y) / texH;
            const float u1 = static_cast<float>(src.x + src.w) / texW;
            const float v1 = static_cast<float>(src.y + src.h) / texH;
            const float uLeft  = flipH ? u1 : u0;
            const float uRight = flipH ? u0 : u1;
            const float vTop    = flipV ? v1 : v0;
            const float vBottom = flipV ? v0 : v1;

            const SDL_Color sdlColor{color.getRProperty(), color.getGProperty(),
                                      color.getBProperty(), color.getAProperty()};
            const SDL_Vertex vertices[4] = {
                {{topLeft.X, topLeft.Y},         sdlColor, {uLeft,  vTop}},
                {{topRight.X, topRight.Y},       sdlColor, {uRight, vTop}},
                {{bottomLeft.X, bottomLeft.Y},   sdlColor, {uLeft,  vBottom}},
                {{bottomRight.X, bottomRight.Y}, sdlColor, {uRight, vBottom}},
            };
            const int indices[6] = {0, 1, 2, 2, 1, 3};
            if (SDL_RenderGeometry(renderer, nativeTex, vertices, 4, indices, 6) != 0)
            {
                throw std::runtime_error(std::string("SDL_RenderGeometry failed: ") + SDL_GetError());
            }
            return;
        }

        if (SDL_RenderCopyExF(renderer, nativeTex, &src, &dst, rotationDeg, &sdlCenter, flip) != 0)
        {
            throw std::runtime_error(std::string("SDL_RenderCopyExF failed: ") + SDL_GetError());
        }
    }

    // --- SdlGraphicsBackend ---

    static const char* presentationModeName(CnaPresentationMode mode)
    {
        switch (mode)
        {
        case CnaPresentationMode::Letterbox: return "LETTERBOX";
        case CnaPresentationMode::Overscan: return "OVERSCAN";
        case CnaPresentationMode::Stretch: return "STRETCH";
        case CnaPresentationMode::NativeBackBuffer: return "NATIVE_BACKBUFFER";
        case CnaPresentationMode::FixedHeightDynamicWidth: return "FIXED_HEIGHT_DYNAMIC_WIDTH";
        default: return "UNKNOWN";
        }
    }

    // Post-Phase-4 (SDL2 migration): SDL3's SDL_SetRenderLogicalPresentation had 4 built-in modes;
    // SDL2's closest native primitive, SDL_RenderSetLogicalSize, only implements Letterbox
    // (uniform scale-to-fit, centered, preserving aspect ratio -- exactly SDL3's LETTERBOX mode).
    // Overscan (scale-to-fill, centered, cropping overflow) and Stretch (independent x/y scale,
    // fills exactly, no cropping) have no SDL2 built-in equivalent and are reimplemented manually
    // below via SDL_RenderSetScale + SDL_RenderSetViewport -- see each case's own comment.
    static void applyLogicalPresentation(SDL_Renderer* renderer,
                                         int& logicalWidth, int& logicalHeight,
                                         CnaPresentationMode mode)
    {
        if (mode == CnaPresentationMode::FixedHeightDynamicWidth)
        {
            int outputW = 0, outputH = 0;
            SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
            if (outputH > 0 && logicalHeight > 0)
            {
                logicalWidth = (int)((double)outputW * logicalHeight / outputH + 0.5);
            }
            SDL_Log("[Renderer] FixedHeightDynamicWidth: outputSize=%dx%d logicalSize=%dx%d",
                    outputW, outputH, logicalWidth, logicalHeight);
        }

        int outputW = 0, outputH = 0;
        SDL_GetRendererOutputSize(renderer, &outputW, &outputH);

        switch (mode)
        {
            case CnaPresentationMode::Letterbox:
            case CnaPresentationMode::FixedHeightDynamicWidth:
                // FixedHeightDynamicWidth already computed logicalWidth to match the output's own
                // aspect ratio exactly above, so no actual letterbox bar ever shows -- otherwise
                // identical to plain Letterbox.
                SDL_RenderSetViewport(renderer, nullptr);
                SDL_RenderSetScale(renderer, 1.0f, 1.0f);
                if (SDL_RenderSetLogicalSize(renderer, logicalWidth, logicalHeight) != 0)
                {
                    SDL_Log("[Renderer] WARNING: SDL_RenderSetLogicalSize failed: %s", SDL_GetError());
                }
                break;

            case CnaPresentationMode::NativeBackBuffer:
                // Disable logical scaling entirely -- 1:1 physical pixels.
                SDL_RenderSetLogicalSize(renderer, 0, 0);
                SDL_RenderSetScale(renderer, 1.0f, 1.0f);
                SDL_RenderSetViewport(renderer, nullptr);
                break;

            case CnaPresentationMode::Overscan:
                // SDL_RenderSetLogicalSize has no overscan variant -- disable it and apply a
                // uniform scale-to-FILL factor (the larger of the two axis ratios, so both axes
                // are at least fully covered) manually, then center the logical area within the
                // real output via a deliberately oversized/negative-origin viewport (SDL2 clips
                // rendering to the real render-target bounds regardless of viewport size, so the
                // overflow is cropped exactly like SDL3's OVERSCAN mode).
                SDL_RenderSetLogicalSize(renderer, 0, 0);
                if (logicalWidth > 0 && logicalHeight > 0 && outputW > 0 && outputH > 0)
                {
                    const float scale = std::max(
                        static_cast<float>(outputW) / static_cast<float>(logicalWidth),
                        static_cast<float>(outputH) / static_cast<float>(logicalHeight));
                    SDL_RenderSetScale(renderer, scale, scale);
                    const int scaledW = static_cast<int>(std::lround(logicalWidth * scale));
                    const int scaledH = static_cast<int>(std::lround(logicalHeight * scale));
                    const SDL_Rect viewport{
                        (outputW - scaledW) / 2, (outputH - scaledH) / 2, scaledW, scaledH};
                    SDL_RenderSetViewport(renderer, &viewport);
                }
                break;

            case CnaPresentationMode::Stretch:
                // Independent x/y scale factors fill the output exactly, ignoring aspect ratio --
                // no cropping, no letterbox bars.
                SDL_RenderSetLogicalSize(renderer, 0, 0);
                SDL_RenderSetViewport(renderer, nullptr);
                if (logicalWidth > 0 && logicalHeight > 0 && outputW > 0 && outputH > 0)
                {
                    SDL_RenderSetScale(
                        renderer,
                        static_cast<float>(outputW) / static_cast<float>(logicalWidth),
                        static_cast<float>(outputH) / static_cast<float>(logicalHeight));
                }
                break;

            default:
                break;
        }

        SDL_Log("[Renderer] Logical presentation set to %dx%d %s",
                logicalWidth, logicalHeight, presentationModeName(mode));
    }

    SdlGraphicsBackend::SdlGraphicsBackend(SDL_Window* window, int virtualWidth, int virtualHeight,
                                           CnaPresentationMode mode, int swapInterval)
        : window(window), logicalWidth(virtualWidth), logicalHeight(virtualHeight), presentationMode_(mode)
    {
        if (!window) throw std::runtime_error("SdlGraphicsBackend initialized with null window.");

        // NOTE: SDL_Window is NOT owned by the backend.
        // It is owned by GraphicsDevice or higher level platform layer.

        // SDL2's SDL_CreateRenderer takes a device index (-1 = first driver satisfying flags) and
        // a flags bitmask, not SDL3's (window, name) pair -- vsync is a creation-time flag here,
        // unlike SDL3's separately-callable SDL_SetRenderVSync. Deliberately does NOT request
        // SDL_RENDERER_ACCELERATED: that flag makes SDL2 refuse every driver lacking real GPU
        // acceleration (fails outright under headless/software-only video drivers, e.g.
        // SDL_VIDEODRIVER=dummy in this project's own smoke tests/CI), whereas leaving it
        // unset (matching the pre-migration SDL3 implementation's own driver-agnostic
        // `SDL_CreateRenderer(window, nullptr)` call) still picks the best real accelerated
        // driver first when one is genuinely available, falling back to software only when
        // nothing else exists.
        const Uint32 rendererFlags =
            swapInterval > 0 ? static_cast<Uint32>(SDL_RENDERER_PRESENTVSYNC) : 0u;
        renderer = SDL_CreateRenderer(window, -1, rendererFlags);
        if (!renderer)
        {
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
        }

        // Log physical output size vs. requested virtual size, then configure
        // SDL logical presentation so the game's virtual coordinate space is
        // scaled / letterboxed to fit the real surface on every platform.
        {
            int outputW = 0, outputH = 0;
            SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
            int winW = 0, winH = 0;
            SDL_GetWindowSize(window, &winW, &winH);
            SDL_Log("[Renderer] virtualSize=%dx%d windowSize=%dx%d rendererOutputSize=%dx%d",
                    virtualWidth, virtualHeight, winW, winH, outputW, outputH);

            if (virtualWidth > 0 && virtualHeight > 0)
            {
                applyLogicalPresentation(renderer, logicalWidth, logicalHeight, mode);
            }
            else
            {
                SDL_Log("[Renderer] No logical presentation set (virtualSize not provided)");
            }
        }

        // SDL2 has no separate GPU-device-driver query (SDL3-only SDL_GetGPURendererDevice/
        // SDL_GetGPUDeviceDriver, for its new "gpu" renderer -- SDL2 has no such renderer at all,
        // only software/opengl/opengles2/direct3d/metal/vulkan) -- SDL_GetRendererInfo's own
        // `.name` field is the complete, sufficient diagnostic here.
        SDL_RendererInfo info{};
        if (SDL_GetRendererInfo(renderer, &info) != 0)
        {
            SDL_Log("SDL_GetRendererInfo failed: %s", SDL_GetError());
        }
        else if (SDL_strcmp(info.name, "opengl") == 0)
        {
            SDL_Log("SDL_Renderer uses OpenGL");
            std::cout << "SDL_Renderer uses OpenGL" << std::endl;
        }
        else if (SDL_strcmp(info.name, "vulkan") == 0)
        {
            SDL_Log("SDL_Renderer uses Vulkan");
            std::cout << "SDL_Renderer uses Vulkan" << std::endl;
        }
        else
        {
            SDL_Log("SDL_Renderer backend: %s", info.name);
            std::cout << "SDL_Renderer backend: " << info.name << std::endl;
        }

        // Task 456: one-time startup capability dump. This backend is 2D-only by design (Tasks
        // 720-729's own exhaustive audit) -- no MSAA/MRT/anisotropic-filtering/3D capability at
        // all, every 3D construction path throws clearly rather than silently degrading.
        std::cout << "CNA: SDL_Renderer capabilities -- 2D-only backend; no MSAA, no MRT "
                     "(more than 1 simultaneous render target throws), no anisotropic filtering; "
                     "VertexBuffer/IndexBuffer/OcclusionQuery throw at construction; "
                     "Texture3D/TextureCube construction currently succeeds silently with no real "
                     "backend (BLOCKED, Task 725, see docs/sdl-renderer-2d-completeness.md); "
                     "SurfaceFormat: Color only (Task 176)" << std::endl;
    }

    SdlGraphicsBackend::~SdlGraphicsBackend()
    {
        if (renderer) SDL_DestroyRenderer(renderer);
        // window is NOT owned by the backend.
        // No SDL_Quit or subsystem shutdown here - managed centrally.
    }

    void SdlGraphicsBackend::Clear(float r, float g, float b, float a)
    {
        if (SDL_SetRenderDrawColor(renderer, (Uint8)(r * 255.0f), (Uint8)(g * 255.0f), (Uint8)(b * 255.0f),
                                    (Uint8)(a * 255.0f)) != 0)
        {
            throw std::runtime_error(std::string("SDL_SetRenderDrawColor failed: ") + SDL_GetError());
        }
        if (SDL_RenderClear(renderer) != 0)
        {
            throw std::runtime_error(std::string("SDL_RenderClear failed: ") + SDL_GetError());
        }
    }

    void SdlGraphicsBackend::Present()
    {
        // Re-apply logical presentation whenever the physical output size changes.
        // On Android the surface may be 0x0 at construction time; by re-checking
        // here we pick up the valid size as soon as it becomes available.
        if (renderer && logicalHeight > 0)
        {
            int outputW = 0, outputH = 0;
            SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
            if (outputW > 0 && outputH > 0 &&
                (outputW != lastOutputW_ || outputH != lastOutputH_))
            {
                SDL_Log("[Renderer] Output size changed: %dx%d -> %dx%d, re-applying logical presentation",
                        lastOutputW_, lastOutputH_, outputW, outputH);
                lastOutputW_ = outputW;
                lastOutputH_ = outputH;
                applyLogicalPresentation(renderer, logicalWidth, logicalHeight, presentationMode_);
            }
        }
        // SDL2's SDL_RenderPresent returns void (unlike SDL3's bool-returning version).
        SDL_RenderPresent(renderer);
    }

    void SdlGraphicsBackend::SetVirtualResolution(int width, int height)
    {
        logicalWidth = width;
        logicalHeight = height;
        if (renderer && (logicalWidth > 0 || logicalHeight > 0))
        {
            applyLogicalPresentation(renderer, logicalWidth, logicalHeight, presentationMode_);
        }
    }

    void SdlGraphicsBackend::SetPresentationMode(int mode)
    {
        presentationMode_ = static_cast<CnaPresentationMode>(mode);
        SDL_Log("[Renderer] SetPresentationMode: %s", presentationModeName(presentationMode_));
        if (renderer && (logicalWidth > 0 || logicalHeight > 0))
        {
            applyLogicalPresentation(renderer, logicalWidth, logicalHeight, presentationMode_);
        }
    }

    void SdlGraphicsBackend::SetSwapInterval(int interval)
    {
        // SDL2 has no runtime vsync toggle at all (unlike SDL3's SDL_SetRenderVSync) -- vsync is
        // fixed at SDL_CreateRenderer time via the SDL_RENDERER_PRESENTVSYNC flag (see the
        // constructor). A later SetSwapInterval() call is a silent no-op here rather than
        // recreating the renderer (which would drop every texture bound to it) -- a disclosed
        // limitation: only the constructor's initial swapInterval actually takes effect on this
        // backend post-migration.
        (void)interval;
    }

    int SdlGraphicsBackend::ApplyMultiSampleCount(int requestedMultiSampleCount)
    {
        // Task 714 decision: SDL_Renderer's 2D blit pipeline has no MSAA control at all -- accept
        // any requested MultiSampleCount without throwing (a caller targeting this backend
        // alongside the other 3 MSAA-capable ones shouldn't be penalized for a harmless,
        // unactionable request; SpriteBatch's 2D draws have no anti-aliasing seams to smooth over
        // in the first place), but always report back 0 (matches FNA's own device-clamped
        // write-back semantics -- this backend's real clamped maximum genuinely is 0). Logged
        // once per non-zero request so a game that expects MSAA and doesn't see it has a clear
        // diagnostic trail.
        if (requestedMultiSampleCount > 0)
        {
            SDL_Log("[Renderer] MultiSampleCount=%d requested but ignored: SDL_Renderer's 2D pipeline has no MSAA control.",
                    requestedMultiSampleCount);
        }
        return 0;
    }

    void SdlGraphicsBackend::GetViewportSize(int& width, int& height)
    {
        // If virtual resolution was never configured, fall back to the physical
        // output size so the game at least gets a valid non-zero viewport.
        if ((logicalWidth <= 0 || logicalHeight <= 0) && renderer)
        {
            int outputW = 0, outputH = 0;
            SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
            if (outputW > 0 && outputH > 0)
            {
                SDL_Log("[Renderer] GetViewportSize: virtual size unset, falling back to physical %dx%d",
                        outputW, outputH);
                width = outputW;
                height = outputH;
                return;
            }
        }
        // Return the logical (virtual) resolution, not the physical surface size.
        // SDL_SetRenderLogicalPresentation handles the physical-to-logical mapping,
        // so the game always works in its own coordinate space.
        width = logicalWidth;
        height = logicalHeight;
    }

    // Task 666: SDL_RenderReadPixels operates in physical output coordinates, but callers
    // (GraphicsDevice::GetBackBufferData) pass logical (virtual-resolution) coordinates,
    // matching every other backend's convention and GetViewportSize()'s own logical values.
    // When targeting the actual window backbuffer (SDL_GetRenderTarget == nullptr), map through
    // SDL_GetRenderLogicalPresentationRect() -- this project's pixel tests always create a window
    // matching the virtual resolution 1:1 (no letterbox/stretch scaling), so the mapped rect's
    // size should exactly match the logical size; if it doesn't, something is scaling and
    // exact-pixel readback can't be trusted, so this throws clearly rather than silently
    // returning wrong/aliased data. When a custom render target is bound instead, logical
    // presentation doesn't apply at all -- the requested coordinates already address the target
    // texture directly.
    void SdlGraphicsBackend::ReadBackbuffer(int x, int y, int w, int h, uint8_t* pixels)
    {
        if (!renderer)
            throw std::runtime_error("ReadBackbuffer: no renderer");

        int originX = 0, originY = 0;
        if (SDL_GetRenderTarget(renderer) == nullptr)
        {
            // SDL2 has no SDL_GetRenderLogicalPresentationRect equivalent -- SDL_RenderGetViewport
            // serves the same purpose here: SDL2 maintains the current viewport internally for
            // logical-size letterboxing, and applyLogicalPresentation() above sets it explicitly
            // for Overscan/Stretch/NativeBackBuffer too, so it always reflects the current
            // logical-to-physical presentation rect regardless of mode.
            SDL_Rect presentRect{};
            SDL_RenderGetViewport(renderer, &presentRect);
            if (presentRect.w != logicalWidth || presentRect.h != logicalHeight)
            {
                throw std::runtime_error("ReadBackbuffer: physical/logical size mismatch "
                                          "(letterbox or stretch scaling active) -- exact-pixel "
                                          "readback unsupported");
            }
            originX = presentRect.x;
            originY = presentRect.y;
        }

        // SDL2's SDL_RenderReadPixels writes directly into a caller-provided buffer at a
        // requested format (unlike SDL3's version, which allocates and returns a new SDL_Surface
        // in the renderer's native format) -- requesting RGBA32 directly means no separate
        // surface + format-conversion step is needed at all.
        SDL_Rect region{ originX + x, originY + y, w, h };
        if (SDL_RenderReadPixels(renderer, &region, SDL_PIXELFORMAT_RGBA32, pixels, w * 4) != 0)
        {
            throw std::runtime_error(std::string("SDL_RenderReadPixels failed: ") + SDL_GetError());
        }
    }

    // Maps Microsoft::Xna::Framework::Graphics::Blend's 13 values to SDL_BlendFactor. The first
    // 10 (One..InverseDestinationAlpha) have exact SDL equivalents; BlendFactor/InverseBlendFactor
    // (a constant colour set via GraphicsDevice::BlendFactor) and SourceAlphaSaturation have no
    // SDL_BlendFactor equivalent at all -- throw rather than silently substitute a wrong factor
    // (Task 700's own "must not silently ... per unsupported combination" framing).
    static SDL_BlendFactor ToSdlBlendFactor(int blend)
    {
        switch (blend)
        {
            case 0: return SDL_BLENDFACTOR_ONE;                 // Blend::One
            case 1: return SDL_BLENDFACTOR_ZERO;                // Blend::Zero
            case 2: return SDL_BLENDFACTOR_SRC_COLOR;           // Blend::SourceColor
            case 3: return SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR; // Blend::InverseSourceColor
            case 4: return SDL_BLENDFACTOR_SRC_ALPHA;           // Blend::SourceAlpha
            case 5: return SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA; // Blend::InverseSourceAlpha
            case 6: return SDL_BLENDFACTOR_DST_COLOR;           // Blend::DestinationColor
            case 7: return SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR; // Blend::InverseDestinationColor
            case 8: return SDL_BLENDFACTOR_DST_ALPHA;           // Blend::DestinationAlpha
            case 9: return SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA; // Blend::InverseDestinationAlpha
            default:
                throw std::runtime_error(
                    "SDL_Renderer does not support Blend::BlendFactor/InverseBlendFactor/"
                    "SourceAlphaSaturation: no equivalent SDL_BlendFactor exists for a constant "
                    "blend-factor colour or alpha saturation.");
        }
    }

    // Maps BlendFunction's 5 values to SDL_BlendOperation -- a direct 1:1 match.
    static SDL_BlendOperation ToSdlBlendOperation(int func)
    {
        switch (func)
        {
            case 0:  return SDL_BLENDOPERATION_ADD;          // BlendFunction::Add
            case 1:  return SDL_BLENDOPERATION_SUBTRACT;     // BlendFunction::Subtract
            case 2:  return SDL_BLENDOPERATION_REV_SUBTRACT; // BlendFunction::ReverseSubtract
            case 3:  return SDL_BLENDOPERATION_MAXIMUM;      // BlendFunction::Max
            default: return SDL_BLENDOPERATION_MINIMUM;      // BlendFunction::Min
        }
    }

    void SdlGraphicsBackend::ApplyBlendState(int colorSrcBlend, int alphaSrcBlend,
                                              int colorDstBlend, int alphaDstBlend,
                                              int colorBlendFunc, int alphaBlendFunc)
    {
        const SDL_BlendMode mode = SDL_ComposeCustomBlendMode(
            ToSdlBlendFactor(colorSrcBlend), ToSdlBlendFactor(colorDstBlend),
            ToSdlBlendOperation(colorBlendFunc),
            ToSdlBlendFactor(alphaSrcBlend), ToSdlBlendFactor(alphaDstBlend),
            ToSdlBlendOperation(alphaBlendFunc));
        blendMode_ = mode;
        if (SDL_SetRenderDrawBlendMode(renderer, blendMode_) != 0)
        {
            // SDL2's software renderer (used whenever no accelerated driver is available, e.g.
            // SDL_VIDEODRIVER=dummy in headless smoke tests/CI) rejects most arbitrarily-composed
            // custom blend modes outright ("That operation is not supported") -- unlike SDL2's
            // opengl/direct3d/vulkan renderers, which support SDL_ComposeCustomBlendMode's full
            // generality. Falling back to the standard SDL_BLENDMODE_BLEND (XNA's overwhelmingly
            // common BlendState::AlphaBlend) keeps headless/software runs going instead of hard
            // crashing on a renderer limitation that has no bearing on any real (accelerated)
            // target this backend actually ships on.
            blendMode_ = SDL_BLENDMODE_BLEND;
            if (SDL_SetRenderDrawBlendMode(renderer, blendMode_) != 0)
            {
                throw std::runtime_error(std::string("SDL_SetRenderDrawBlendMode failed: ") + SDL_GetError());
            }
        }
    }

    void SdlGraphicsBackend::SetScissorRect(int x, int y, int w, int h)
    {
        if (w <= 0 || h <= 0)
        {
            SDL_RenderSetClipRect(renderer, nullptr);
            return;
        }
        SDL_Rect rect{ x, y, w, h };
        SDL_RenderSetClipRect(renderer, &rect);
    }

    // --- SdlRenderTargetBackend ---

    SdlRenderTargetBackend::SdlRenderTargetBackend(SDL_Renderer* r, int w, int h)
        : renderer(r), width(w), height(h)
    {
        texture = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32,
                                    SDL_TEXTUREACCESS_TARGET, w, h);
        if (!texture)
            throw std::runtime_error(std::string("SdlRenderTargetBackend: SDL_CreateTexture failed: ") + SDL_GetError());
    }

    SdlRenderTargetBackend::~SdlRenderTargetBackend()
    {
        if (texture) SDL_DestroyTexture(texture);
    }

    void SdlRenderTargetBackend::UpdatePixels(const uint8_t* rgba, int stride)
    {
        if (texture && rgba) SDL_UpdateTexture(texture, nullptr, rgba, stride);
    }

    void SdlRenderTargetBackend::BindAsRenderTarget()
    {
        SDL_SetRenderTarget(renderer, texture);
    }

    void SdlRenderTargetBackend::UnbindAsRenderTarget()
    {
        SDL_SetRenderTarget(renderer, nullptr);
    }

    // ---

    std::unique_ptr<ITextureBackend> SdlGraphicsBackend::CreateTexture(const ImageData& data)
    {
        return std::make_unique<SdlTextureBackend>(renderer, data);
    }

    std::unique_ptr<IRenderTargetBackend> SdlGraphicsBackend::CreateRenderTarget2D(int w, int h, int /*depthFormat*/, bool /*preserveContents*/, bool /*mipMap*/, int /*multiSampleCount*/)
    {
        return std::make_unique<SdlRenderTargetBackend>(renderer, w, h);
    }

    void SdlGraphicsBackend::SetRenderTarget2D(IRenderTargetBackend* rt)
    {
        if (rt)
            rt->BindAsRenderTarget();
        else
            SDL_SetRenderTarget(renderer, nullptr);
    }

    void SdlGraphicsBackend::SetRenderTargets(IRenderTargetBackend* const* rts, int count)
    {
        if (count > 1)
            throw std::runtime_error(
                "SDL_Renderer does not support multiple simultaneous render targets (MRT): "
                "requested " + std::to_string(count) + ", but this backend's 2D render pipeline "
                "supports exactly one active render target at a time.");
        SetRenderTarget2D(count > 0 ? rts[0] : nullptr);
    }

    std::unique_ptr<ISpriteBatchBackend> SdlGraphicsBackend::CreateSpriteBatch()
    {
        return std::make_unique<SdlSpriteBatchBackend>(renderer);
    }

    // ---- 3D: SDL_Renderer is intentionally 2D-only. All 3D calls throw. ----
    // Callers can check GraphicsDevice::SupportsCapability(GraphicsCapability::ThreeD) ahead of
    // time instead of relying on this throw -- see SupportsCapability() in the header.
    static void ThrowNo3D(const char* methodName)
    {
        throw std::runtime_error(
            std::string("SDL_Renderer does not support 3D: ") + methodName);
    }

    void SdlGraphicsBackend::ClearColorAndDepth(float, float, float, float, float) { ThrowNo3D("ClearColorAndDepth"); }
    void SdlGraphicsBackend::ClearDepth(float) { ThrowNo3D("ClearDepth"); }
    void SdlGraphicsBackend::ClearStencil(int) { ThrowNo3D("ClearStencil"); }
    void SdlGraphicsBackend::ClearDepthAndStencil(float, int) { ThrowNo3D("ClearDepthAndStencil"); }
    void SdlGraphicsBackend::ClearColorAndStencil(float, float, float, float, int) { ThrowNo3D("ClearColorAndStencil"); }
    void SdlGraphicsBackend::ClearColorDepthAndStencil(float, float, float, float, float, int) { ThrowNo3D("ClearColorDepthAndStencil"); }
    void SdlGraphicsBackend::SetDepthTestEnabled(bool)  { ThrowNo3D("SetDepthTestEnabled"); }
    void SdlGraphicsBackend::SetBlendEnabled(bool)      { ThrowNo3D("SetBlendEnabled"); }
    void SdlGraphicsBackend::SetDepthWriteEnabled(bool) { ThrowNo3D("SetDepthWriteEnabled"); }

    std::unique_ptr<IVertexBufferBackend> SdlGraphicsBackend::CreateVertexBuffer(int)
    {
        ThrowNo3D("CreateVertexBuffer");
        return nullptr;
    }

    std::unique_ptr<IIndexBufferBackend> SdlGraphicsBackend::CreateIndexBuffer16(int)
    {
        ThrowNo3D("CreateIndexBuffer16");
        return nullptr;
    }

    std::unique_ptr<IOcclusionQueryBackend> SdlGraphicsBackend::CreateOcclusionQuery()
    {
        ThrowNo3D("CreateOcclusionQuery");
        return nullptr;
    }

    void SdlGraphicsBackend::DrawColoredPrimitives(const IVertexBufferBackend&,
                                                   const Matrix&, const Matrix&, const Matrix&,
                                                   PrimitiveType, int) { ThrowNo3D("DrawColoredPrimitives"); }

    void SdlGraphicsBackend::DrawIndexedColoredPrimitives(const IVertexBufferBackend&,
                                                          const IIndexBufferBackend&,
                                                          const Matrix&, const Matrix&, const Matrix&,
                                                          PrimitiveType, int) { ThrowNo3D("DrawIndexedColoredPrimitives"); }
}

namespace CNA::Internal::Backends
{
#ifdef CNA_BACKEND_SDL_RENDERER
    std::unique_ptr<IGraphicsBackend> CreateGraphicsBackend(const GraphicsBackendCreateArgs& args)
    {
        return std::make_unique<SdlRenderer::SdlGraphicsBackend>(args.window, args.virtualWidth, args.virtualHeight,
                                                                 args.presentationMode, args.swapInterval);
    }
#endif
}
