// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "SceneDrawData.h"
#include "Engine/Shaders/Includes/Common/FrameGlobalContext.glsl"

namespace Syn
{
    SceneDrawData::SceneDrawData(uint32_t frameCount)
        : 
        Models(frameCount),
        Debug(frameCount),
        PointLights(frameCount),
        SpotLights(frameCount),
        ForwardPlus(frameCount),
        Chunks(frameCount),
		Ssao(frameCount),
        DirectionLights(frameCount),
        DirectionLightShadow(frameCount),
        SpotLightShadow(frameCount),
        PointLightShadow(frameCount)
    {
        VkBufferUsageFlags contextUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        frameContextBuffer.Initialize({ "SceneDrawData_FrameContextBuffer", BufferStrategy::Hybrid, frameCount, sizeof(FrameGlobalContext), contextUsage, 1, 1});
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
        frameContextBuffer.RecordSync(cmd, frameIndex);
        Models.CoherentToGpuBufferSync(cmd, frameIndex);
        Debug.CoherentToGpuBufferSync(cmd, frameIndex);
        PointLights.CoherentToGpuBufferSync(cmd, frameIndex);
        SpotLights.CoherentToGpuBufferSync(cmd, frameIndex);
        DirectionLights.CoherentToGpuBufferSync(cmd, frameIndex);
        ForwardPlus.CoherentToGpuBufferSync(cmd, frameIndex);
		Chunks.CoherentToGpuBufferSync(cmd, frameIndex);
		Ssao.CoherentToGpuBufferSync(cmd, frameIndex);
        DirectionLightShadow.CoherentToGpuBufferSync(cmd, frameIndex);
        SpotLightShadow.CoherentToGpuBufferSync(cmd, frameIndex);
        PointLightShadow.CoherentToGpuBufferSync(cmd, frameIndex);

        Vk::GlobalBarrierInfo barrierInfo{};
        barrierInfo.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrierInfo.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrierInfo.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        barrierInfo.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
        Vk::BufferUtils::InsertGlobalBarrier(cmd, barrierInfo);

        uint32_t expected = syncFramesRemaining.load(std::memory_order_relaxed);
        while (expected > 0 && !syncFramesRemaining.compare_exchange_weak(expected, expected - 1, std::memory_order_relaxed)) {}
    }
}