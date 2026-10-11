#include "ForwardPlusPipelineHarada.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/HARADA/Passes/ClusterUberCullingPassHarada.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/HARADA/Passes/ClusterLightWriteSyncPassHarada.h"

namespace Syn {
    ForwardPlusPipelineHarada::ForwardPlusPipelineHarada()
        : RenderPipeline("ForwardPlusPipelineHarada", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterUberCullingPassHarada>());
        AddPass(std::make_unique<ClusterLightWriteSyncPassHarada>());
    }

    bool ForwardPlusPipelineHarada::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::HARADA;
    }
}