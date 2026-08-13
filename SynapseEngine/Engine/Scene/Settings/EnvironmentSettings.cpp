// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "EnvironmentSettings.h"

namespace Syn
{
    EnvironmentSettings::EnvironmentSettings()
        : enableSky(true)
        , skyMode(SkyMode::EquirectangularTexture)
        , skyTextureId(UINT32_MAX)
        , skyIntensity(1.0f)
        , skyExposureEV(0.0f)
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