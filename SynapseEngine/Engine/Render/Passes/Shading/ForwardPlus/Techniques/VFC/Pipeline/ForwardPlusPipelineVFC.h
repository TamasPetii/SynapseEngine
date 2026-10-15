#pragma once
#include "Engine/Render/RenderPipeline.h"

namespace Syn {
    class SYN_API ForwardPlusPipelineVFC : public RenderPipeline {
    public:
        ForwardPlusPipelineVFC();
        bool ShouldExecute(const RenderContext& context) const override;
        bool IsProfileable() const override { return false; }
    };
}