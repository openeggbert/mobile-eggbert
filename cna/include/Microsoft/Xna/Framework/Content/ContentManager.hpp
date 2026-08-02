// SPDX-License-Identifier: MS-PL
#pragma once

#include "System/Any.hpp"
#include <experimental/filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "CNA/CNAHelper.hpp"
#include "CNA/Logger.hpp"
#include "SharpRuntime/Prop.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/Content/LooseFileContentTypeReader.hpp"
#include "System/IServiceProvider.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "System/IDisposable.hpp"

#if defined(__ANDROID__)
#include <SDL2/SDL.h>
#endif

namespace Microsoft::Xna::Framework::Audio  { class SoundEffect; }
namespace Microsoft::Xna::Framework::Graphics { class GraphicsDevice; class Texture2D; }
namespace CNA::Internal::Backends { class ITextureBackend; }

namespace Microsoft::Xna::Framework::Content
{
    using log = CNA::Logger;

    /**
     * @brief Content manager with extensible type reader support and asset caching.
     *
     * Supports loading assets by type. Custom types are registered via RegisterTypeReader<T>().
     * Built-in loaders (Texture2D, SoundEffect) are registered in the constructor.
     */
    class ContentManager : public System::IDisposable
    {
    private:
        std::string rootDirectory_ = "Content";
        Graphics::GraphicsDevice* graphicsDevice_ = nullptr;
        System::IServiceProvider* serviceProvider_ = nullptr;
        bool disposed_ = false;

        // Keyed by (T's type_index, normalized logical name), not name alone -- otherwise a
        // second Load<T2>() for a logical name a different T1 already cached under would
        // any_cast<T2>() a System::Any actually holding T1, throwing std::bad_cast (an
        // unrelated, undocumented exception type) instead of a clear ContentLoadException
        // naming both types.
        struct AssetCacheKey
        {
            std::type_index typeIndex;
            std::string normalizedName;

            bool operator==(const AssetCacheKey& other) const
            {
                return typeIndex == other.typeIndex && normalizedName == other.normalizedName;
            }
        };

        struct AssetCacheKeyHash
        {
            std::size_t operator()(const AssetCacheKey& k) const
            {
                return std::hash<std::type_index>()(k.typeIndex) ^
                       (std::hash<std::string>()(k.normalizedName) << 1);
            }
        };

        std::unordered_map<AssetCacheKey, System::Any, AssetCacheKeyHash> loadedAssets_;
        std::unordered_map<std::type_index, System::Any> typeReaders_;

        struct WeakTextureEntry {
            std::weak_ptr<CNA::Internal::Backends::ITextureBackend> backend;
            std::weak_ptr<std::vector<uint8_t>> cpuPixels;
            Graphics::SurfaceFormat fmt;
            int levelCount;
        };
        std::unordered_map<std::string, WeakTextureEntry> textureCache_;

        DEF_PROP(std::string, RootDirectory, getter1, setter1, member0, static0, constret1, ref1, constmet1)

        [[nodiscard]] std::string BuildAssetPath(const std::string& assetName) const;
        [[nodiscard]] std::string NormalizeKey(const std::string& assetName) const;

        void RegisterBuiltinLoaders();

    public:
        /**
         * @brief Constructs a ContentManager with the given service provider.
         *
         * @param serviceProvider Service provider used to resolve graphics and other services.
         */
        explicit ContentManager(System::IServiceProvider* serviceProvider);

        /**
         * @brief Constructs a ContentManager with the given service provider and root directory.
         *
         * @param serviceProvider Service provider used to resolve graphics and other services.
         * @param rootDirectory   Root path prepended to all asset names.
         */
        ContentManager(System::IServiceProvider* serviceProvider,
                       const std::string& rootDirectory);

        /** @brief Constructs a ContentManager with a default root directory of "Content". */
        NOXNA ContentManager();

        /** @brief Destroys the content manager and releases all loaded assets. */
        ~ContentManager() override = default;

        /** @brief Releases all resources used by this content manager. */
        void Dispose() override;

        /**
         * @brief Gets the service provider associated with this content manager.
         *
         * @return Pointer to the IServiceProvider, or nullptr if none was provided.
         */
        [[nodiscard]] System::IServiceProvider* getServiceProviderProperty() const;

        /**
         * @brief Sets the graphics device used when loading GPU resources such as textures.
         *
         * @param graphicsDevice The graphics device to associate with this manager.
         */
        void setGraphicsDevice(Graphics::GraphicsDevice& graphicsDevice);

        /**
         * @brief Returns the graphics device associated with this content manager.
         *
         * @return Reference to the associated GraphicsDevice.
         */
        [[nodiscard]] Graphics::GraphicsDevice& getGraphicsDeviceInternal() const;

        /** @brief Unloads all cached assets and frees the associated resources. */
        void Unload();

        /**
         * @brief Registers a custom type reader for assets of type T.
         *
         * @tparam T     Asset type this reader produces.
         * @param reader Unique pointer to the type reader to register.
         */
        template <typename T>
        void RegisterTypeReader(std::unique_ptr<LooseFileContentTypeReader<T>> reader)
        {
            typeReaders_[std::type_index(typeid(T))] =
                std::shared_ptr<LooseFileContentTypeReader<T>>(std::move(reader));
        }

