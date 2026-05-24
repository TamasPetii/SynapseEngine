#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuModelBuffers.h"

namespace Syn
{
    struct SYN_API CpuModelData
    {
        uint32_t globalVertexCount = 0;
        uint32_t globalIndexCount = 0;
        uint32_t globalMeshCount = 0;
        uint32_t globalAverageLodIndexCount = 0;

        GpuMeshCollider globalCollider;
        std::vector<GpuMeshCollider> meshColliders;

        std::vector<GpuMeshDescriptor> meshDescriptors;
        std::vector<GpuMeshletDrawDescriptor> meshletDrawDescriptors;

        std::vector<MeshDrawBlueprint> baseDrawCommands;
        std::vector<uint32_t> meshMaterialIndices;

        std::vector<glm::vec3> physicsVertices;
        std::vector<uint32_t> physicsIndices;
    };
}

