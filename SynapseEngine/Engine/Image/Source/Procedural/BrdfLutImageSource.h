#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include "Engine/Image/Source/Procedural/ProceduralImageSource.h"

namespace Syn
{
    class SYN_API BrdfLutImageSource : public ProceduralImageSource
    {
    public:
        BrdfLutImageSource(uint32_t resolution = 512, uint32_t sampleCount = 1024);
        ~BrdfLutImageSource() override = default;

        std::optional<RawImage> Produce() override;
    private:
        float RadicalInverse_VdC(uint32_t bits);
        glm::vec2 Hammersley(uint32_t i, uint32_t N);
        glm::vec3 ImportanceSampleGGX(glm::vec2 Xi, glm::vec3 N, float roughness);
        float GeometrySchlickGGX(float NdotV, float roughness);
        float GeometrySmith(glm::vec3 N, glm::vec3 V, glm::vec3 L, float roughness);
        glm::vec2 IntegrateBRDF(float NdotV, float roughness);
    private:
        uint32_t _resolution;
        uint32_t _sampleCount;
    };
}