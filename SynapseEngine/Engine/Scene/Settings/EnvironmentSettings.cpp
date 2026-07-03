#include "EnvironmentSettings.h"

namespace Syn
{
    EnvironmentSettings::EnvironmentSettings()
        : enableSky(true)
        , skyMode(SkyMode::EquirectangularTexture)
        , skyTextureId(UINT32_MAX)
        , skyIntensity(1.0f)
        , skyTint(glm::vec3(1.0f))
        , skyRotation(glm::vec3(0.0f))
        , ambientIntensity(1.0f)
        , irradianceTextureId(UINT32_MAX)
        , prefilteredTextureId(UINT32_MAX)
        , brdfLutTextureId(UINT32_MAX)
        , enableFog(false)
        , fogColor(glm::vec3(0.5f, 0.6f, 0.7f))
        , fogDensity(0.01f)
    {}
}