#pragma once
#include "Engine/Render/RenderPipeline.h"

namespace Syn {
    class SYN_API ForwardPlusPipelineDBAC : public RenderPipeline {
    public:
        ForwardPlusPipelineDBAC();
        bool ShouldExecute(const RenderContext& context) const override;
        bool IsProfileable() const override { return false; }
    };
}