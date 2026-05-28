#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API PassGroupNames
    {
        static constexpr const char* UndefinedPasses = "UndefinedPasses";
        static constexpr const char* BillboardPasses = "BillboardPasses";
        static constexpr const char* BloomPasses = "BloomPasses";
        static constexpr const char* CullingPasses = "CullingPasses";
        static constexpr const char* HizPasses = "HizPasses";
        static constexpr const char* PresentPasses = "PresentPasses";
        static constexpr const char* InitSetupPasses = "InitSetupPasses";
        static constexpr const char* ShadingSetupPasses = "ShadingSetupPasses";
        static constexpr const char* DeferredGBufferPasses = "DeferredGBufferPasses";
        static constexpr const char* DeferredLightingPasses = "DeferredLightingPasses";
        static constexpr const char* ForwardPlusClusterPasses = "ForwardPlusClusterPasses";
        static constexpr const char* ForwardPlusDepthPrePasses = "ForwardPlusDepthPrePasses";
        static constexpr const char* ForwardPlusLightingPasses = "ForwardPlusLightingPasses";
        static constexpr const char* WboitPasses = "WboitPasses";
        static constexpr const char* DebugPasses = "DebugPasses";
        static constexpr const char* WireframePasses = "WireframePasses";
        static constexpr const char* MortonPasses = "MortonPasses";
        static constexpr const char* SsaoPasses = "SsaoPasses";
    };
}