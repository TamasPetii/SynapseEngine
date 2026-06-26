#include "VertexWeldingProcessor.h"
#include <unordered_map>

namespace std {
    template<> struct hash<glm::vec3> {
        size_t operator()(const glm::vec3& v) const {
            int32_t x = static_cast<int32_t>(v.x * 1000.0f);
            int32_t y = static_cast<int32_t>(v.y * 1000.0f);
            int32_t z = static_cast<int32_t>(v.z * 1000.0f);
            size_t h1 = hash<int32_t>{}(x);
            size_t h2 = hash<int32_t>{}(y);
            size_t h3 = hash<int32_t>{}(z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

namespace Syn
{
    void VertexWeldingProcessor::Process(CpuModelData& cpuData)
    {
        if (cpuData.batchedIndicesPerLod.empty() || cpuData.vertices.empty()) return;

        cpuData.physicsIndicesPerLod.clear();
        cpuData.physicsIndicesPerLod.resize(4);
        cpuData.physicsVertices.clear();

        std::unordered_map<glm::vec3, uint32_t> uniqueVertices;

        for (uint32_t lod = 0; lod < 4; ++lod)
        {
            cpuData.physicsIndicesPerLod[lod].reserve(cpuData.batchedIndicesPerLod[lod].size());

            for (uint32_t originalIndex : cpuData.batchedIndicesPerLod[lod])
            {
                const glm::vec3& position = cpuData.vertices[originalIndex];

                auto it = uniqueVertices.find(position);
                uint32_t newPhysicsVertexIndex;

                if (it != uniqueVertices.end())
                {
                    newPhysicsVertexIndex = it->second;
                }
                else
                {
                    newPhysicsVertexIndex = static_cast<uint32_t>(cpuData.physicsVertices.size());
                    cpuData.physicsVertices.push_back(position);
                    uniqueVertices[position] = newPhysicsVertexIndex;
                }

                cpuData.physicsIndicesPerLod[lod].push_back(newPhysicsVertexIndex);
            }
        }
    }
}