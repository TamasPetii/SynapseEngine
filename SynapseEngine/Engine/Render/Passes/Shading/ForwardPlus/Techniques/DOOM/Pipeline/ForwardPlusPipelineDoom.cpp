#include "ForwardPlusPipelineDoom.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DOOM/Passes/ClusterSetupPassDoom.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DOOM/Passes/ClusterDispatchSetupPassDoom.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DOOM/Passes/ClusterPointLightSinglePassDoom.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DOOM/Passes/ClusterSpotLightSinglePassDoom.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DOOM/Passes/ClusterLightWriteSyncPassDoom.h"

namespace Syn {
    ForwardPlusPipelineDoom::ForwardPlusPipelineDoom()
        : RenderPipeline("ForwardPlusPipelineDoom", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterSetupPassDoom>());
        AddPass(std::make_unique<ClusterDispatchSetupPassDoom>());
        AddPass(std::make_unique<ClusterPointLightSinglePassDoom>());
        AddPass(std::make_unique<ClusterSpotLightSinglePassDoom>());
        AddPass(std::make_unique<ClusterLightWriteSyncPassDoom>());
    }

    bool ForwardPlusPipelineDoom::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::DOOM;
    }
}