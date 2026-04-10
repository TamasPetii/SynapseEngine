#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>
#include <vector>
#include <string>

#include "Engine/Mesh/MeshAllocationInfo.h"
#include "Engine/Mesh/MeshDrawBlueprint.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Material/MaterialRenderType.h"
#include <atomic>

namespace Syn
{
    struct DebugMeshletInstance {
        uint32_t entityId;
        uint32_t globalMeshletIdx;
    };

    struct SYN_API SceneDrawData
    {
        SceneDrawData(uint32_t frameCount);

        static constexpr uint32_t MAX_INSTANCES = 10000000;
        static constexpr uint32_t MAX_INDIRECT_COMMANDS = 100000;

        // --- Model Data ---
        std::vector<uint32_t> paddedTraditionalCounts;
        std::vector<uint32_t> paddedMeshletCounts;

        uint32_t traditionalCmdOffsets[MaterialRenderType::Count] = { 0 };
        uint32_t traditionalCmdCounts[MaterialRenderType::Count] = { 0 };

        uint32_t meshletCmdOffsets[MaterialRenderType::Count] = { 0 };
        uint32_t meshletCmdCounts[MaterialRenderType::Count] = { 0 };

        std::atomic<uint32_t> syncFramesRemaining{ 0 };

        void RecordGpuSync(VkCommandBuffer cmd, uint32_t frameIndex);

        void RequestGlobalSync(uint32_t framesInFlight) {
            uint32_t current = syncFramesRemaining.load(std::memory_order_relaxed);
            while (current < framesInFlight &&
                !syncFramesRemaining.compare_exchange_weak(current, framesInFlight, std::memory_order_release, std::memory_order_relaxed)) {
            }
        }

        std::vector<std::shared_ptr<Vk::Buffer>> mappedDrawCountBuffers;

        std::vector<std::shared_ptr<Vk::Buffer>> mappedInstanceBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> mappedIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> mappedIndirectCommandDescriptorBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> mappedMaterialIndexBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> mappedModelAllocationBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> mappedMeshAllocationBuffers;

        std::vector<std::shared_ptr<Vk::Buffer>> gpuInstanceBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> gpuIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> gpuIndirectCommandDescriptorBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> gpuMaterialIndexBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> gpuModelAllocationBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> gpuMeshAllocationBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> gpuModelComputeCountBuffers;

        uint32_t activeDescriptorCount = 0;
        uint32_t activeTraditionalCount = 0;
        uint32_t activeMeshletCount = 0;
        uint32_t totalAllocatedInstances = 0;
        size_t requiredMaterialBufferSize = 0;

        std::vector<uint32_t> cpuInstanceBuffer;
        std::vector<MeshAllocationInfo> meshAllocations;
        std::vector<ModelAllocationInfo> modelAllocations;
        std::vector<MeshDrawDescriptor> drawDescriptors;
        std::vector<VkDrawIndirectCommand> traditionalCommands;
        std::vector<VkDrawMeshTasksIndirectCommandEXT> meshletCommands;

        // --- Debug Data ---
        std::vector<std::shared_ptr<Vk::Buffer>> aabbIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> sphereIndirectCommandBuffers;
        std::vector<VkDrawIndirectCommand> aabbIndirectCommands;
        std::vector<VkDrawIndirectCommand> sphereIndirectCommands;

        std::vector<std::shared_ptr<Vk::Buffer>> debugInstanceBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> debugAabbIndirectBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> debugSphereIndirectBuffers;
        VkDrawIndirectCommand debugAabbCmdTemplate{};
        VkDrawIndirectCommand debugSphereCmdTemplate{};
        uint32_t totalMaxMeshletInstances = 0;

        // --- Point Light Data ---
        VkDrawIndirectCommand pointLightCmdTemplate{};
        std::vector<uint32_t> pointLightCpuInstanceBuffer;
        std::vector<std::shared_ptr<Vk::Buffer>> pointLightIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> debugPointLightSphereCmdBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> debugPointLightAabbCmdBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> editorPointLightBillboardCmdBuffers;

        // --- Spot Light Data ---
        VkDrawIndirectCommand spotLightCmdTemplate{};
        std::vector<uint32_t> spotLightCpuInstanceBuffer;
        std::vector<std::shared_ptr<Vk::Buffer>> spotLightIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> debugSpotLightSphereCmdBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> debugSpotLightAabbCmdBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> editorSpotLightBillboardCmdBuffers;

		// --- Direction Light Data ---
        VkDrawIndirectCommand directionLightCmdTemplate{};
        std::vector<uint32_t> directionLightCpuInstanceBuffer;
        std::vector<std::shared_ptr<Vk::Buffer>> directionLightIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> editorDirectionLightBillboardCmdBuffers;
    private:
        void InitModelBuffers(uint32_t frameCount);
        void InitDebugBuffers(uint32_t frameCount);
        void InitPointLightBuffers(uint32_t frameCount);
        void InitSpotLightBuffers(uint32_t frameCount);
        void InitDirectionLightBuffers(uint32_t frameCount);
    };
}