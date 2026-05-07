#include "ClusterPrefixSumPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn 
{
    #include "Engine/Shaders/Includes/PushConstants/ClusterPrefixSumPC.glsl"

    bool ClusterPrefixSumPass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->enableForwardPlus;
    }

    void ClusterPrefixSumPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("ClusterPrefixSumProgram", {
            ShaderNames::ClusterPrefixSum
            }, config);
    }

    void ClusterPrefixSumPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        ClusterPrefixSumPC pc{};
		pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, true);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ClusterPrefixSumPC), &pc);
    }

    void ClusterPrefixSumPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();

        Vk::BufferBarrierInfo listBarrier{};
        listBarrier.buffer = drawData->ForwardPlus.clusterListBuffer.GetHandle(context.frameIndex, true);
        listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);

        vkCmdDispatch(context.cmd, 1, 1, 1);
        
        Vk::BufferBarrierInfo listBarrier{};
        listBarrier.buffer = drawData->ForwardPlus.clusterListBuffer.GetHandle(context.frameIndex, true);
        listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);
    }
}