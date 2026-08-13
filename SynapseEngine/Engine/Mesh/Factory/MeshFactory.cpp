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

#include "MeshFactory.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"

#include "Engine/Mesh/Source/File/FileMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/CapsuleMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/ConeMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/CubeMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/CylinderMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/GridMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/HemisphereMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/PyramidMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/ProxyPyramidMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/QuadMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/ScreenQuadMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/SphereMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/TorusMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/IcoSphereMeshSource.h"

namespace Syn
{
    std::shared_ptr<StaticMesh> MeshFactory::LoadFromFile(const std::string& filePath)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        return builder->BuildFromFile(filePath);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCapsule(float radius, float height, uint32_t sides, uint32_t hemisphereSegments)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        CapsuleMeshSource source(radius, height, sides, hemisphereSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCone(float radius, float height, uint32_t radialSegments, uint32_t heightSegments)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        ConeMeshSource source(radius, height, radialSegments, heightSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateProxyCone(float radius, float height, uint32_t radialSegments, uint32_t heightSegments)
    {
        return CreateCone(radius, height, radialSegments, heightSegments);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCube(float size)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        CubeMeshSource source(size);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t radialSegments, uint32_t heightSegments)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        CylinderMeshSource source(bottomRadius, topRadius, height, radialSegments, heightSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateGrid(float width, float depth, uint32_t segmentsX, uint32_t segmentsZ)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        GridMeshSource source(width, depth, segmentsX, segmentsZ);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateHemisphere(float radius, uint32_t sectors, uint32_t stacks)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        HemisphereMeshSource source(radius, sectors, stacks);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreatePyramid(float baseSize, float height)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        PyramidMeshSource source(baseSize, height);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateProxyPyramid(float baseSize, float height)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        ProxyPyramidMeshSource source(baseSize, height);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateQuad(float width, float height)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        QuadMeshSource source(width, height);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateScreenQuad(float width, float height)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        ScreenQuadMeshSource source(width, height);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateSphere(float radius, uint32_t sectors, uint32_t stacks)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        SphereMeshSource source(radius, sectors, stacks);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateProxySphere(float radius, uint32_t sectors, uint32_t stacks)
    {
        return CreateSphere(radius, sectors, stacks);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateTorus(float mainRadius, float tubeRadius, uint32_t mainSegments, uint32_t tubeSegments)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        TorusMeshSource source(mainRadius, tubeRadius, mainSegments, tubeSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateIcoSphere(float radius, uint32_t subdivisions)
    {
        auto builder = ServiceLocator::Get<StaticMeshBuilder>();
        if (!builder) return nullptr;

        IcoSphereMeshSource source(radius, subdivisions);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateProxyIcoSphere(float radius, uint32_t subdivisions)
    {
		return CreateIcoSphere(radius, subdivisions);
    }
}