#include "SpotLightShadowBufferResetPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn {
    void SpotLightShadowBufferResetPass::Execute(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::BufferFillInfo fillBase{};
        fillBase.buffer = drawData->SpotLights.indirectBuffer.GetHandle(fIdx);
        fillBase.offset = sizeof(uint32_t);
        fillBase.size = sizeof(uint32_t);
        fillBase.data = 0;
        Vk::BufferUtils::FillBuffer(context.cmd, fillBase);

        Vk::BufferFillInfo fillShadow{};
        fillShadow.buffer = drawData->SpotLightShadow.visibleCountDispatchBuffer.GetHandle(fIdx);
        fillShadow.offset = 0;
        fillShadow.size = sizeof(uint32_t);
        fillShadow.data = 0;
        Vk::BufferUtils::FillBuffer(context.cmd, fillShadow);

        Vk::BufferFillInfo fillMesh{};
        fillMesh.buffer = drawData->SpotLightShadow.visibleMeshCountDispatchBuffer.GetHandle(fIdx);
        fillMesh.offset = 0;
        fillMesh.size = sizeof(uint32_t);
        fillMesh.data = 0;
        Vk::BufferUtils::FillBuffer(context.cmd, fillMesh);

        VkDispatchIndirectCommand finalizeCmd{ 0, 1, 1 };
        Vk::BufferUpdateInfo updateFinalize{};
        updateFinalize.buffer = drawData->SpotLightShadow.finalizeDispatchBuffer.GetHandle(fIdx);
        updateFinalize.offset = 0;
        updateFinalize.size = sizeof(VkDispatchIndirectCommand);
        updateFinalize.pData = &finalizeCmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateFinalize);

        Vk::BufferBarrierInfo fillBarrier{};
        fillBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        fillBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        fillBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        fillBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

        fillBarrier.buffer = fillBase.buffer;
        fillBarrier.size = fillBase.size;
        fillBarrier.offset = fillBase.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, fillBarrier);

        fillBarrier.buffer = fillShadow.buffer;
        fillBarrier.size = fillShadow.size;
        fillBarrier.offset = fillShadow.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, fillBarrier);

        fillBarrier.buffer = fillMesh.buffer;
        fillBarrier.size = fillMesh.size;
        fillBarrier.offset = fillMesh.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, fillBarrier);

        Vk::BufferBarrierInfo finalizeBarrier{};
        finalizeBarrier.buffer = updateFinalize.buffer;
        finalizeBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        finalizeBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        finalizeBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        finalizeBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, finalizeBarrier);
    }
}