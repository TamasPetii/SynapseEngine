#pragma once
#include "Engine/SynApi.h"
#include "ShapeMeshSource.h"

namespace Syn
{
    class SYN_API CylinderMeshSource : public ShapeMeshSource
    {
    public:
        CylinderMeshSource(float bottomRadius = 1.0f, float topRadius = 1.0f, float height = 2.0f, uint32_t radialSegments = 32, uint32_t heightSegments = 1);
        virtual ~CylinderMeshSource() override = default;
    protected:
        virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) override;
        virtual void GenerateIndices(std::vector<uint32_t>& outIndices) override;
        virtual void GenerateUVs(std::span<glm::vec2> outUVs) override;
        virtual void GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals) override;
    private:
        float _bottomRadius;
        float _topRadius;
        float _height;
        uint32_t _radialSegments;
        uint32_t _heightSegments;
    };
}