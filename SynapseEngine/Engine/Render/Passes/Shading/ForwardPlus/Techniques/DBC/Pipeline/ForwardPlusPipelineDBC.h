#pragma once
#include "Engine/Render/RenderPipeline.h"

namespace Syn {
    class SYN_API ForwardPlusPipelineDBC : public RenderPipeline {
    public:
        ForwardPlusPipelineDBC();
        bool ShouldExecute(const RenderContext& context) const override;
        bool IsProfileable() const override { return false; }
    };
}