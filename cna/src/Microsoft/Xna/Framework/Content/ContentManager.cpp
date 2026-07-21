// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "System/IServiceProvider.hpp"
#include "CNA/Internal/Backends/Common/IGraphicsBackend.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>

namespace Microsoft::Xna::Framework::Content
{
    // ---------------------------------------------------------------------------
    // Property accessors
    // ---------------------------------------------------------------------------

    const std::string& ContentManager::getRootDirectoryProperty() const
    {
        return rootDirectory_;
    }

    void ContentManager::setRootDirectoryProperty(const std::string& v)
    {
        rootDirectory_ = v;
    }

    void ContentManager::setRootDirectoryProperty(std::string&& v)
    {
        rootDirectory_ = std::move(v);
    }

    // ---------------------------------------------------------------------------
    // Constructor / lifecycle
    // ---------------------------------------------------------------------------

    ContentManager::ContentManager(System::IServiceProvider* serviceProvider)
        : serviceProvider_(serviceProvider)
    {
        RegisterBuiltinLoaders();
    }

    ContentManager::ContentManager(System::IServiceProvider* serviceProvider,
                                   const std::string& rootDirectory)
        : serviceProvider_(serviceProvider), rootDirectory_(rootDirectory)
    {
        RegisterBuiltinLoaders();
    }

    ContentManager::ContentManager()
    {
        RegisterBuiltinLoaders();
    }

    System::IServiceProvider* ContentManager::getServiceProviderProperty() const
    {
        return serviceProvider_;
    }

    void ContentManager::setGraphicsDevice(Graphics::GraphicsDevice& graphicsDevice)
    {
        graphicsDevice_ = &graphicsDevice;
    }

    Graphics::GraphicsDevice& ContentManager::getGraphicsDeviceInternal() const
    {
        if (graphicsDevice_ == nullptr)
        {
            throw ContentLoadException(
                "ContentManager: GraphicsDevice is not set. "
                "Call setGraphicsDevice() before loading GPU resources.");
        }
        return *graphicsDevice_;
    }

    void ContentManager::Dispose()
    {
        if (!disposed_)
        {
            Unload();
            disposed_ = true;
        }
    }

    void ContentManager::Unload()
    {
        loadedAssets_.clear();
        textureCache_.clear();
    }

    // ---------------------------------------------------------------------------
    // Path helpers
    // ---------------------------------------------------------------------------

    std::string ContentManager::BuildAssetPath(const std::string& assetName) const
    {
        if (assetName.empty())
        {
            return rootDirectory_;
        }
        if (rootDirectory_.empty())
        {
            return assetName;
        }
        namespace fs = std::filesystem;
        return (fs::path(rootDirectory_) / assetName).string();
    }

    std::string ContentManager::NormalizeKey(const std::string& assetName) const
    {
        std::string key = assetName;
        std::replace(key.begin(), key.end(), '\\', '/');
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return key;
    }

    // ---------------------------------------------------------------------------
    // Built-in type readers
    // ---------------------------------------------------------------------------

    namespace
    {
        class Texture2DTypeReader : public LooseFileContentTypeReader<Graphics::Texture2D>
        {
        public:
            [[nodiscard]] std::vector<std::string> GetExtensions() const override
            {
                return {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tga", ".tif", ".tiff", ".qoi"};
            }

            Graphics::Texture2D Read(const std::string& path, ContentManager& cm) override
            {
                Graphics::GraphicsDevice& gd = cm.getGraphicsDeviceInternal();
                return Graphics::Texture2D(path, gd);
            }
        };

        class SoundEffectTypeReader : public LooseFileContentTypeReader<Audio::SoundEffect>
        {
        public:
            [[nodiscard]] std::vector<std::string> GetExtensions() const override
            {
                return {".wav"};
            }

            Audio::SoundEffect Read(const std::string& path, ContentManager& /*cm*/) override
            {
                return Audio::SoundEffect(path);
            }
        };
    } // anonymous namespace

    // ---------------------------------------------------------------------------

