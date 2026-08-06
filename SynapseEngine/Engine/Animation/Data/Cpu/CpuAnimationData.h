#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuAnimationBuffers.h"

namespace Syn
{
    struct SYN_API CpuAnimationData
    {
		uint32_t baseModelId = UINT32_MAX;
		GpuAnimationDescriptor descriptor;
		GpuMeshCollider globalCollider;
        std::vector<GpuMeshCollider> frameGlobalColliders;
        std::vector<GpuMeshCollider> frameMeshColliders;
        std::vector<BoneTrack> tracks;
    };
}