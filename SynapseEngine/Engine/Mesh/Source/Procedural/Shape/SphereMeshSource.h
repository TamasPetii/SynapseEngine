#pragma once
#include "Engine/SynApi.h"
#include "ShapeMeshSource.h"

namespace Syn
{
    class SYN_API SphereMeshSource : public ShapeMeshSource
    {
    public:
        SphereMeshSource(float radius = 1.0f, uint32_t sectors = 32, uint32_t stacks = 32);
        virtual ~SphereMeshSource() override = default;
    protected:
        virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) override;
        virtual void GenerateIndices(std::vector<uint32_t>& outIndices) override;
        virtual void GenerateUVs(std::vector<glm::vec2>& outUVs) override;
        virtual void GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals) override;
    private:
        float _radius;
        uint32_t _sectors;
        uint32_t _stacks;
    };
}