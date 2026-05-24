#include "PyramidMeshSource.h"

namespace Syn
{
    PyramidMeshSource::PyramidMeshSource(float baseSize, float height)
        : ShapeMeshSource("Pyramid"), _baseSize(baseSize), _height(height)
    {}

    void PyramidMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        float h = _height * 0.5f;
        float r = _baseSize;

        glm::vec3 top(0.0f, h, 0.0f);
        glm::vec3 fl(-r, -h, r);  // Front-Left
        glm::vec3 fr(r, -h, r);   // Front-Right
        glm::vec3 br(r, -h, -r);  // Back-Right
        glm::vec3 bl(-r, -h, -r); // Back-Left

        outPositions.reserve(16);

        // Front face (0, 1, 2)
        outPositions.push_back(top); outPositions.push_back(fl); outPositions.push_back(fr);
        // Right face (3, 4, 5)
        outPositions.push_back(top); outPositions.push_back(fr); outPositions.push_back(br);
        // Back face (6, 7, 8)
        outPositions.push_back(top); outPositions.push_back(br); outPositions.push_back(bl);
        // Left face (9, 10, 11)
        outPositions.push_back(top); outPositions.push_back(bl); outPositions.push_back(fl);
		// Bottom face (12, 13, 14, 15)
        outPositions.push_back(fl); outPositions.push_back(bl); outPositions.push_back(br); outPositions.push_back(fr);
    }

    void PyramidMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        outIndices.reserve(18);
        outIndices.push_back(0); outIndices.push_back(1); outIndices.push_back(2);
        outIndices.push_back(3); outIndices.push_back(4); outIndices.push_back(5);
        outIndices.push_back(6); outIndices.push_back(7); outIndices.push_back(8);
        outIndices.push_back(9); outIndices.push_back(10); outIndices.push_back(11);
        outIndices.push_back(12); outIndices.push_back(13); outIndices.push_back(14);
        outIndices.push_back(12); outIndices.push_back(14); outIndices.push_back(15);
    }

    void PyramidMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        if (outUVs.size() < 16) return;

        for (int i = 0; i < 4; ++i) {
            outUVs[i * 3 + 0] = glm::vec2(0.5f, 1.0f);
            outUVs[i * 3 + 1] = glm::vec2(0.0f, 0.0f);
            outUVs[i * 3 + 2] = glm::vec2(1.0f, 0.0f);
        }

        outUVs[12] = glm::vec2(0.0f, 0.0f);
        outUVs[13] = glm::vec2(0.0f, 1.0f);
        outUVs[14] = glm::vec2(1.0f, 1.0f);
        outUVs[15] = glm::vec2(1.0f, 0.0f);
    }

    void PyramidMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            glm::vec3 v0 = positions[i0];
            glm::vec3 v1 = positions[i1];
            glm::vec3 v2 = positions[i2];

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            outNormals[i0] = normal;
            outNormals[i1] = normal;
            outNormals[i2] = normal;
        }
    }
}