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
        bool isGpu = settings->enableGeometryGpuCulling;

        drawData->ForwardPlus.CheckResize(settings->tileSize, width, height, fIdx);

        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        FrameGlobalContext ctx = {};

        ctx.globalDrawCountBufferAddr = drawData->Models.drawCountBuffer.GetAddress(fIdx, isGpu);
        ctx.globalInstanceIndexBufferAddr = drawData->Models.instanceBuffer.GetAddress(fIdx, isGpu);
        ctx.globalIndirectCommandBufferAddr = drawData->Models.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.globalIndirectCommandDescriptorBufferAddr = drawData->Models.descriptorBuffer.GetAddress(fIdx, isGpu);
        ctx.globalModelAllocationBufferAddr = drawData->Models.modelAllocBuffer.GetAddress(fIdx, isGpu);
        ctx.globalMeshAllocationBufferAddr = drawData->Models.meshAllocBuffer.GetAddress(fIdx, isGpu);

        ctx.cameraVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::CameraVisibleData, fIdx);
        ctx.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        ctx.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);

        ctx.transformBufferAddr = compManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        ctx.transformSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
		ctx.transformModelLinkBufferAddr = compManager->GetBufferAddr(BufferNames::TransformModelLinkData, fIdx);

		ctx.staticChunkDataBufferAddr = drawData->Chunks.chunkDataBuffer.GetAddress(fIdx, isGpu);
        ctx.staticChunkVisibleIndexBufferAddr = drawData->Chunks.chunkVisibilityBuffer.GetAddress(fIdx, isGpu);
        ctx.staticChunkCountBufferAddr = drawData->Chunks.chunkIndirectDispatchBuffer.GetAddress(fIdx, isGpu);

        ctx.sceneAabbBufferAddr = drawData->Chunks.sceneAabbBuffer.GetAddress(fIdx, isGpu);
        ctx.mortonChunkIndirectDispatchBufferAddr = drawData->Chunks.mortonIndirectDispatchBuffer.GetAddress(fIdx, isGpu);
        ctx.mortonChunkIndirectDrawBufferAddr = drawData->Chunks.mortonIndirectDrawBuffer.GetAddress(fIdx, isGpu);
        ctx.mortonKeysBufferAddr = compManager->GetBufferAddr(BufferNames::MortonKeysData, fIdx);
        ctx.mortonValuesBufferAddr = compManager->GetBufferAddr(BufferNames::MortonValuesData, fIdx);
        ctx.mortonChunkDataBufferAddr = compManager->GetBufferAddr(BufferNames::MortonChunkData, fIdx);
        ctx.mortonChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::MortonChunkVisibileIndex, fIdx);
        ctx.mortonChunkTransformsIndexBufferAddr = compManager->GetBufferAddr(BufferNames::MortonChunkTransformsIndex, fIdx);
        ctx.mortonChunkVisibleIndirectDispatchBufferAddr = drawData->Chunks.mortonChunkVisibleIndirectDispatchBuffer.GetAddress(fIdx, isGpu);

        ctx.modelAddressBufferAddr = modelManager->GetAddressBuffer()->GetDeviceAddress();
        ctx.modelBufferAddr = compManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        ctx.modelSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);
		ctx.modelCountBufferAddr = drawData->Models.computeCountBuffer.GetAddress(fIdx, isGpu);
        ctx.modelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::ModelVisibleData, fIdx);

        ctx.animationAddressBufferAddr = animationManager->GetAddressBuffer()->GetDeviceAddress();
        ctx.animationBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        ctx.animationSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        
        ctx.materialLookupBufferAddr = drawData->Models.materialIndexBuffer.GetAddress(fIdx, isGpu);
        ctx.materialBufferAddr = materialManager->GetAddressBuffer()->GetDeviceAddress();

        //Direction Light Buffers
        ctx.directionLightIndirectCommandBufferAddr = drawData->DirectionLights.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.directionLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightVisibleData, fIdx);
        ctx.directionLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightData, fIdx);
        ctx.directionLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightSparseMap, fIdx);
        
        //Direction Light Shadow Buffers
        ctx.directionLightShadowIndirectGeometryCommandBufferAddr = drawData->DirectionLightShadow.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.directionLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowSparseMap, fIdx);
        ctx.directionLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowData, fIdx);
		ctx.directionLightShadowColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowColliderData, fIdx);
		ctx.directionLightShadowInstanceBufferAddr = drawData->DirectionLightShadow.instanceBuffer.GetAddress(fIdx, isGpu);
		ctx.directionLightVisibleShadowIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightVisibleShadowData, fIdx);
		ctx.directionLightShadowModelCountBufferAddr = drawData->DirectionLightShadow.modelDispatchBuffer.GetAddress(fIdx, isGpu);
		ctx.directionLightShadowModelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowModelVisibleData, fIdx);
		ctx.directionLightShadowChunkCountBufferAddr = drawData->DirectionLightShadow.staticChunkDispatchBuffer.GetAddress(fIdx, isGpu);
		ctx.directionLightShadowChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowStaticChunkVisibleIndex, fIdx);
        ctx.directionLightShadowMortonChunkCountBufferAddr = drawData->DirectionLightShadow.mortonChunkDispatchBuffer.GetAddress(fIdx, isGpu);
        ctx.directionLightShadowMortonChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowMortonChunkVisibleIndex, fIdx);

        //Spot Light Buffers
        ctx.spotLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightSparseMap, fIdx);
        ctx.spotLightIndirectCommandBufferAddr = drawData->SpotLights.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightVisibleData, fIdx);
        ctx.spotLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightData, fIdx);
        ctx.spotLightColliderBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightColliderData, fIdx);

        //Spot Light Shadow Buffers
        ctx.spotLightShadowIndirectGeometryCommandBufferAddr = drawData->SpotLightShadow.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowSparseMap, fIdx);
        ctx.spotLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowData, fIdx);
        ctx.spotLightShadowInstanceBufferAddr = drawData->SpotLightShadow.instanceBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightDrawDescriptorBufferAddr = drawData->SpotLightShadow.descriptorBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightVisibleShadowIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowVisibleData, fIdx);       
        ctx.spotLightShadowModelCountBufferAddr = drawData->SpotLightShadow.modelDispatchBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightShadowModelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowModelVisibleData, fIdx);
        ctx.spotLightShadowChunkCountBufferAddr = drawData->SpotLightShadow.staticChunkDispatchBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightShadowChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowStaticChunkVisibleIndex, fIdx);
        ctx.spotLightShadowMortonChunkCountBufferAddr = drawData->SpotLightShadow.mortonChunkDispatchBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightShadowMortonChunkVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowMortonChunkVisibleIndex, fIdx);


        ctx.pointLightIndirectCommandBufferAddr = drawData->PointLights.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.pointLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightVisibleData, fIdx);
        ctx.pointLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightData, fIdx);
        ctx.pointLightColliderBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightColliderData, fIdx);
        ctx.pointLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightSparseMap, fIdx);
        ctx.pointLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowSparseMap, fIdx);
        ctx.pointLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowData, fIdx);


        ctx.forwardPlusTileGridListBufferAddr = drawData->ForwardPlus.tileGridBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusClusterCountBufferAddr = drawData->ForwardPlus.clusterCountBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusClusterListBufferAddr = drawData->ForwardPlus.clusterListBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusPointLightIndexListBufferAddr = drawData->ForwardPlus.pointLightIndexBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusSpotLightIndexListBufferAddr = drawData->ForwardPlus.spotLightIndexBuffer.GetAddress(fIdx, true);

        ctx.wireframeMeshAabbIndirectCommandBufferAddr = drawData->Debug.modelAabbIndirectBuffer.GetAddress(fIdx, true);
        ctx.wireframeMeshSphereIndirectCommandBufferAddr = drawData->Debug.modelSphereIndirectBuffer.GetAddress(fIdx, true);

		ctx.ssaoKernelBufferAddr = drawData->Ssao.kernelBuffer.GetAddress(fIdx, true);

        ctx.hierarchySparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::HierarchySparseMap, fIdx);
        ctx.selectionOutlineBufferAddr = compManager->GetBufferAddr(BufferNames::SelectionOutlineData, fIdx);

        ctx.boxColliderSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::BoxColliderSparseMap, fIdx);
        ctx.boxColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::BoxColliderData, fIdx);
        ctx.sphereColliderSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SphereColliderSparseMap, fIdx);
        ctx.sphereColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::SphereColliderData, fIdx);
        ctx.capsuleColliderSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CapsuleColliderSparseMap, fIdx);
        ctx.capsuleColliderDataBufferAddr = compManager->GetBufferAddr(BufferNames::CapsuleColliderData, fIdx);

		ctx.enableSsao = settings->enableSsao ? 1 : 0;
		ctx.enableSsaoLight = settings->enableSsaoLight ? 1 : 0;

        ctx.screenWidth = static_cast<float>(rtGroup->GetWidth());
        ctx.screenHeight = static_cast<float>(rtGroup->GetHeight());
        ctx.ambientStrength = settings->ambientStrength;
        ctx.emissiveStrength = settings->emissiveStrength;
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

        ctx.enableMeshletConeCulling = settings->enableMeshletConeCulling ? 1 : 0;

        ctx.enableChunkFrustumCulling = settings->enableFrustumCulling && settings->enableChunkFrustumCulling ? 1 : 0;
        ctx.enableModelFrustumCulling = settings->enableFrustumCulling && settings->enableModelFrustumCulling ? 1 : 0;
        ctx.enableMeshFrustumCulling = settings->enableFrustumCulling && settings->enableMeshFrustumCulling ? 1 : 0;
        ctx.enableMeshletFrustumCulling = settings->enableFrustumCulling && settings->enableMeshletFrustumCulling ? 1 : 0;
        ctx.enablePointLightFrustumCulling = settings->enableFrustumCulling && settings->enablePointLightFrustumCulling ? 1 : 0;
        ctx.enableSpotLightFrustumCulling = settings->enableFrustumCulling && settings->enableSpotLightFrustumCulling ? 1 : 0;

        ctx.enableChunkOcclusionCulling = settings->enableOcclusionCulling && settings->enableChunkOcclusionCulling ? 1 : 0;
        ctx.enableModelOcclusionCulling = settings->enableOcclusionCulling && settings->enableModelOcclusionCulling ? 1 : 0;
        ctx.enableMeshOcclusionCulling = settings->enableOcclusionCulling && settings->enableMeshOcclusionCulling ? 1 : 0;
        ctx.enableMeshletOcclusionCulling = settings->enableOcclusionCulling && settings->enableMeshletOcclusionCulling ? 1 : 0;
        ctx.enablePointLightOcclusionCulling = settings->enableFrustumCulling && settings->enablePointLightOcclusionCulling ? 1 : 0;
        ctx.enableSpotLightOcclusionCulling = settings->enableFrustumCulling && settings->enableSpotLightOcclusionCulling ? 1 : 0;

        ctx.enableForwardPlusEmissiveAo = scene->GetSettings()->enableForwardPlusEmissiveAo ? 1 : 0;
        ctx.enableForwardPlusPointLights = scene->GetSettings()->enableForwardPlusPointLights ? 1 : 0;
        ctx.enableForwardPlusSpotLights = scene->GetSettings()->enableForwardPlusSpotLights ? 1 : 0;
        ctx.enableForwardPlusDirectionalLights = scene->GetSettings()->enableForwardPlusDirectionalLights ? 1 : 0;

        ctx.globalIndirectCommandCount = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
        ctx.globalTraditionalCommandsCount = drawData->Models.activeTraditionalCount;
        ctx.globalMeshletCommandsCount = drawData->Models.activeMeshletCount;

        ctx.mainCameraEntity = scene->GetSceneCameraEntity();
        ctx.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();

		auto [modelPool, directionLightPool, pointLightPool, spotLightPool, cameraPool, transformPool] = scene->GetRegistry()->GetPools<ModelComponent, DirectionLightComponent, PointLightComponent, SpotLightComponent, CameraComponent, TransformComponent>();

		ctx.staticChunkCount = drawData->Chunks.chunkCounter.load(std::memory_order_relaxed);
        ctx.modelCount = static_cast<uint32_t>(modelPool->Size());
        ctx.directionLightCount = static_cast<uint32_t>(directionLightPool->Size());
        ctx.pointLightCount = static_cast<uint32_t>(pointLightPool->Size());
        ctx.spotLightCount = static_cast<uint32_t>(spotLightPool->Size());

        ctx.enableStaticBvhCulling = scene->GetSettings()->enableStaticBvhCulling || context.scene->GetSettings()->enableMortonBvhCulling ? 1 : 0;
        ctx.allTransformCount = static_cast<uint32_t>(transformPool->Size());
        ctx.staticTransformCount = static_cast<uint32_t>(transformPool->GetStaticEntities().size());
        ctx.dynamicTransformCount = static_cast<uint32_t>(transformPool->GetDynamicEntities().size());
        ctx.streamTransformCount = static_cast<uint32_t>(transformPool->GetStreamEntities().size());
        ctx.nonStaticTransformCount = ctx.allTransformCount - ctx.staticTransformCount;

        ctx.tileSize = scene->GetSettings()->tileSize;
        ctx.tileCountX = ComputeGroupSize::CalculateDispatchCount(rtGroup->GetWidth(), ctx.tileSize);
        ctx.tileCountY = ComputeGroupSize::CalculateDispatchCount(rtGroup->GetHeight(), ctx.tileSize);
        ctx.hizMipLevel = std::log2(static_cast<float>(ctx.tileSize));

        float tanHalfFov = std::tan(glm::radians(cameraPool->Get(ctx.mainCameraEntity).fov) * 0.5f);
        ctx.sliceScaleFactor = 1.0f / std::log2(1.0f + (2.0f * tanHalfFov / static_cast<float>(ctx.tileCountY)));

        if (auto mappedFrameContext = drawData->frameContextBuffer.GetMapped(fIdx)) {
            mappedFrameContext->Write(&ctx, sizeof(FrameGlobalContext), 0);
        }

		drawData->CoherentToGpuBufferSync(context.cmd, fIdx);
    }
}