#include "WireframeSetupPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Scene/SceneDrawData.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/WireframeSetupPC.glsl"

    void WireframeSetupPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("WireframeSetupProgram", {
            ShaderNames::WireframeSetup
            });
    }

    void WireframeSetupPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) {
            _shouldDispatch = false;
            return;
        }

        auto drawData = scene->GetSceneDrawData();
        uint32_t totalCommands = drawData->activeTraditionalCount + drawData->activeMeshletCount;
        if (totalCommands == 0) {
            _shouldDispatch = false;
            return;
        }

        _shouldDispatch = true;
        uint32_t fIdx = context.frameIndex;

        WireframeSetupPC pc{};
        pc.globalIndirectCmdsAddr = drawData->globalIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.aabbCmdsAddr = drawData->aabbIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.sphereCmdsAddr = drawData->sphereIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.totalTraditionalCommands = drawData->activeTraditionalCount;
        pc.totalCommands = totalCommands;

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(WireframeSetupPC), &pc);
    }

    void WireframeSetupPass::Dispatch(const RenderContext& context) {
        if (!_shouldDispatch) return;

        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        uint32_t totalCommands = drawData->activeTraditionalCount + drawData->activeMeshletCount;
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(totalCommands, ComputeGroupSize::Buffer256D);

        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo aabbBarrier{};
        aabbBarrier.buffer = drawData->aabbIndirectCommandBuffers[fIdx]->Handle();
        aabbBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        aabbBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        aabbBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        aabbBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, aabbBarrier);

        Vk::BufferBarrierInfo sphereBarrier{};
        sphereBarrier.buffer = drawData->sphereIndirectCommandBuffers[fIdx]->Handle();
        sphereBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sphereBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sphereBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        sphereBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sphereBarrier);
    }
}