#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API Material {
        glm::vec4 color = glm::vec4(1.0f);
        glm::vec3 emissiveColor = glm::vec3(0.0f);
        float emissiveIntensity = 1.0f;
        glm::vec2 uvScale = glm::vec2(1.0f);
        float metalness = 0.0f;
        float roughness = 1.0f;
        float aoStrength = 1.0f;
        bool useBloom = false;
        bool doubleSided = false;
        bool isTransparent = false;

        uint32_t albedoTexture = UINT32_MAX;
        uint32_t normalTexture = UINT32_MAX;
        uint32_t metalnessTexture = UINT32_MAX;
        uint32_t roughnessTexture = UINT32_MAX;
        uint32_t metallicRoughnessTexture = UINT32_MAX;
        uint32_t emissiveTexture = UINT32_MAX;
        uint32_t ambientOcclusionTexture = UINT32_MAX;
    };

    struct SYN_API GpuMaterial {
        GpuMaterial(const Material& material);

        glm::vec4 color;
        glm::vec3 emissiveColor;
        float emissiveIntensity;
        glm::vec2 uvScale;
        float metalness;
        float roughness;
        float aoStrength;
        uint32_t packedFlags;
        uint32_t albedoTexture;
        uint32_t normalTexture;
        uint32_t metalnessTexture;
        uint32_t roughnessTexture;
        uint32_t metallicRoughnessTexture;
        uint32_t emissiveTexture;
        uint32_t ambientOcclusionTexture;
        uint32_t padding0;
        uint32_t padding1;
        uint32_t padding2;
        //Todo: uint16_t?
    };
}

