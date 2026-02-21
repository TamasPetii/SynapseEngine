#include "CubeMeshSource.h"

namespace Syn
{
    CubeMeshSource::CubeMeshSource(float size)
        : ShapeMeshSource("Cube"), _size(size)
    {}

    void CubeMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        float h = _size * 0.5f;

        outPositions = {
            // Front face (+Z)
            {-h, -h,  h}, { h, -h,  h}, { h,  h,  h}, {-h,  h,  h},
            // Back face (-Z)
            { h, -h, -h}, {-h, -h, -h}, {-h,  h, -h}, { h,  h, -h},
            // Left face (-X)
            {-h, -h, -h}, {-h, -h,  h}, {-h,  h,  h}, {-h,  h, -h},
            // Right face (+X)
            { h, -h,  h}, { h, -h, -h}, { h,  h, -h}, { h,  h,  h},
            // Top face (+Y)
            {-h,  h,  h}, { h,  h,  h}, { h,  h, -h}, {-h,  h, -h},
            // Bottom face (-Y)
            {-h, -h, -h}, { h, -h, -h}, { h, -h,  h}, {-h, -h,  h}
        };
    }

    void CubeMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        outIndices.reserve(36);

        for (uint32_t i = 0; i < 6; ++i)
        {
            uint32_t offset = i * 4;

            outIndices.push_back(offset + 0);
            outIndices.push_back(offset + 1);
            outIndices.push_back(offset + 2);

            outIndices.push_back(offset + 2);
            outIndices.push_back(offset + 3);
            outIndices.push_back(offset + 0);
        }
    }

    void CubeMeshSource::GenerateUVs(std::vector<glm::vec2>& outUVs)
    {
        outUVs.reserve(24);

        glm::vec2 faceUVs[4] = {
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f}
        };

        for (int i = 0; i < 6; ++i)
        {
            outUVs.push_back(faceUVs[0]);
            outUVs.push_back(faceUVs[1]);
            outUVs.push_back(faceUVs[2]);
            outUVs.push_back(faceUVs[3]);
        }
    }

    void CubeMeshSource::GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals)
    {
        outNormals.reserve(24);

        glm::vec3 faceNormals[6] = {
            { 0.0f,  0.0f,  1.0f}, // Front
            { 0.0f,  0.0f, -1.0f}, // Back
            {-1.0f,  0.0f,  0.0f}, // Left
            { 1.0f,  0.0f,  0.0f}, // Right
            { 0.0f,  1.0f,  0.0f}, // Top
            { 0.0f, -1.0f,  0.0f}  // Bottom
        };

        for (int i = 0; i < 6; ++i)
        {
            outNormals.push_back(faceNormals[i]);
            outNormals.push_back(faceNormals[i]);
            outNormals.push_back(faceNormals[i]);
            outNormals.push_back(faceNormals[i]);
        }
    }
}