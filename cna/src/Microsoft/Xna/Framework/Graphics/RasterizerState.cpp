// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    const RasterizerState RasterizerState::CullClockwise{"RasterizerState.CullClockwise", CullMode::CullClockwiseFace};
    const RasterizerState RasterizerState::CullCounterClockwise{"RasterizerState.CullCounterClockwise", CullMode::CullCounterClockwiseFace};
    const RasterizerState RasterizerState::CullNone{"RasterizerState.CullNone", CullMode::None};

    RasterizerState::RasterizerState()
        : cullMode_(CullMode::CullCounterClockwiseFace)
        , depthBias_(0.0f)
        , fillMode_(FillMode::Solid)
        , scissorTestEnable_(false)
        , slopeScaleDepthBias_(0.0f)
    {
    }

    RasterizerState::RasterizerState(const std::string& name, CullMode cullMode)
        : RasterizerState()
    {
        setNameProperty(name);
        cullMode_ = cullMode;
    }

    CullMode RasterizerState::getCullModeProperty() const { return cullMode_; }

    float RasterizerState::getDepthBiasProperty() const { return depthBias_; }

    FillMode RasterizerState::getFillModeProperty() const { return fillMode_; }

    bool RasterizerState::getScissorTestEnableProperty() const { return scissorTestEnable_; }

    float RasterizerState::getSlopeScaleDepthBiasProperty() const { return slopeScaleDepthBias_; }

    GetTypeNameCPP(RasterizerState, "Microsoft.Xna.Framework.Graphics.RasterizerState")
}
