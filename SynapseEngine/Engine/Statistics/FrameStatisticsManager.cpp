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

#include "FrameStatisticsManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include <cstddef>

namespace Syn {

    FrameStatisticsManager::FrameStatisticsManager(uint32_t framesInFlight) {
        _cpuStatsPerFrame.resize(framesInFlight);
        _gpuStatsPerFrame.resize(framesInFlight);

        RenderBufferConfig config{};
        config.debugName = "StatsReadbackBuffer";
        config.strategy = BufferStrategy::MappedOnly;
        config.frames = framesInFlight;
        config.elementSize = sizeof(GpuCullingReadback);
        config.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        config.upWindow = 1;
        config.downWindow = 1;
        _readbackBuffer.Initialize(config);
        _readbackBuffer.UpdateCapacityAll(framesInFlight);
    }

    void FrameStatisticsManager::RecordReadback(VkCommandBuffer cmd, uint32_t frameIndex, Scene* scene) {
        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();

        Vk::BufferBarrierInfo computeToTransferBarrier{};
        computeToTransferBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        computeToTransferBarrier.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
        computeToTransferBarrier.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        computeToTransferBarrier.dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
        computeToTransferBarrier.size = sizeof(uint32_t);

        std::vector<VkBuffer> buffersToWait = {
            drawData->SpotLights.indirectBuffer.GetHandle(frameIndex),
            drawData->SpotLightShadow.visibleCountDispatchBuffer.GetHandle(frameIndex),
            drawData->SpotLightShadow.visibleMeshCountDispatchBuffer.GetHandle(frameIndex),
            drawData->PointLights.indirectBuffer.GetHandle(frameIndex),
            drawData->PointLightShadow.visibleCountDispatchBuffer.GetHandle(frameIndex),
            drawData->PointLightShadow.visibleMeshCountDispatchBuffer.GetHandle(frameIndex)
        };

        for (VkBuffer buffer : buffersToWait) {
            computeToTransferBarrier.buffer = buffer;
            Vk::BufferUtils::InsertBarrier(cmd, computeToTransferBarrier);
        }

        Vk::BufferCopyInfo copyInfo{};
        copyInfo.size = sizeof(uint32_t);
        copyInfo.dstBuffer = _readbackBuffer.GetHandle(frameIndex);

        copyInfo.srcBuffer = drawData->SpotLights.indirectBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = offsetof(VkDrawIndirectCommand, instanceCount);
        copyInfo.dstOffset = offsetof(GpuCullingReadback, spotVisibleLights);
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        copyInfo.srcBuffer = drawData->SpotLightShadow.visibleCountDispatchBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = offsetof(GpuCullingReadback, spotVisibleShadowLights);
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        copyInfo.srcBuffer = drawData->SpotLightShadow.visibleMeshCountDispatchBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = offsetof(GpuCullingReadback, spotVisibleShadowInstances);
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        copyInfo.srcBuffer = drawData->PointLights.indirectBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = offsetof(VkDrawIndirectCommand, instanceCount);
        copyInfo.dstOffset = offsetof(GpuCullingReadback, pointVisibleLights);
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        copyInfo.srcBuffer = drawData->PointLightShadow.visibleCountDispatchBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = offsetof(GpuCullingReadback, pointVisibleShadowLights);
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        copyInfo.srcBuffer = drawData->PointLightShadow.visibleMeshCountDispatchBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = offsetof(GpuCullingReadback, pointVisibleShadowInstances);
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        Vk::BufferBarrierInfo transferToHostBarrier{};
        transferToHostBarrier.buffer = _readbackBuffer.GetHandle(frameIndex);
        transferToHostBarrier.size = sizeof(GpuCullingReadback);
        transferToHostBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transferToHostBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        transferToHostBarrier.dstStage = VK_PIPELINE_STAGE_2_HOST_BIT;
        transferToHostBarrier.dstAccess = VK_ACCESS_2_HOST_READ_BIT;
        Vk::BufferUtils::InsertBarrier(cmd, transferToHostBarrier);
    }

