#include "ClusterLightWriteSyncPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn {

    bool ClusterLightWriteSyncPass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->enableForwardPlus;
    }

    void ClusterLightWriteSyncPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::BufferBarrierInfo pBarrier{};
        pBarrier.buffer = drawData->ForwardPlus.pointLightIndexBuffer.GetHandle(fIdx, true);
        pBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        pBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        pBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        pBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, pBarrier);

        Vk::BufferBarrierInfo sBarrier{};
        sBarrier.buffer = drawData->ForwardPlus.spotLightIndexBuffer.GetHandle(fIdx, true);
        sBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        sBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sBarrier);
    }
}