#include "CapsuleMeshSource.h"
#include <glm/gtc/constants.hpp>

namespace Syn
{
    CapsuleMeshSource::CapsuleMeshSource(float radius, float height, uint32_t sides, uint32_t hemisphereSegments)
        : ShapeMeshSource("Capsule"), _radius(radius), _height(height), _sides(sides), _hemisphereSegments(hemisphereSegments)
    {}

    void CapsuleMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        uint32_t totalRings = (_hemisphereSegments * 2) + 2;

        for (uint32_t i = 0; i < totalRings; ++i)
        {
            float phi = 0.0f;
            float yOffset = 0.0f;

            // Felső félgömb
            if (i <= _hemisphereSegments)
            {
                phi = static_cast<float>(i) / _hemisphereSegments * (glm::pi<float>() * 0.5f);
                yOffset = _height * 0.5f;
            }
            // Alsó félgömb
            else
            {
                uint32_t bottomIndex = i - (_hemisphereSegments + 1);
                phi = glm::pi<float>() * 0.5f + static_cast<float>(bottomIndex) / _hemisphereSegments * (glm::pi<float>() * 0.5f);
                yOffset = -_height * 0.5f;
            }

            float y = _radius * std::cos(phi) + yOffset;
            float ringRadius = _radius * std::sin(phi);

            for (uint32_t j = 0; j <= _sides; ++j)
            {
                float theta = static_cast<float>(j) / _sides * 2.0f * glm::pi<float>();

                float x = ringRadius * std::cos(theta);
                float z = ringRadius * std::sin(theta);

                outPositions.emplace_back(x, y, z);
            }
        }
    }

    void CapsuleMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        uint32_t totalRings = (_hemisphereSegments * 2) + 2;

        for (uint32_t i = 0; i < totalRings - 1; ++i)
        {
            for (uint32_t j = 0; j < _sides; ++j)
            {
                uint32_t current = i * (_sides + 1) + j;
                uint32_t next = current + (_sides + 1);

                outIndices.push_back(current);
                outIndices.push_back(next);
                outIndices.push_back(current + 1);

                outIndices.push_back(current + 1);
                outIndices.push_back(next);
                outIndices.push_back(next + 1);
            }
        }
    }

    void CapsuleMeshSource::GenerateUVs(std::vector<glm::vec2>& outUVs)
    {
        uint32_t totalRings = (_hemisphereSegments * 2) + 2;
        uint32_t index = 0;

        float topArcLength = glm::pi<float>() * _radius * 0.5f;
        float cylinderLength = _height;
        float bottomArcLength = topArcLength;
        float totalLength = topArcLength + cylinderLength + bottomArcLength;

        for (uint32_t i = 0; i < totalRings; ++i)
        {
            float v = 0.0f;

            if (i <= _hemisphereSegments)
            {
                // Felső félgömb
                float phi = static_cast<float>(i) / _hemisphereSegments * (glm::pi<float>() * 0.5f);
                float currentArc = phi * _radius;
                v = currentArc / totalLength;
            }
            else
            {
                // Alsó félgömb
                uint32_t bottomIndex = i - (_hemisphereSegments + 1);
                float phi = static_cast<float>(bottomIndex) / _hemisphereSegments * (glm::pi<float>() * 0.5f);
                float currentArc = phi * _radius;
                v = (topArcLength + cylinderLength + currentArc) / totalLength;
            }

            for (uint32_t j = 0; j <= _sides; ++j)
            {
                float u = static_cast<float>(j) / _sides;
                outUVs[index++] = glm::vec2(u, v);
            }
        }
    }

    void CapsuleMeshSource::GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals)
    {
        uint32_t totalRings = (_hemisphereSegments * 2) + 2;
        uint32_t index = 0;

        for (uint32_t i = 0; i < totalRings; ++i)
        {
            float yOffset = (i <= _hemisphereSegments) ? (_height * 0.5f) : (-_height * 0.5f);
            glm::vec3 sphereCenter(0.0f, yOffset, 0.0f);

            for (uint32_t j = 0; j <= _sides; ++j)
            {
                glm::vec3 normal = glm::normalize(positions[index] - sphereCenter);
                outNormals[index] = normal;
                index++;
            }
        }
    }
}