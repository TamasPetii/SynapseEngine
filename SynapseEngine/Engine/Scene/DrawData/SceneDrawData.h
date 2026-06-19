#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Settings/SceneSettings.h"
#include "ModelDrawGroup.h"
#include "DebugDrawGroup.h"
#include "PointLightDrawGroup.h"
#include "SpotLightDrawGroup.h"
#include "DirectionLightDrawGroup.h"
#include "ForwardPlusDrawGroup.h"
#include "ChunkDrawGroup.h"
#include "SsaoDrawGroup.h"
#include <atomic>
#include "IDrawGroup.h"
#include "DirectionLightShadowDrawGroup.h"
#include "SpotLightShadowDrawGroup.h"

namespace Syn
{
    class SYN_API SceneDrawData : public IDrawGroup
    {
    public:
        SceneDrawData(uint32_t frameCount);

        void RequestGlobalSync(uint32_t framesInFlight);
		virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer frameContextBuffer;
        ModelDrawGroup Models;
        DebugDrawGroup Debug;
        PointLightDrawGroup PointLights;
        ForwardPlusDrawGroup ForwardPlus;
		ChunkDrawGroup Chunks;
		SsaoDrawGroup Ssao;
        DirectionLightDrawGroup DirectionLights;
        DirectionLightShadowDrawGroup DirectionLightShadow;
        SpotLightDrawGroup SpotLights;
        SpotLightShadowDrawGroup SpotLightShadow;

        std::atomic<uint32_t> syncFramesRemaining{ 0 };
    };
}