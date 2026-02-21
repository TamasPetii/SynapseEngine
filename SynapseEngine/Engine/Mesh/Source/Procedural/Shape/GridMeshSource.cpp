#include "GridMeshSource.h"

namespace Syn
{
    GridMeshSource::GridMeshSource(float width, float depth, uint32_t segmentsX, uint32_t segmentsZ)
        : ShapeMeshSource("Grid"), _width(width), _depth(depth), _segmentsX(segmentsX), _segmentsZ(segmentsZ)
    {}

    void GridMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        float halfWidth = _width * 0.5f;
        float halfDepth = _depth * 0.5f;

        for (uint32_t z = 0; z <= _segmentsZ; ++z)
        {
            float v = static_cast<float>(z) / _segmentsZ;
            float posZ = -halfDepth + (v * _depth);

            for (uint32_t x = 0; x <= _segmentsX; ++x)
            {
                float u = static_cast<float>(x) / _segmentsX;
                float posX = -halfWidth + (u * _width);

                outPositions.emplace_back(posX, 0.0f, posZ);
            }
        }
    }

    void GridMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        for (uint32_t z = 0; z < _segmentsZ; ++z)
        {
            for (uint32_t x = 0; x < _segmentsX; ++x)
            {
                uint32_t current = z * (_segmentsX + 1) + x;
                uint32_t next = current + (_segmentsX + 1);

                outIndices.push_back(current);
                outIndices.push_back(next);
                outIndices.push_back(current + 1);

                outIndices.push_back(current + 1);
                outIndices.push_back(next);
                outIndices.push_back(next + 1);
            }
        }
    }

    void GridMeshSource::GenerateUVs(std::vector<glm::vec2>& outUVs)
    {
        uint32_t index = 0;
        for (uint32_t z = 0; z <= _segmentsZ; ++z)
        {
            float v = static_cast<float>(z) / _segmentsZ;
            for (uint32_t x = 0; x <= _segmentsX; ++x)
            {
                float u = static_cast<float>(x) / _segmentsX;
                outUVs[index++] = glm::vec2(u, v);
            }
        }
    }

    void GridMeshSource::GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals)
    {
        uint32_t index = 0;

        for (size_t i = 0; i < positions.size(); ++i)
        {
            outNormals[index++] = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}