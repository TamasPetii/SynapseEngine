#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Mesh/MeshDrawBlueprint.h"

namespace Syn
{
    struct SYN_API GpuModelBuffers
    {
        // Geometry Buffers
        std::unique_ptr<Vk::Buffer> vertexPositions;
        std::unique_ptr<Vk::Buffer> vertexAttributes;
        std::unique_ptr<Vk::Buffer> indices;

        // Traditional Pipeline Buffers
        std::unique_ptr<Vk::Buffer> meshDescriptors;
        std::unique_ptr<Vk::Buffer> meshColliders;
        std::unique_ptr<Vk::Buffer> lodDescriptors;

        // Mesh Shader Buffers
        std::unique_ptr<Vk::Buffer> meshletVertexIndices;
        std::unique_ptr<Vk::Buffer> meshletTriangleIndices;
        std::unique_ptr<Vk::Buffer> meshletDescriptors;
        std::unique_ptr<Vk::Buffer> meshletDrawDescriptors;
        std::unique_ptr<Vk::Buffer> meshletColliders;

        //Hierarchy Buffers
        std::unique_ptr<Vk::Buffer> nodeTransforms;

        //Draw Data

        std::vector<MeshDrawBlueprint> baseDrawCommands;
    };

    struct SYN_API GpuModelAddresses
    {
        VkDeviceAddress vertexPositions;
        VkDeviceAddress vertexAttributes;
        VkDeviceAddress indices;
        VkDeviceAddress meshDescriptors;
        VkDeviceAddress meshColliders;
        VkDeviceAddress lodDescriptors;
        VkDeviceAddress meshletVertexIndices;
        VkDeviceAddress meshletTriangleIndices;
        VkDeviceAddress meshletDescriptors;
        VkDeviceAddress meshletDrawDescriptors;
        VkDeviceAddress meshletColliders;
        VkDeviceAddress nodeTransforms;
    };
}