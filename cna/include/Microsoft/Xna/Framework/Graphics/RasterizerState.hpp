// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/FillMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsResource.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    /** @brief Defines rasterizer state for the graphics pipeline. */
    class RasterizerState : public GraphicsResource
    {
    public:
        /** @brief Preset: cull clockwise-wound faces. */
        static const RasterizerState CullClockwise;
        /** @brief Preset: cull counter-clockwise-wound faces (XNA default). */
        static const RasterizerState CullCounterClockwise;
        /** @brief Preset: no face culling. */
        static const RasterizerState CullNone;

        /** @brief Creates a RasterizerState with XNA-compatible default values. */
        RasterizerState();

        /** @brief Returns the fully-qualified .NET type name of this object. */
        NOXNA [[nodiscard]] const std::string& GetTypeName() const override;

        /**
         * @brief Gets the culling mode for rasterized faces.
         * @return Current cull mode.
         */
        [[nodiscard]] CullMode getCullModeProperty() const;

        /**
         * @brief Gets the depth bias added to each pixel's depth value.
         * @return Current depth bias.
         */
        [[nodiscard]] float getDepthBiasProperty() const;

        /**
         * @brief Gets the fill mode used when rendering primitives.
         * @return Current fill mode.
         */
        [[nodiscard]] FillMode getFillModeProperty() const;

        /**
         * @brief Gets whether scissor testing is enabled.
         * @return true if scissor testing is enabled.
         */
        [[nodiscard]] bool getScissorTestEnableProperty() const;

        /**
         * @brief Gets the slope-scale depth bias applied to polygon edges.
         * @return Current slope-scale depth bias.
         */
        [[nodiscard]] float getSlopeScaleDepthBiasProperty() const;

    private:
        RasterizerState(const std::string& name, CullMode cullMode);

        CullMode cullMode_;
        float depthBias_;
        FillMode fillMode_;
        bool scissorTestEnable_;
        float slopeScaleDepthBias_;
    };
}
