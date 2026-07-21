// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"

#include <SDL2/SDL.h>

#include <stdexcept>
#include <utility>

#ifdef __linux__
#include <fstream>
#include <string>
#endif

namespace Microsoft::Xna::Framework::Graphics
{
    std::vector<std::unique_ptr<GraphicsAdapter>> GraphicsAdapter::adapters_;

    namespace
    {
        // SDL2 identifies displays by a plain zero-based index (SDL_GetDisplayName(int)),
        // unlike SDL3's opaque SDL_DisplayID/SDL_GetDisplays() array -- this file's own
        // `displayIndex` parameters already ARE that index, so no separate ID-lookup layer is
        // needed anymore (the pre-migration getDisplayIdByIndex() helper this replaces is gone).
        std::string getDisplayName(int displayIndex, SharpRuntime::intcs fallbackIndex)
        {
            const char* name = SDL_GetDisplayName(displayIndex);
            if (name != nullptr && *name != '\0')
            {
                return std::string(name);
            }

            return "Display " + std::to_string(fallbackIndex);
        }
    }

    GraphicsAdapter& GraphicsAdapter::getDefaultAdapterProperty()
    {
        const auto& adapters = getAdaptersProperty();
        if (adapters.empty())
        {
            throw std::runtime_error("No graphics adapters are available.");
        }

        return *adapters[0];
    }

    GraphicsAdapter::GraphicsAdapter(
        SharpRuntime::intcs displayIndex,
        DisplayModeCollection modes,
        std::string name,
        std::string description,
        SharpRuntime::intcs vendorId,
        SharpRuntime::intcs deviceId
    )
        : displayIndex_(displayIndex),
          supportedDisplayModes_(std::move(modes)),
          description_(std::move(description)),
          deviceName_(std::move(name)),
          useNullDevice_(false),
          useReferenceDevice_(false),
          vendorId_(vendorId),
          deviceId_(deviceId)
    {
    }

    void GraphicsAdapter::queryPciIds(SharpRuntime::intcs& vendorId, SharpRuntime::intcs& deviceId)
    {
        vendorId = 0;
        deviceId = 0;
#ifdef __linux__
        // Try each DRM card slot in order; first readable one wins.
        for (int card = 0; card < 4; ++card)
        {
            const std::string base = "/sys/class/drm/card" + std::to_string(card) + "/device/";
            std::ifstream vf(base + "vendor");
            std::ifstream df(base + "device");
            if (!vf.is_open() || !df.is_open())
                continue;
            std::string vs, ds;
            std::getline(vf, vs);
            std::getline(df, ds);
            if (vs.empty() || ds.empty())
                continue;
            try
            {
                vendorId = static_cast<SharpRuntime::intcs>(std::stoul(vs, nullptr, 16));
                deviceId = static_cast<SharpRuntime::intcs>(std::stoul(ds, nullptr, 16));
            }
            catch (...) {}
            break;
        }
#endif
    }

    const std::string& GraphicsAdapter::getDeviceNameProperty() const
    {
        return deviceName_;
    }

    const std::vector<std::unique_ptr<GraphicsAdapter>>& GraphicsAdapter::getAdaptersProperty()
    {
        if (adapters_.empty())
        {
            AdaptersChanged();
        }

        return adapters_;
    }

    void GraphicsAdapter::AdaptersChanged()
    {
        adapters_.clear();

        SharpRuntime::intcs vendorId = 0, deviceId = 0;
        queryPciIds(vendorId, deviceId);

        const int count = SDL_GetNumVideoDisplays();

        if (count <= 0)
        {
            adapters_.push_back(std::unique_ptr<GraphicsAdapter>(
                new GraphicsAdapter(
                    0,
                    DisplayModeCollection({DisplayMode(800, 480, SurfaceFormat::Color)}),
                    "\\\\.\\DISPLAY1",
                    "Default Display",
                    vendorId, deviceId
                )
            ));
            return;
        }

        for (int i = 0; i < count; ++i)
        {
            // Matches FNA's SDL3_FNAPlatform.GetGraphicsAdapters(): DeviceName is a synthetic
            // Windows-style path (not the real display name — real XNA convention, kept even on
            // non-Windows platforms), while Description is the actual display name.
            const std::string deviceName = "\\\\.\\DISPLAY" + std::to_string(i + 1);
            const std::string description = getDisplayName(i, i);
            // All displays share the same GPU — pass PCI IDs to every adapter.
            adapters_.push_back(std::unique_ptr<GraphicsAdapter>(
                new GraphicsAdapter(
                    i,
                    DisplayModeCollection(queryDisplayModes(i)),
                    deviceName,
                    description,
                    vendorId, deviceId
                )
            ));
        }
    }

    const std::string& GraphicsAdapter::GetTypeName() const
    {
        static const std::string typeName = "Microsoft.Xna.Framework.Graphics.GraphicsAdapter";
        return typeName;
    }

    std::vector<DisplayMode> GraphicsAdapter::queryDisplayModes(SharpRuntime::intcs displayIndex)
    {
        std::vector<DisplayMode> result;

        if (displayIndex < 0 || displayIndex >= SDL_GetNumVideoDisplays())
        {
            result.emplace_back(800, 480, SurfaceFormat::Color);
            return result;
        }

        const int count = SDL_GetNumDisplayModes(displayIndex);
        if (count > 0)
        {
            // Matches FNA's SDL3_FNAPlatform.GetGraphicsAdapters(): iterate in reverse and skip
            // width/height duplicates caused by multiple refresh rates at the same resolution.
            for (int i = count - 1; i >= 0; --i)
            {
                SDL_DisplayMode mode{};
                if (SDL_GetDisplayMode(displayIndex, i, &mode) != 0)
                {
                    continue;
                }

                bool dupe = false;
                for (const DisplayMode& existing : result)
                {
                    if (mode.w == existing.getWidthProperty() && mode.h == existing.getHeightProperty())
                    {
                        dupe = true;
                        break;
                    }
                }

                if (!dupe)
                {
                    result.emplace_back(mode.w, mode.h, SurfaceFormat::Color);
                }
            }
        }

        if (result.empty())
        {
            result.push_back(queryCurrentDisplayMode(displayIndex));
        }

        return result;
    }

    DisplayMode GraphicsAdapter::queryCurrentDisplayMode(SharpRuntime::intcs displayIndex)
    {
        if (displayIndex < 0 || displayIndex >= SDL_GetNumVideoDisplays())
        {
            return DisplayMode(800, 480, SurfaceFormat::Color);
        }

        SDL_DisplayMode mode{};
        if (SDL_GetCurrentDisplayMode(displayIndex, &mode) != 0)
        {
            return DisplayMode(800, 480, SurfaceFormat::Color);
        }

        return DisplayMode(mode.w, mode.h, SurfaceFormat::Color);
    }
}
