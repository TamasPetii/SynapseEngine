#include "ForwardPlusPipelineDBBC.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DBBC/Passes/ClusterUberCullingPassDBBC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DBBC/Passes/ClusterLightWriteSyncPassDBBC.h"

namespace Syn {
    ForwardPlusPipelineDBBC::ForwardPlusPipelineDBBC()
        : RenderPipeline("ForwardPlusPipelineDBBC", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterUberCullingPassDBBC>());
        AddPass(std::make_unique<ClusterLightWriteSyncPassDBBC>());
    }

    bool ForwardPlusPipelineDBBC::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::DBBC;
    }
}