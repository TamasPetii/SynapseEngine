#include "PostProcessSettings.h"

namespace Syn
{
    PostProcessSettings::PostProcessSettings()
        : enableBloom(true)
        , bloomThreshold(1.0f)
        , bloomKnee(0.1f)
        , bloomFilterRadius(0.005f)
        , bloomExposure(1.0f)
        , bloomStrength(1.0f)
        , enableSsao(false)
        , enableSsaoLight(false)
        , aoRadius(0.95f)
        , aoIntensity(5.0f)
        , maxOcclusionDistance(10.0f)
        , depthSharpness(0.0f)
        , bias(0.05f)
        , sampleCount(64)
    {}
}