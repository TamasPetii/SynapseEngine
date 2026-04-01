#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API ShaderNames
    {
        static constexpr const char* TraditionalVert = "../Engine/Shaders/Passes/Geometry/Traditional.vert";
        static constexpr const char* TraditionalFrag = "../Engine/Shaders/Passes/Geometry/Traditional.frag";
        static constexpr const char* TraditionalTransparentFrag = "../Engine/Shaders/Passes/Geometry/TraditionalTransparent.frag";
        static constexpr const char* TraditionalTransparentPickingFrag = "../Engine/Shaders/Passes/Geometry/TraditionalTransparentPicking.frag";
        
        static constexpr const char* MeshletMesh = "../Engine/Shaders/Passes/Geometry/Meshlet.mesh";
        static constexpr const char* MeshletTask = "../Engine/Shaders/Passes/Geometry/Meshlet.task";
        static constexpr const char* MeshletFrag = "../Engine/Shaders/Passes/Geometry/Meshlet.frag";
        static constexpr const char* MeshletTransparentFrag = "../Engine/Shaders/Passes/Geometry/MeshletTransparent.frag";
        static constexpr const char* MeshletTransparentPickingFrag = "../Engine/Shaders/Passes/Geometry/MeshletTransparentPicking.frag";
        
        static constexpr const char* MeshCulling = "../Engine/Shaders/Passes/Culling/MeshCulling.comp";
        static constexpr const char* ModelCulling = "../Engine/Shaders/Passes/Culling/ModelCulling.comp";
        static constexpr const char* PointLightCulling = "../Engine/Shaders/Passes/Culling/PointLightCulling.comp";

        static constexpr const char* HizLinearizeDepth = "../Engine/Shaders/Passes/Hiz/HizLinearizeDepth.comp";
        static constexpr const char* HizDownsample = "../Engine/Shaders/Passes/Hiz/HizDownsample.comp";

        static constexpr const char* FullscreenVert = "../Engine/Shaders/Passes/Common/Fullscreen.vert";
        static constexpr const char* CompositeFrag = "../Engine/Shaders/Passes/Common/Composite.frag";
        static constexpr const char* TransparentCompositeFrag = "../Engine/Shaders/Passes/Common/TransparentComposite.frag";

        static constexpr const char* BloomPrefilter = "../Engine/Shaders/Passes/PostProcess/BloomPrefilter.comp";
        static constexpr const char* BloomUpsample = "../Engine/Shaders/Passes/PostProcess/BloomUpsample.comp";
        static constexpr const char* BloomDownsample = "../Engine/Shaders/Passes/PostProcess/BloomDownsample.comp";
        static constexpr const char* BloomComposite = "../Engine/Shaders/Passes/PostProcess/BloomComposite.comp";

        static constexpr const char* WireframeSetup = "../Engine/Shaders/Passes/Wireframe/WireframeSetup.comp";
        static constexpr const char* WireframeVert = "../Engine/Shaders/Passes/Wireframe/Wireframe.vert";
        static constexpr const char* WireframeMeshletVert = "../Engine/Shaders/Passes/Wireframe/WireframeMeshlet.vert";
        static constexpr const char* WireframeFrag = "../Engine/Shaders/Passes/Wireframe/Wireframe.frag";

		static constexpr const char* DeferredEmissiveAoFrag = "../Engine/Shaders/Passes/Deferred/DeferredEmissiveAo.frag";
        static constexpr const char* DeferredPointLightVert = "../Engine/Shaders/Passes/Deferred/DeferredPointLight.vert";
        static constexpr const char* DeferredPointLightFrag = "../Engine/Shaders/Passes/Deferred/DeferredPointLight.frag";
        static constexpr const char* DeferredSpotLightVert = "../Engine/Shaders/Passes/Deferred/DeferredSpotLight.vert";
        static constexpr const char* DeferredSpotLightFrag = "../Engine/Shaders/Passes/Deferred/DeferredSpotLight.frag";
        static constexpr const char* DeferredDirectionLightVert = "../Engine/Shaders/Passes/Deferred/DeferredDirectionLight.vert";
        static constexpr const char* DeferredDirectionLightFrag = "../Engine/Shaders/Passes/Deferred/DeferredDirectionLight.frag";
    };
}