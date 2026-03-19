#include "QuadMeshSource.h"

namespace Syn
{
    QuadMeshSource::QuadMeshSource(float width, float depth)
        : ShapeMeshSource("Quad"), _width(width), _height(depth)
    {}

    void QuadMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        float hw = _width * 0.5f;
        float hd = _height * 0.5f;

        outPositions = {
            {-hw, 0.0f, -hd},
            { hw, 0.0f, -hd},
            {-hw, 0.0f,  hd},
            { hw, 0.0f,  hd} 
        };
    }

    void QuadMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        outIndices = {
            0, 2, 1,
            1, 2, 3
        };
    }

    void QuadMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        uint32_t index = 0;
        outUVs[index++] = glm::vec2(0.0f, 0.0f);
        outUVs[index++] = glm::vec2(1.0f, 0.0f);
        outUVs[index++] = glm::vec2(0.0f, 1.0f);
        outUVs[index++] = glm::vec2(1.0f, 1.0f);
    }

    void QuadMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        uint32_t index = 0;

        for (int i = 0; i < 4; ++i)
        {
            outNormals[index++] = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}