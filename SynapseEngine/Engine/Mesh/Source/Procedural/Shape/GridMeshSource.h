#pragma once
#include "Engine/SynApi.h"
#include "ShapeMeshSource.h"

namespace Syn
{
    class SYN_API GridMeshSource : public ShapeMeshSource
    {
    public:
        GridMeshSource(float width = 10.0f, float depth = 10.0f, uint32_t segmentsX = 10, uint32_t segmentsZ = 10);
        virtual ~GridMeshSource() override = default;
    protected:
        virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) override;
        virtual void GenerateIndices(std::vector<uint32_t>& outIndices) override;
        virtual void GenerateUVs(std::vector<glm::vec2>& outUVs) override;
        virtual void GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals) override;
    private:
        float _width;
        float _depth;
        uint32_t _segmentsX;
        uint32_t _segmentsZ;
    };
}