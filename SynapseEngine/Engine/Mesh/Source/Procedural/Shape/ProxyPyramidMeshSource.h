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

#pragma once
#include "Engine/SynApi.h"
#include "ShapeMeshSource.h"

namespace Syn
{
    class SYN_API ProxyPyramidMeshSource : public ShapeMeshSource
    {
    public:
        ProxyPyramidMeshSource(float baseSize = 1.0f, float height = 2.0f);
        virtual ~ProxyPyramidMeshSource() override = default;
    protected:
        virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) override;
        virtual void GenerateIndices(std::vector<uint32_t>& outIndices) override;
        virtual void GenerateUVs(std::span<glm::vec2> outUVs) override;
        virtual void GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals) override;
    private:
        float _baseSize;
        float _height;
    };
}