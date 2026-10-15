#pragma once
#include "Engine/Render/RenderPipeline.h"

namespace Syn {
    class SYN_API ForwardPlusPipelineDBBC : public RenderPipeline {
    public:
        ForwardPlusPipelineDBBC();
        bool ShouldExecute(const RenderContext& context) const override;
        bool IsProfileable() const override { return false; }
    };
}