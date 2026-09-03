#pragma once
#include "Engine/Render/RenderPipeline.h"

namespace Syn {
    class SYN_API DirectionLightCpuShadowPipeline : public RenderPipeline 
    {
    public:
        DirectionLightCpuShadowPipeline();
        bool ShouldExecute(const RenderContext& context) const override;
    };
}