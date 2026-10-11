#include "ForwardPlusPipelineHaradaPlus.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Harada+/Passes/TileSetupPassHaradaPlus.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Harada+/Passes/ClusterSetupPassHaradaPlus.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Harada+/Passes/ClusterDispatchSetupPassHaradaPlus.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Harada+/Passes/ClusterPointLightSinglePassHaradaPlus.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Harada+/Passes/ClusterSpotLightSinglePassHaradaPlus.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Harada+/Passes/ClusterLightWriteSyncPassHaradaPlus.h"

namespace Syn 
{
    ForwardPlusPipelineHaradaPlus::ForwardPlusPipelineHaradaPlus()
        : RenderPipeline("ForwardPlusPipelineHaradaPlus", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<TileSetupPassHaradaPlus>());
        AddPass(std::make_unique<ClusterSetupPassHaradaPlus>());
        AddPass(std::make_unique<ClusterDispatchSetupPassHaradaPlus>());
        AddPass(std::make_unique<ClusterPointLightSinglePassHaradaPlus>());
        AddPass(std::make_unique<ClusterSpotLightSinglePassHaradaPlus>());
        AddPass(std::make_unique<ClusterLightWriteSyncPassHaradaPlus>());
    }

    bool ForwardPlusPipelineHaradaPlus::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::HARADA_PLUS;
    }
}