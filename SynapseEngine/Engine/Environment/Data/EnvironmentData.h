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