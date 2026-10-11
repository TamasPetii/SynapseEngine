#include "ForwardPlusPipelineWicked.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/WICKED/Passes/ClusterUberCullingPassWicked.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/WICKED/Passes/ClusterLightWriteSyncPassWicked.h"

namespace Syn {
    ForwardPlusPipelineWicked::ForwardPlusPipelineWicked()
        : RenderPipeline("ForwardPlusPipelineWicked", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterUberCullingPassWicked>());
        AddPass(std::make_unique<ClusterLightWriteSyncPassWicked>());
    }

    bool ForwardPlusPipelineWicked::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::WICKED;
    }
}