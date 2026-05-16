#include "ClusterDispatchSetupPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"

namespace Syn {
    #include "Engine/Shaders/Includes/PushConstants/ClusterDispatchSetupPC.glsl"

    void ClusterDispatchSetupPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("ClusterDispatchSetupProgram", {
            ShaderNames::ClusterDispatchSetup
            }, config);
    }

    void ClusterDispatchSetupPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        ClusterDispatchSetupPC pc{};
        pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, true);
        pc.dispatchArgsBufferAddr = drawData->ForwardPlus.dispatchArgsBuffer.GetAddress(fIdx, true);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ClusterDispatchSetupPC), &pc);
    }

    void ClusterDispatchSetupPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        vkCmdDispatch(context.cmd, 1, 1, 1);

        Vk::BufferBarrierInfo barrier{};
        barrier.buffer = drawData->ForwardPlus.dispatchArgsBuffer.GetHandle(fIdx, true);
        barrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        barrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        barrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, barrier);
    }
}