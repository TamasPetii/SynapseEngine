#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API PostProcessSettings
    {
        PostProcessSettings();

        // Bloom Parameters
        bool enableBloom;
        float bloomThreshold;
        float bloomKnee;
        float bloomFilterRadius;
        float bloomExposure;
        float bloomStrength;

        // Ssao Parameters
        bool enableSsao;
        bool enableSsaoLight;
        float aoRadius;
        float aoIntensity;
        float maxOcclusionDistance;
        float depthSharpness;
        float bias;
        int sampleCount;
    };
}