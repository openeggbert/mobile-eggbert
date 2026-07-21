// SPDX-License-Identifier: MS-PL
#pragma once

#include <string>
#include <vector>

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Graphics/DisplayMode.hpp"
#include "System/Object.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    /** @brief Read-only collection of DisplayMode objects representing supported display resolutions. */
    class DisplayModeCollection : public System::Object
    {
    public:
        /** @brief Constructs an empty DisplayModeCollection. */
        DisplayModeCollection();
        /**
         * @brief Constructs a DisplayModeCollection from a vector of display modes.
         * @param modes The display modes to store in this collection.
         */
        explicit DisplayModeCollection(std::vector<DisplayMode> modes);

        /** @brief Returns the fully qualified .NET type name of this class. */
        NOXNA [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        std::vector<DisplayMode> modes_;
    };
}
