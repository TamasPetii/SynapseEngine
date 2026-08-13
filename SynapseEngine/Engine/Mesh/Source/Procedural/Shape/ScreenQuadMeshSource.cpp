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

#include "ScreenQuadMeshSource.h"

namespace Syn
{
    ScreenQuadMeshSource::ScreenQuadMeshSource(float width, float height)
        : ShapeMeshSource("ScreenQuad"), _width(width), _height(height)
    {}

    void ScreenQuadMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        float hw = _width * 0.5f;
        float hh = _height * 0.5f;

        outPositions = {
            {-hw, -hh, 0.0f},
            { hw, -hh, 0.0f},
            {-hw,  hh, 0.0f},
            { hw,  hh, 0.0f}
        };
    }

    void ScreenQuadMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        outIndices = {
            0, 1, 2,
            2, 1, 3
        };
    }

    void ScreenQuadMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        uint32_t index = 0;
        outUVs[index++] = glm::vec2(0.0f, 1.0f);
        outUVs[index++] = glm::vec2(1.0f, 1.0f);
        outUVs[index++] = glm::vec2(0.0f, 0.0f);
        outUVs[index++] = glm::vec2(1.0f, 0.0f);
    }

    void ScreenQuadMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        uint32_t index = 0;

        for (int i = 0; i < 4; ++i)
        {
            outNormals[index++] = glm::vec3(0.0f, 0.0f, 1.0f);
        }
    }
}