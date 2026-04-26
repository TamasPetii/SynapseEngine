#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Syn
{
    struct TexturePayload {
        std::string path = "";
        std::string formatHint = "";
        std::vector<uint8_t> embeddedData;
        bool isUncompressed = false;
        uint32_t width = 0;
        uint32_t height = 0;

        bool IsEmbedded() const { return !embeddedData.empty(); }
    };

    struct SYN_API MaterialInfo {
        std::string name = "";
        TexturePayload albedo;
        TexturePayload normal;
        TexturePayload metalness;
        TexturePayload roughness;
        TexturePayload metallicRoughness;
        TexturePayload emissive;
        TexturePayload ambientOcclusion;

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