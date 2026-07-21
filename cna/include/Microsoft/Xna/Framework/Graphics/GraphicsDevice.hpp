// SPDX-License-Identifier: MS-PL
#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ClearOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceStatus.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ResourceCreatedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Graphics/ResourceDestroyedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/IDisposable.hpp"
#include "System/Object.hpp"
#include "CNA/CNAHelper.hpp"
#include "CNA/GraphicsBackendType.hpp"
#include "CNA/GraphicsCapability.hpp"

struct SDL_Window;
struct SDL_Renderer;

namespace Microsoft::Xna::Framework
{
    class Game;
    class GameWindow;
    class GraphicsDeviceManager;
}

namespace CNA::Internal::Backends
{
    class IGraphicsBackend;
}

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice : public System::Object, public System::IDisposable
    {
    public:
        // --- Events ---
        /** @brief Raised when this device is disposed. */
        System::EventHandler<System::EventArgs> Disposing;
        /** @brief Raised when the device is lost (XNA compliance; never raised on desktop). */
        System::EventHandler<System::EventArgs> DeviceLost;
        /** @brief Raised after the device has been reset. */
        System::EventHandler<System::EventArgs> DeviceReset;
        /** @brief Raised before the device is reset. */
        System::EventHandler<System::EventArgs> DeviceResetting;
        /** @brief Raised when a graphics resource is created. */
        System::EventHandler<ResourceCreatedEventArgs> ResourceCreated;
        /** @brief Raised when a graphics resource is destroyed. */
        System::EventHandler<ResourceDestroyedEventArgs> ResourceDestroyed;

        // --- Constructors ---
        /** @brief Initializes a GraphicsDevice with no window (headless mode). */
        GraphicsDevice();

        /**
         * @brief Initializes a new GraphicsDevice for the given adapter and presentation settings.
         *
         * @param adapter                The graphics adapter to use.
         * @param graphicsProfile        The graphics profile (Reach or HiDef).
         * @param presentationParameters The presentation options (back-buffer size, format, etc.).
         */
        GraphicsDevice(GraphicsAdapter& adapter, GraphicsProfile graphicsProfile,
                       const PresentationParameters& presentationParameters);

        /** @brief Destructor. */
        NOXNA ~GraphicsDevice() override;

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;
        GraphicsDevice(GraphicsDevice&&) = delete;
        GraphicsDevice& operator=(GraphicsDevice&&) = delete;

        // --- State properties ---
        /** @brief Returns the graphics adapter associated with this device. */
        [[nodiscard]] GraphicsAdapter& getAdapterProperty() const;
        /** @brief Returns the graphics profile used to create this device. */
        [[nodiscard]] GraphicsProfile getGraphicsProfileProperty() const;
        /** @brief Returns the presentation parameters for this device (mutable). */
        [[nodiscard]] PresentationParameters& getPresentationParametersProperty();
        /** @brief Returns the presentation parameters for this device (const). */
        [[nodiscard]] const PresentationParameters& getPresentationParametersProperty() const;

        // --- GL State ---
        /** @brief Returns the texture collection for pixel shader sampler slots. */
        [[nodiscard]] TextureCollection& getTexturesProperty();
        /** @brief Returns the texture collection for vertex shader sampler slots. */
        [[nodiscard]] TextureCollection& getVertexTexturesProperty();

        /**
         * @brief Sets the blend state.
         * @param value The new blend state to apply.
         */
        void setBlendStateProperty(const BlendState& value);

        /**
         * @brief Sets the depth-stencil state.
         * @param value The new depth-stencil state to apply.
         */
        void setDepthStencilStateProperty(const DepthStencilState& value);

        /**
         * @brief Sets the rasterizer state.
         * @param value The new rasterizer state to apply.
         */
        void setRasterizerStateProperty(const RasterizerState& value);

        /** @brief Returns the current viewport. */
        [[nodiscard]] const Viewport& getViewportProperty() const;

        /** @brief Returns the current blend factor color. */
        [[nodiscard]] Color getBlendFactorProperty() const;
        /**
         * @brief Sets the blend factor color.
         * @param value The color to use as blend factor.
         */
        void setBlendFactorProperty(const Color& value);

        /** @brief Returns the current multisample mask. */
        [[nodiscard]] int getMultiSampleMaskProperty() const;
        /**
         * @brief Sets the multisample mask.
         * @param value The bitmask for multisample anti-aliasing.
         */
        void setMultiSampleMaskProperty(int value);

        /** @brief Returns the current reference stencil value. */
        [[nodiscard]] int getReferenceStencilProperty() const;
        /**
         * @brief Sets the reference stencil value.
         * @param value The reference value for stencil operations.
         */
        void setReferenceStencilProperty(int value);

        // --- Core operations ---
        /**
         * @brief Clears the back buffer to the specified color.
         * @param color The color to clear to.
         */
        void Clear(const Color& color);
        /**
         * @brief Clears the specified buffers.
         * @param options Flags indicating which buffers to clear.
         * @param color   Color value for the color buffer.
         * @param depth   Depth value for the depth buffer (0–1).
         * @param stencil Stencil value for the stencil buffer.
         */
        void Clear(ClearOptions options, const Color& color, float depth, int stencil);

        /** @brief Presents the rendered frame to the display. */
        void Present();

        /**
         * @brief Resets the device with new presentation parameters and a specific adapter.
         * @param presentationParameters The new presentation parameters.
         * @param adapter                The graphics adapter to use.
         */
        void Reset(const PresentationParameters& presentationParameters, GraphicsAdapter& adapter);
        /**
         * @brief Resets the device with new presentation parameters and an optional adapter pointer.
         * @param presentationParameters The new presentation parameters.
         * @param adapter                Pointer to the graphics adapter, or nullptr to keep the current one.
         */
        void Reset(const PresentationParameters& presentationParameters, GraphicsAdapter* adapter);

        /** @brief Releases all resources held by this device. */
        void Dispose() override;

        /**
         * @brief Fires ResourceCreated for the given resource.
         *
         * Called by GraphicsResource constructors when a device is attached.
         * @param resource The newly created resource.
         */
        NOXNA void OnResourceCreated(System::Object* resource);

        /**
         * @brief Fires ResourceDestroyed with the given name and tag.
         *
         * Called by GraphicsResource::Dispose before the resource is marked disposed.
         * @param name The Name of the resource being destroyed.
         * @param tag  The Tag of the resource being destroyed (may be nullptr).
         */
        NOXNA void OnResourceDestroyed(const std::string& name, System::Object* tag);

        /**
         * @brief Registers a resource for tracking.
         *
         * Called by GraphicsResource constructor. The device will dispose registered
         * resources before its own backend is destroyed, preventing use-after-free.
         * @param resource The resource to track.
         */
        NOXNA void AddResourceReference(GraphicsResource* resource);

        /**
         * @brief Unregisters a previously tracked resource.
         *
         * Called by GraphicsResource::Dispose(bool). Safe to call during device disposal
         * (the tracking list is cleared before iteration).
         * @param resource The resource to remove.
         */
        NOXNA void RemoveResourceReference(GraphicsResource* resource);

        /** @brief Enables or disables depth testing. */
        NOXNA void SetDepthTestEnabled(bool enabled);
        /** @brief Enables or disables blending. */
        NOXNA void SetBlendEnabled(bool enabled);
        /** @brief Enables or disables depth writes. */
        NOXNA void SetDepthWriteEnabled(bool enabled);
        /**
         * @brief Task/plan_dx9.md D9-103 finding: real XNA fixes GraphicsProfile at device
         * construction (the public GraphicsDevice.GraphicsProfile property is read-only), but
         * CNA's own GraphicsDeviceManager architecture eagerly default-constructs Game's
         * GraphicsDevice_ member (hardcoded GraphicsProfile::Reach) BEFORE GraphicsDeviceManager
         * -- and therefore before a game's own GraphicsDeviceManager.GraphicsProfile request --
         * even exists. Without this, GraphicsDeviceManager::CreateDevice()/ApplyChanges() has no
         * way to make the FIRST real device creation honor a non-Reach request at all: a game's
         * `graphics.GraphicsProfile = GraphicsProfile.HiDef; graphics.ApplyChanges();` would
         * silently keep using Reach. Called once, internally, from
         * GraphicsDeviceManager::applyToExistingBackend() right before Reset() -- not exposed as
         * a general public runtime profile-switch (real XNA has none either).
         */
        NOXNA void SetGraphicsProfileEXT(GraphicsProfile profile);
        /**
         * @brief Disables GL context-loss recovery (CPU shadow copies + ResourceRegistry).
         *
         * Must be called before the device is initialized. Safe on desktop where
         * context loss never occurs; saves approximately one copy of texture RAM per loaded texture.
         *
         * @param enabled Pass false to disable context recovery.
         */
        NOXNA void SetContextRecoveryEnabled(bool enabled);

        /** @brief Returns a reference to the active graphics backend. */
        NOXNA [[nodiscard]] CNA::Internal::Backends::IGraphicsBackend& GetBackend() const;

        /**
         * @brief Returns whether the active backend (and, for device-dependent entries, the
         * current runtime device/driver) supports the given CNA::GraphicsCapability.
         *
         * Query this before relying on a feature that isn't universally supported (e.g. 3D on
         * the 2D-only SDL_Renderer/DX3/Canvas backends), instead of calling it and handling the
         * resulting exception.
         *
         * @param capability The capability to check.
         * @return True if supported by the active backend/device.
         */
        NOXNA [[nodiscard]] bool SupportsCapability(CNA::GraphicsCapability capability) const;

        /** @brief Returns the fully qualified .NET type name of this class. */
        NOXNA [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        SDL_Window* window_;
        bool ownsWindow_;
        std::unique_ptr<CNA::Internal::Backends::IGraphicsBackend> backend_;
        Viewport viewport_;
        int virtualWidth_;
        int virtualHeight_;
        int lastKnownViewportWidth_ = -1;
        int lastKnownViewportHeight_ = -1;
        bool contextRecoveryEnabled_ = true;
        GraphicsAdapter* adapter_;
        GraphicsProfile graphicsProfile_;
        PresentationParameters presentationParameters_;
        bool isDisposed_;
        /// plan_dx9.md D9-34: tracks the real device-lifecycle state reported by a backend via
        /// GraphicsBackendCreateArgs::deviceEventCallback (BackendDeviceEvent::Lost -> Lost,
        /// Resetting -> NotReset, Reset -> Normal). Every backend except D3D9 never calls that
        /// callback, so this stays Normal there, matching the pre-existing hardcoded behavior.
        GraphicsDeviceStatus deviceStatus_ = GraphicsDeviceStatus::Normal;

        BlendState blendState_;
        DepthStencilState depthStencilState_;
        RasterizerState rasterizerState_;
        Color blendFactor_;
        int multiSampleMask_ = -1;
        int referenceStencil_ = 0;

        TextureCollection textures_;
        TextureCollection vertexTextures_;

        bool renderTargetBound_ = false;
        std::vector<GraphicsResource*> resources_;

        [[nodiscard]] SDL_Renderer* GetRendererInternal() const;
        [[nodiscard]] SDL_Window* GetWindowInternal() const;

        void createOrAttachWindow();
        void createBackend();
        void destroyNativeResources();
        void UpdateViewportFromWindow();
        void SetVirtualResolution(int width, int height);
        void SetPresentationMode(int mode);
        void applyPresentationParametersToWindow();

        friend class Texture2D;
        friend class ShaderEffect;
        friend class SpriteBatch;
        friend class Microsoft::Xna::Framework::GameWindow;
        friend class Microsoft::Xna::Framework::GraphicsDeviceManager;
        friend class Microsoft::Xna::Framework::Game;
    };
}
