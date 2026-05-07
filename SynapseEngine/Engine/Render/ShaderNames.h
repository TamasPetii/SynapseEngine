#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API ShaderNames
    {
        static constexpr const char* FullscreenVert = "../Engine/Shaders/Passes/Common/Fullscreen.vert";
        static constexpr const char* CompositeFrag = "../Engine/Shaders/Passes/Common/Composite.frag";

        static constexpr const char* BillboardVert = "../Engine/Shaders/Passes/Billboard/Billboard.vert";
        static constexpr const char* BillboardFrag = "../Engine/Shaders/Passes/Billboard/Billboard.frag";

        static constexpr const char* BloomPrefilter = "../Engine/Shaders/Passes/PostProcess/BloomPrefilter.comp";
        static constexpr const char* BloomUpsample = "../Engine/Shaders/Passes/PostProcess/BloomUpsample.comp";
        static constexpr const char* BloomDownsample = "../Engine/Shaders/Passes/PostProcess/BloomDownsample.comp";
        static constexpr const char* BloomComposite = "../Engine/Shaders/Passes/PostProcess/BloomComposite.comp";

        static constexpr const char* CullingCommandReset = "../Engine/Shaders/Passes/Culling/CullingCommandReset.comp";
        static constexpr const char* MeshCulling = "../Engine/Shaders/Passes/Culling/MeshCulling.comp";
        static constexpr const char* ModelCulling = "../Engine/Shaders/Passes/Culling/ModelCulling.comp";
        static constexpr const char* PointLightCulling = "../Engine/Shaders/Passes/Culling/PointLightCulling.comp";
        static constexpr const char* SpotLightCulling = "../Engine/Shaders/Passes/Culling/SpotLightCulling.comp";

        static constexpr const char* HizLinearizeDepth = "../Engine/Shaders/Passes/Hiz/HizLinearizeDepth.comp";
        static constexpr const char* HizDownsample = "../Engine/Shaders/Passes/Hiz/HizDownsample.comp";

        static constexpr const char* MeshletTask = "../Engine/Shaders/Passes/Shading/Common/Meshlet.task";
        static constexpr const char* MeshletMesh = "../Engine/Shaders/Passes/Shading/Common/Meshlet.mesh";
        static constexpr const char* TraditionalVert = "../Engine/Shaders/Passes/Shading/Common/Traditional.vert";

        static constexpr const char* OpaqueDeferredFrag = "../Engine/Shaders/Passes/Shading/Deferred/GBuffer/OpaqueDeferred.frag";
        static constexpr const char* DeferredEmissiveAoFrag = "../Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredEmissiveAo.frag";
        static constexpr const char* DeferredPointLightVert = "../Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredPointLight.vert";
        static constexpr const char* DeferredPointLightFrag = "../Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredPointLight.frag";
        static constexpr const char* DeferredSpotLightVert = "../Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredSpotLight.vert";
        static constexpr const char* DeferredSpotLightFrag = "../Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredSpotLight.frag";
        static constexpr const char* DeferredDirectionLightVert = "../Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredDirectionLight.vert";
        static constexpr const char* DeferredDirectionLightFrag = "../Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredDirectionLight.frag";

        static constexpr const char* ClusterSetup = "../Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterSetup.comp";
        static constexpr const char* ClusterPointLightCount = "../Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightCount.comp";
        static constexpr const char* ClusterSpotLightCount = "../Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightCount.comp";
        static constexpr const char* ClusterPrefixSum = "../Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterPrefixSum.comp";
        static constexpr const char* ClusterPointLightWrite = "../Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightWrite.comp";
        static constexpr const char* ClusterSpotLightWrite = "../Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightWrite.comp";

        static constexpr const char* PreDepthFrag = "../Engine/Shaders/Passes/Shading/ForwardPlus/DepthPrepass/PreDepth.frag";
        static constexpr const char* MeshletPreDepthMesh = "../Engine/Shaders/Passes/Shading/ForwardPlus/DepthPrepass/MeshletPreDepth.mesh";
        static constexpr const char* TraditionalPreDepthVert = "../Engine/Shaders/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalPreDepth.vert";

        static constexpr const char* OpaqueForwardFrag = "../Engine/Shaders/Passes/Shading/ForwardPlus/Lighting/OpaqueForward.frag";

        static constexpr const char* TransparentCompositeFrag = "../Engine/Shaders/Passes/Shading/Wboit/TransparentComposite.frag";
        static constexpr const char* TransparentForwardFrag = "../Engine/Shaders/Passes/Shading/Wboit/TransparentForward.frag";

        static constexpr const char* WireframeSetup = "../Engine/Shaders/Passes/Wireframe/WireframeSetup.comp";
        static constexpr const char* WireframeVert = "../Engine/Shaders/Passes/Wireframe/Wireframe.vert";
        static constexpr const char* WireframeFrag = "../Engine/Shaders/Passes/Wireframe/Wireframe.frag";
        static constexpr const char* WireframeLightVert = "../Engine/Shaders/Passes/Wireframe/WireframeLight.vert";
    };
}