    void ContentManager::RegisterBuiltinLoaders()
    {
        RegisterTypeReader<Graphics::Texture2D>(std::make_unique<Texture2DTypeReader>());
        RegisterTypeReader<Audio::SoundEffect>(std::make_unique<SoundEffectTypeReader>());
    }

} // namespace Microsoft::Xna::Framework::Content

// ---------------------------------------------------------------------------
// Explicit specialisation: weak-cache for Texture2D
// ---------------------------------------------------------------------------
// Textures are NOT stored in loadedAssets_ (strong cache). Instead only weak
// references are kept. When the last external Texture2D copy is destroyed its
// GPU backend is freed immediately, preventing per-world RAM growth caused by
// world-specific background textures accumulating in the cache.
// ---------------------------------------------------------------------------

namespace Microsoft::Xna::Framework::Content
{
    template<>
    Graphics::Texture2D ContentManager::Load<Graphics::Texture2D>(const std::string& assetName)
    {
        if (disposed_)
            throw std::runtime_error("ContentManager has been disposed.");

        const std::string key = NormalizeKey(assetName);
        log::Debug(std::string("Loading texture: ") + assetName);

        auto cacheIt = textureCache_.find(key);
        if (cacheIt != textureCache_.end())
        {
            auto backendSp   = cacheIt->second.backend.lock();
            auto cpuPixelsSp = cacheIt->second.cpuPixels.lock(); // may be null when context recovery disabled
            if (backendSp)
            {
                // Reuse the existing GPU backend — no reload from disk needed.
                const int w = backendSp->GetWidth();
                const int h = backendSp->GetHeight();
                return Graphics::Texture2D::ReconstructFromCache(
                    getGraphicsDeviceInternal(),
                    w, h,
                    cacheIt->second.fmt,
                    cacheIt->second.levelCount,
                    std::move(backendSp),
                    std::move(cpuPixelsSp));
            }
            // Backend expired — remove stale entry and fall through to reload.
            textureCache_.erase(cacheIt);
        }

        // Load fresh from disk.
        auto readerIt = typeReaders_.find(std::type_index(typeid(Graphics::Texture2D)));
        if (readerIt == typeReaders_.end())
            throw ContentLoadException(
                std::string("ContentManager::Load<Texture2D>(): No reader registered, asset '")
                + assetName + "'.");

        auto* readerPtr = std::any_cast<
            std::shared_ptr<LooseFileContentTypeReader<Graphics::Texture2D>>>(&readerIt->second);
        if (!readerPtr || !*readerPtr)
            throw ContentLoadException(
                std::string("ContentManager::Load<Texture2D>(): Reader is null, asset '")
                + assetName + "'.");

        LooseFileContentTypeReader<Graphics::Texture2D>& reader = **readerPtr;
        const std::string resolvedPath = ResolveAssetPath(assetName, reader);

        Graphics::Texture2D result = reader.Read(resolvedPath, *this);

        // Cache weak references so the GPU backend is freed as soon as the
        // caller drops all its Texture2D copies.
        WeakTextureEntry entry;
        entry.backend    = result.GetBackendWeak();
        entry.cpuPixels  = result.GetCpuPixelsWeak();
        entry.fmt        = result.getFormatProperty();
        entry.levelCount = result.getLevelCountProperty();
        textureCache_[key] = std::move(entry);

        return result;
    }

    // See the declaration in ContentManager.hpp for why this type is not cached: SoundEffect
    // is move-only (T-3G), so there is no way to hand back "the same loaded instance" to a
    // second caller anyway -- reader.Read() below already does a fresh decode per call, which
    // is exactly what a cache MISS did before this specialisation existed.
    template<>
    Audio::SoundEffect ContentManager::Load<Audio::SoundEffect>(const std::string& assetName)
    {
        if (disposed_)
            throw std::runtime_error("ContentManager has been disposed.");

        log::Debug(std::string("Loading asset: ") + assetName);

        auto readerIt = typeReaders_.find(std::type_index(typeid(Audio::SoundEffect)));
        if (readerIt == typeReaders_.end())
            throw ContentLoadException(
                std::string("ContentManager::Load<T>(): No reader registered for type, asset '")
                + assetName + "'.");

        auto* readerPtr = std::any_cast<
            std::shared_ptr<LooseFileContentTypeReader<Audio::SoundEffect>>>(&readerIt->second);
        if (!readerPtr || !*readerPtr)
            throw ContentLoadException(
                std::string("ContentManager::Load<T>(): Reader is null for asset '")
                + assetName + "'.");

        LooseFileContentTypeReader<Audio::SoundEffect>& reader = **readerPtr;
        const std::string resolvedPath = ResolveAssetPath(assetName, reader);

        return reader.Read(resolvedPath, *this);
    }
} // namespace Microsoft::Xna::Framework::Content
