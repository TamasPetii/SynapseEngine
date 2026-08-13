// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API PassGroupNames
    {
        static constexpr const char* UndefinedPasses = "UndefinedPasses";
        static constexpr const char* BillboardPasses = "BillboardPasses";
        static constexpr const char* BloomPasses = "BloomPasses";
        static constexpr const char* DirectionalLightCullingPasses = "DirectionalLightCullingPasses";
        static constexpr const char* PointLightCullingPasses = "PointLightCullingPasses";
        static constexpr const char* SpotLightCullingPasses = "SpotLightCullingPasses";
        static constexpr const char* GeometryCullingPasses = "GeometryCullingPasses";
        static constexpr const char* DirectionLightShadowCullingPasses = "DirectionLightShadowCullingPasses";
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
		static constexpr const char* DirectionLightShadowPasses = "DirectionLightShadowPasses";
        static constexpr const char* PointLightShadowPasses = "PointLightShadowPasses";
        static constexpr const char* SpotLightShadowPasses = "SpotLightShadowPasses";
		static constexpr const char* PostProcessPasses = "PostProcessPasses";
        static constexpr const char* UtilityPasses = "UtilityPasses";
    };
}