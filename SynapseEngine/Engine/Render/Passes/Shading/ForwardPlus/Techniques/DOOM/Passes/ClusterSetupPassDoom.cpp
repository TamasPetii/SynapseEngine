#include "ClusterSetupPassDoom.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {
#include "Engine/Shaders/Includes/PushConstants/ClusterSetupPC.glsl"

    void ClusterSetupPassDoom::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("ClusterSetupProgramDoom", {
            ShaderNames::ClusterSetupDOOM
            }, config);
    }

    void ClusterSetupPassDoom::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ClusterSetupPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void ClusterSetupPassDoom::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, scene->GetSettings()->lighting.tileSize);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, scene->GetSettings()->lighting.tileSize);

        VkDispatchIndirectCommand resetCmd{ 0, 1, 1 };

        Vk::BufferUpdateInfo updateInfo{};
        updateInfo.buffer = drawData->ForwardPlus.clusterCountBuffer.GetHandle(fIdx);
        updateInfo.offset = 0;
        updateInfo.size = sizeof(VkDispatchIndirectCommand);
        updateInfo.pData = &resetCmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateInfo);

        Vk::BufferBarrierInfo updateBarrier{};
        updateBarrier.buffer = updateInfo.buffer;
        updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        updateBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        updateBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        Vk::BufferBarrierInfo tileGridBarrier{};
        tileGridBarrier.buffer = drawData->ForwardPlus.tileGridBuffer.GetHandle(fIdx);
        tileGridBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        tileGridBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        tileGridBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        tileGridBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, tileGridBarrier);

        Vk::BufferBarrierInfo clusterListBarrier{};
        clusterListBarrier.buffer = drawData->ForwardPlus.clusterListBuffer.GetHandle(fIdx);
        clusterListBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        clusterListBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        clusterListBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        clusterListBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, clusterListBarrier);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->ForwardPlus.clusterCountBuffer.GetHandle(fIdx);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);
    }
}