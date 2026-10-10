#pragma once
#include "Engine/Render/RenderPipeline.h"

namespace Syn {
    class SYN_API ForwardPlusPipelineDoom : public RenderPipeline {
    public:
        ForwardPlusPipelineDoom();
        bool ShouldExecute(const RenderContext& context) const override;
        bool IsProfileable() const override { return false; }
    };
}