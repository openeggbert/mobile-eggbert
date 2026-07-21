// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Graphics/CompareFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsResource.hpp"
#include "Microsoft/Xna/Framework/Graphics/StencilOperation.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    /** @brief Defines depth-stencil state for the graphics pipeline. */
    class DepthStencilState : public GraphicsResource
    {
    public:
        /** @brief Preset: depth test and write both enabled with LessEqual comparison (XNA default). */
        static const DepthStencilState Default;
        /** @brief Preset: depth test enabled but writes disabled (read-only depth). */
        static const DepthStencilState DepthRead;
        /** @brief Preset: depth test and write both disabled. */
        static const DepthStencilState None;

        /** @brief Creates a DepthStencilState with XNA-compatible default values. */
        DepthStencilState();

        /** @brief Returns the fully-qualified .NET type name of this object. */
        NOXNA [[nodiscard]] const std::string& GetTypeName() const override;

        /**
         * @brief Gets whether depth buffering is enabled.
         * @return true if depth buffering is enabled.
         */
        [[nodiscard]] bool getDepthBufferEnableProperty() const;
        /**
         * @brief Gets whether writing to the depth buffer is enabled.
         * @return true if depth writes are enabled.
         */
        [[nodiscard]] bool getDepthBufferWriteEnableProperty() const;
        /**
         * @brief Gets the comparison function used for depth testing.
         * @return Current depth buffer comparison function.
         */
        [[nodiscard]] CompareFunction getDepthBufferFunctionProperty() const;
        /**
         * @brief Gets whether stencil testing is enabled.
         * @return true if stencil testing is enabled.
         */
        [[nodiscard]] bool getStencilEnableProperty() const;
        /**
         * @brief Gets the comparison function used for stencil testing.
         * @return Current stencil comparison function.
         */
        [[nodiscard]] CompareFunction getStencilFunctionProperty() const;
        /**
         * @brief Gets the mask applied when reading the stencil buffer.
         * @return Current stencil read mask.
         */
        [[nodiscard]] int getStencilMaskProperty() const;
        /**
         * @brief Gets the mask applied when writing to the stencil buffer.
         * @return Current stencil write mask.
         */
        [[nodiscard]] int getStencilWriteMaskProperty() const;
        /**
         * @brief Gets the reference value used for stencil comparisons.
         * @return Current stencil reference value.
         */
        [[nodiscard]] int getReferenceStencilProperty() const;
        /**
         * @brief Gets the stencil operation when the stencil test fails.
         * @return Current stencil fail operation.
         */
        [[nodiscard]] StencilOperation getStencilFailProperty() const;
        /**
         * @brief Gets the stencil operation when the stencil test passes but depth test fails.
         * @return Current stencil depth-buffer fail operation.
         */
        [[nodiscard]] StencilOperation getStencilDepthBufferFailProperty() const;
        /**
         * @brief Gets the stencil operation when both the stencil and depth tests pass.
         * @return Current stencil pass operation.
         */
        [[nodiscard]] StencilOperation getStencilPassProperty() const;
        /**
         * @brief Gets whether two-sided stencil mode is enabled.
         * @return true if two-sided stencil mode is enabled.
         */
        [[nodiscard]] bool getTwoSidedStencilModeProperty() const;
        /**
         * @brief Gets the stencil comparison function for counter-clockwise faces.
         * @return Current counter-clockwise stencil comparison function.
         */
        [[nodiscard]] CompareFunction getCounterClockwiseStencilFunctionProperty() const;
        /**
         * @brief Gets the stencil fail operation for counter-clockwise faces.
         * @return Current counter-clockwise stencil fail operation.
         */
        [[nodiscard]] StencilOperation getCounterClockwiseStencilFailProperty() const;
        /**
         * @brief Gets the stencil depth-buffer fail operation for counter-clockwise faces.
         * @return Current counter-clockwise stencil depth-buffer fail operation.
         */
        [[nodiscard]] StencilOperation getCounterClockwiseStencilDepthBufferFailProperty() const;
        /**
         * @brief Gets the stencil pass operation for counter-clockwise faces.
         * @return Current counter-clockwise stencil pass operation.
         */
        [[nodiscard]] StencilOperation getCounterClockwiseStencilPassProperty() const;

    private:
        DepthStencilState(const std::string& name, bool depthEnable, bool depthWriteEnable);

        bool depthBufferEnable_;
        bool depthBufferWriteEnable_;
        CompareFunction depthBufferFunction_;
        bool stencilEnable_;
        CompareFunction stencilFunction_;
        int stencilMask_;
        int stencilWriteMask_;
        int referenceStencil_;
        StencilOperation stencilFail_;
        StencilOperation stencilDepthBufferFail_;
        StencilOperation stencilPass_;
        bool twoSidedStencilMode_;
        CompareFunction counterClockwiseStencilFunction_;
        StencilOperation counterClockwiseStencilFail_;
        StencilOperation counterClockwiseStencilDepthBufferFail_;
        StencilOperation counterClockwiseStencilPass_;
    };
}
