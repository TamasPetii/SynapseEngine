#include "TorusMeshSource.h"
#include <glm/gtc/constants.hpp>

namespace Syn
{
    TorusMeshSource::TorusMeshSource(float mainRadius, float tubeRadius, uint32_t mainSegments, uint32_t tubeSegments)
        : ShapeMeshSource("Torus"), _mainRadius(mainRadius), _tubeRadius(tubeRadius), _mainSegments(mainSegments), _tubeSegments(tubeSegments)
    {}

    void TorusMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        for (uint32_t i = 0; i <= _mainSegments; ++i)
        {
            float u = static_cast<float>(i) / _mainSegments;
            float theta = u * 2.0f * glm::pi<float>();

            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);

            for (uint32_t j = 0; j <= _tubeSegments; ++j)
            {
                float v = static_cast<float>(j) / _tubeSegments;
                float phi = v * 2.0f * glm::pi<float>();

                float cosPhi = std::cos(phi);
                float sinPhi = std::sin(phi);

                float x = (_mainRadius + _tubeRadius * cosPhi) * cosTheta;
                float y = _tubeRadius * sinPhi;
                float z = (_mainRadius + _tubeRadius * cosPhi) * sinTheta;

                outPositions.emplace_back(x, y, z);
            }
        }
    }

    void TorusMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        for (uint32_t i = 0; i < _mainSegments; ++i)
        {
            for (uint32_t j = 0; j < _tubeSegments; ++j)
            {
                uint32_t current = i * (_tubeSegments + 1) + j;
                uint32_t next = current + (_tubeSegments + 1);

                outIndices.push_back(current);
                outIndices.push_back(current + 1);
                outIndices.push_back(next);

                outIndices.push_back(current + 1);
                outIndices.push_back(next + 1);
                outIndices.push_back(next);
            }
        }
    }

    void TorusMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        uint32_t index = 0;
        for (uint32_t i = 0; i <= _mainSegments; ++i)
        {
            float u = static_cast<float>(i) / _mainSegments;

            for (uint32_t j = 0; j <= _tubeSegments; ++j)
            {
                float v = static_cast<float>(j) / _tubeSegments;
                outUVs[index++] = glm::vec2(u, v);
            }
        }
    }

    void TorusMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        uint32_t index = 0;
        for (uint32_t i = 0; i <= _mainSegments; ++i)
        {
            float u = static_cast<float>(i) / _mainSegments;
            float theta = u * 2.0f * glm::pi<float>();

            glm::vec3 tubeCenter(_mainRadius * std::cos(theta), 0.0f, _mainRadius * std::sin(theta));

            for (uint32_t j = 0; j <= _tubeSegments; ++j)
            {
                glm::vec3 normal = glm::normalize(positions[index] - tubeCenter);
                outNormals[index] = normal;
                index++;
            }
        }
    }
}