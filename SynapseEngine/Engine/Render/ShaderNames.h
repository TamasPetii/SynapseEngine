#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API ShaderNames
    {
        static constexpr const char* FullscreenVert = "Engine/Shaders/Passes/Common/Fullscreen.vert";
        static constexpr const char* CompositeFrag = "Engine/Shaders/Passes/Common/Composite.frag";

        static constexpr const char* BillboardVert = "Engine/Shaders/Passes/Billboard/Billboard.vert";
        static constexpr const char* BillboardFrag = "Engine/Shaders/Passes/Billboard/Billboard.frag";

        static constexpr const char* BloomPrefilter = "Engine/Shaders/Passes/PostProcess/BloomPrefilter.comp";
        static constexpr const char* BloomUpsample = "Engine/Shaders/Passes/PostProcess/BloomUpsample.comp";
        static constexpr const char* BloomDownsample = "Engine/Shaders/Passes/PostProcess/BloomDownsample.comp";
        static constexpr const char* BloomComposite = "Engine/Shaders/Passes/PostProcess/BloomComposite.comp";

        static constexpr const char* StaticSceneAABB = "Engine/Shaders/Passes/Morton/StaticSceneAABB.comp";
        static constexpr const char* MortonGenerator = "Engine/Shaders/Passes/Morton/MortonGenerator.comp";
        static constexpr const char* ChunkBuilder = "Engine/Shaders/Passes/Morton/ChunkBuilder.comp";

        static constexpr const char* PointLightCulling = "Engine/Shaders/Passes/Culling/PointLightCulling.comp";
        static constexpr const char* SpotLightCulling = "Engine/Shaders/Passes/Culling/SpotLightCulling.comp";

        static constexpr const char* HizLinearizeDepth = "Engine/Shaders/Passes/Hiz/HizLinearizeDepth.comp";
        static constexpr const char* HizDownsample = "Engine/Shaders/Passes/Hiz/HizDownsample.comp";
        static constexpr const char* HizCopyComp = "Engine/Shaders/Passes/Hiz/HizCopy.comp";
        
        static constexpr const char* MeshletTask = "Engine/Shaders/Passes/Shading/Common/Meshlet.task";
        static constexpr const char* MeshletMesh = "Engine/Shaders/Passes/Shading/Common/Meshlet.mesh";
        static constexpr const char* TraditionalVert = "Engine/Shaders/Passes/Shading/Common/Traditional.vert";

        static constexpr const char* OpaqueDeferredFrag = "Engine/Shaders/Passes/Shading/Deferred/GBuffer/OpaqueDeferred.frag";
        static constexpr const char* DeferredEmissiveAoFrag = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredEmissiveAo.frag";
        static constexpr const char* DeferredPointLightVert = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredPointLight.vert";
        static constexpr const char* DeferredPointLightFrag = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredPointLight.frag";
        static constexpr const char* DeferredSpotLightVert = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredSpotLight.vert";
        static constexpr const char* DeferredSpotLightFrag = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredSpotLight.frag";
        static constexpr const char* DeferredDirectionLightVert = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredDirectionLight.vert";
        static constexpr const char* DeferredDirectionLightFrag = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredDirectionLight.frag";

        static constexpr const char* ClusterSpotLightSingle = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightSingle.comp";
        static constexpr const char* ClusterPointLightSingle = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightSingle.comp";
        static constexpr const char* ClusterDispatchSetup = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterDispatchSetup.comp";
        static constexpr const char* ClusterSetup = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterSetup.comp";
        static constexpr const char* ClusterPointLightCount = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightCount.comp";
        static constexpr const char* ClusterSpotLightCount = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightCount.comp";
        static constexpr const char* ClusterPrefixSum = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterPrefixSum.comp";
        static constexpr const char* ClusterPointLightWrite = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightWrite.comp";
        static constexpr const char* ClusterSpotLightWrite = "Engine/Shaders/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightWrite.comp";

        static constexpr const char* PreDepthFrag = "Engine/Shaders/Passes/Shading/ForwardPlus/DepthPrepass/PreDepth.frag";
        static constexpr const char* MeshletPreDepthMesh = "Engine/Shaders/Passes/Shading/ForwardPlus/DepthPrepass/MeshletPreDepth.mesh";
        static constexpr const char* TraditionalPreDepthVert = "Engine/Shaders/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalPreDepth.vert";

        static constexpr const char* OpaqueForwardFrag = "Engine/Shaders/Passes/Shading/ForwardPlus/Lighting/OpaqueForward.frag";

        static constexpr const char* TransparentCompositeFrag = "Engine/Shaders/Passes/Shading/Wboit/TransparentComposite.frag";
        static constexpr const char* TransparentForwardFrag = "Engine/Shaders/Passes/Shading/Wboit/TransparentForward.frag";

        static constexpr const char* WireframeSetup = "Engine/Shaders/Passes/Wireframe/WireframeSetup.comp";
        static constexpr const char* WireframeMeshVert = "Engine/Shaders/Passes/Wireframe/WireframeMesh.vert";
        static constexpr const char* WireframeMeshletMesh = "Engine/Shaders/Passes/Wireframe/WireframeMeshlet.mesh";
        static constexpr const char* WireframeFrag = "Engine/Shaders/Passes/Wireframe/Wireframe.frag";
        static constexpr const char* WireframeDebugVert = "Engine/Shaders/Passes/Wireframe/WireframeDebug.vert";

        static constexpr const char* DebugVisibilityFrag = "Engine/Shaders/Passes/Shading/Visibility/DebugVisibility.frag";
        static constexpr const char* DpHvoComp = "Engine/Shaders/Passes/Ssao/DpHvo.comp";
        static constexpr const char* DpHvoBlurComp = "Engine/Shaders/Passes/Ssao/DpHvoBlur.comp";
        static constexpr const char* SsaoComp = "Engine/Shaders/Passes/Ssao/Ssao.comp";
        static constexpr const char* SsaoBlurComp = "Engine/Shaders/Passes/Ssao/SsaoBlur.comp";

        static constexpr const char* DirectionLightShadowFarg = "Engine/Shaders/Passes/Shadow/Direction/DirectionLightShadow.frag";
        static constexpr const char* DirectionLightShadowTraditionalVert = "Engine/Shaders/Passes/Shadow/Direction/DirectionLightShadowTraditional.vert";
		static constexpr const char* DirectionLightShadowMeshletTask = "Engine/Shaders/Passes/Shadow/Direction/DirectionLightShadowMeshlet.task";
		static constexpr const char* DirectionLightShadowMeshletMesh = "Engine/Shaders/Passes/Shadow/Direction/DirectionLightShadowMeshlet.mesh";
   
        static constexpr const char* GeometryCullingCommandResetComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryCullingCommandReset.comp";
        static constexpr const char* GeometryMeshCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryMeshCulling.comp";
        static constexpr const char* GeometryModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryModelCulling.comp";
        static constexpr const char* GeometryStaticChunkCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryStaticChunkCulling.comp";
        static constexpr const char* GeometryStaticModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryStaticModelCulling.comp";
        static constexpr const char* GeometryMortonChunkCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryMortonChunkCulling.comp";
        static constexpr const char* GeometryMortonModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryMortonModelCulling.comp";
    
        static constexpr const char* GeometryWorkGraphModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryWorkGraphModelCulling.comp";
        static constexpr const char* GeometryWorkGraphStaticChunkCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryWorkGraphStaticChunkCulling.comp";
		static constexpr const char* GeometryWorkGraphStaticModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryWorkGraphStaticModelCulling.comp";
		static constexpr const char* GeometryWorkGraphMortonChunkCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryWorkGraphMortonChunkCulling.comp";
		static constexpr const char* GeometryWorkGraphMortonModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryWorkGraphMortonModelCulling.comp";
		static constexpr const char* GeometryWorkGraphMeshCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryWorkGraphMeshCulling.comp";
    
        static constexpr const char* DirectionLightShadowCullingCommandResetComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowCullingCommandReset.comp";
        static constexpr const char* DirectionLightShadowMeshCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowMeshCulling.comp";
        static constexpr const char* DirectionLightShadowModelCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowModelCulling.comp";
        static constexpr const char* DirectionLightShadowStaticChunkCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowStaticChunkCulling.comp";
        static constexpr const char* DirectionLightShadowStaticModelCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowStaticModelCulling.comp";
        static constexpr const char* DirectionLightShadowMortonChunkCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowMortonChunkCulling.comp";
        static constexpr const char* DirectionLightShadowMortonModelCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowMortonModelCulling.comp";

        static constexpr const char* DirectionLightShadowWorkGraphModelCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowWorkGraphModelCulling.comp";
        static constexpr const char* DirectionLightShadowWorkGraphStaticChunkCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowWorkGraphStaticChunkCulling.comp";
        static constexpr const char* DirectionLightShadowWorkGraphStaticModelCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowWorkGraphStaticModelCulling.comp";
        static constexpr const char* DirectionLightShadowWorkGraphMortonChunkCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowWorkGraphMortonChunkCulling.comp";
        static constexpr const char* DirectionLightShadowWorkGraphMortonModelCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowWorkGraphMortonModelCulling.comp";
        static constexpr const char* DirectionLightShadowWorkGraphMeshCullingComp = "Engine/Shaders/Passes/Culling/DirectionLight/DirectionLightShadowWorkGraphMeshCulling.comp";

    };
}