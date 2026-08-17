#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Texture.h"
#include <glm/glm.hpp>
#include <memory>

namespace Syn 
{
    struct EnvironmentData {
        uint32_t baseCubemapIndex;
        uint32_t irradianceCubeIndex;
        uint32_t prefilteredCubeIndex;
        uint32_t brdfLutTexIndex;
        glm::vec3 skyTint;
        float intensity;
    };

    struct SYN_API Environment {
        std::shared_ptr<Texture> baseCubemap;
        std::shared_ptr<Texture> irradianceMap;
        std::shared_ptr<Texture> prefilteredMap;

        float intensity = 1.0f;
        glm::vec3 skyTint = glm::vec3(1.0f);

        std::shared_ptr<Texture> transientSourceImage;
    };
}