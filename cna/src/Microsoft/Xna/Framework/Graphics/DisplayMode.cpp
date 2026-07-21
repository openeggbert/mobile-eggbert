// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/DisplayMode.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    DisplayMode::DisplayMode()
        : width_(0),
          height_(0),
          format_(SurfaceFormat::Color)
    {
    }

    DisplayMode::DisplayMode(SharpRuntime::intcs width, SharpRuntime::intcs height, SurfaceFormat format)
        : width_(width),
          height_(height),
          format_(format)
    {
    }

    SharpRuntime::intcs DisplayMode::getWidthProperty() const
    {
        return width_;
    }

    SharpRuntime::intcs DisplayMode::getHeightProperty() const
    {
        return height_;
    }

    const std::string& DisplayMode::GetTypeName() const
    {
        static const std::string typeName = "Microsoft.Xna.Framework.Graphics.DisplayMode";
        return typeName;
    }
}
