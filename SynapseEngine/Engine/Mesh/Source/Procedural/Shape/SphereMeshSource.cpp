// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "SphereMeshSource.h"
#include <glm/gtc/constants.hpp>

namespace Syn
{
    SphereMeshSource::SphereMeshSource(float radius, uint32_t sectors, uint32_t stacks)
        : ShapeMeshSource("Sphere"), _radius(radius), _sectors(sectors), _stacks(stacks)
    {}

    void SphereMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        for (uint32_t stack = 0; stack <= _stacks; ++stack)
        {
            float v = static_cast<float>(stack) / _stacks;
            float phi = v * glm::pi<float>();

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
    }

    void SphereMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
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
    }

    void SphereMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        uint32_t index = 0;
        for (uint32_t stack = 0; stack <= _stacks; ++stack)
        {
            float v = static_cast<float>(stack) / _stacks;
            for (uint32_t sector = 0; sector <= _sectors; ++sector)
            {
                float u = static_cast<float>(sector) / _sectors;

                outUVs[index++] = glm::vec2(u, v);
            }
        }
    }

    void SphereMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        for (size_t i = 0; i < positions.size(); ++i)
        {
            if (glm::length(positions[i]) > 0.0001f) {
                outNormals[i] = glm::normalize(positions[i]);
            }
            else {
                outNormals[i] = glm::vec3(0.0f, 1.0f, 0.0f);
            }
        }
    }
}