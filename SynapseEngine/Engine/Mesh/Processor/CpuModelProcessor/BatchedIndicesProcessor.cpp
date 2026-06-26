#include "BatchedIndicesProcessor.h"

namespace Syn
{
    constexpr uint32_t MAX_LODS = 4;

    void BatchedIndicesProcessor::Process(CpuModelData& cpuData)
    {
        cpuData.batchedIndicesPerLod.clear();
        cpuData.batchedIndicesPerLod.resize(MAX_LODS);

        if (cpuData.indices.empty() || cpuData.meshDescriptors.empty()) return;

        uint32_t globalMeshCount = cpuData.globalMeshCount;

        for (uint32_t lod = 0; lod < MAX_LODS; ++lod)
        {
            for (uint32_t m = 0; m < globalMeshCount; ++m)
            {
                uint32_t descIndex = (m * MAX_LODS) + lod;

                if (descIndex >= cpuData.meshDescriptors.size()) 
                    continue;

                const auto& meshDesc = cpuData.meshDescriptors[descIndex];

                uint32_t indexOffset = meshDesc.indexOffset;
                uint32_t indexCount = meshDesc.indexCount;

                if (indexCount == 0) continue;

                cpuData.batchedIndicesPerLod[lod].reserve(cpuData.batchedIndicesPerLod[lod].size() + indexCount);

                for (uint32_t i = 0; i < indexCount; ++i)
                    cpuData.batchedIndicesPerLod[lod].push_back(cpuData.indices[indexOffset + i]);
            }
        }
    }
}