#pragma once
#include <string>
#include <vector>
#include "Engine/Material/Material.h"

namespace Syn {
    struct TextureOption {
        uint32_t id;
        std::string name;
    };

    struct MaterialPropertiesState {
        bool hasSelection = false;
        uint32_t selectedMaterialId = 0xFFFFFFFF;
        std::string materialName = "";

        Material materialData;
        std::vector<TextureOption> availableTextures;

        std::string albedoName = "None";
        std::string normalName = "None";
        std::string metalnessName = "None";
        std::string roughnessName = "None";
        std::string metallicRoughnessName = "None";
        std::string emissiveName = "None";
        std::string aoName = "None";
    };
}