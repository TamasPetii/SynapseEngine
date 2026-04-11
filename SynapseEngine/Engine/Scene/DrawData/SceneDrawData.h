#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/SceneSettings.h"
#include "ModelDrawGroup.h"
#include "DebugDrawGroup.h"
#include "PointLightDrawGroup.h"
#include "SpotLightDrawGroup.h"
#include "DirectionLightDrawGroup.h"
#include <atomic>

namespace Syn
{
    class SYN_API SceneDrawData
    {
    public:
        SceneDrawData(uint32_t frameCount);

        void RecordGpuSync(VkCommandBuffer cmd, uint32_t frameIndex);
        void RequestGlobalSync(uint32_t framesInFlight);

        ModelDrawGroup Models;
        DebugDrawGroup Debug;
        PointLightDrawGroup PointLights;
        SpotLightDrawGroup SpotLights;
        DirectionLightDrawGroup DirectionLights;
        std::atomic<uint32_t> syncFramesRemaining{ 0 };
    };
}