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

namespace Syn {

    #include "Engine/Shaders/Includes/Common/FrameGlobalContext.glsl"

    void GlobalFrameSetupPass::Transfer(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGpuCulling;

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

        ctx.modelAddressBufferAddr = modelManager->GetModelAddressBuffer()->GetDeviceAddress();
        ctx.modelBufferAddr = compManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        ctx.modelSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);
		ctx.modelCountBufferAddr = drawData->Models.computeCountBuffer.GetAddress(fIdx, isGpu);
        ctx.modelVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::ModelVisibleData, fIdx);

        ctx.animationAddressBufferAddr = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        ctx.animationBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        ctx.animationSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        
        ctx.materialLookupBufferAddr = drawData->Models.materialIndexBuffer.GetAddress(fIdx, isGpu);
        ctx.materialBufferAddr = materialManager->GetMaterialBuffer()->GetDeviceAddress();

        ctx.directionLightIndirectCommandBufferAddr = drawData->DirectionLights.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.directionLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightVisibleData, fIdx);
        ctx.directionLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightData, fIdx);
        ctx.directionLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightSparseMap, fIdx);
        ctx.directionLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowSparseMap, fIdx);
        ctx.directionLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::DirectionLightShadowData, fIdx);

        ctx.pointLightIndirectCommandBufferAddr = drawData->PointLights.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.pointLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightVisibleData, fIdx);
        ctx.pointLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightData, fIdx);
        ctx.pointLightColliderBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightColliderData, fIdx);
        ctx.pointLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightSparseMap, fIdx);
        ctx.pointLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowSparseMap, fIdx);
        ctx.pointLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::PointLightShadowData, fIdx);

        ctx.spotLightIndirectCommandBufferAddr = drawData->SpotLights.indirectBuffer.GetAddress(fIdx, isGpu);
        ctx.spotLightVisibleIndexBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightVisibleData, fIdx);
        ctx.spotLightDataBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightData, fIdx);
        ctx.spotLightColliderBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightColliderData, fIdx);
        ctx.spotLightSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightSparseMap, fIdx);
        ctx.spotLightShadowSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowSparseMap, fIdx);
        ctx.spotLightShadowDataBufferAddr = compManager->GetBufferAddr(BufferNames::SpotLightShadowData, fIdx);

        ctx.forwardPlusTileGridListBufferAddr = drawData->ForwardPlus.tileGridBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusClusterCountBufferAddr = drawData->ForwardPlus.clusterCountBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusClusterListBufferAddr = drawData->ForwardPlus.clusterListBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusPointLightIndexListBufferAddr = drawData->ForwardPlus.pointLightIndexBuffer.GetAddress(fIdx, true);
        ctx.forwardPlusSpotLightIndexListBufferAddr = drawData->ForwardPlus.spotLightIndexBuffer.GetAddress(fIdx, true);

        ctx.wireframeMeshletInstanceIndexBufferAddr = drawData->Debug.instanceBuffer.GetAddress(fIdx, true);
        ctx.wireframeMeshAabbIndirectCommandBufferAddr = drawData->Debug.meshletAabbIndirectBuffer.GetAddress(fIdx, true);
        ctx.wireframeMeshSphereIndirectCommandBufferAddr = drawData->Debug.meshletSphereIndirectBuffer.GetAddress(fIdx, true);

        ctx.screenWidth = static_cast<float>(rtGroup->GetWidth());
        ctx.screenHeight = static_cast<float>(rtGroup->GetHeight());
        ctx.ambientStrength = scene->GetSettings()->ambientStrength;
        ctx.emissiveStrength = scene->GetSettings()->emissiveStrength;
        ctx.alphaLimitDiscard = 0.025f;

        ctx.enableConeCulling = scene->GetSettings()->enableConeCulling ? 1 : 0;
        ctx.enableFrustumCulling = scene->GetSettings()->enableFrustumCulling ? 1 : 0;
        ctx.enableOcclusionCulling = scene->GetSettings()->enableOcclusionCulling ? 1 : 0;

        ctx.globalIndirectCommandCount = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
        ctx.globalTraditionalCommandsCount = drawData->Models.activeTraditionalCount;
        ctx.globalMeshletCommandsCount = drawData->Models.activeMeshletCount;

        ctx.mainCameraEntity = scene->GetSceneCameraEntity();
        ctx.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();

		auto [modelPool, directionLightPool, pointLightPool, spotLightPool, cameraPool] = scene->GetRegistry()->GetPools<ModelComponent, DirectionLightComponent, PointLightComponent, SpotLightComponent, CameraComponent>();

        ctx.modelCount = modelPool->Size();
        ctx.directionLightCount = directionLightPool->Size();
        ctx.pointLightCount = pointLightPool->Size();
        ctx.spotLightCount = spotLightPool->Size();

        ctx.tileSize = drawData->ForwardPlus.tileSize;
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