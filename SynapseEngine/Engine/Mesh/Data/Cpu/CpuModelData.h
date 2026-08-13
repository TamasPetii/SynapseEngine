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

#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuModelBuffers.h"
#include <optional>
#include <vector>

namespace Syn
{
    struct SYN_API CpuModelData
    {
        uint32_t globalVertexCount = 0;
        uint32_t globalIndexCount = 0;
        uint32_t globalLod0IndexCount = 0;
        uint32_t globalMeshCount = 0;
        uint32_t globalAverageLodIndexCount = 0;

        GpuMeshCollider globalCollider;
        std::vector<GpuMeshCollider> meshColliders;
        std::vector<GpuMeshDescriptor> meshDescriptors;
        std::vector<GpuMeshletDrawDescriptor> meshletDrawDescriptors;
        std::vector<GpuMeshLodDescriptor> lodDescriptors;
        std::vector<MeshInstanceDescriptor> meshNodeDescriptors;

        std::vector<MeshDrawBlueprint> baseDrawCommands;
        std::vector<uint32_t> meshMaterialIndices;

        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;

        //Todo: Optional
        std::vector<glm::vec3> worldPositions;
        std::vector<uint32_t> meshletVertexIndices;
		std::vector<uint8_t> meshletTriangleIndices;     
        std::vector<GpuMeshletDescriptor> meshletDescriptors;

        std::vector<glm::mat4> nodeTransforms;
        std::vector<glm::vec3> physicsVertices;
        std::vector<std::vector<uint32_t>> batchedIndicesPerLod;
        std::vector<std::vector<uint32_t>> physicsIndicesPerLod;
    };
}

