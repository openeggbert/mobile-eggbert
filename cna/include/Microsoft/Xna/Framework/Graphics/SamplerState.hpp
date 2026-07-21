// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsResource.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureAddressMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    /** @brief Defines sampler state for texture sampling. */
    class SamplerState : public GraphicsResource
    {
    public:
        /** @brief Preset: linear filtering with clamp addressing on all axes. */
        static const SamplerState LinearClamp;
        /** @brief Preset: linear filtering with wrap addressing on all axes. */
        static const SamplerState LinearWrap;

        /** @brief Creates a SamplerState with XNA-compatible default values. */
        SamplerState();

        /** @brief Returns the fully-qualified .NET type name of this object. */
        NOXNA [[nodiscard]] const std::string& GetTypeName() const override;

        /**
         * @brief Gets the texture address mode for the U (horizontal) coordinate.
         * @return Current U address mode.
         */
        [[nodiscard]] TextureAddressMode getAddressUProperty() const;

        /**
         * @brief Gets the texture address mode for the V (vertical) coordinate.
         * @return Current V address mode.
         */
        [[nodiscard]] TextureAddressMode getAddressVProperty() const;

        /**
         * @brief Gets the texture filtering mode.
         * @return Current texture filter.
         */
        [[nodiscard]] TextureFilter getFilterProperty() const;

        /**
         * @brief Gets the maximum anisotropy level used for anisotropic filtering.
         * @return Current maximum anisotropy.
         */
        [[nodiscard]] int getMaxAnisotropyProperty() const;

    private:
        SamplerState(const std::string& name,
                     TextureFilter filter,
                     TextureAddressMode addressU,
                     TextureAddressMode addressV,
                     TextureAddressMode addressW);

        TextureAddressMode addressU_;
        TextureAddressMode addressV_;
        TextureAddressMode addressW_;
        TextureFilter filter_;
        int maxAnisotropy_;
    };
}
