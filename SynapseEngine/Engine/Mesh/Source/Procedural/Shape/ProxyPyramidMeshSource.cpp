#include "ProxyPyramidMeshSource.h"

namespace Syn
{
    ProxyPyramidMeshSource::ProxyPyramidMeshSource(float baseSize, float height)
        : ShapeMeshSource("ProxyPyramid"), _baseSize(baseSize), _height(height)
    {}

    void ProxyPyramidMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        float halfHeight = _height * 0.5f;
        float r = _baseSize;

        outPositions.reserve(5);
        outPositions.emplace_back(0.0f, halfHeight, 0.0f);
        outPositions.emplace_back(-r, -halfHeight, r); // Front-Left
        outPositions.emplace_back(r, -halfHeight, r); // Front-Right
        outPositions.emplace_back(r, -halfHeight, -r); // Back-Right
        outPositions.emplace_back(-r, -halfHeight, -r); // Back-Left
    }

    void ProxyPyramidMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        outIndices.push_back(0); outIndices.push_back(1); outIndices.push_back(2); // Front
        outIndices.push_back(0); outIndices.push_back(2); outIndices.push_back(3); // Right
        outIndices.push_back(0); outIndices.push_back(3); outIndices.push_back(4); // Back
        outIndices.push_back(0); outIndices.push_back(4); outIndices.push_back(1); // Left

        outIndices.push_back(1); outIndices.push_back(3); outIndices.push_back(2);
        outIndices.push_back(1); outIndices.push_back(4); outIndices.push_back(3);
    }

    void ProxyPyramidMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        outUVs[0] = glm::vec2(0.5f, 1.0f);
        outUVs[1] = glm::vec2(0.0f, 0.0f);
        outUVs[2] = glm::vec2(1.0f, 0.0f);
        outUVs[3] = glm::vec2(1.0f, 1.0f);
        outUVs[4] = glm::vec2(0.0f, 1.0f);
    }

    void ProxyPyramidMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        outNormals[0] = glm::vec3(0.0f, 1.0f, 0.0f);

        for (size_t i = 1; i < outNormals.size() && i < positions.size(); ++i)
        {
            outNormals[i] = glm::normalize(glm::vec3(positions[i].x, 0.5f, positions[i].z));
        }
    }
}