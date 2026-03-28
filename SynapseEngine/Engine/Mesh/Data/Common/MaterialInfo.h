#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API MaterialInfo {
        std::string name = "";
        std::string albedoPath = "";
        std::string normalPath = "";
        std::string metalnessPath = "";
        std::string roughnessPath = "";
        std::string metallicRoughnessPath = "";
        std::string emissivePath = "";
        std::string ambientOcclusionPath = "";

        glm::vec4 color = glm::vec4(1.0f);
        glm::vec3 emissiveFactor = glm::vec3(0.0f);
        float emissiveIntensity = 1.0f;
        glm::vec2 uvScale = glm::vec2(1.0f);
        float metallicFactor = 0.0f;
        float roughnessFactor = 1.0f;
        float aoStrength = 1.0f;
        bool doubleSided = false;
        bool isTransparent = false;
    };
}