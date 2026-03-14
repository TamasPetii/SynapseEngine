#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>
#include <vector>
#include <string>

#include "Engine/Mesh/MeshAllocationInfo.h"
#include "Engine/Mesh/MeshDrawBlueprint.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"

namespace Syn
{
    struct SYN_API SceneDrawData
    {
        SceneDrawData(uint32_t frameCount);

        static constexpr uint32_t MAX_INSTANCES = 10000000;
        static constexpr uint32_t MAX_INDIRECT_COMMANDS = 200000;
        static constexpr uint32_t MESHLET_OFFSET_START = 100000;

        std::vector<std::shared_ptr<Vk::Buffer>> globalInstanceBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> globalDrawCountBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> globalIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> globalIndirectCommandDescriptorBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> globalModelAllocationBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> globalMeshAllocationBuffers;

        std::vector<std::shared_ptr<Vk::Buffer>> aabbIndirectCommandBuffers;
        std::vector<std::shared_ptr<Vk::Buffer>> sphereIndirectCommandBuffers;

        uint32_t activeDescriptorCount = 0;
        uint32_t activeTraditionalCount = 0;
        uint32_t activeMeshletCount = 0;
        uint32_t totalAllocatedInstances = 0;

        std::vector<ModelAllocationInfo> modelAllocations;
        std::vector<MeshAllocationInfo> meshAllocations;
        std::vector<MeshDrawDescriptor> drawDescriptors;
        std::vector<VkDrawIndirectCommand> traditionalCommands;
        std::vector<VkDrawMeshTasksIndirectCommandEXT> meshletCommands;
        std::vector<uint32_t> cpuInstanceBuffer;

        std::vector<VkDrawIndirectCommand> aabbIndirectCommands;
        std::vector<VkDrawIndirectCommand> sphereIndirectCommands;
    };
}