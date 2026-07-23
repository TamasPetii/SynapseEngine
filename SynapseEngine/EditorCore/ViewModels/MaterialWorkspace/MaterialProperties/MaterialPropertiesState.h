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