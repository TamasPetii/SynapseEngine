#pragma once
#include "Engine/SynApi.h"
#include "ShapeMeshSource.h"

namespace Syn
{
    class SYN_API CapsuleMeshSource : public ShapeMeshSource
    {
    public:
        CapsuleMeshSource(float radius = 0.5f, float height = 1.0f, uint32_t sides = 32, uint32_t hemisphereSegments = 16);
        virtual ~CapsuleMeshSource() override = default;
    protected:
        virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) override;
        virtual void GenerateIndices(std::vector<uint32_t>& outIndices) override;
        virtual void GenerateUVs(std::span<glm::vec2> outUVs) override;
        virtual void GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals) override;
    private:
        float _radius;
        float _height;
        uint32_t _sides;
        uint32_t _hemisphereSegments;
    };
}