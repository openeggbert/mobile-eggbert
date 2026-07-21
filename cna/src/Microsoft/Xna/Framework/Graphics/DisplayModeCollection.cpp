// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/DisplayModeCollection.hpp"

#include <utility>

namespace Microsoft::Xna::Framework::Graphics
{
    DisplayModeCollection::DisplayModeCollection()
        : modes_()
    {
    }

    DisplayModeCollection::DisplayModeCollection(std::vector<DisplayMode> modes)
        : modes_(std::move(modes))
    {
    }

    const std::string& DisplayModeCollection::GetTypeName() const
    {
        static const std::string typeName = "Microsoft.Xna.Framework.Graphics.DisplayModeCollection";
        return typeName;
    }
}
