#include "CullingCommandResetPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/CullingCommandResetPC.glsl"

    bool CullingCommandResetPass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->enableGpuCulling;
    }

    void CullingCommandResetPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("CommandResetProgram", {
            ShaderNames::CullingCommandReset
            }, config);
    }

    void CullingCommandResetPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        _totalCommands = drawData->activeTraditionalCount + drawData->activeMeshletCount;

        CullingCommandResetPC pc{};
        pc.globalIndirectCmdsAddr = drawData->gpuIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.modelComputeCountAddr = drawData->gpuModelComputeCountBuffers[fIdx]->GetDeviceAddress();
        pc.totalTraditionalCommands = drawData->activeTraditionalCount;
        pc.totalCommands = _totalCommands;

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(),VK_SHADER_STAGE_ALL, 0, sizeof(CullingCommandResetPC), &pc);
    }

    void CullingCommandResetPass::Dispatch(const RenderContext& context) {
        uint32_t dispatchCount = std::max(1u, ComputeGroupSize::CalculateDispatchCount(_totalCommands, ComputeGroupSize::Buffer256D));

        vkCmdDispatch(context.cmd, dispatchCount, 1, 1);

        Vk::GlobalBarrierInfo barrier{};
        barrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        barrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        barrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertGlobalBarrier(context.cmd, barrier);
    }
}