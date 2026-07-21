// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/Blend.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsResource.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    /** @brief Represents a sprite and 3D blending configuration. */
    class BlendState : public GraphicsResource
    {
    public:
        /** @brief Preset: additive blending (source added on top of destination). */
        static const BlendState Additive;
        /** @brief Preset: alpha blending for premultiplied-alpha textures. */
        static const BlendState AlphaBlend;
        /** @brief Preset: alpha blending for non-premultiplied-alpha textures. */
        static const BlendState NonPremultiplied;
        /** @brief Preset: opaque blending (source overwrites destination). */
        static const BlendState Opaque;

        /** @brief Creates a BlendState with XNA-compatible default values. */
        BlendState();

        /** @brief Returns the fully-qualified .NET type name of this object. */
        NOXNA [[nodiscard]] const std::string& GetTypeName() const override;

        /**
         * @brief Gets the blend function for the alpha channel.
         * @return Current alpha blend function.
         */
        [[nodiscard]] BlendFunction getAlphaBlendFunctionProperty() const;

        /**
         * @brief Gets the destination blend factor for the alpha channel.
         * @return Current destination alpha blend factor.
         */
        [[nodiscard]] Blend getAlphaDestinationBlendProperty() const;

        /**
         * @brief Gets the source blend factor for the alpha channel.
         * @return Current source alpha blend factor.
         */
        [[nodiscard]] Blend getAlphaSourceBlendProperty() const;

        /**
         * @brief Gets the blend function for the color channels.
         * @return Current color blend function.
         */
        [[nodiscard]] BlendFunction getColorBlendFunctionProperty() const;

        /**
         * @brief Gets the destination blend factor for the color channels.
         * @return Current destination color blend factor.
         */
        [[nodiscard]] Blend getColorDestinationBlendProperty() const;

        /**
         * @brief Gets the source blend factor for the color channels.
         * @return Current source color blend factor.
         */
        [[nodiscard]] Blend getColorSourceBlendProperty() const;

        /**
         * @brief Gets the four-component blend factor used when BlendFactor or InverseBlendFactor is specified.
         * @return Current blend factor color.
         */
        [[nodiscard]] Color getBlendFactorProperty() const;

    private:
        BlendState(const std::string& name, Blend colorSrc, Blend alphaSrc, Blend colorDst, Blend alphaDst);

        BlendFunction alphaBlendFunction_;
        Blend alphaDestinationBlend_;
        Blend alphaSourceBlend_;
        BlendFunction colorBlendFunction_;
        Blend colorDestinationBlend_;
        Blend colorSourceBlend_;
        Color blendFactor_;
    };
}
