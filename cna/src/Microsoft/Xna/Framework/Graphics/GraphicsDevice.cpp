// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

#include "CNA/Internal/Backends/Common/IGraphicsBackend.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/TextInputEXT.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"

#ifdef CNA_BACKEND_BGFX
#include "CNA/Internal/Backends/Bgfx/BgfxGraphicsBackend.hpp"
#endif


#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/ObjectDisposedException.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    using CNA::Internal::Backends::CreateGraphicsBackend;
    using CNA::Internal::Backends::GraphicsBackendCreateArgs;

    namespace
    {
        std::runtime_error makeSdlError(const char* operation)
        {
            return std::runtime_error(std::string(operation) + " failed: " + SDL_GetError());
        }

        int toSwapInterval(PresentInterval pi)
        {
            switch (pi)
            {
                case PresentInterval::Immediate: return 0;
                case PresentInterval::Two:       return 2;
                default:                         return 1; // Default and One
            }
        }

        [[nodiscard]] bool hasClearFlag(ClearOptions options, ClearOptions flag)
        {
            return (static_cast<int>(options) & static_cast<int>(flag)) != 0;
        }

        void LogWindowDebugState(SDL_Window* window, const char* context)
        {
            if (window == nullptr)
            {
                SDL_Log("[WindowDebug] %s: window=null", context);
                return;
            }

            const SDL_WindowFlags flags = SDL_GetWindowFlags(window);
            const bool borderless = (flags & SDL_WINDOW_BORDERLESS) != 0;
            const bool fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;

            SDL_Log(
                "[WindowDebug] %s: flags=0x%llx borderless=%s fullscreen=%s",
                context,
                static_cast<unsigned long long>(flags),
                borderless ? "true" : "false",
                fullscreen ? "true" : "false"
            );
        }

        [[nodiscard]] SDL_WindowFlags getBackendWindowFlags()
        {
            SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;

#ifdef CNA_BACKEND_EASYGL
            windowFlags |= SDL_WINDOW_OPENGL;
#endif

#ifdef CNA_BACKEND_VULKAN
            windowFlags |= SDL_WINDOW_VULKAN;
#endif

#ifdef CNA_BACKEND_BGFX
            const auto rendererType = CNA::Internal::Backends::Bgfx::Detail::ResolveRendererType(
                SDL_getenv("CNA_BGFX_RENDERER"));
            switch (rendererType)
            {
            case bgfx::RendererType::Vulkan:
                windowFlags |= SDL_WINDOW_VULKAN;
                break;

            case bgfx::RendererType::OpenGL:
            case bgfx::RendererType::OpenGLES:
            case bgfx::RendererType::Count:
                windowFlags |= SDL_WINDOW_OPENGL;
                break;

            default:
                break;
            }
#endif

            return windowFlags;
        }
    }

    GraphicsDevice::GraphicsDevice()
        : GraphicsDevice(
            GraphicsAdapter::getDefaultAdapterProperty(),
            GraphicsProfile::Reach,
            PresentationParameters()
        )
    {
    }

    GraphicsDevice::GraphicsDevice(
        GraphicsAdapter& adapter,
        GraphicsProfile graphicsProfile,
        const PresentationParameters& presentationParameters
    )
        : window_(nullptr),
          ownsWindow_(false),
          backend_(nullptr),
          viewport_(),
          virtualWidth_(presentationParameters.getBackBufferWidthProperty()),
          virtualHeight_(presentationParameters.getBackBufferHeightProperty()),
          adapter_(&adapter),
          graphicsProfile_(graphicsProfile),
          presentationParameters_(presentationParameters),
          isDisposed_(false),
          blendState_(BlendState::Opaque),
          depthStencilState_(DepthStencilState::Default),
          rasterizerState_(RasterizerState::CullCounterClockwise),
          blendFactor_(Color::White)
    {
#ifdef __ANDROID__
        SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
#endif

        // plan_headless.md design decision 2 / plan_software.md design decision 4: the Headless and
        // Software backends never create a real window and never touch SDL's video subsystem at
        // all, so both can run in CI containers with no display server present -- not just a
        // headless-but-present one.
#if !defined(CNA_BACKEND_HEADLESS) && !defined(CNA_BACKEND_SOFTWARE)
        // PresentationParameters::HeadlessEXT is the runtime opt-in equivalent of the compile-time
        // guard above: a backend that normally wants a window (D3D12) can be asked for a genuinely
        // off-screen device instead. Skipping SDL_INIT_VIDEO is the point -- it is what lets such a
        // device run with no display server at all, not merely without a visible window.
        if (!presentationParameters_.getHeadlessEXTProperty())
        {
            if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
            {
                throw makeSdlError("SDL_InitSubSystem(SDL_INIT_VIDEO)");
            }
        }
#endif

        // The Touch Panel needs this for normalized-to-pixel touch coordinate scaling.
        Microsoft::Xna::Framework::Input::Touch::TouchPanel::setDisplayWidthProperty(virtualWidth_);
        Microsoft::Xna::Framework::Input::Touch::TouchPanel::setDisplayHeightProperty(virtualHeight_);

        createOrAttachWindow();
        applyPresentationParametersToWindow();
        createBackend();
        UpdateViewportFromWindow();

        // Task 896/955: blendState_/depthStencilState_/rasterizerState_ above were only ever
        // set as C++-level fields, never pushed to the backend's actual GPU state — every
        // backend started from its own hardcoded internal default (e.g. EasyGL's depth test is
        // plain OpenGL, which defaults to disabled, until something explicitly enables it) until
        // a game explicitly set one of these 3 state properties itself. Real FNA's own
        // GraphicsDevice constructor does exactly this same 3-line sync unconditionally
        // (GraphicsDevice.cs: "BlendState = BlendState.Opaque; DepthStencilState =
        // DepthStencilState.Default; RasterizerState = RasterizerState.CullCounterClockwise;") —
        // Task 896 ported only the 3rd line; this now ports the other 2 as well, matching FNA.
        setBlendStateProperty(blendState_);
        setDepthStencilStateProperty(depthStencilState_);
        setRasterizerStateProperty(rasterizerState_);
    }

    GraphicsDevice::~GraphicsDevice()
    {
        Dispose();
    }

    GraphicsAdapter& GraphicsDevice::getAdapterProperty() const
    {
        return adapter_ != nullptr ? *adapter_ : GraphicsAdapter::getDefaultAdapterProperty();
    }

    GraphicsProfile GraphicsDevice::getGraphicsProfileProperty() const
    {
        return graphicsProfile_;
    }

    PresentationParameters& GraphicsDevice::getPresentationParametersProperty()
    {
        return presentationParameters_;
    }

    const PresentationParameters& GraphicsDevice::getPresentationParametersProperty() const
    {
        return presentationParameters_;
    }

    const Viewport& GraphicsDevice::getViewportProperty() const
    {
        return viewport_;
    }

    void GraphicsDevice::Clear(const Color& color)
    {
        // Task 928: real XNA/FNA's single-argument overload clears the target, depth buffer,
        // AND stencil together -- Clear(ClearOptions.Target | ClearOptions.DepthBuffer |
        // ClearOptions.Stencil, color, Viewport.MaxDepth, 0) -- not just the color target. The
        // depth value used is the device's own CURRENT viewport's MaxDepth (not a hardcoded 1.0),
        // matching FNA's exact `Viewport.MaxDepth` reference (a GraphicsDevice property, not a
        // static constant).
        Clear(ClearOptions::Target | ClearOptions::DepthBuffer | ClearOptions::Stencil,
              color, getViewportProperty().getMaxDepthProperty(), 0);
    }

    void GraphicsDevice::Clear(ClearOptions options, const Color& color, float depth, int stencil)
    {
        if (backend_ == nullptr)
        {
            return;
        }

        if (hasClearFlag(options, ClearOptions::DepthBuffer))
        {
            if (depth < 0.0f || depth > 1.0f)
                throw System::ArgumentOutOfRangeException(
                    "depth", std::to_string(depth),
                    "'depth' must be between 0.0 and 1.0.");
        }

        // Matches FNA's own GraphicsDevice.Clear(ClearOptions, ...), which masks DepthBuffer/
        // Stencil out of `options` when the currently active target has no real depth-stencil
        // buffer, rather than forwarding a clear request the backend cannot honor. Ask the
        // BACKEND (Task 708's own precedent for RenderTarget2D), not the merely-requested XNA-
        // level format, since a backend may honor no depth/stencil buffer at all regardless of
        // what was requested (SDL_Renderer is entirely 2D-only and never has one). Without this,
        // GraphicsDevice::Clear(const Color&) -- which unconditionally requests
        // Target|DepthBuffer|Stencil, matching FNA's own single-argument overload -- crashes on
        // SDL_RENDERER instead of degrading to a color-only clear.
        //
        // NOXNA pruning note: this used to also check the currently-bound RenderTarget2D's own
        // depth buffer (via currentRenderTargets_) -- render-target binding (SetRenderTarget/
        // SetRenderTargets/RenderTarget2D) was removed as dead code (zero callers anywhere in the
        // reachable codebase), so only the backbuffer's own depth-stencil support is relevant now.
        const bool hasRealDepthBuffer = backend_->SupportsDepthStencil();
        if (!hasRealDepthBuffer)
        {
            options &= ClearOptions::Target;
        }

        const float r = static_cast<float>(color.getRProperty()) / 255.0f;
        const float g = static_cast<float>(color.getGProperty()) / 255.0f;
        const float b = static_cast<float>(color.getBProperty()) / 255.0f;
        const float a = static_cast<float>(color.getAProperty()) / 255.0f;

        const bool clearTarget  = hasClearFlag(options, ClearOptions::Target);
        const bool clearDepth   = hasClearFlag(options, ClearOptions::DepthBuffer);
        // Task 871: ClearOptions::Stencil was previously entirely ignored here -- neither checked
        // against `options` nor threaded through to any backend, so a requested stencil clear
        // silently did nothing on every backend.
        const bool clearStencil = hasClearFlag(options, ClearOptions::Stencil);

        if (clearTarget && clearDepth && clearStencil)
        {
            backend_->ClearColorDepthAndStencil(r, g, b, a, depth, stencil);
        }
        else if (clearTarget && clearDepth)
        {
            backend_->ClearColorAndDepth(r, g, b, a, depth);
        }
        else if (clearTarget && clearStencil)
        {
            backend_->ClearColorAndStencil(r, g, b, a, stencil);
        }
        else if (clearDepth && clearStencil)
        {
            backend_->ClearDepthAndStencil(depth, stencil);
        }
        else if (clearTarget)
        {
            backend_->Clear(r, g, b, a);
        }
        else if (clearDepth)
        {
            backend_->ClearDepth(depth);
        }
        else if (clearStencil)
        {
            backend_->ClearStencil(stencil);
        }
    }

    void GraphicsDevice::Present()
    {
        if (renderTargetBound_)
            throw System::InvalidOperationException("Cannot present while render targets are bound");

        if (backend_ != nullptr)
        {
            backend_->Present();
            UpdateViewportFromWindow();
        }
    }

    void GraphicsDevice::Reset(const PresentationParameters& presentationParameters, GraphicsAdapter& adapter)
    {
        Reset(presentationParameters, &adapter);
    }

    void GraphicsDevice::Reset(const PresentationParameters& presentationParameters, GraphicsAdapter* adapter)
    {
        DeviceResetting.Raise(this, System::EventArgs::Empty);

        presentationParameters_ = presentationParameters;
        if (adapter != nullptr)
        {
            adapter_ = adapter;
        }

        virtualWidth_ = presentationParameters_.getBackBufferWidthProperty();
        virtualHeight_ = presentationParameters_.getBackBufferHeightProperty();

        // The Touch Panel needs this too, for the same reason as the constructor.
        Microsoft::Xna::Framework::Input::Touch::TouchPanel::setDisplayWidthProperty(virtualWidth_);
        Microsoft::Xna::Framework::Input::Touch::TouchPanel::setDisplayHeightProperty(virtualHeight_);

        applyPresentationParametersToWindow();

        if (backend_ != nullptr)
        {
            backend_->SetVirtualResolution(virtualWidth_, virtualHeight_);

            // Task 902: reconfigure the backend's actual MSAA sample count in place, mirroring
            // FNA's own PresentationParameters.MultiSampleCount = FNA3D_GetMaxMultiSampleCount(...)
            // write-back of the real, device-clamped value after FNA3D_ResetBackbuffer().
            const int appliedMultiSampleCount = backend_->ApplyMultiSampleCount(
                presentationParameters_.getMultiSampleCountProperty());
            presentationParameters_.setMultiSampleCountProperty(appliedMultiSampleCount);

            // Previously missing: this Reset() overload never forwarded PresentationInterval to
            // the backend, unlike SetPresentationParameters()'s own identical field -- meaning
            // GraphicsDeviceManager.SynchronizeWithVerticalRetrace/ApplyChanges() (which always
            // goes through this path, not SetPresentationParameters()) never actually reached
            // IGraphicsBackend::SetSwapInterval() on any backend. Matches SetPresentationParameters()'s
            // own forwarding exactly.
            backend_->SetSwapInterval(toSwapInterval(presentationParameters_.getPresentationIntervalProperty()));

            // plan_dx9.md D9-30/D9-33: same "actually reach the backend" rationale as
            // ApplyMultiSampleCount above, for back-buffer/depth-stencil format and fullscreen --
            // needed because Game commonly constructs this GraphicsDevice (and its backend) with
            // default PresentationParameters before GraphicsDeviceManager.ApplyChanges() ever runs.
            backend_->UpdatePresentationFormatEXT(
                static_cast<int>(presentationParameters_.getBackBufferFormatProperty()),
                static_cast<int>(presentationParameters_.getDepthStencilFormatProperty()),
                presentationParameters_.getIsFullScreenProperty());
        }

        UpdateViewportFromWindow();
        DeviceReset.Raise(this, System::EventArgs::Empty);
    }

    void GraphicsDevice::Dispose()
    {
        if (isDisposed_)
        {
            return;
        }

        // Copy and clear the resource list before iterating.
        // This makes RemoveResourceReference a no-op when called re-entrantly
        // from within the resources' own Dispose() methods (matches FNA pattern).
        std::vector<GraphicsResource*> toDispose = std::move(resources_);
        resources_.clear();

        for (GraphicsResource* res : toDispose)
            static_cast<System::IDisposable*>(res)->Dispose();

        Disposing.Raise(this, System::EventArgs::Empty);
        destroyNativeResources();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        isDisposed_ = true;
    }

    void GraphicsDevice::OnResourceCreated(System::Object* resource)
    {
        if (!ResourceCreated.Empty())
            ResourceCreated.Raise(this, ResourceCreatedEventArgs(resource));
    }

    void GraphicsDevice::OnResourceDestroyed(const std::string& name, System::Object* tag)
    {
        if (!ResourceDestroyed.Empty())
            ResourceDestroyed.Raise(this, ResourceDestroyedEventArgs(name, tag));
    }

    void GraphicsDevice::AddResourceReference(GraphicsResource* resource)
    {
        resources_.push_back(resource);
    }

    void GraphicsDevice::RemoveResourceReference(GraphicsResource* resource)
    {
        for (std::size_t i = 0; i < resources_.size(); ++i)
        {
            if (resources_[i] == resource)
            {
                // Unordered removal — list order does not matter
                resources_[i] = resources_.back();
                resources_.pop_back();
                return;
            }
        }
    }

    void GraphicsDevice::SetDepthTestEnabled(bool enabled)
    {
        if (backend_ != nullptr) backend_->SetDepthTestEnabled(enabled);
    }

    void GraphicsDevice::SetBlendEnabled(bool enabled)
    {
        if (backend_ != nullptr) backend_->SetBlendEnabled(enabled);
    }

    void GraphicsDevice::SetDepthWriteEnabled(bool enabled)
    {
        if (backend_ != nullptr) backend_->SetDepthWriteEnabled(enabled);
    }

    void GraphicsDevice::SetGraphicsProfileEXT(GraphicsProfile profile)
    {
        graphicsProfile_ = profile;
    }

    CNA::Internal::Backends::IGraphicsBackend& GraphicsDevice::GetBackend() const
    {
        if (backend_ == nullptr)
        {
            throw std::runtime_error("GraphicsDevice backend is not available.");
        }

        return *backend_;
    }

    bool GraphicsDevice::SupportsCapability(CNA::GraphicsCapability capability) const
    {
        return GetBackend().SupportsCapability(capability);
    }

    const std::string& GraphicsDevice::GetTypeName() const
    {
        static const std::string typeName = "Microsoft.Xna.Framework.Graphics.GraphicsDevice";
        return typeName;
    }

    SDL_Renderer* GraphicsDevice::GetRendererInternal() const
    {
        return backend_ != nullptr ? backend_->GetRendererInternal() : nullptr;
    }

    SDL_Window* GraphicsDevice::GetWindowInternal() const
    {
        return backend_ != nullptr ? backend_->GetWindowInternal() : window_;
    }

    void GraphicsDevice::createOrAttachWindow()
    {
#if defined(CNA_BACKEND_HEADLESS) || defined(CNA_BACKEND_SOFTWARE)
        // No real window, ever -- see the constructor's matching guard above.
        // GraphicsBackendCreateArgs::window stays nullptr; UpdateViewportFromWindow() already
        // falls back to the backend's own GetViewportSize() first and only touches window_ if
        // that yields nothing, and applyPresentationParametersToWindow() already early-returns
        // when window_ is null, so neither needs its own guard.
        window_ = nullptr;
        ownsWindow_ = false;
#else
        // Runtime opt-in, same effect as the compile-time branch above. Only backends that can
        // genuinely run without a swap chain support this (D3D12 today) -- see
        // PresentationParameters::getHeadlessEXTProperty()'s own doc comment. A backend that cannot
        // (D3D11's constructor always creates a swap chain; EasyGL's GL context is bound to a
        // window) will throw from its own constructor, which is the honest outcome: it is a real
        // "this backend cannot do that" error, not something GraphicsDevice should paper over.
        if (presentationParameters_.getHeadlessEXTProperty())
        {
            window_ = nullptr;
            ownsWindow_ = false;
            return;
        }

        const auto requestedHandle = presentationParameters_.getDeviceWindowHandleProperty();
        if (requestedHandle != 0)
        {
            window_ = reinterpret_cast<SDL_Window*>(requestedHandle);
            ownsWindow_ = false;
            return;
        }

        SDL_WindowFlags windowFlags = getBackendWindowFlags();

        const int width = presentationParameters_.getBackBufferWidthProperty() > 0
                              ? presentationParameters_.getBackBufferWidthProperty()
                              : 1024;

        const int height = presentationParameters_.getBackBufferHeightProperty() > 0
                               ? presentationParameters_.getBackBufferHeightProperty()
                               : 768;

        window_ = SDL_CreateWindow("Game", width, height, windowFlags);
        if (window_ == nullptr)
        {
            throw makeSdlError("SDL_CreateWindow");
        }

        ownsWindow_ = true;
        presentationParameters_.
            setDeviceWindowHandleProperty(reinterpret_cast<PresentationParameters::IntPtr>(window_));

        // Publish the window to the text-input subsystem (mirrors FNA, which sets
        // TextInputEXT.WindowHandle at window creation). Required for StartTextInput etc.
        Microsoft::Xna::Framework::Input::TextInputEXT::setWindowHandleProperty(
            reinterpret_cast<std::uintptr_t>(window_));

        // Publish the same window to Mouse (mirrors FNA setting Mouse.WindowHandle at window
        // creation, SDL3_FNAPlatform.cs). Lets SetPosition / relative-mouse-mode target the
        // real window instead of relying on the SDL_GetMouseFocus() fallback.
        Microsoft::Xna::Framework::Input::Mouse::setWindowHandleProperty(
            reinterpret_cast<std::uintptr_t>(window_));

        LogWindowDebugState(window_, "after SDL_CreateWindow");
#endif
    }

    void GraphicsDevice::SetContextRecoveryEnabled(bool enabled)
    {
        contextRecoveryEnabled_ = enabled;
        if (backend_)
            backend_->SetContextRecoveryEnabled(enabled);
    }

    void GraphicsDevice::createBackend()
    {
        GraphicsBackendCreateArgs args;
        args.window = window_;
        args.virtualWidth = virtualWidth_;
        args.virtualHeight = virtualHeight_;
        args.contextRecoveryEnabled = contextRecoveryEnabled_;
        args.multiSampleCount = presentationParameters_.getMultiSampleCountProperty();
        args.swapInterval = toSwapInterval(presentationParameters_.getPresentationIntervalProperty());
        // plan_dx9.md D9-30: real presentation-parameter fidelity for backends that need it (D3D9);
        // every other backend continues to ignore these exactly as before the fields existed.
        args.backBufferFormat = static_cast<int>(presentationParameters_.getBackBufferFormatProperty());
        args.depthStencilFormat = static_cast<int>(presentationParameters_.getDepthStencilFormatProperty());
        args.isFullScreen = presentationParameters_.getIsFullScreenProperty();
        args.graphicsProfile = static_cast<int>(graphicsProfile_);
        // plan_dx9.md D9-34: forward a REAL, backend-detected device-lost/reset event to this
        // GraphicsDevice's own public XNA events. Nine of the ten backends never call this.
        args.deviceEventCallback = [this](CNA::Internal::Backends::BackendDeviceEvent event)
        {
            switch (event)
            {
                case CNA::Internal::Backends::BackendDeviceEvent::Lost:
                    deviceStatus_ = GraphicsDeviceStatus::Lost;
                    DeviceLost.Raise(this, System::EventArgs::Empty);
                    break;
                case CNA::Internal::Backends::BackendDeviceEvent::Resetting:
                    deviceStatus_ = GraphicsDeviceStatus::NotReset;
                    DeviceResetting.Raise(this, System::EventArgs::Empty);
                    break;
                case CNA::Internal::Backends::BackendDeviceEvent::Reset:
                    deviceStatus_ = GraphicsDeviceStatus::Normal;
                    DeviceReset.Raise(this, System::EventArgs::Empty);
                    break;
            }
        };

        backend_ = CreateGraphicsBackend(args);

        if (backend_ != nullptr)
        {
            backend_->SetVirtualResolution(virtualWidth_, virtualHeight_);
        }
    }

    void GraphicsDevice::destroyNativeResources()
    {
        backend_.reset();

        if (window_ != nullptr && ownsWindow_)
        {
            // Clear the text-input window handle if it points at this window
            // (mirrors FNA DisposeWindow, SDL3_FNAPlatform.cs:463-466).
            if (Microsoft::Xna::Framework::Input::TextInputEXT::getWindowHandleProperty()
                == reinterpret_cast<std::uintptr_t>(window_))
            {
                Microsoft::Xna::Framework::Input::TextInputEXT::setWindowHandleProperty(0);
            }
            if (Microsoft::Xna::Framework::Input::Mouse::getWindowHandleProperty()
                == reinterpret_cast<std::uintptr_t>(window_))
            {
                Microsoft::Xna::Framework::Input::Mouse::setWindowHandleProperty(0);
            }
            SDL_DestroyWindow(window_);
        }

        window_ = nullptr;
        ownsWindow_ = false;
    }

    void GraphicsDevice::UpdateViewportFromWindow()
    {
        int width = 0;
        int height = 0;

        if (backend_ != nullptr)
        {
            backend_->GetViewportSize(width, height);
        }

        if ((width <= 0 || height <= 0) && window_ != nullptr)
        {
            SDL_GetWindowSize(window_, &width, &height);
        }

        if (width <= 0 || height <= 0)
        {
            return;
        }

        // Compared against the last size *this method itself* produced, not against
        // viewport_'s current width/height: viewport_ may hold a game-set custom
        // sub-region Viewport (e.g. split-screen) whose dimensions legitimately differ
        // from the backbuffer, and FNA's Present() never touches Viewport at all. Using
        // viewport_ as the "did anything change" signal would silently stomp such a
        // Viewport back to full-window size on the very next Present() call even though
        // no resize occurred.
        if (width == lastKnownViewportWidth_ && height == lastKnownViewportHeight_)
        {
            return;
        }

        lastKnownViewportWidth_ = width;
        lastKnownViewportHeight_ = height;

        viewport_.setXProperty(0);
        viewport_.setYProperty(0);
        viewport_.setMinDepthProperty(0.0f);
        viewport_.setMaxDepthProperty(1.0f);
        viewport_.setWidthProperty(width);
        viewport_.setHeightProperty(height);

        // Mutates viewport_'s fields directly (not via setViewportProperty(), to preserve the
        // "compared against lastKnownViewportWidth/Height_, not viewport_" semantics above) --
        // push the reset value to the backend explicitly (Task 880) so a window resize actually
        // updates the GPU-side viewport too, not just the C++-side Viewport property.
        if (backend_)
            backend_->SetViewport(0, 0, width, height, 0.0f, 1.0f);
    }

    void GraphicsDevice::SetVirtualResolution(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        virtualWidth_ = width;
        virtualHeight_ = height;

        presentationParameters_.setBackBufferWidthProperty(width);
        presentationParameters_.setBackBufferHeightProperty(height);

        if (backend_ != nullptr)
        {
            backend_->SetVirtualResolution(width, height);
        }

        UpdateViewportFromWindow();
    }

    void GraphicsDevice::SetPresentationMode(int mode)
    {
        if (backend_ != nullptr)
        {
            backend_->SetPresentationMode(mode);
        }
    }

    void GraphicsDevice::applyPresentationParametersToWindow()
    {
        if (window_ == nullptr)
        {
            return;
        }

        // Task 902: fullscreen switching may not be available in headless / virtual-display
        // test environments (Xvfb). The PP value is already stored above this call, so a
        // backend that cannot actually switch fullscreen still has the correct stored state --
        // matches GraphicsDeviceManager::applyToExistingBackend()'s identical non-fatal handling
        // (Task 224), which this method now supersedes as the single fullscreen-application path.
        const bool fullScreen = presentationParameters_.getIsFullScreenProperty();
        if (!SDL_SetWindowFullscreen(window_, fullScreen))
        {
            SDL_ClearError();
        }

        const int width = presentationParameters_.getBackBufferWidthProperty();
        const int height = presentationParameters_.getBackBufferHeightProperty();
        if (width > 0 && height > 0)
        {
#ifndef __ANDROID__

            if (!SDL_SetWindowSize(window_, width, height))
            {
                throw makeSdlError("SDL_SetWindowSize");
            }
#endif
        }
    }

    // --- New XNA 4.0 API methods ---

    TextureCollection& GraphicsDevice::getTexturesProperty() { return textures_; }
    TextureCollection& GraphicsDevice::getVertexTexturesProperty() { return vertexTextures_; }

    void GraphicsDevice::setBlendStateProperty(const BlendState& value)
    {
        blendState_ = value;
        if (backend_)
            backend_->ApplyBlendState(
                (int)value.getColorSourceBlendProperty(),
                (int)value.getAlphaSourceBlendProperty(),
                (int)value.getColorDestinationBlendProperty(),
                (int)value.getAlphaDestinationBlendProperty(),
                (int)value.getColorBlendFunctionProperty(),
                (int)value.getAlphaBlendFunctionProperty());
        // FNA applies BlendState.BlendFactor atomically as part of FNA3D_SetBlendState — the
        // state's own baked-in blend factor becomes the device's current one, the same way
        // GraphicsDevice.BlendFactor's own setter would.
        setBlendFactorProperty(value.getBlendFactorProperty());
    }

    void GraphicsDevice::setDepthStencilStateProperty(const DepthStencilState& value)
    {
        depthStencilState_ = value;
        if (backend_)
            backend_->ApplyDepthStencilState(
                value.getDepthBufferEnableProperty(),
                value.getDepthBufferWriteEnableProperty(),
                (int)value.getDepthBufferFunctionProperty(),
                value.getStencilEnableProperty(),
                (int)value.getStencilFunctionProperty(),
                (int)value.getStencilPassProperty(),
                (int)value.getStencilFailProperty(),
                (int)value.getStencilDepthBufferFailProperty(),
                value.getStencilMaskProperty(),
                value.getStencilWriteMaskProperty(),
                value.getReferenceStencilProperty(),
                value.getTwoSidedStencilModeProperty(),
                (int)value.getCounterClockwiseStencilFunctionProperty(),
                (int)value.getCounterClockwiseStencilPassProperty(),
                (int)value.getCounterClockwiseStencilFailProperty(),
                (int)value.getCounterClockwiseStencilDepthBufferFailProperty());
        // FNA applies a DepthStencilState's own ReferenceStencil atomically as part of the whole
        // native state struct, the same way BlendState's own BlendFactor is applied (Task 309) -
        // keep GraphicsDevice.ReferenceStencil in sync with whatever state was just assigned.
        setReferenceStencilProperty(value.getReferenceStencilProperty());
    }

    void GraphicsDevice::setRasterizerStateProperty(const RasterizerState& value)
    {
        rasterizerState_ = value;
        if (backend_)
            backend_->ApplyRasterizerState(
                (int)value.getCullModeProperty(),
                (int)value.getFillModeProperty(),
                value.getScissorTestEnableProperty(),
                value.getDepthBiasProperty(),
                value.getSlopeScaleDepthBiasProperty());
    }

    Color GraphicsDevice::getBlendFactorProperty() const { return blendFactor_; }
    void GraphicsDevice::setBlendFactorProperty(const Color& value)
    {
        blendFactor_ = value;
        if (backend_)
            backend_->SetBlendFactor(
                value.getRProperty() / 255.0f,
                value.getGProperty() / 255.0f,
                value.getBProperty() / 255.0f,
                value.getAProperty() / 255.0f);
    }

    int GraphicsDevice::getMultiSampleMaskProperty() const { return multiSampleMask_; }
    void GraphicsDevice::setMultiSampleMaskProperty(int value) { multiSampleMask_ = value; }

    int GraphicsDevice::getReferenceStencilProperty() const { return referenceStencil_; }
    void GraphicsDevice::setReferenceStencilProperty(int value)
    {
        referenceStencil_ = value;
        if (backend_)
            backend_->SetReferenceStencil(value);
    }
}
