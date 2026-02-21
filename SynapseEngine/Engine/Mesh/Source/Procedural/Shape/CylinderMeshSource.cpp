#include "CylinderMeshSource.h"
#include <glm/gtc/constants.hpp>

namespace Syn
{
    CylinderMeshSource::CylinderMeshSource(float bottomRadius, float topRadius, float height, uint32_t radialSegments, uint32_t heightSegments)
        : ShapeMeshSource("Cylinder"), _bottomRadius(bottomRadius), _topRadius(topRadius), _height(height), _radialSegments(radialSegments), _heightSegments(heightSegments)
    {}

    void CylinderMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        float halfHeight = _height * 0.5f;

        // (Side Surface)
        for (uint32_t y = 0; y <= _heightSegments; ++y)
        {
            float v = static_cast<float>(y) / _heightSegments;
            float radius = glm::mix(_bottomRadius, _topRadius, v);
            float posY = -halfHeight + (v * _height);

            for (uint32_t x = 0; x <= _radialSegments; ++x)
            {
                float u = static_cast<float>(x) / _radialSegments;
                float theta = u * 2.0f * glm::pi<float>();

                outPositions.emplace_back(radius * std::cos(theta), posY, radius * std::sin(theta));
            }
        }

        // (Bottom Cap)
        outPositions.emplace_back(0.0f, -halfHeight, 0.0f);
        for (uint32_t x = 0; x <= _radialSegments; ++x)
        {
            float u = static_cast<float>(x) / _radialSegments;
            float theta = u * 2.0f * glm::pi<float>();
            outPositions.emplace_back(_bottomRadius * std::cos(theta), -halfHeight, _bottomRadius * std::sin(theta));
        }

        // (Top Cap)
        outPositions.emplace_back(0.0f, halfHeight, 0.0f);
        for (uint32_t x = 0; x <= _radialSegments; ++x)
        {
            float u = static_cast<float>(x) / _radialSegments;
            float theta = u * 2.0f * glm::pi<float>();
            outPositions.emplace_back(_topRadius * std::cos(theta), halfHeight, _topRadius * std::sin(theta));
        }
    }

    void CylinderMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        // (Side Surface Indices)
        for (uint32_t y = 0; y < _heightSegments; ++y)
        {
            for (uint32_t x = 0; x < _radialSegments; ++x)
            {
                uint32_t current = y * (_radialSegments + 1) + x;
                uint32_t next = current + (_radialSegments + 1);

                outIndices.push_back(current);
                outIndices.push_back(next);
                outIndices.push_back(current + 1);

                outIndices.push_back(current + 1);
                outIndices.push_back(next);
                outIndices.push_back(next + 1);
            }
        }

        uint32_t offset = (_heightSegments + 1) * (_radialSegments + 1);

        // (Bottom Cap Indices)
        uint32_t bottomCenter = offset;
        uint32_t bottomEdge = offset + 1;
        for (uint32_t x = 0; x < _radialSegments; ++x)
        {
            outIndices.push_back(bottomCenter);
            outIndices.push_back(bottomEdge + x + 1);
            outIndices.push_back(bottomEdge + x);
        }

        offset += _radialSegments + 2;

        // (Top Cap Indices)
        uint32_t topCenter = offset;
        uint32_t topEdge = offset + 1;
        for (uint32_t x = 0; x < _radialSegments; ++x)
        {
            outIndices.push_back(topCenter);
            outIndices.push_back(topEdge + x);
            outIndices.push_back(topEdge + x + 1);
        }
    }

    void CylinderMeshSource::GenerateUVs(std::vector<glm::vec2>& outUVs)
    {
        uint32_t index = 0;

        // (Side Surface UV)
        for (uint32_t y = 0; y <= _heightSegments; ++y)
        {
            float v = static_cast<float>(y) / _heightSegments;
            for (uint32_t x = 0; x <= _radialSegments; ++x)
            {
                float u = static_cast<float>(x) / _radialSegments;
                outUVs[index++] = glm::vec2(u, v);
            }
        }

        // (Bottom Cap UV)
        outUVs[index++] = glm::vec2(0.5f, 0.5f);
        for (uint32_t x = 0; x <= _radialSegments; ++x)
        {
            float u = static_cast<float>(x) / _radialSegments;
            float theta = u * 2.0f * glm::pi<float>();
            outUVs[index++] = glm::vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta));
        }

        // (Top Cap UV)
        outUVs[index++] = glm::vec2(0.5f, 0.5f);
        for (uint32_t x = 0; x <= _radialSegments; ++x)
        {
            float u = static_cast<float>(x) / _radialSegments;
            float theta = u * 2.0f * glm::pi<float>();
            outUVs[index++] = glm::vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta));
        }
    }

    void CylinderMeshSource::GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals)
    {
        float slopeY = (_bottomRadius - _topRadius) / _height;
        uint32_t index = 0;

        // (Side Surface Normals)
        for (uint32_t y = 0; y <= _heightSegments; ++y)
        {
            for (uint32_t x = 0; x <= _radialSegments; ++x)
            {
                float u = static_cast<float>(x) / _radialSegments;
                float theta = u * 2.0f * glm::pi<float>();

                glm::vec3 normal(std::cos(theta), slopeY, std::sin(theta));
                outNormals[index++] = glm::normalize(normal);
            }
        }

        // (Bottom Cap Normals)
        for (uint32_t x = 0; x < _radialSegments + 2; ++x)
        {
            outNormals[index++] = glm::vec3(0.0f, -1.0f, 0.0f);
        }

        // (Top Cap Normals)
        for (uint32_t x = 0; x < _radialSegments + 2; ++x)
        {
            outNormals[index++] = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}