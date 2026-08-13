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
    struct SYN_API RenderPipelineNames
    {
        static constexpr const char* ScenePipeline = "ScenePipeline";
    };

    struct SYN_API RenderTargetGroupNames
    {
        static constexpr const char* Main = "Main";
    };

    struct SYN_API RenderTargetNames
    {
        static constexpr const char* Main = "Main";
        static constexpr const char* MainMSAA = "MainMSAA";
        static constexpr const char* ColorMetallic = "ColorMetallic";
        static constexpr const char* NormalRoughness = "NormalRoughness";
        static constexpr const char* EmissiveAo = "EmissiveAo";
        static constexpr const char* EntityIndex = "EntityIndex";
        static constexpr const char* EntityIndexMSAA = "EntityIndexMSAA";
        static constexpr const char* DepthPyramid = "DepthPyramid";
        static constexpr const char* Bloom = "Bloom";
        static constexpr const char* TransparentAccum = "TransparentAccum";
        static constexpr const char* TransparentAccumMSAA = "TransparentAccumMSAA";
        static constexpr const char* TransparentReveal = "TransparentReveal";
        static constexpr const char* TransparentRevealMSAA = "TransparentRevealMSAA";

        static constexpr const char* OpaqueDepth = "OpaqueDepth";
        static constexpr const char* OpaqueDepthMSAA = "OpaqueDepthMSAA";
        static constexpr const char* TransparentDepth = "TransparentDepth";
        static constexpr const char* SsaoAo = "SsaoAo";
        static constexpr const char* SsaoAoIntermediate = "SsaoAoIntermediate";

		static constexpr const char* DirectionLightShadowAtlas = "DirectionLightShadowAtlas";
        static constexpr const char* DirectionLightShadowDepthPyramid = "DirectionLightShadowDepthPyramid";
        
        static constexpr const char* SpotLightShadowAtlas = "SpotLightShadowAtlas";
        static constexpr const char* SpotLightShadowDepthPyramid = "SpotLightShadowDepthPyramid";

        static constexpr const char* PointLightShadowAtlas = "PointLightShadowAtlas";
        static constexpr const char* PointLightShadowDepthPyramid = "PointLightShadowDepthPyramid";

        static constexpr const char* PreviewAtlas = "PreviewAtlas";
    };

    struct SYN_API RenderTargetViewNames
    {
        static constexpr const char* Color = "Color";
        static constexpr const char* Metallic = "Metallic";
        static constexpr const char* Normal = "Normal";
        static constexpr const char* Roughness = "Roughness";
        static constexpr const char* Emissive = "Emissive";
        static constexpr const char* AmbientOcclusion = "AmbientOcclusion";

		static constexpr const char* DepthOpaqueMax = "DepthOpaqueMax";
		static constexpr const char* DepthTransparentMin = "DepthTransparentMin";

        static constexpr const char* DirectionLightShadowDepthPyramidMin = "DirectionLightShadowDepthPyramidMin";
        static constexpr const char* DirectionLightShadowDepthPyramidMax = "DirectionLightShadowDepthPyramidMax";

        static constexpr const char* SpotLightShadowDepthPyramidMin = "SpotLightShadowDepthPyramidMin";
        static constexpr const char* SpotLightShadowDepthPyramidMax = "SpotLightShadowDepthPyramidMax";

        static constexpr const char* PointLightShadowDepthPyramidMin = "PointLightShadowDepthPyramidMin";
        static constexpr const char* PointLightShadowDepthPyramidMax = "PointLightShadowDepthPyramidMax";
    };
}