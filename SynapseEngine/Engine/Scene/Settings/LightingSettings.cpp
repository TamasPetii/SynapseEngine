#include "LightingSettings.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn
{
    LightingSettings::LightingSettings()
        : pipelineType(PipelineType::ForwardPlus)
        , tileSize(ComputeGroupSize::Image64D)
        , msaaSamples(4)
        , ambientStrength(0.05f)
        , emissiveStrength(1.00f)
        , enableDeferredEmissiveAo(true)
        , enableDeferredPointLights(true)
        , enableDeferredSpotLights(true)
        , enableDeferredDirectionalLights(true)
        , enableForwardPlusEmissiveAo(true)
        , enableForwardPlusPointLights(true)
        , enableForwardPlusSpotLights(true)
        , enableForwardPlusDirectionalLights(true)
    {}
}