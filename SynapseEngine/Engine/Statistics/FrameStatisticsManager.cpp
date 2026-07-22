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
            drawData->PointLights.indirectBuffer.GetHandle(frameIndex),
            drawData->PointLightShadow.visibleCountDispatchBuffer.GetHandle(frameIndex)
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

        copyInfo.srcBuffer = drawData->PointLights.indirectBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = offsetof(VkDrawIndirectCommand, instanceCount);
        copyInfo.dstOffset = offsetof(GpuCullingReadback, pointVisibleLights);
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        copyInfo.srcBuffer = drawData->PointLightShadow.visibleCountDispatchBuffer.GetHandle(frameIndex);
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = offsetof(GpuCullingReadback, pointVisibleShadowLights);
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
        auto& currentStatsMap = _cpuStatsPerFrame[frameIndex];
        currentStatsMap.clear();

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

        uint32_t totalDirLights = static_cast<uint32_t>(dirLightPool ? dirLightPool->Size() : 0);
        uint32_t totalSpotLights = static_cast<uint32_t>(spotLightPool ? spotLightPool->Size() : 0);
        uint32_t totalPointLights = static_cast<uint32_t>(pointLightPool ? pointLightPool->Size() : 0);

        uint32_t visibleDirLights = drawData->DirectionLights.cmdTemplate.instanceCount;
        uint32_t visibleSpotLights = gpuReadbackData.spotVisibleLights;
        uint32_t visiblePointLights = gpuReadbackData.pointVisibleLights;

        uint32_t shadowDirLights = drawData->DirectionLightShadow.visibleLightCount;
        uint32_t shadowSpotLights = gpuReadbackData.spotVisibleShadowLights;
        uint32_t shadowPointLights = gpuReadbackData.pointVisibleShadowLights;

        {
            CpuRenderStats stats{};
            stats.totalModels = drawData->Models.activeDescriptorCount;
            stats.traditionalDrawDescriptors = drawData->Models.activeTraditionalCount;
            stats.meshletDrawDescriptors = drawData->Models.activeMeshletCount;
            stats.totalDrawDescriptors = stats.traditionalDrawDescriptors + stats.meshletDrawDescriptors;
            stats.totalAllocatedInstances = drawData->Models.totalAllocatedInstances;
            stats.totalMaxMeshlets = drawData->Debug.totalMaxMeshletInstances;

            stats.maxPossibleVertices = drawData->Models.maxPossibleVertices;
            stats.maxPossibleIndices = drawData->Models.maxPossibleIndices;
            stats.maxPossibleTriangles = drawData->Models.maxPossibleTriangles;

            stats.totalLights = totalDirLights + totalSpotLights + totalPointLights;
            stats.visibleLights = visibleDirLights + visibleSpotLights + visiblePointLights;
            stats.visibleShadowLights = shadowDirLights + shadowSpotLights + shadowPointLights;

            currentStatsMap[StatCategory::Scene] = stats;
        }

        {
            CpuRenderStats stats{};
            stats.totalModels = drawData->Models.activeDescriptorCount;
            stats.traditionalDrawDescriptors = drawData->Models.activeTraditionalCount;
            stats.meshletDrawDescriptors = drawData->Models.activeMeshletCount;
            stats.totalDrawDescriptors = stats.traditionalDrawDescriptors + stats.meshletDrawDescriptors;
            stats.totalAllocatedInstances = drawData->Models.totalAllocatedInstances;
            stats.totalMaxMeshlets = drawData->Debug.totalMaxMeshletInstances;

            stats.totalLights = totalDirLights;
            stats.visibleLights = visibleDirLights;
            stats.visibleShadowLights = shadowDirLights;
            stats.appendedInstances = shadowDirLights * CASCADES_PER_LIGHT;

            stats.maxPossibleVertices = drawData->Models.maxPossibleVertices * SHADOW_MULTIPLIER;
            stats.maxPossibleTriangles = drawData->Models.maxPossibleTriangles * SHADOW_MULTIPLIER;

            currentStatsMap[StatCategory::DirectionalShadow] = stats;
        }

        {
            CpuRenderStats stats{};
            stats.totalModels = drawData->Models.activeDescriptorCount;
            stats.traditionalDrawDescriptors = drawData->Models.activeTraditionalCount;
            stats.meshletDrawDescriptors = drawData->Models.activeMeshletCount;
            stats.totalDrawDescriptors = stats.traditionalDrawDescriptors + stats.meshletDrawDescriptors;
            stats.totalAllocatedInstances = drawData->Models.totalAllocatedInstances;
            stats.totalMaxMeshlets = drawData->Debug.totalMaxMeshletInstances;

            stats.totalLights = totalSpotLights;
            stats.visibleLights = visibleSpotLights;
            stats.visibleShadowLights = shadowSpotLights;
            stats.appendedInstances = shadowSpotLights * SPOT_SHADOW_MULTIPLIER;

            stats.maxPossibleVertices = drawData->Models.maxPossibleVertices * SPOT_SHADOW_MULTIPLIER;
            stats.maxPossibleTriangles = drawData->Models.maxPossibleTriangles * SPOT_SHADOW_MULTIPLIER;

            currentStatsMap[StatCategory::SpotShadow] = stats;
        }

        {
            CpuRenderStats stats{};
            stats.totalModels = drawData->Models.activeDescriptorCount;
            stats.traditionalDrawDescriptors = drawData->Models.activeTraditionalCount;
            stats.meshletDrawDescriptors = drawData->Models.activeMeshletCount;
            stats.totalDrawDescriptors = stats.traditionalDrawDescriptors + stats.meshletDrawDescriptors;
            stats.totalAllocatedInstances = drawData->Models.totalAllocatedInstances;
            stats.totalMaxMeshlets = drawData->Debug.totalMaxMeshletInstances;

            stats.totalLights = totalPointLights;
            stats.visibleLights = visiblePointLights;
            stats.visibleShadowLights = shadowPointLights;
            stats.appendedInstances = shadowPointLights * POINT_SHADOW_MULTIPLIER;

            stats.maxPossibleVertices = drawData->Models.maxPossibleVertices * POINT_SHADOW_MULTIPLIER;
            stats.maxPossibleTriangles = drawData->Models.maxPossibleTriangles * POINT_SHADOW_MULTIPLIER;

            currentStatsMap[StatCategory::PointShadow] = stats;
        }
    }

    const std::map<StatCategory, CpuRenderStats>& FrameStatisticsManager::GetCpuStats(uint32_t frameIndex) const {
        return _cpuStatsPerFrame[frameIndex];
    }

    const std::vector<RenderPassStats>& FrameStatisticsManager::GetGpuStats(uint32_t frameIndex) const {
        return _gpuStatsPerFrame[frameIndex];
    }
}