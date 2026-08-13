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
#include "Engine/Mesh/Data/StaticMesh.h"
#include <string>
#include <memory>

namespace Syn
{
    class SYN_API MeshFactory
    {
    public:
        MeshFactory() = delete;

        static std::shared_ptr<StaticMesh> LoadFromFile(const std::string& filePath);

        static std::shared_ptr<StaticMesh> CreateCapsule(float radius = 0.5f, float height = 1.0f, uint32_t sides = 32, uint32_t hemisphereSegments = 16);

        static std::shared_ptr<StaticMesh> CreateCone(float radius = 1.0f, float height = 2.0f, uint32_t radialSegments = 32, uint32_t heightSegments = 1);

        static std::shared_ptr<StaticMesh> CreateProxyCone(float radius = 1.0f, float height = 2.0f, uint32_t radialSegments = 10, uint32_t heightSegments = 1);

        static std::shared_ptr<StaticMesh> CreateCube(float size = 2.0f);

        static std::shared_ptr<StaticMesh> CreateCylinder(float bottomRadius = 1.0f, float topRadius = 1.0f, float height = 2.0f, uint32_t radialSegments = 32, uint32_t heightSegments = 1);

        static std::shared_ptr<StaticMesh> CreateGrid(float width = 10.0f, float depth = 10.0f, uint32_t segmentsX = 10, uint32_t segmentsZ = 10);

        static std::shared_ptr<StaticMesh> CreateHemisphere(float radius = 1.0f, uint32_t sectors = 32, uint32_t stacks = 16);

        static std::shared_ptr<StaticMesh> CreatePyramid(float baseSize = 1.0f, float height = 2.0f);

        static std::shared_ptr<StaticMesh> CreateProxyPyramid(float baseSize = 1.0f, float height = 2.0f);

        static std::shared_ptr<StaticMesh> CreateQuad(float width = 1.0f, float height = 1.0f);

        static std::shared_ptr<StaticMesh> CreateScreenQuad(float width = 1.0f, float height = 1.0f);

        static std::shared_ptr<StaticMesh> CreateSphere(float radius = 1.0f, uint32_t sectors = 32, uint32_t stacks = 32);

        static std::shared_ptr<StaticMesh> CreateProxySphere(float radius = 1.0f, uint32_t sectors = 4, uint32_t stacks = 4);

        static std::shared_ptr<StaticMesh> CreateTorus(float mainRadius = 1.0f, float tubeRadius = 0.3f, uint32_t mainSegments = 48, uint32_t tubeSegments = 24);
    
        static std::shared_ptr<StaticMesh> CreateIcoSphere(float radius = 1.0f, uint32_t subdivisions = 3);

        static std::shared_ptr<StaticMesh> CreateProxyIcoSphere(float radius = 1.0f, uint32_t subdivisions = 1);
    };
}