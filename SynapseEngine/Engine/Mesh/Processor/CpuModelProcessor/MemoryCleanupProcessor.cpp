#include "MemoryCleanupProcessor.h"

namespace Syn
{
    void MemoryCleanupProcessor::Process(CpuModelData& cpuData)
    {
        cpuData.worldPositions.clear();
        cpuData.worldPositions.shrink_to_fit();

        //This is needed for animation!

        /*
        cpuData.vertices.clear();
        cpuData.vertices.shrink_to_fit();
       
        cpuData.indices.clear();
        cpuData.indices.shrink_to_fit();

        cpuData.batchedIndicesPerLod.clear();
        cpuData.batchedIndicesPerLod.shrink_to_fit();

        cpuData.meshletVertexIndices->clear();
        cpuData.meshletVertexIndices->shrink_to_fit();

        cpuData.meshletTriangleIndices->clear();
        cpuData.meshletTriangleIndices->shrink_to_fit();

        cpuData.meshletDescriptors->clear();
        cpuData.meshletDescriptors->shrink_to_fit();
        */
    }
}