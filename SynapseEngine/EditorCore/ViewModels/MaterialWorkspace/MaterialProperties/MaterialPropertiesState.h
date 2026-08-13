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
#include <string>
#include <vector>
#include <cstdint>
#include "Engine/Material/Material.h"

namespace Syn 
{
    struct TextureOption {
        uint32_t id;
        std::string name;
    };

    struct SamplerOption {
        uint32_t id;
        std::string name;
    };

    struct MaterialPropertiesState {
        bool hasSelection = false;
        uint32_t selectedMaterialId = 0xFFFFFFFF;
        std::string materialName = "";

        Material materialData;
        std::vector<TextureOption> availableTextures;
        std::vector<SamplerOption> availableSamplers;

        std::string albedoName = "None";
        std::string albedoSamplerName = "Default";

        std::string normalName = "None";
        std::string normalSamplerName = "Default";

        std::string metalnessName = "None";
        std::string metalnessSamplerName = "Default";

        std::string roughnessName = "None";
        std::string roughnessSamplerName = "Default";

        std::string metallicRoughnessName = "None";
        std::string metallicRoughnessSamplerName = "Default";

        std::string emissiveName = "None";
        std::string emissiveSamplerName = "Default";

        std::string aoName = "None";
        std::string aoSamplerName = "Default";

        std::string opacityName = "None";
        std::string opacitySamplerName = "Default";

        std::string clearcoatName = "None";
        std::string clearcoatSamplerName = "Default";

        std::string clearcoatRoughnessName = "None";
        std::string clearcoatRoughnessSamplerName = "Default";

        std::string clearcoatNormalName = "None";
        std::string clearcoatNormalSamplerName = "Default";

        std::string specularName = "None";
        std::string specularSamplerName = "Default";

        std::string specularColorName = "None";
        std::string specularColorSamplerName = "Default";
    };
}