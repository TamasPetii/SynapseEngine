#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"

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

        //Indirect buffers
        std::unique_ptr<Vk::Buffer> indirectBuffer;
        std::unique_ptr<Vk::Buffer> indirectMeshletBuffer;
    };
}