    void FrameStatisticsManager::ResolveFrame(VkCommandBuffer cmd, Scene* scene, uint32_t frameIndex, const std::vector<RenderPassStats>& gpuStats) {
        _gpuStatsPerFrame[frameIndex] = gpuStats;

        RecordReadback(cmd, frameIndex, scene);

        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();
        auto& stats = _cpuStatsPerFrame[frameIndex];
        stats = RawCpuRenderStats{};

        GpuCullingReadback gpuReadbackData{};
        if (auto mappedBuf = _readbackBuffer.GetMapped(frameIndex)) {
            if (void* ptr = mappedBuf->Map()) {
                gpuReadbackData = *static_cast<GpuCullingReadback*>(ptr);
                mappedBuf->Unmap();
            }
        }

        auto dirLightPool = scene->GetRegistry()->GetPool<DirectionLightComponent>();
        auto spotLightPool = scene->GetRegistry()->GetPool<SpotLightComponent>();
        auto pointLightPool = scene->GetRegistry()->GetPool<PointLightComponent>();

        stats.totalModels = drawData->Models.activeDescriptorCount;
        stats.activeTraditionalCount = drawData->Models.activeTraditionalCount;
        stats.activeMeshletCount = drawData->Models.activeMeshletCount;
        stats.totalAllocatedInstances = drawData->Models.totalAllocatedInstances;
        stats.totalMaxMeshletInstances = drawData->Debug.totalMaxMeshletInstances;
        stats.maxPossibleVertices = drawData->Models.maxPossibleVertices;
        stats.maxPossibleIndices = drawData->Models.maxPossibleIndices;
        stats.maxPossibleTriangles = drawData->Models.maxPossibleTriangles;

        stats.totalDirLights = static_cast<uint32_t>(dirLightPool ? dirLightPool->Size() : 0);
        stats.totalSpotLights = static_cast<uint32_t>(spotLightPool ? spotLightPool->Size() : 0);
        stats.totalPointLights = static_cast<uint32_t>(pointLightPool ? pointLightPool->Size() : 0);

        stats.visibleDirLights = drawData->DirectionLights.cmdTemplate.instanceCount;
        stats.visibleSpotLights = gpuReadbackData.spotVisibleLights;
        stats.visiblePointLights = gpuReadbackData.pointVisibleLights;

        stats.shadowDirLights = drawData->DirectionLightShadow.visibleLightCount;
        stats.shadowSpotLights = gpuReadbackData.spotVisibleShadowLights;
        stats.shadowPointLights = gpuReadbackData.pointVisibleShadowLights;

        stats.appendedDirInstances = stats.shadowDirLights * CASCADES_PER_LIGHT;
        stats.appendedSpotInstances = gpuReadbackData.spotVisibleShadowInstances;
        stats.appendedPointInstances = gpuReadbackData.pointVisibleShadowInstances;

        stats.maxDirVertices = stats.maxPossibleVertices * SHADOW_MULTIPLIER;
        stats.maxSpotVertices = stats.maxPossibleVertices * SPOT_SHADOW_MULTIPLIER;
        stats.maxPointVertices = stats.maxPossibleVertices * POINT_SHADOW_MULTIPLIER;

        stats.maxDirTriangles = stats.maxPossibleTriangles * SHADOW_MULTIPLIER;
        stats.maxSpotTriangles = stats.maxPossibleTriangles * SPOT_SHADOW_MULTIPLIER;
        stats.maxPointTriangles = stats.maxPossibleTriangles * POINT_SHADOW_MULTIPLIER;
    }

    const RawCpuRenderStats& FrameStatisticsManager::GetCpuStats(uint32_t frameIndex) const {
        return _cpuStatsPerFrame[frameIndex];
    }

    const std::vector<RenderPassStats>& FrameStatisticsManager::GetGpuStats(uint32_t frameIndex) const {
        return _gpuStatsPerFrame[frameIndex];
    }
}