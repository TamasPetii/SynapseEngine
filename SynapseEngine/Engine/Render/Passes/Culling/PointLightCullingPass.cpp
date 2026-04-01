#include "PointLightCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/PointLightComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/PointLightCullingPC.glsl"

    void PointLightCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("PointLightCullingProgram", {
            ShaderNames::PointLightCulling
            }, config);
    }

    void PointLightCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        auto registry = scene->GetRegistry();
        auto pointLightPool = registry->GetPool<PointLightComponent>();
        _totalLightsToTest = pointLightPool ? static_cast<uint32_t>(pointLightPool->Size()) : 0;

        if (_totalLightsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        PointLightCullingPC pc{};
        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.pointLightColliderDataAddr = compManager->GetBufferAddr(BufferNames::PointLightColliderData, fIdx);
        pc.visibleLightAddr = compManager->GetBufferAddr(BufferNames::PointLightVisibleData, fIdx);
        pc.indirectCommandAddr = drawData->pointLightIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.totalLightsToTest = _totalLightsToTest;
        pc.activeCameraEntity = scene->GetSceneCameraEntity();

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(PointLightCullingPC), &pc);
    }

    void PointLightCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene || !scene->GetSceneDrawData()->useGpuCulling || _totalLightsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalLightsToTest, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo cmdBarrier{};
        cmdBarrier.buffer = drawData->pointLightIndirectCommandBuffers[fIdx]->Handle();
        cmdBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        cmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        cmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, cmdBarrier);

        Vk::BufferBarrierInfo visibleDataBarrier{};
        visibleDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::PointLightVisibleData, fIdx).buffer->Handle();
        visibleDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleDataBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        visibleDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleDataBarrier);
    }
}