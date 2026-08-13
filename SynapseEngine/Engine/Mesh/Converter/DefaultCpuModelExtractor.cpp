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

#include "DefaultCpuModelExtractor.h"
#include "Engine/Render/ComputeGroupSize.h"
#include <utility>

namespace Syn
{
    void DefaultCpuModelExtractor::Extract(GpuBatchedModel& gpuData, CpuModelData& outCpuData) const
    {
        outCpuData.globalVertexCount = gpuData.globalVertexCount;
        outCpuData.globalIndexCount = gpuData.globalIndexCount;
        outCpuData.globalMeshCount = gpuData.globalMeshCount;
        outCpuData.globalAverageLodIndexCount = gpuData.globalAverageLodIndexCount;
        outCpuData.globalLod0IndexCount = gpuData.globalLod0IndexCount;

        size_t totalLodCount = gpuData.indexedData.meshDescriptors.size();

		outCpuData.lodDescriptors = gpuData.indexedData.lodDescriptors;
        outCpuData.globalCollider = gpuData.globalCollider;
        outCpuData.meshColliders = gpuData.indexedData.meshColliders;
        outCpuData.meshDescriptors = gpuData.indexedData.meshDescriptors;
        outCpuData.meshletDrawDescriptors = gpuData.meshletData.drawDescriptors;
        outCpuData.meshletVertexIndices = gpuData.meshletData.vertexIndices;
        outCpuData.meshletTriangleIndices = gpuData.meshletData.triangleIndices;
		outCpuData.meshletDescriptors = gpuData.meshletData.meshletDescriptors;
        outCpuData.baseDrawCommands.reserve(totalLodCount);

        for (size_t i = 0; i < totalLodCount; ++i)
        {
            const auto& tradDesc = outCpuData.meshDescriptors[i];

            bool hasMeshlet = i < outCpuData.meshletDrawDescriptors.size();
            const auto& meshletDesc = hasMeshlet ? outCpuData.meshletDrawDescriptors[i] : GpuMeshletDrawDescriptor{};

            MeshDrawBlueprint blueprint{};

            blueprint.traditionalCmd.vertexCount = tradDesc.indexCount;
            blueprint.traditionalCmd.instanceCount = 0;
            blueprint.traditionalCmd.firstVertex = tradDesc.indexOffset;
            blueprint.traditionalCmd.firstInstance = 0;

            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(meshletDesc.meshletCount, ComputeGroupSize::Buffer32D);
            blueprint.meshletCmd.groupCountX = 0;
            blueprint.meshletCmd.groupCountY = groupCountY;
            blueprint.meshletCmd.groupCountZ = 1;

            blueprint.pipelineRenderType = PipelineRenderType::Meshlet;

            outCpuData.baseDrawCommands.push_back(blueprint);
        }

        outCpuData.vertices.reserve(gpuData.vertexData.vertexPositions.size());
        for (const auto& v : gpuData.vertexData.vertexPositions) {
            outCpuData.vertices.push_back(v.position);
        }

        outCpuData.indices = gpuData.indexedData.indices;
        outCpuData.meshNodeDescriptors = gpuData.meshNodeDescriptors;

        outCpuData.nodeTransforms.reserve(gpuData.nodeTransforms.size());
        for (const auto& node : gpuData.nodeTransforms)
        {
            outCpuData.nodeTransforms.push_back(node.transform);
        }
    }
}