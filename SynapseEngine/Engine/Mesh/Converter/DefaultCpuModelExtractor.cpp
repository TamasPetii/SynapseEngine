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

            blueprint.isMeshletPipeline = false ? MeshDrawBlueprint::PIPELINE_MESHLET : MeshDrawBlueprint::PIPELINE_TRADITIONAL;

            outCpuData.baseDrawCommands.push_back(blueprint);
        }

        outCpuData.vertices.reserve(gpuData.vertexData.vertexPositions.size());
        for (const auto& v : gpuData.vertexData.vertexPositions) {
            outCpuData.vertices.push_back(v.position);
        }

        outCpuData.indices = gpuData.indexedData.indices;
    }
}