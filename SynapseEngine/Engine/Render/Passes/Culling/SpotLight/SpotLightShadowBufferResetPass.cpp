#include "SpotLightShadowBufferResetPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn {
    void SpotLightShadowBufferResetPass::Execute(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        bool isSpotCullingGpu = context.scene->GetSettings()->culling.spotLightCullingDevice == CullingDeviceType::GPU;

        if (isSpotCullingGpu) {
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

            Vk::BufferBarrierInfo fillShadowBarrier{};
            fillShadowBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            fillShadowBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            fillShadowBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            fillShadowBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

            fillShadowBarrier.buffer = fillBase.buffer;
            fillShadowBarrier.size = fillBase.size;
            fillShadowBarrier.offset = fillBase.offset;
            Vk::BufferUtils::InsertBarrier(context.cmd, fillShadowBarrier);

            fillShadowBarrier.buffer = fillShadow.buffer;
            fillShadowBarrier.size = fillShadow.size;
            fillShadowBarrier.offset = fillShadow.offset;
            Vk::BufferUtils::InsertBarrier(context.cmd, fillShadowBarrier);
        }

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

        Vk::BufferBarrierInfo alwaysBarrier{};
        alwaysBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        alwaysBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        alwaysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        alwaysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

        alwaysBarrier.buffer = fillMesh.buffer;
        alwaysBarrier.size = fillMesh.size;
        alwaysBarrier.offset = fillMesh.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, alwaysBarrier);

        alwaysBarrier.buffer = updateFinalize.buffer;
        alwaysBarrier.size = updateFinalize.size;
        alwaysBarrier.offset = updateFinalize.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, alwaysBarrier);
    }
}