#include "ClusterSpotLightCountPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn {
    #include "Engine/Shaders/Includes/PushConstants/ClusterLightCountPC.glsl"

  
    bool ClusterSpotLightCountPass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->enableForwardPlus;
    }

    void ClusterSpotLightCountPass::Initialize() {
        _shaderProgram = ServiceLocator::GetShaderManager()->CreateProgram(
            "ClusterSpotLightCountProgram",
            { ShaderNames::ClusterSpotLightCount },
            { .useDescriptorBuffers = false }
        );
    }

    void ClusterSpotLightCountPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        ClusterLightCountPC pc{};
        pc.clusterCountAddr = drawData->ForwardPlus.clusterCountBuffer.GetAddress(fIdx, true);
        pc.clusterListAddr = drawData->ForwardPlus.clusterListBuffer.GetAddress(fIdx, true);

        pc.lightIndirectCmdAddr = drawData->SpotLights.indirectBuffer.GetAddress(fIdx, isGpu);
        pc.visibleLightAddr = compManager->GetBufferAddr(BufferNames::SpotLightVisibleData, fIdx);
        pc.lightColliderDataAddr = compManager->GetBufferAddr(BufferNames::SpotLightColliderData, fIdx);

        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.activeCameraEntity = scene->GetSceneCameraEntity();

        pc.screenWidth = static_cast<float>(rtGroup->GetWidth());
        pc.screenHeight = static_cast<float>(rtGroup->GetHeight());
        pc.tileSize = drawData->ForwardPlus.tileSize;
        pc.tileCountX = ComputeGroupSize::CalculateDispatchCount(rtGroup->GetWidth(), pc.tileSize);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ClusterLightCountPC), &pc);
    }

    void ClusterSpotLightCountPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        vkCmdDispatch(context.cmd, drawData->ForwardPlus.maxClusters, 1, 1);
    }
}