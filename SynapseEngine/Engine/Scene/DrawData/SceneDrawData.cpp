#include "SceneDrawData.h"

namespace Syn
{
    SceneDrawData::SceneDrawData(uint32_t frameCount)
        : Models(frameCount),
        Debug(frameCount),
        PointLights(frameCount),
        SpotLights(frameCount),
        DirectionLights(frameCount)
    {}

    void SceneDrawData::RequestGlobalSync(uint32_t framesInFlight) {
        uint32_t current = syncFramesRemaining.load(std::memory_order_relaxed);
        while (current < framesInFlight &&
            !syncFramesRemaining.compare_exchange_weak(current, framesInFlight, std::memory_order_release, std::memory_order_relaxed)) {
        }
    }

    void SceneDrawData::RecordGpuSync(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        uint32_t currentSync = syncFramesRemaining.load(std::memory_order_relaxed);
        if (currentSync == 0) return;

        Models.RecordSync(cmd, frameIndex);

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