#include "HemisphereMeshSource.h"
#include <glm/gtc/constants.hpp>

namespace Syn
{
    HemisphereMeshSource::HemisphereMeshSource(float radius, uint32_t sectors, uint32_t stacks)
        : ShapeMeshSource("Hemisphere"), _radius(radius), _sectors(sectors), _stacks(stacks)
    {}

    void HemisphereMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        // (Dome)
        for (uint32_t stack = 0; stack <= _stacks; ++stack)
        {
            float v = static_cast<float>(stack) / _stacks;
            float phi = v * (glm::pi<float>() * 0.5f);

            for (uint32_t sector = 0; sector <= _sectors; ++sector)
            {
                float u = static_cast<float>(sector) / _sectors;
                float theta = u * 2.0f * glm::pi<float>();

                float x = _radius * std::sin(phi) * std::cos(theta);
                float y = _radius * std::cos(phi);
                float z = _radius * std::sin(phi) * std::sin(theta);

                outPositions.emplace_back(x, y, z);
            }
        }

        // (Bottom Cap)
        outPositions.emplace_back(0.0f, 0.0f, 0.0f);
        for (uint32_t sector = 0; sector <= _sectors; ++sector)
        {
            float u = static_cast<float>(sector) / _sectors;
            float theta = u * 2.0f * glm::pi<float>();

            outPositions.emplace_back(_radius * std::cos(theta), 0.0f, _radius * std::sin(theta));
        }
    }

    void HemisphereMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        // (Dome Indices)
        for (uint32_t stack = 0; stack < _stacks; ++stack)
        {
            for (uint32_t sector = 0; sector < _sectors; ++sector)
            {
                uint32_t current = stack * (_sectors + 1) + sector;
                uint32_t next = current + (_sectors + 1);

                outIndices.push_back(current);
                outIndices.push_back(current + 1);
                outIndices.push_back(next);

                outIndices.push_back(current + 1);
                outIndices.push_back(next + 1);
                outIndices.push_back(next);
            }
        }

        // (Bottom Cap Indices)
        uint32_t offset = (_stacks + 1) * (_sectors + 1);
        uint32_t centerIndex = offset;
        uint32_t edgeIndex = offset + 1;

        for (uint32_t sector = 0; sector < _sectors; ++sector)
        {
            outIndices.push_back(centerIndex);
            outIndices.push_back(edgeIndex + sector);
            outIndices.push_back(edgeIndex + sector + 1);
        }
    }

    void HemisphereMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        uint32_t index = 0;

        // (Dome UV)
        for (uint32_t stack = 0; stack <= _stacks; ++stack)
        {
            float v = static_cast<float>(stack) / _stacks;
            for (uint32_t sector = 0; sector <= _sectors; ++sector)
            {
                float u = static_cast<float>(sector) / _sectors;
                outUVs[index++] = glm::vec2(u, v);
            }
        }

        // (Bottom Cap UV)
        outUVs[index++] = glm::vec2(0.5f, 0.5f);
        for (uint32_t sector = 0; sector <= _sectors; ++sector)
        {
            float u = static_cast<float>(sector) / _sectors;
            float theta = u * 2.0f * glm::pi<float>();
            outUVs[index++] = glm::vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta));
        }
    }

    void HemisphereMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        uint32_t index = 0;

        // (Dome Normals)
        uint32_t domeVerts = (_stacks + 1) * (_sectors + 1);
        for (uint32_t i = 0; i < domeVerts; ++i)
        {
            if (glm::length(positions[index]) > 0.0001f) {
                outNormals[index] = glm::normalize(positions[index]);
            }
            else {
                outNormals[index] = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            index++;
        }

        // (Bottom Cap Normals)
        uint32_t capVerts = _sectors + 2;
        for (uint32_t i = 0; i < capVerts; ++i)
        {
            outNormals[index++] = glm::vec3(0.0f, -1.0f, 0.0f);
        }
    }
}