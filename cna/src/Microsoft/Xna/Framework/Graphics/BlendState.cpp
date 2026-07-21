// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    const BlendState BlendState::Additive        {"BlendState.Additive",        Blend::SourceAlpha, Blend::SourceAlpha, Blend::One,                Blend::One};
    const BlendState BlendState::AlphaBlend      {"BlendState.AlphaBlend",      Blend::One,         Blend::One,         Blend::InverseSourceAlpha, Blend::InverseSourceAlpha};
    const BlendState BlendState::NonPremultiplied{"BlendState.NonPremultiplied",Blend::SourceAlpha, Blend::SourceAlpha, Blend::InverseSourceAlpha, Blend::InverseSourceAlpha};
    const BlendState BlendState::Opaque          {"BlendState.Opaque",          Blend::One,         Blend::One,         Blend::Zero,               Blend::Zero};

    BlendState::BlendState()
        : alphaBlendFunction_(BlendFunction::Add)
        , alphaDestinationBlend_(Blend::Zero)
        , alphaSourceBlend_(Blend::One)
        , colorBlendFunction_(BlendFunction::Add)
        , colorDestinationBlend_(Blend::Zero)
        , colorSourceBlend_(Blend::One)
        , blendFactor_(Color::White)
    {
    }

    BlendState::BlendState(const std::string& name, Blend colorSrc, Blend alphaSrc, Blend colorDst, Blend alphaDst)
        : BlendState()
    {
        setNameProperty(name);
        colorSourceBlend_      = colorSrc;
        alphaSourceBlend_      = alphaSrc;
        colorDestinationBlend_ = colorDst;
        alphaDestinationBlend_ = alphaDst;
    }

    BlendFunction BlendState::getAlphaBlendFunctionProperty() const { return alphaBlendFunction_; }

    Blend BlendState::getAlphaDestinationBlendProperty() const { return alphaDestinationBlend_; }

    Blend BlendState::getAlphaSourceBlendProperty() const { return alphaSourceBlend_; }

    BlendFunction BlendState::getColorBlendFunctionProperty() const { return colorBlendFunction_; }

    Blend BlendState::getColorDestinationBlendProperty() const { return colorDestinationBlend_; }

    Blend BlendState::getColorSourceBlendProperty() const { return colorSourceBlend_; }

    Color BlendState::getBlendFactorProperty() const { return blendFactor_; }

    GetTypeNameCPP(BlendState, "Microsoft.Xna.Framework.Graphics.BlendState")
}
