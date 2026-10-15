#include "ForwardPlusPipelineVFC.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/VFC/Passes/ClusterSetupPassVFC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterDispatchSetupPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightSinglePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightSinglePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterLightWriteSyncPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightPrefixSumPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightPrefixSumPass.h"

namespace Syn {
    ForwardPlusPipelineVFC::ForwardPlusPipelineVFC()
        : RenderPipeline("ForwardPlusPipelineVFC", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterSetupPassVFC>());

        AddPass(std::make_unique<ClusterDispatchSetupPass>());

        AddPass(std::make_unique<ClusterPointLightSinglePass>());
        AddPass(std::make_unique<ClusterSpotLightSinglePass>());

        AddPass(std::make_unique<ClusterPointLightCountPass>());
        AddPass(std::make_unique<ClusterPointLightPrefixSumPass>());
        AddPass(std::make_unique<ClusterPointLightWritePass>());

        AddPass(std::make_unique<ClusterSpotLightCountPass>());
        AddPass(std::make_unique<ClusterSpotLightPrefixSumPass>());
        AddPass(std::make_unique<ClusterSpotLightWritePass>());

        AddPass(std::make_unique<ClusterLightWriteSyncPass>());
    }

    bool ForwardPlusPipelineVFC::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::VFC;
    }
}