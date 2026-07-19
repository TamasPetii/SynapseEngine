#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Syn
{
    constexpr uint32_t INVALID_SAMPLER_INDEX = 0xFF;

    struct SYN_API Material {
        glm::vec4 color = glm::vec4(1.0f);
        glm::vec3 emissiveColor = glm::vec3(0.0f);
        float emissiveIntensity = 1.0f;
        glm::vec2 uvScale = glm::vec2(1.0f);
        float metalness = 0.0f;
        float roughness = 1.0f;
        float aoStrength = 1.0f;
        bool doubleSided = false;
        bool isTransparent = false;
        bool isAlphaTested = false;

        uint32_t albedoTexture = UINT32_MAX;
        uint32_t albedoSampler = UINT32_MAX;

        uint32_t normalTexture = UINT32_MAX;
        uint32_t normalSampler = UINT32_MAX;

        uint32_t metalnessTexture = UINT32_MAX;
        uint32_t metalnessSampler = UINT32_MAX;

        uint32_t roughnessTexture = UINT32_MAX;
        uint32_t roughnessSampler = UINT32_MAX;

        uint32_t metallicRoughnessTexture = UINT32_MAX;
        uint32_t metallicRoughnessSampler = UINT32_MAX;

        uint32_t emissiveTexture = UINT32_MAX;
        uint32_t emissiveSampler = UINT32_MAX;

        uint32_t ambientOcclusionTexture = UINT32_MAX;
        uint32_t ambientOcclusionSampler = UINT32_MAX;

        uint32_t opacityTexture = UINT32_MAX;
        uint32_t opacitySampler = UINT32_MAX;
    };

    struct SYN_API GpuMaterial {
        GpuMaterial();
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
        uint32_t opacityTexture;
        uint32_t padding1;
        uint32_t padding2;
    };
}

