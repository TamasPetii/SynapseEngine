#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API RenderPipelineNames
    {
        static constexpr const char* DeferredPipeline = "DeferredPipeline";
    };

    struct SYN_API RenderTargetGroupNames
    {
        static constexpr const char* Deferred = "Deferred";
    };

    struct SYN_API RenderTargetNames
    {
        static constexpr const char* Main = "Main";
        static constexpr const char* ColorMetallic = "ColorMetallic";
        static constexpr const char* NormalRoughness = "NormalRoughness";
        static constexpr const char* EmissiveAo = "EmissiveAo";
        static constexpr const char* EntityIndex = "EntityIndex";
        static constexpr const char* DepthPyramid = "DepthPyramid";
        static constexpr const char* Bloom = "Bloom";
        static constexpr const char* TransparentAccum = "TransparentAccum";
        static constexpr const char* TransparentReveal = "TransparentReveal";

        static constexpr const char* OpaqueDepth = "OpaqueDepth";
        static constexpr const char* TransparentDepth = "TransparentDepth";
        static constexpr const char* SsaoAo = "SsaoAo";
        static constexpr const char* SsaoAoIntermediate = "SsaoAoIntermediate";

		static constexpr const char* DirectionLightShadowAtlas = "DirectionLightShadowAtlas";
        static constexpr const char* DirectionLightShadowDepthPyramid = "DirectionLightShadowDepthPyramid";
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
    };
}