        /**
         * @brief Loads an asset of type T from the content root.
         *
         * Results are cached — subsequent calls with the same asset name return the
         * already-loaded instance. If assetName has no file extension, each extension
         * returned by the registered reader's GetExtensions() is tried in order.
         *
         * @tparam T Asset type.
         * @param assetName Relative file path inside the content root (with or without extension).
         * @return Loaded asset instance.
         * @throws ContentLoadException if the asset cannot be loaded.
         */
        template <typename T>
        [[nodiscard]] T Load(const std::string& assetName)
        {
            if (disposed_)
            {
                throw std::runtime_error("ContentManager has been disposed.");
            }

            const std::string key = NormalizeKey(assetName);
            const AssetCacheKey cacheKey{std::type_index(typeid(T)), key};
            log::Debug(std::string("Loading asset: ") + assetName);

            auto cacheIt = loadedAssets_.find(cacheKey);
            if (cacheIt != loadedAssets_.end())
            {
                return System::any_cast<T>(cacheIt->second);
            }

            auto readerIt = typeReaders_.find(std::type_index(typeid(T)));
            if (readerIt == typeReaders_.end())
            {
                throw ContentLoadException(
                    std::string("ContentManager::Load<T>(): No reader registered for type, asset '")
                    + assetName + "'.");
            }

            auto* readerPtr = System::any_cast<std::shared_ptr<LooseFileContentTypeReader<T>>>(&readerIt->second);
            if (!readerPtr || !*readerPtr)
            {
                throw ContentLoadException(
                    std::string("ContentManager::Load<T>(): Reader is null for asset '")
                    + assetName + "'.");
            }

            LooseFileContentTypeReader<T>& reader = **readerPtr;
            const std::string resolvedPath = ResolveAssetPath(assetName, reader);

            T result = reader.Read(resolvedPath, *this);
            loadedAssets_[cacheKey] = result;
            return result;
        }

    private:
        /**
         * @brief Resolves the full filesystem path for an asset, trying reader extensions
         *        when the literal asset path does not exist.
         *
         * @tparam T       Asset type.
         * @param assetName Relative asset name.
         * @param reader    Type reader whose extensions are tried.
         * @return Full resolved filesystem path.
         */
        template <typename T>
        [[nodiscard]] std::string ResolveAssetPath(
            const std::string& assetName,
            LooseFileContentTypeReader<T>& reader) const
        {
            const std::string base = BuildAssetPath(assetName);

#if defined(__ANDROID__)
            // APK assets are not visible to std::experimental::filesystem.
            // SDL can open them through Android's AAssetManager, so use it
            // when resolving extensionless XNA content names such as
            // "backgrounds/wait" -> "backgrounds/wait.png".
            const auto androidAssetExists = [](const std::string& candidate)
            {
                SDL_RWops* stream = SDL_RWFromFile(candidate.c_str(), "rb");
                if (stream == nullptr)
                {
                    return false;
                }
                SDL_RWclose(stream);
                return true;
            };

            if (androidAssetExists(base))
            {
                return base;
            }

            const auto androidExtensions = reader.GetExtensions();
            for (const auto& ext : androidExtensions)
            {
                const std::string candidate = base + ext;
                if (androidAssetExists(candidate))
                {
                    return candidate;
                }
            }
#endif

            // If the literal path already exists, use it as-is. This covers
            // assetName with an explicit, correct extension. Checking
            // existence rather than std::experimental::filesystem::path::has_extension()
            // matters because asset names can legitimately contain a '.'
            // that is not a file extension (e.g. localized names like
            // "Flag.en-US"), which has_extension() would otherwise
            // misinterpret as already-resolved and never try appending
            // a reader extension.
            if (std::experimental::filesystem::exists(base))
            {
                return base;
            }

            // Try each extension declared by the reader.
            const auto extensions = reader.GetExtensions();
            for (const auto& ext : extensions)
            {
                const std::string candidate = base + ext;
                if (std::experimental::filesystem::exists(candidate))
                {
                    return candidate;
                }
            }

            // Fall back to bare path (reader may handle the extension itself).
            return base;
        }
    };

    // Explicit specialisation: Texture2D assets use a weak cache so that the
    // GPU backend is freed as soon as the last external Texture2D copy is dropped,
    // preventing per-world RAM growth when worlds load unique background textures.
    template<>
    Graphics::Texture2D ContentManager::Load<Graphics::Texture2D>(const std::string& assetName);

    // Explicit specialisation: SoundEffect is move-only with per-owner Dispose-cascade
    // semantics (T-3G) -- sharing one cached instance across unrelated Load<SoundEffect>()
    // call sites would let disposing one caller's copy silently cascade-stop another,
    // unrelated caller's still-playing instances. Each call gets its own independently-owned
    // SoundEffect instead; the generic loadedAssets_ any-cache (which requires T to be
    // CopyConstructible) is skipped entirely for this type.
    template<>
    Audio::SoundEffect ContentManager::Load<Audio::SoundEffect>(const std::string& assetName);
}
