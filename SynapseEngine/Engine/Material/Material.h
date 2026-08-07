#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace Syn
{
    constexpr uint32_t INVALID_SAMPLER_INDEX = 0xFF;

    struct SYN_API Material 
    {
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

        float clearcoatFactor = 0.0f;
        float clearcoatRoughness = 0.0f;
        float ior = 1.5f;
        float specularFactor = 1.0f;
        glm::vec3 specularColor = glm::vec3(1.0f);

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

        uint32_t clearcoatTexture = UINT32_MAX;
        uint32_t clearcoatSampler = UINT32_MAX;

        uint32_t clearcoatRoughnessTexture = UINT32_MAX;
        uint32_t clearcoatRoughnessSampler = UINT32_MAX;

        uint32_t clearcoatNormalTexture = UINT32_MAX;
        uint32_t clearcoatNormalSampler = UINT32_MAX;

        uint32_t specularTexture = UINT32_MAX;
        uint32_t specularSampler = UINT32_MAX;

        uint32_t specularColorTexture = UINT32_MAX;
        uint32_t specularColorSampler = UINT32_MAX;

        uint32_t videoTexture = UINT32_MAX;
        uint32_t videoSampler = UINT32_MAX;
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
        float clearcoatFactor;
        float clearcoatRoughness;
        glm::vec3 specularColor;
        float specularFactor;
        float ior;
        uint32_t albedoTexture;
        uint32_t normalTexture;
        uint32_t metalnessTexture;
        uint32_t roughnessTexture;
        uint32_t metallicRoughnessTexture;
        uint32_t emissiveTexture;
        uint32_t ambientOcclusionTexture;
        uint32_t opacityTexture;
        uint32_t clearcoatTexture;
        uint32_t clearcoatRoughnessTexture;
        uint32_t clearcoatNormalTexture;
        uint32_t specularTexture;
        uint32_t specularColorTexture;
        uint32_t videoTexture;
        uint32_t padding1;
    };
}

