#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    enum class SYN_API SkyMode
    {
        None = 0,
        EquirectangularTexture = 1,
        OctahedralTexture = 2,
        Procedural = 3
    };

    struct SYN_API EnvironmentSettings
    {
        EnvironmentSettings();

        bool enableSky;
        SkyMode skyMode;

        uint32_t skyTextureId;
        float skyIntensity;
        float skyExposureEV;
        glm::vec3 skyTint;
        glm::vec3 skyRotation;

        float ambientIntensity;
        uint32_t irradianceTextureId;
        uint32_t prefilteredTextureId;
        uint32_t brdfLutTextureId;

        bool enableFog;
        glm::vec3 fogColor;
        float fogDensity;
    };
}