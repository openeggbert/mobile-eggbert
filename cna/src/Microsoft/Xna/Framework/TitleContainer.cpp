// SPDX-License-Identifier: MS-PL

#include "Microsoft/Xna/Framework/TitleContainer.hpp"

#include <cstdlib>
#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "CNA/Logger.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/IO/FileStream.hpp"
#include "System/IO/MemoryStream.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace
{
    /**
     * @brief Purely lexical path normalization: collapses "." components and resolves ".."
     * against a preceding real component, without touching the filesystem.
     *
     * C++14-compatible replacement for std::filesystem::path::lexically_normal(), which is a
     * final-C++17 addition not present in the pre-standard Filesystem TS
     * (std::experimental::filesystem) this build uses instead.
     */
    std::experimental::filesystem::path LexicallyNormal(const std::experimental::filesystem::path& p)
    {
        namespace fs = std::experimental::filesystem;

        const fs::path rootName = p.root_name();
        const fs::path rootDir  = p.root_directory();

        std::vector<fs::path> stack;
        for (const fs::path& part : p.relative_path())
        {
            if (part == ".")
            {
                continue;
            }
            if (part == "..")
            {
                if (!stack.empty() && stack.back() != "..")
                {
                    stack.pop_back();
                }
                else if (rootDir.empty())
                {
                    // Relative path with no root: an unresolvable leading ".." is preserved.
                    stack.push_back(part);
                }
                // Absolute path: ".." above the root has nowhere to go, so it is dropped.
                continue;
            }
            stack.push_back(part);
        }

        fs::path result = rootName;
        result += rootDir;
        for (const fs::path& component : stack)
        {
            result /= component;
        }
        return result.empty() ? fs::path(".") : result;
    }
}

namespace Microsoft::Xna::Framework
{
    std::unique_ptr<System::IO::Stream> TitleContainer::OpenStream(const std::string& name)
    {
        const std::string safeName = NormalizeFilePathSeparators(name);
        const std::string realName = ResolveRealPath(safeName);

        CNA::Logger::Info("[TitleContainer] OpenStream requested: " + name);
        CNA::Logger::Info("[TitleContainer] Resolved path: " + realName);

        if (std::experimental::filesystem::exists(realName))
        {
            return std::make_unique<System::IO::FileStream>(realName);
        }

#if defined(__ANDROID__)
        // Android assets are often readable through SDL even when they are not visible
        // as normal files. Try the relative name first, then the resolved name.
        const std::string androidAssetName = IsPathRooted(safeName) ? realName : safeName;

        std::size_t dataSize = 0;
        void* rawData = SDL_LoadFile(androidAssetName.c_str(), &dataSize);
        if (rawData == nullptr && androidAssetName != realName)
        {
            rawData = SDL_LoadFile(realName.c_str(), &dataSize);
        }

        if (rawData != nullptr)
        {
            auto* bytes = static_cast<SharpRuntime::bytecs*>(rawData);
            auto stream = std::make_unique<System::IO::MemoryStream>(
                bytes,
                static_cast<SharpRuntime::intcs>(dataSize)
            );
            SDL_free(rawData);
            return stream;
        }
#endif

        throw std::runtime_error("[TitleContainer] Failed to open stream: " + realName);
    }

    void* TitleContainer::ReadToPointer(const std::string& name, IntPtr& size)
    {
        const std::string safeName = NormalizeFilePathSeparators(name);
        const std::string realName = ResolveRealPath(safeName);

        size = 0;

        std::ifstream file(realName, std::ios::binary | std::ios::ate);
        if (file)
        {
            const std::streamsize length = file.tellg();
            if (length < 0)
            {
                throw std::runtime_error("[TitleContainer] Failed to determine file size: " + realName);
            }

            file.seekg(0, std::ios::beg);

            void* buffer = std::malloc(static_cast<std::size_t>(length));
            if (buffer == nullptr && length > 0)
            {
                throw std::bad_alloc();
            }

            if (length > 0 && !file.read(static_cast<char*>(buffer), length))
            {
                std::free(buffer);
                throw std::runtime_error("[TitleContainer] Failed to read file: " + realName);
            }

            size = static_cast<IntPtr>(length);
            return buffer;
        }

#if defined(__ANDROID__)
        const std::string androidAssetName = IsPathRooted(safeName) ? realName : safeName;

        std::size_t dataSize = 0;
        void* rawData = SDL_LoadFile(androidAssetName.c_str(), &dataSize);
        if (rawData == nullptr && androidAssetName != realName)
        {
            rawData = SDL_LoadFile(realName.c_str(), &dataSize);
        }

        if (rawData != nullptr)
        {
            void* buffer = std::malloc(dataSize);
            if (buffer == nullptr && dataSize > 0)
            {
                SDL_free(rawData);
                throw std::bad_alloc();
            }

            if (dataSize > 0)
            {
                std::memcpy(buffer, rawData, dataSize);
            }

            SDL_free(rawData);
            size = static_cast<IntPtr>(dataSize);
            return buffer;
        }
#endif

        throw std::runtime_error("[TitleContainer] File not found: " + realName);
    }

    void TitleContainer::FreePointer(void* pointer)
    {
        std::free(pointer);
    }

    std::string TitleContainer::NormalizeFilePathSeparators(const std::string& name)
    {
        std::string result = name;
        for (char& ch : result)
        {
            if (ch == '\\')
            {
                ch = '/';
            }
        }
        return result;
    }

    bool TitleContainer::IsPathRooted(const std::string& name)
    {
        if (name.empty())
        {
            return false;
        }

        if (name[0] == '/')
        {
            return true;
        }

#if defined(_WIN32)
        if (name.size() >= 3 &&
            ((name[0] >= 'A' && name[0] <= 'Z') || (name[0] >= 'a' && name[0] <= 'z')) &&
            name[1] == ':' &&
            (name[2] == '/' || name[2] == '\\'))
        {
            return true;
        }
#endif

        return false;
    }

    std::string TitleContainer::CombineTitlePath(const std::string& name)
    {
        const std::experimental::filesystem::path base(TitleLocation::getPathProperty());
        return LexicallyNormal(base / std::experimental::filesystem::path(name)).string();
    }

    std::string TitleContainer::ResolveRealPath(const std::string& name)
    {
        if (IsPathRooted(name))
        {
            return LexicallyNormal(std::experimental::filesystem::path(name)).string();
        }

        return CombineTitlePath(name);
    }
}
