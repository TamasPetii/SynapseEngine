#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Texture.h"
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include "Engine/Image/SamplerNames.h"

namespace Syn
{
    struct EnvironmentData {
        glm::mat4 skyRotationMatrix;

        glm::vec3 skyTint;
        float skyExposureEV;

        float intensity;
        float ambientIntensity;
        uint32_t packedSamplers;
        uint32_t skyTextureIndex;
    };

    struct SYN_API Environment 
    {
        std::shared_ptr<Texture> baseCubemap;
        std::shared_ptr<Texture> irradianceMap;
        std::shared_ptr<Texture> prefilteredMap;

        float intensity = 1.0f;
        float ambientIntensity = 1.0f;
        float skyExposureEV = 0.0f;
        glm::vec3 skyTint = glm::vec3(1.0f);
        glm::vec3 skyRotation = glm::vec3(0.0f);

        uint32_t skyTextureId = UINT32_MAX;
        std::string cubemapSamplerName = SamplerNames::SkyBoxSampler;
        std::string skySphereSamplerName = SamplerNames::SkySphereSampler;
        std::shared_ptr<Texture> transientSourceImage;
    };
}