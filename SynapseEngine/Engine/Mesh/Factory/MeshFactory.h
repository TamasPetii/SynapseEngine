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

        static std::shared_ptr<StaticMesh> CreateCube(float size = 2.0f);

        static std::shared_ptr<StaticMesh> CreateCylinder(float bottomRadius = 1.0f, float topRadius = 1.0f, float height = 2.0f, uint32_t radialSegments = 32, uint32_t heightSegments = 1);

        static std::shared_ptr<StaticMesh> CreateGrid(float width = 10.0f, float depth = 10.0f, uint32_t segmentsX = 10, uint32_t segmentsZ = 10);

        static std::shared_ptr<StaticMesh> CreateHemisphere(float radius = 1.0f, uint32_t sectors = 32, uint32_t stacks = 16);

        static std::shared_ptr<StaticMesh> CreatePyramid(float baseRadius = 1.0f, float height = 2.0f, uint32_t heightSegments = 1);

        static std::shared_ptr<StaticMesh> CreateQuad(float width = 1.0f, float height = 1.0f);

        static std::shared_ptr<StaticMesh> CreateScreenQuad(float width = 1.0f, float height = 1.0f);

        static std::shared_ptr<StaticMesh> CreateSphere(float radius = 1.0f, uint32_t sectors = 32, uint32_t stacks = 32);

        static std::shared_ptr<StaticMesh> CreateTorus(float mainRadius = 1.0f, float tubeRadius = 0.3f, uint32_t mainSegments = 48, uint32_t tubeSegments = 24);
    };
}