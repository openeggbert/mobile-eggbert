// SPDX-License-Identifier: MS-PL
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Graphics/DisplayMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/DisplayModeCollection.hpp"
#include "System/Object.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    /** @brief Describes a graphics adapter/display available to the system. */
    class GraphicsAdapter final : public System::Object
    {
    public:
        using IntPtr = std::uintptr_t;

        /** @brief Returns the device or display name. */
        [[nodiscard]] const std::string& getDeviceNameProperty() const;

        /**
         * @brief Returns the default graphics adapter (adapter index 0).
         *
         * Re-evaluated on every call, matching FNA's `DefaultAdapter` property — the returned
         * reference must not be cached across a call to AdaptersChanged(), which destroys and
         * recreates every GraphicsAdapter instance.
         */
        [[nodiscard]] static GraphicsAdapter& getDefaultAdapterProperty();

        /** @brief Returns the list of all available graphics adapters. */
        [[nodiscard]] static const std::vector<std::unique_ptr<GraphicsAdapter>>& getAdaptersProperty();

        /** @brief Refreshes the cached list of available graphics adapters. */
        static void AdaptersChanged();

        /** @brief Returns the fully qualified .NET type name of this class. */
        NOXNA [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        GraphicsAdapter(SharpRuntime::intcs displayIndex, DisplayModeCollection modes, std::string name,
                        std::string description,
                        SharpRuntime::intcs vendorId = 0, SharpRuntime::intcs deviceId = 0);

        SharpRuntime::intcs displayIndex_;
        DisplayModeCollection supportedDisplayModes_;
        std::string description_;
        std::string deviceName_;
        bool useNullDevice_;
        bool useReferenceDevice_;
        SharpRuntime::intcs vendorId_;
        SharpRuntime::intcs deviceId_;

        static void queryPciIds(SharpRuntime::intcs& vendorId, SharpRuntime::intcs& deviceId);

        static std::vector<std::unique_ptr<GraphicsAdapter>> adapters_;

        [[nodiscard]] static std::vector<DisplayMode> queryDisplayModes(SharpRuntime::intcs displayIndex);
        [[nodiscard]] static DisplayMode queryCurrentDisplayMode(SharpRuntime::intcs displayIndex);
    };
}
