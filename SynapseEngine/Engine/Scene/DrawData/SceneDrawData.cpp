#include "SceneDrawData.h"
#include "Engine/Shaders/Includes/Common/FrameGlobalContext.glsl"

namespace Syn
{
    SceneDrawData::SceneDrawData(uint32_t frameCount)
        : Models(frameCount),
        Debug(frameCount),
        PointLights(frameCount),
        SpotLights(frameCount),
        DirectionLights(frameCount),
        ForwardPlus(frameCount)
    {
        VkBufferUsageFlags contextUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        frameContextBuffer.Initialize({ BufferStrategy::Hybrid_Static, frameCount, sizeof(FrameGlobalContext), contextUsage, 1, 1});
        frameContextBuffer.UpdateCapacityAll(1);
    }

    void SceneDrawData::RequestGlobalSync(uint32_t framesInFlight) {
        uint32_t current = syncFramesRemaining.load(std::memory_order_relaxed);
        while (current < framesInFlight &&
            !syncFramesRemaining.compare_exchange_weak(current, framesInFlight, std::memory_order_release, std::memory_order_relaxed)) {
        }
    }

    void SceneDrawData::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        frameContextBuffer.RecordSync(cmd, frameIndex, 1);

        uint32_t currentSync = syncFramesRemaining.load(std::memory_order_relaxed);
        if (currentSync == 0) return;

        Models.CoherentToGpuBufferSync(cmd, frameIndex);
        Debug.CoherentToGpuBufferSync(cmd, frameIndex);
        PointLights.CoherentToGpuBufferSync(cmd, frameIndex);
        SpotLights.CoherentToGpuBufferSync(cmd, frameIndex);
        DirectionLights.CoherentToGpuBufferSync(cmd, frameIndex);
        ForwardPlus.CoherentToGpuBufferSync(cmd, frameIndex);

        Vk::GlobalBarrierInfo barrierInfo{};
        barrierInfo.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrierInfo.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrierInfo.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        barrierInfo.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
        Vk::BufferUtils::InsertGlobalBarrier(cmd, barrierInfo);

        uint32_t expected = currentSync;
        while (expected > 0 && !syncFramesRemaining.compare_exchange_weak(expected, expected - 1, std::memory_order_relaxed)) {}
    }
}