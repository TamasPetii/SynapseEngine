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
        static constexpr const char* Depth = "Depth";
        static constexpr const char* TransparentAccum = "TransparentAccum";
        static constexpr const char* TransparentReveal = "TransparentReveal";
        static constexpr const char* EditorPickingDepth = "EditorPickingDepth";

        static constexpr const char* DebugTopologyPipeline = "DebugTopologyPipeline";
        static constexpr const char* DebugMeshletLod = "DebugMeshletLod";
        static constexpr const char* DebugMaterialUv = "DebugMaterialUv";
    };

    struct SYN_API RenderTargetViewNames
    {
        static constexpr const char* Color = "Color";
        static constexpr const char* Metallic = "Metallic";
        static constexpr const char* Normal = "Normal";
        static constexpr const char* Roughness = "Roughness";
        static constexpr const char* Emissive = "Emissive";
        static constexpr const char* AmbientOcclusion = "AmbientOcclusion";

        static constexpr const char* DebugTopology = "DebugTopology";
        static constexpr const char* DebugPipeline = "DebugPipeline";
        static constexpr const char* DebugMeshlet = "DebugMeshlet";
        static constexpr const char* DebugLodGrayscale = "DebugLodGrayscale";
        static constexpr const char* DebugMaterial = "DebugMaterial";
        static constexpr const char* DebugUv = "DebugUv";
    };
}