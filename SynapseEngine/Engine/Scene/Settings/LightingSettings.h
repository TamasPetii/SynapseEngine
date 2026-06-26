#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
    enum SYN_API PipelineType
    {
        Deferred,
        ForwardPlus
    };

    struct SYN_API LightingSettings
    {
        LightingSettings();

        PipelineType pipelineType;
        uint32_t tileSize;

        float ambientStrength;
        float emissiveStrength;

        // Deferred Features Toggles
        bool enableDeferredEmissiveAo;
        bool enableDeferredPointLights;
        bool enableDeferredSpotLights;
        bool enableDeferredDirectionalLights;

        // Forward+ Features Toggles
        bool enableForwardPlusEmissiveAo;
        bool enableForwardPlusPointLights;
        bool enableForwardPlusSpotLights;
        bool enableForwardPlusDirectionalLights;
    };
}