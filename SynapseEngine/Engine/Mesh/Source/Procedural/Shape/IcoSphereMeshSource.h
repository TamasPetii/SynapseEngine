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
#include <map>
#include <vector>

namespace Syn
{
    class SYN_API IcoSphereMeshSource : public ShapeMeshSource
    {
    public:
        IcoSphereMeshSource(float radius = 1.0f, uint32_t subdivisions = 0);
        virtual ~IcoSphereMeshSource() override = default;

    protected:
        virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) override;
        virtual void GenerateIndices(std::vector<uint32_t>& outIndices) override;
        virtual void GenerateUVs(std::span<glm::vec2> outUVs) override;
        virtual void GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals) override;

    private:
        void GenerateGeometry();
        uint32_t GetMiddlePoint(uint32_t p1, uint32_t p2, std::map<int64_t, uint32_t>& cache);

        float _radius;
        uint32_t _subdivisions;
        bool _isGenerated = false;

        std::vector<glm::vec3> _cachedPositions;
        std::vector<uint32_t> _cachedIndices;
        std::vector<glm::vec2> _cachedUVs;
        std::vector<glm::vec3> _cachedNormals;
    };
}