#include "GlobalFrameSetupPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Scene/BufferNames.h"

#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn {

    #include "Engine/Shaders/Includes/Common/FrameGlobalContext.glsl"

    void GlobalFrameSetupPass::Transfer(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto settings = scene->GetSettings();
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();

        drawData->ForwardPlus.CheckResize(settings->lighting.tileSize, width, height, fIdx);

        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        FrameGlobalContext ctx = {};

        ctx.globalDrawCountBufferAddr = drawData->Models.drawCountBuffer.GetAddress(fIdx);
        ctx.globalInstanceIndexBufferAddr = drawData->Models.instanceBuffer.GetAddress(fIdx);
        ctx.globalIndirectCommandBufferAddr = drawData->Models.indirectBuffer.GetAddress(fIdx);
        ctx.globalIndirectCommandDescriptorBufferAddr = drawData->Models.descriptorBuffer.GetAddress(fIdx);
        ctx.globalModelAllocationBufferAddr = drawData->Models.modelAllocBuffer.GetAddress(fIdx);
        ctx.globalMeshAllocationBufferAddr = drawData->Models.meshAllocBuffer.GetAddress(fIdx);

        ctx.cameraVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::CameraVisibleData, fIdx);
        ctx.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        ctx.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        
        ctx.transformBufferAddr = compManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        ctx.transformSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
		ctx.transformModelLinkBufferAddr = compManager->GetBufferAddr(BufferNames::TransformModelLinkData, fIdx);

        ctx.tagSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::TagSparseMap, fIdx);
        ctx.tagDataBufferAddr = compManager->GetBufferAddr(BufferNames::TagData, fIdx);

		ctx.staticChunkDataBufferAddr = drawData->Chunks.chunkDataBuffer.GetAddress(fIdx);
        ctx.staticChunkVisibleIndexBufferAddr = drawData->Chunks.chunkVisibilityBuffer.GetAddress(fIdx);
        ctx.staticChunkCountBufferAddr = drawData->Chunks.chunkIndirectDispatchBuffer.GetAddress(fIdx);

        ctx.sceneAabbBufferAddr = drawData->Chunks.sceneAabbBuffer.GetAddress(fIdx);
        ctx.mortonChunkIndirectDispatchBufferAddr = drawData->Chunks.mortonIndirectDispatchBuffer.GetAddress(fIdx);
        ctx.mortonChunkIndirectDrawBufferAddr = drawData->Chunks.mortonIndirectDrawBuffer.GetAddress(fIdx);
        ctx.mortonKeysBufferAddr = compManager->GetBufferAddr(BufferNames::MortonKeysData, fIdx);
        ctx.mortonValuesBufferAddr = compManager->GetBufferAddr(BufferNames::MortonValuesData, fIdx);
        ctx.mortonChunkDataBufferAddr = compManager->GetBufferAddr(BufferNames::MortonChunkData, fIdx);
        ctx.mortonChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::MortonChunkVisibileIndex, fIdx);
        ctx.mortonChunkTransformsIndexBufferAddr = compManager->GetBufferAddr(BufferNames::MortonChunkTransformsIndex, fIdx);
        ctx.mortonChunkVisibleIndirectDispatchBufferAddr = drawData->Chunks.mortonChunkVisibleIndirectDispatchBuffer.GetAddress(fIdx);

        ctx.modelAddressBufferAddr = modelManager->GetAddressBufferDeviceAddress();
        ctx.modelBufferAddr = compManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        ctx.modelSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);
		ctx.modelCountBufferAddr = drawData->Models.computeCountBuffer.GetAddress(fIdx);
        ctx.modelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::ModelVisibleData, fIdx);

        ctx.animationAddressBufferAddr = animationManager->GetAddressBufferDeviceAddress();
        ctx.animationBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        ctx.animationSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        
        ctx.materialLookupBufferAddr = drawData->Models.materialIndexBuffer.GetAddress(fIdx);
        ctx.materialBufferAddr = materialManager->GetAddressBufferDeviceAddress();

        //Direction Light Buffers
        ctx.directionLightIndirectCommandBufferAddr = drawData->DirectionLights.indirectBuffer.GetAddress(fIdx);
        ctx.directionLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightVisibleData, fIdx);
        ctx.directionLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightData, fIdx);
        ctx.directionLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightSparseMap, fIdx);
        
        //Direction Light Shadow Buffers
        ctx.directionLightShadowIndirectGeometryCommandBufferAddr = drawData->DirectionLightShadow.indirectBuffer.GetAddress(fIdx);
        ctx.directionLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowSparseMap, fIdx);
        ctx.directionLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowData, fIdx);
		ctx.directionLightShadowColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowColliderData, fIdx);
		ctx.directionLightShadowInstanceBufferAddr = drawData->DirectionLightShadow.instanceBuffer.GetAddress(fIdx);
		ctx.directionLightVisibleShadowIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightVisibleShadowData, fIdx);
		ctx.directionLightShadowModelCountBufferAddr = drawData->DirectionLightShadow.modelDispatchBuffer.GetAddress(fIdx);
		ctx.directionLightShadowModelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowModelVisibleData, fIdx);
		ctx.directionLightShadowChunkCountBufferAddr = drawData->DirectionLightShadow.staticChunkDispatchBuffer.GetAddress(fIdx);
		ctx.directionLightShadowChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowStaticChunkVisibleIndex, fIdx);
        ctx.directionLightShadowMortonChunkCountBufferAddr = drawData->DirectionLightShadow.mortonChunkDispatchBuffer.GetAddress(fIdx);
        ctx.directionLightShadowMortonChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowMortonChunkVisibleIndex, fIdx);

        //Spot Light Buffers
        ctx.spotLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightSparseMap, fIdx);
        ctx.spotLightIndirectCommandBufferAddr = drawData->SpotLights.indirectBuffer.GetAddress(fIdx);
        ctx.spotLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightVisibleData, fIdx);
        ctx.spotLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightData, fIdx);
        ctx.spotLightColliderBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightColliderData, fIdx);

        //Spot Light Shadow Buffers
        ctx.spotLightShadowIndirectGeometryCommandBufferAddr = drawData->SpotLightShadow.indirectBuffer.GetAddress(fIdx);
        ctx.spotLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowSparseMap, fIdx);
        ctx.spotLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowData, fIdx);
        ctx.spotLightShadowInstanceBufferAddr = drawData->SpotLightShadow.instanceBuffer.GetAddress(fIdx);
        ctx.spotLightShadowUnsortedInstanceBufferAddr = drawData->SpotLightShadow.unsortedInstanceBuffer.GetAddress(fIdx);
        ctx.spotLightDrawDescriptorBufferAddr = drawData->SpotLightShadow.descriptorBuffer.GetAddress(fIdx);
        ctx.spotLightVisibleShadowIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowVisibleData, fIdx);       
        ctx.spotLightShadowModelCountBufferAddr = drawData->SpotLightShadow.modelDispatchBuffer.GetAddress(fIdx);
        ctx.spotLightShadowModelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowModelVisibleData, fIdx);
        ctx.spotLightShadowChunkCountBufferAddr = drawData->SpotLightShadow.staticChunkDispatchBuffer.GetAddress(fIdx);
        ctx.spotLightShadowChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowStaticChunkVisibleIndex, fIdx);
        ctx.spotLightShadowMortonChunkCountBufferAddr = drawData->SpotLightShadow.mortonChunkDispatchBuffer.GetAddress(fIdx);
        ctx.spotLightShadowMortonChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowMortonChunkVisibleIndex, fIdx);
        ctx.spotLightShadowGridLookupBufferAddr = drawData->SpotLightShadow.gridLookupBuffer.GetAddress(fIdx);
        ctx.spotLightShadowVisibleCountBufferAddr = drawData->SpotLightShadow.visibleCountDispatchBuffer.GetAddress(fIdx);
        ctx.spotLightShadowDrawCallKeyBufferAddr = drawData->SpotLightShadow.drawCallKeyBuffer.GetAddress(fIdx);
        ctx.spotLightShadowSortValuesBufferAddr = drawData->SpotLightShadow.sortValuesBuffer.GetAddress(fIdx);
        ctx.spotLightShadowVisibleMeshCountBufferAddr = drawData->SpotLightShadow.visibleMeshCountDispatchBuffer.GetAddress(fIdx);
        ctx.spotLightShadowFinalizeDispatchBufferAddr = drawData->SpotLightShadow.finalizeDispatchBuffer.GetAddress(fIdx);
        ctx.spotLightShadowAtlasSortKeyBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowAtlasSortKeyBuffer, fIdx);
        ctx.spotLightShadowAtlasSortValueBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowAtlasSortValueBuffer, fIdx);

        //Point Light Buffers
        ctx.pointLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightSparseMap, fIdx);
        ctx.pointLightIndirectCommandBufferAddr = drawData->PointLights.indirectBuffer.GetAddress(fIdx);
        ctx.pointLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightVisibleData, fIdx);
        ctx.pointLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightData, fIdx);
        ctx.pointLightColliderBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightColliderData, fIdx);

        //Point Light Shadow Buffers
        ctx.pointLightShadowIndirectGeometryCommandBufferAddr = drawData->PointLightShadow.indirectBuffer.GetAddress(fIdx);
        ctx.pointLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowSparseMap, fIdx);
        ctx.pointLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowData, fIdx);
        ctx.pointLightShadowInstanceBufferAddr = drawData->PointLightShadow.instanceBuffer.GetAddress(fIdx);
        ctx.pointLightShadowUnsortedInstanceBufferAddr = drawData->PointLightShadow.unsortedInstanceBuffer.GetAddress(fIdx);
        ctx.pointLightDrawDescriptorBufferAddr = drawData->PointLightShadow.descriptorBuffer.GetAddress(fIdx);
        ctx.pointLightVisibleShadowIndexBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowVisibleData, fIdx);
        ctx.pointLightShadowModelCountBufferAddr = drawData->PointLightShadow.modelDispatchBuffer.GetAddress(fIdx);
        ctx.pointLightShadowModelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowModelVisibleData, fIdx);
        ctx.pointLightShadowChunkCountBufferAddr = drawData->PointLightShadow.staticChunkDispatchBuffer.GetAddress(fIdx);
        ctx.pointLightShadowChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowStaticChunkVisibleIndex, fIdx);
        ctx.pointLightShadowMortonChunkCountBufferAddr = drawData->PointLightShadow.mortonChunkDispatchBuffer.GetAddress(fIdx);
        ctx.pointLightShadowMortonChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowMortonChunkVisibleIndex, fIdx);
        ctx.pointLightShadowGridLookupBufferAddr = drawData->PointLightShadow.gridLookupBuffer.GetAddress(fIdx);
        ctx.pointLightShadowVisibleCountBufferAddr = drawData->PointLightShadow.visibleCountDispatchBuffer.GetAddress(fIdx);
        ctx.pointLightShadowDrawCallKeyBufferAddr = drawData->PointLightShadow.drawCallKeyBuffer.GetAddress(fIdx);
        ctx.pointLightShadowSortValuesBufferAddr = drawData->PointLightShadow.sortValuesBuffer.GetAddress(fIdx);
        ctx.pointLightShadowVisibleMeshCountBufferAddr = drawData->PointLightShadow.visibleMeshCountDispatchBuffer.GetAddress(fIdx);
        ctx.pointLightShadowFinalizeDispatchBufferAddr = drawData->PointLightShadow.finalizeDispatchBuffer.GetAddress(fIdx);
        ctx.pointLightShadowAtlasSortKeyBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowAtlasSortKeyBuffer, fIdx);
        ctx.pointLightShadowAtlasSortValueBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowAtlasSortValueBuffer, fIdx);

        ctx.forwardPlusTileGridListBufferAddr = drawData->ForwardPlus.tileGridBuffer.GetAddress(fIdx);
        ctx.forwardPlusClusterCountBufferAddr = drawData->ForwardPlus.clusterCountBuffer.GetAddress(fIdx);
        ctx.forwardPlusClusterListBufferAddr = drawData->ForwardPlus.clusterListBuffer.GetAddress(fIdx);
        ctx.forwardPlusPointLightIndexListBufferAddr = drawData->ForwardPlus.pointLightIndexBuffer.GetAddress(fIdx);
        ctx.forwardPlusSpotLightIndexListBufferAddr = drawData->ForwardPlus.spotLightIndexBuffer.GetAddress(fIdx);

        ctx.wireframeMeshAabbIndirectCommandBufferAddr = drawData->Debug.modelAabbIndirectBuffer.GetAddress(fIdx);
        ctx.wireframeMeshSphereIndirectCommandBufferAddr = drawData->Debug.modelSphereIndirectBuffer.GetAddress(fIdx);

		ctx.ssaoKernelBufferAddr = drawData->Ssao.kernelBuffer.GetAddress(fIdx);

        ctx.hierarchySparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::HierarchySparseMap, fIdx);
        ctx.selectionOutlineBufferAddr = compManager->GetBufferAddr(BufferNames::SelectionOutlineData, fIdx);

        ctx.boxColliderSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::BoxColliderSparseMap, fIdx);
        ctx.boxColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::BoxColliderData, fIdx);
        ctx.sphereColliderSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SphereColliderSparseMap, fIdx);
        ctx.sphereColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::SphereColliderData, fIdx);
        ctx.capsuleColliderSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CapsuleColliderSparseMap, fIdx);
        ctx.capsuleColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::CapsuleColliderData, fIdx);

		ctx.enableSsao = settings->postProcess.enableSsao ? 1 : 0;
		ctx.enableSsaoLight = settings->postProcess.enableSsaoLight ? 1 : 0;

        ctx.screenWidth = static_cast<float>(rtGroup->GetWidth());
        ctx.screenHeight = static_cast<float>(rtGroup->GetHeight());
        ctx.ambientStrength = settings->lighting.ambientStrength;
        ctx.emissiveStrength = settings->lighting.emissiveStrength;
        ctx.alphaLimitDiscard = 0.025f;

        ctx.activeDirectionLightShadowCount = drawData->DirectionLightShadow.visibleLightCount;
		ctx.directionLightShadowLodBias = SHADOW_LOD_BIAS;
		ctx.directionLightShadowMaxDirLights = MAX_DIR_LIGHTS;
		ctx.directionLightShadowMaxCascades = CASCADES_PER_LIGHT;
		ctx.directionLightShadowMultiplier = SHADOW_MULTIPLIER;
		ctx.directionLightShadowAtlasSize = SHADOW_ATLAS_SIZE;
		ctx.directionLightShadowMinBlockSize = SHADOW_MIN_BLOCK_SIZE;
		ctx.directionLightShadowGridSize = SHADOW_GRID_SIZE;
        ctx.directionLightShadowHizMipLevels = SHADOW_HIZ_MIP_LEVELS;

        ctx.spotLightShadowLodBias = SPOT_SHADOW_LOD_BIAS;
        ctx.spotLightShadowMultiplier = SPOT_SHADOW_MULTIPLIER;
        ctx.spotLightShadowAtlasSize = SPOT_SHADOW_ATLAS_SIZE;
        ctx.spotLightShadowMinBlockSize = SPOT_SHADOW_MIN_BLOCK_SIZE;
        ctx.spotLightShadowGridSize = SPOT_SHADOW_GRID_SIZE;
        ctx.spotLightShadowHizMipLevels = SPOT_SHADOW_HIZ_MIP_LEVELS;

        ctx.pointLightShadowLodBias = POINT_SHADOW_LOD_BIAS;
        ctx.pointLightShadowMultiplier = POINT_SHADOW_MULTIPLIER;
        ctx.pointLightShadowAtlasSize = POINT_SHADOW_ATLAS_SIZE;
        ctx.pointLightShadowMinBlockSize = POINT_SHADOW_MIN_BLOCK_SIZE;
        ctx.pointLightShadowGridSize = POINT_SHADOW_GRID_SIZE;
        ctx.pointLightShadowHizMipLevels = POINT_SHADOW_HIZ_MIP_LEVELS;

        ctx.enableMeshletConeCulling = settings->culling.enableMeshletConeCulling ? 1 : 0;

        ctx.enableChunkFrustumCulling = settings->culling.enableFrustumCulling && settings->culling.enableChunkFrustumCulling ? 1 : 0;
        ctx.enableModelFrustumCulling = settings->culling.enableFrustumCulling && settings->culling.enableModelFrustumCulling ? 1 : 0;
        ctx.enableMeshFrustumCulling = settings->culling.enableFrustumCulling && settings->culling.enableMeshFrustumCulling ? 1 : 0;
        ctx.enableMeshletFrustumCulling = settings->culling.enableFrustumCulling && settings->culling.enableMeshletFrustumCulling ? 1 : 0;
        ctx.enablePointLightFrustumCulling = settings->culling.enableFrustumCulling && settings->culling.enablePointLightFrustumCulling ? 1 : 0;
        ctx.enableSpotLightFrustumCulling = settings->culling.enableFrustumCulling && settings->culling.enableSpotLightFrustumCulling ? 1 : 0;

        ctx.enableChunkOcclusionCulling = settings->culling.enableOcclusionCulling && settings->culling.enableChunkOcclusionCulling ? 1 : 0;
        ctx.enableModelOcclusionCulling = settings->culling.enableOcclusionCulling && settings->culling.enableModelOcclusionCulling ? 1 : 0;
        ctx.enableMeshOcclusionCulling = settings->culling.enableOcclusionCulling && settings->culling.enableMeshOcclusionCulling ? 1 : 0;
        ctx.enableMeshletOcclusionCulling = settings->culling.enableOcclusionCulling && settings->culling.enableMeshletOcclusionCulling ? 1 : 0;
        ctx.enablePointLightOcclusionCulling = settings->culling.enableFrustumCulling && settings->culling.enablePointLightOcclusionCulling ? 1 : 0;
        ctx.enableSpotLightOcclusionCulling = settings->culling.enableFrustumCulling && settings->culling.enableSpotLightOcclusionCulling ? 1 : 0;

        ctx.enableForwardPlusEmissiveAo = settings->lighting.enableForwardPlusEmissiveAo ? 1 : 0;
        ctx.enableForwardPlusPointLights = settings->lighting.enableForwardPlusPointLights ? 1 : 0;
        ctx.enableForwardPlusSpotLights = settings->lighting.enableForwardPlusSpotLights ? 1 : 0;
        ctx.enableForwardPlusDirectionalLights = settings->lighting.enableForwardPlusDirectionalLights ? 1 : 0;

        ctx.globalIndirectCommandCount = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
        ctx.globalTraditionalCommandsCount = drawData->Models.activeTraditionalCount;
        ctx.globalMeshletCommandsCount = drawData->Models.activeMeshletCount;

        ctx.mainCameraEntity = scene->GetSceneCameraEntity();
        ctx.activeCameraEntity = settings->debug.useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();

		auto [modelPool, directionLightPool, pointLightPool, spotLightPool, cameraPool, transformPool] = scene->GetRegistry()->GetPools<ModelComponent, DirectionLightComponent, PointLightComponent, SpotLightComponent, CameraComponent, TransformComponent>();

		ctx.staticChunkCount = drawData->Chunks.chunkCounter.load(std::memory_order_relaxed);
        ctx.modelCount = static_cast<uint32_t>(modelPool->Size());
        ctx.directionLightCount = static_cast<uint32_t>(directionLightPool->Size());
        ctx.pointLightCount = static_cast<uint32_t>(pointLightPool->Size());
        ctx.spotLightCount = static_cast<uint32_t>(spotLightPool->Size());

        ctx.enableGeometryBvhCulling = !(settings->culling.geometrySpatialAcceleration == SpatialAccelerationType::None) ? 1 : 0;
        ctx.enableDirectionLightBvhCulling = !(settings->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::None) ? 1 : 0;
        ctx.enableSpotLightBvhCulling = !(settings->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::None) ? 1 : 0;
        ctx.enablePointLightBvhCulling = !(settings->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::None) ? 1 : 0;

        ctx.allTransformCount = static_cast<uint32_t>(transformPool->Size());
        ctx.staticTransformCount = static_cast<uint32_t>(transformPool->GetStaticEntities().size());
        ctx.dynamicTransformCount = static_cast<uint32_t>(transformPool->GetDynamicEntities().size());
        ctx.streamTransformCount = static_cast<uint32_t>(transformPool->GetStreamEntities().size());
        ctx.nonStaticTransformCount = ctx.allTransformCount - ctx.staticTransformCount;

        ctx.tileSize = settings->lighting.tileSize;
        ctx.tileCountX = ComputeGroupSize::CalculateDispatchCount(rtGroup->GetWidth(), ctx.tileSize);
        ctx.tileCountY = ComputeGroupSize::CalculateDispatchCount(rtGroup->GetHeight(), ctx.tileSize);
        ctx.hizMipLevel = std::log2(static_cast<float>(ctx.tileSize));

        float tanHalfFov = std::tan(glm::radians(cameraPool->Get(ctx.mainCameraEntity).fov) * 0.5f);
        ctx.sliceScaleFactor = 1.0f / std::log2(1.0f + (2.0f * tanHalfFov / static_cast<float>(ctx.tileCountY)));

        drawData->frameContextBuffer.Write(fIdx , &ctx, sizeof(FrameGlobalContext), 0);

        //Todo: Kiszervezni lambdába innen!
		drawData->CoherentToGpuBufferSync(context.cmd, fIdx);
        ServiceLocator::GetAnimationManager()->RecordSync(context.cmd);
        ServiceLocator::GetModelManager()->RecordSync(context.cmd);
        ServiceLocator::GetMaterialManager()->RecordSync(context.cmd);
    }
}