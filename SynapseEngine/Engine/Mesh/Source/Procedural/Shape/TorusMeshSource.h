#pragma once
#include "Engine/SynApi.h"
#include "ShapeMeshSource.h"

namespace Syn
{
    class SYN_API TorusMeshSource : public ShapeMeshSource
    {
    public:
        TorusMeshSource(float mainRadius = 1.0f, float tubeRadius = 0.3f, uint32_t mainSegments = 48, uint32_t tubeSegments = 24);
        virtual ~TorusMeshSource() override = default;
    protected:
        virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) override;
        virtual void GenerateIndices(std::vector<uint32_t>& outIndices) override;
        virtual void GenerateUVs(std::vector<glm::vec2>& outUVs) override;
        virtual void GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals) override;
    private:
        float _mainRadius;
        float _tubeRadius;
        uint32_t _mainSegments;
        uint32_t _tubeSegments;
    };
}