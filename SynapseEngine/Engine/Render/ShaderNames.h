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
    struct SYN_API ShaderDefines
    {
        static constexpr const char* EnableAlphaTest = "ENABLE_ALPHA_TEST";
        static constexpr const char* EnableWorkGraph = "ENABLE_WORK_GRAPH";
    };

    struct SYN_API ShaderNames
    {
        static constexpr const char* FullscreenVert = "Engine/Shaders/Passes/Common/Fullscreen.vert";
        static constexpr const char* CompositeFrag = "Engine/Shaders/Passes/Common/Composite.frag";

        static constexpr const char* BillboardVert = "Engine/Shaders/Passes/Billboard/Billboard.vert";
        static constexpr const char* BillboardFrag = "Engine/Shaders/Passes/Billboard/Billboard.frag";

        static constexpr const char* BloomPrefilter = "Engine/Shaders/Passes/PostProcess/Bloom/BloomPrefilter.comp";
        static constexpr const char* BloomUpsample = "Engine/Shaders/Passes/PostProcess/Bloom/BloomUpsample.comp";
        static constexpr const char* BloomDownsample = "Engine/Shaders/Passes/PostProcess/Bloom/BloomDownsample.comp";
        static constexpr const char* BloomComposite = "Engine/Shaders/Passes/PostProcess/Bloom/BloomComposite.comp";

        static constexpr const char* SkyVert = "Engine/Shaders/Passes/PostProcess/Sky/Sky.vert";
        static constexpr const char* SkySphereFrag = "Engine/Shaders/Passes/PostProcess/Sky/SkySphere.frag";
        static constexpr const char* SkyBoxFrag = "Engine/Shaders/Passes/PostProcess/Sky/SkyBox.frag";

        static constexpr const char* SelectionOutlineFrag = "Engine/Shaders/Passes/PostProcess/Outline/SelectionOutline.frag";

        static constexpr const char* StaticSceneAABB = "Engine/Shaders/Passes/Morton/StaticSceneAABB.comp";
        static constexpr const char* MortonGenerator = "Engine/Shaders/Passes/Morton/MortonGenerator.comp";
        static constexpr const char* ChunkBuilder = "Engine/Shaders/Passes/Morton/ChunkBuilder.comp";

        static constexpr const char* HizLinearizeDepth = "Engine/Shaders/Passes/Hiz/HizLinearizeDepth.comp";
        static constexpr const char* HizDownsample = "Engine/Shaders/Passes/Hiz/HizDownsample.comp";
        static constexpr const char* HizCopyComp = "Engine/Shaders/Passes/Hiz/HizCopy.comp";
        static constexpr const char* HizLinearizeSingleDepth = "Engine/Shaders/Passes/Hiz/HizLinearizeSingleDepth.comp";
        static constexpr const char* SpotHizLinearizeSingleDepth = "Engine/Shaders/Passes/Hiz/SpotHizLinearizeSingleDepth.comp";
        static constexpr const char* PointHizLinearizeSingleDepth = "Engine/Shaders/Passes/Hiz/PointHizLinearizeSingleDepth.comp";

        static constexpr const char* MeshletTask = "Engine/Shaders/Passes/Shading/Common/Meshlet.task";
        static constexpr const char* MeshletMesh = "Engine/Shaders/Passes/Shading/Common/Meshlet.mesh";
        static constexpr const char* TraditionalVert = "Engine/Shaders/Passes/Shading/Common/Traditional.vert";

        static constexpr const char* OpaqueDeferredFrag = "Engine/Shaders/Passes/Shading/Deferred/GBuffer/OpaqueDeferred.frag";
        
        static constexpr const char* DeferredEmissiveAoVert = "Engine/Shaders/Passes/Shading/Deferred/Lighting/DeferredEmissiveAo.vert";
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

        static constexpr const char* WireframePhysicsVert = "Engine/Shaders/Passes/Wireframe/WireframePhysics.vert";
        static constexpr const char* WireframeSetup = "Engine/Shaders/Passes/Wireframe/WireframeSetup.comp";
        static constexpr const char* WireframeMeshVert = "Engine/Shaders/Passes/Wireframe/WireframeMesh.vert";
        static constexpr const char* WireframeMeshletMesh = "Engine/Shaders/Passes/Wireframe/WireframeMeshlet.mesh";
        static constexpr const char* WireframeFrag = "Engine/Shaders/Passes/Wireframe/Wireframe.frag";
        static constexpr const char* WireframeDebugVert = "Engine/Shaders/Passes/Wireframe/WireframeDebug.vert";

        static constexpr const char* DebugVisibilityFrag = "Engine/Shaders/Passes/Shading/Visibility/DebugVisibility.frag";
        static constexpr const char* DpHvoComp = "Engine/Shaders/Passes/PostProcess/Ssao/DpHvo.comp";
        static constexpr const char* DpHvoBlurComp = "Engine/Shaders/Passes/PostProcess/Ssao/DpHvoBlur.comp";
        static constexpr const char* SsaoComp = "Engine/Shaders/Passes/PostProcess/Ssao/Ssao.comp";
        static constexpr const char* SsaoBlurComp = "Engine/Shaders/Passes/PostProcess/Ssao/SsaoBlur.comp";

        static constexpr const char* InfiniteGridFrag = "Engine/Shaders/Passes/PostProcess/InfiniteGrid/InfiniteGrid.frag";

        static constexpr const char* GeometryCullingCommandResetComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryCullingCommandReset.comp";
        static constexpr const char* GeometryMeshCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryMeshCulling.comp";
        static constexpr const char* GeometryModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryModelCulling.comp";
        static constexpr const char* GeometryStaticChunkCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryStaticChunkCulling.comp";
        static constexpr const char* GeometryStaticModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryStaticModelCulling.comp";
        static constexpr const char* GeometryMortonChunkCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryMortonChunkCulling.comp";
        static constexpr const char* GeometryMortonModelCullingComp = "Engine/Shaders/Passes/Culling/Geometry/GeometryMortonModelCulling.comp";

        static constexpr const char* DirectionLightShadowFrag = "Engine/Shaders/Passes/Shadow/DirectionLight/DirectionLightShadow.frag";
        static constexpr const char* DirectionLightShadowTransparentFrag = "Engine/Shaders/Passes/Shadow/DirectionLight/DirectionLightShadowTransparent.frag";
        static constexpr const char* DirectionLightShadowTraditionalVert = "Engine/Shaders/Passes/Shadow/DirectionLight/DirectionLightShadowTraditional.vert";
        static constexpr const char* DirectionLightShadowMeshletTask = "Engine/Shaders/Passes/Shadow/DirectionLight/DirectionLightShadowMeshlet.task";
        static constexpr const char* DirectionLightShadowMeshletMesh = "Engine/Shaders/Passes/Shadow/DirectionLight/DirectionLightShadowMeshlet.mesh";

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

        static constexpr const char* SpotLightCulling = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightCulling.comp";
        static constexpr const char* SpotLightShadowFrag = "Engine/Shaders/Passes/Shadow/SpotLight/SpotLightShadow.frag";
        static constexpr const char* SpotLightShadowTransparentFrag = "Engine/Shaders/Passes/Shadow/SpotLight/SpotLightShadowTransparent.frag";
        static constexpr const char* SpotLightShadowTraditionalVert = "Engine/Shaders/Passes/Shadow/SpotLight/SpotLightShadowTraditional.vert";
        static constexpr const char* SpotLightShadowMeshletTask = "Engine/Shaders/Passes/Shadow/SpotLight/SpotLightShadowMeshlet.task";
        static constexpr const char* SpotLightShadowMeshletMesh = "Engine/Shaders/Passes/Shadow/SpotLight/SpotLightShadowMeshlet.mesh";

        static constexpr const char* SpotLightShadowAtlasAllocatorComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowAtlasAllocator.comp";
        static constexpr const char* SpotLightShadowCullingCommandResetComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowCullingCommandReset.comp";
        static constexpr const char* SpotLightShadowModelCullingComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowModelCulling.comp";
        static constexpr const char* SpotLightShadowMeshCullingComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowMeshCulling.comp";
        static constexpr const char* SpotLightShadowFinalizeSetupComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowFinalizeSetup.comp";
        static constexpr const char* SpotLightShadowFinalizeComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowFinalize.comp";
        static constexpr const char* SpotLightShadowMortonChunkCullingComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowMortonChunkCulling.comp";
        static constexpr const char* SpotLightShadowMortonModelCullingComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowMortonModelCulling.comp";
        static constexpr const char* SpotLightShadowStaticChunkCullingComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowStaticChunkCulling.comp";
        static constexpr const char* SpotLightShadowStaticModelCullingComp = "Engine/Shaders/Passes/Culling/SpotLight/SpotLightShadowStaticModelCulling.comp";

        static constexpr const char* PointLightCulling = "Engine/Shaders/Passes/Culling/PointLight/PointLightCulling.comp";
        static constexpr const char* PointLightShadowFrag = "Engine/Shaders/Passes/Shadow/PointLight/PointLightShadow.frag";
        static constexpr const char* PointLightShadowTransparentFrag = "Engine/Shaders/Passes/Shadow/PointLight/PointLightShadowTransparent.frag";
        static constexpr const char* PointLightShadowTraditionalVert = "Engine/Shaders/Passes/Shadow/PointLight/PointLightShadowTraditional.vert";
        static constexpr const char* PointLightShadowMeshletTask = "Engine/Shaders/Passes/Shadow/PointLight/PointLightShadowMeshlet.task";
        static constexpr const char* PointLightShadowMeshletMesh = "Engine/Shaders/Passes/Shadow/PointLight/PointLightShadowMeshlet.mesh";

        static constexpr const char* PointLightShadowAtlasAllocatorComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowAtlasAllocator.comp";
        static constexpr const char* PointLightShadowCullingCommandResetComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowCullingCommandReset.comp";
        static constexpr const char* PointLightShadowModelCullingComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowModelCulling.comp";
        static constexpr const char* PointLightShadowMeshCullingComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowMeshCulling.comp";
        static constexpr const char* PointLightShadowFinalizeSetupComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowFinalizeSetup.comp";
        static constexpr const char* PointLightShadowFinalizeComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowFinalize.comp";
        static constexpr const char* PointLightShadowMortonChunkCullingComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowMortonChunkCulling.comp";
        static constexpr const char* PointLightShadowMortonModelCullingComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowMortonModelCulling.comp";
        static constexpr const char* PointLightShadowStaticChunkCullingComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowStaticChunkCulling.comp";
        static constexpr const char* PointLightShadowStaticModelCullingComp = "Engine/Shaders/Passes/Culling/PointLight/PointLightShadowStaticModelCulling.comp";

        static constexpr const char* MaterialPreviewVert = "Engine/Shaders/Passes/Preview/MaterialPreview.vert";
        static constexpr const char* MaterialPreviewFrag = "Engine/Shaders/Passes/Preview/MaterialPreview.frag";

        static constexpr const char* ModelPreviewVert = "Engine/Shaders/Passes/Preview/ModelPreview.vert";
        static constexpr const char* ModelPreviewFrag = "Engine/Shaders/Passes/Preview/ModelPreview.frag";

        static constexpr const char* AnimationPreviewVert = "Engine/Shaders/Passes/Preview/AnimationPreview.vert";
        static constexpr const char* AnimationPreviewFrag = "Engine/Shaders/Passes/Preview/AnimationPreview.frag";

        static constexpr const char* YuvToRgbComp = "Engine/Shaders/Passes/Video/YuvToRgb.comp";

        static constexpr const char* EquirectangularToCube = "Engine/Shaders/Passes/Environment/EquirectangularToCube.comp";
        static constexpr const char* IrradianceConvolution = "Engine/Shaders/Passes/Environment/IrradianceConvolution.comp";
        static constexpr const char* PrefilterConvolution = "Engine/Shaders/Passes/Environment/PrefilterConvolution.comp";
    }; 
}