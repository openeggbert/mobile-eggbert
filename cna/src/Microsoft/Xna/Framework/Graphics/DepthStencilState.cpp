// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    const DepthStencilState DepthStencilState::Default{"DepthStencilState.Default", true, true};
    const DepthStencilState DepthStencilState::DepthRead{"DepthStencilState.DepthRead", true, false};
    const DepthStencilState DepthStencilState::None{"DepthStencilState.None", false, false};

    DepthStencilState::DepthStencilState()
        : depthBufferEnable_(true)
        , depthBufferWriteEnable_(true)
        , depthBufferFunction_(CompareFunction::LessEqual)
        , stencilEnable_(false)
        , stencilFunction_(CompareFunction::Always)
        , stencilMask_(0x7FFFFFFF)
        , stencilWriteMask_(0x7FFFFFFF)
        , referenceStencil_(0)
        , stencilFail_(StencilOperation::Keep)
        , stencilDepthBufferFail_(StencilOperation::Keep)
        , stencilPass_(StencilOperation::Keep)
        , twoSidedStencilMode_(false)
        , counterClockwiseStencilFunction_(CompareFunction::Always)
        , counterClockwiseStencilFail_(StencilOperation::Keep)
        , counterClockwiseStencilDepthBufferFail_(StencilOperation::Keep)
        , counterClockwiseStencilPass_(StencilOperation::Keep)
    {
    }

    DepthStencilState::DepthStencilState(const std::string& name, bool depthEnable, bool depthWriteEnable)
        : DepthStencilState()
    {
        setNameProperty(name);
        depthBufferEnable_      = depthEnable;
        depthBufferWriteEnable_ = depthWriteEnable;
    }

    bool DepthStencilState::getDepthBufferEnableProperty() const { return depthBufferEnable_; }

    bool DepthStencilState::getDepthBufferWriteEnableProperty() const { return depthBufferWriteEnable_; }

    CompareFunction DepthStencilState::getDepthBufferFunctionProperty() const { return depthBufferFunction_; }

    bool DepthStencilState::getStencilEnableProperty() const { return stencilEnable_; }

    CompareFunction DepthStencilState::getStencilFunctionProperty() const { return stencilFunction_; }

    int DepthStencilState::getStencilMaskProperty() const { return stencilMask_; }

    int DepthStencilState::getStencilWriteMaskProperty() const { return stencilWriteMask_; }

    int DepthStencilState::getReferenceStencilProperty() const { return referenceStencil_; }

    StencilOperation DepthStencilState::getStencilFailProperty() const { return stencilFail_; }

    StencilOperation DepthStencilState::getStencilDepthBufferFailProperty() const { return stencilDepthBufferFail_; }

    StencilOperation DepthStencilState::getStencilPassProperty() const { return stencilPass_; }

    bool DepthStencilState::getTwoSidedStencilModeProperty() const { return twoSidedStencilMode_; }

    CompareFunction DepthStencilState::getCounterClockwiseStencilFunctionProperty() const { return counterClockwiseStencilFunction_; }

    StencilOperation DepthStencilState::getCounterClockwiseStencilFailProperty() const { return counterClockwiseStencilFail_; }

    StencilOperation DepthStencilState::getCounterClockwiseStencilDepthBufferFailProperty() const { return counterClockwiseStencilDepthBufferFail_; }

    StencilOperation DepthStencilState::getCounterClockwiseStencilPassProperty() const { return counterClockwiseStencilPass_; }

    GetTypeNameCPP(DepthStencilState, "Microsoft.Xna.Framework.Graphics.DepthStencilState")
}
