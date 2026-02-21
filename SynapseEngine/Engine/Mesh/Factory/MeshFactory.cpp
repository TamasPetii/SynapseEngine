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
#include "Engine/Mesh/Source/Procedural/Shape/QuadMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/ScreenQuadMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/SphereMeshSource.h"
#include "Engine/Mesh/Source/Procedural/Shape/TorusMeshSource.h"

namespace Syn
{
    std::shared_ptr<StaticMesh> MeshFactory::LoadFromFile(const std::string& filePath)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        return builder->BuildFromFile(filePath);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCapsule(float radius, float height, uint32_t sides, uint32_t hemisphereSegments)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        CapsuleMeshSource source(radius, height, sides, hemisphereSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCone(float radius, float height, uint32_t radialSegments, uint32_t heightSegments)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        ConeMeshSource source(radius, height, radialSegments, heightSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCube(float size)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        CubeMeshSource source(size);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t radialSegments, uint32_t heightSegments)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        CylinderMeshSource source(bottomRadius, topRadius, height, radialSegments, heightSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateGrid(float width, float depth, uint32_t segmentsX, uint32_t segmentsZ)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        GridMeshSource source(width, depth, segmentsX, segmentsZ);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateHemisphere(float radius, uint32_t sectors, uint32_t stacks)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        HemisphereMeshSource source(radius, sectors, stacks);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreatePyramid(float baseRadius, float height, uint32_t heightSegments)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        PyramidMeshSource source(baseRadius, height, heightSegments);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateQuad(float width, float height)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        QuadMeshSource source(width, height);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateScreenQuad(float width, float height)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        ScreenQuadMeshSource source(width, height);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateSphere(float radius, uint32_t sectors, uint32_t stacks)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        SphereMeshSource source(radius, sectors, stacks);
        return builder->BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> MeshFactory::CreateTorus(float mainRadius, float tubeRadius, uint32_t mainSegments, uint32_t tubeSegments)
    {
        auto builder = ServiceLocator::GetStaticMeshBuilder();
        if (!builder) return nullptr;

        TorusMeshSource source(mainRadius, tubeRadius, mainSegments, tubeSegments);
        return builder->BuildFromSource(source);
    }
}