#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuAnimationBuffers.h"

namespace Syn
{
    struct SYN_API CpuAnimationData
    {
		GpuAnimationDescriptor descriptor;
		GpuMeshCollider globalCollider;
        std::vector<GpuMeshCollider> frameGlobalColliders;
        std::vector<GpuMeshCollider> frameMeshColliders;
        std::vector<BoneTrack> tracks;
    };
}