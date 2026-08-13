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