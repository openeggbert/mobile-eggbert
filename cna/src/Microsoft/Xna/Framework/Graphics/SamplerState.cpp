// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    const SamplerState SamplerState::LinearClamp     {"SamplerState.LinearClamp",      TextureFilter::Linear,      TextureAddressMode::Clamp,  TextureAddressMode::Clamp,  TextureAddressMode::Clamp};
    const SamplerState SamplerState::LinearWrap      {"SamplerState.LinearWrap",       TextureFilter::Linear,      TextureAddressMode::Wrap,   TextureAddressMode::Wrap,   TextureAddressMode::Wrap};

    SamplerState::SamplerState()
        : addressU_(TextureAddressMode::Wrap)
        , addressV_(TextureAddressMode::Wrap)
        , addressW_(TextureAddressMode::Wrap)
        , filter_(TextureFilter::Linear)
        , maxAnisotropy_(4)
    {
    }

    SamplerState::SamplerState(const std::string& name,
                               TextureFilter filter,
                               TextureAddressMode addressU,
                               TextureAddressMode addressV,
                               TextureAddressMode addressW)
        : SamplerState()
    {
        setNameProperty(name);
        filter_   = filter;
        addressU_ = addressU;
        addressV_ = addressV;
        addressW_ = addressW;
    }

    TextureAddressMode SamplerState::getAddressUProperty() const { return addressU_; }

    TextureAddressMode SamplerState::getAddressVProperty() const { return addressV_; }

    TextureFilter SamplerState::getFilterProperty() const { return filter_; }

    int SamplerState::getMaxAnisotropyProperty() const { return maxAnisotropy_; }

    GetTypeNameCPP(SamplerState, "Microsoft.Xna.Framework.Graphics.SamplerState")
}
