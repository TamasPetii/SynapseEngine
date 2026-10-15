#include "ForwardPlusPipelineDBC.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DBC/Passes/ClusterUberCullingPassDBC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DBC/Passes/ClusterLightWriteSyncPassDBC.h"

namespace Syn {
    ForwardPlusPipelineDBC::ForwardPlusPipelineDBC()
        : RenderPipeline("ForwardPlusPipelineDBC", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterUberCullingPassDBC>());
        AddPass(std::make_unique<ClusterLightWriteSyncPassDBC>());
    }

    bool ForwardPlusPipelineDBC::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::DBC;
    }
}