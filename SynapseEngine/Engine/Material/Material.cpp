#include "Material.h"
#include "Engine/SynMacro.h"
#include <limits>

namespace Syn {
    GpuMaterial::GpuMaterial()
        : color(1.0f, 1.0f, 1.0f, 1.0f)
        , emissiveColor(0.0f, 0.0f, 0.0f)
        , emissiveIntensity(1.0f)
        , uvScale(1.0f, 1.0f)
        , metalness(0.0f)
        , roughness(1.0f)
        , aoStrength(1.0f)
        , packedFlags(0)
        , albedoTexture(UINT32_MAX)
        , normalTexture(UINT32_MAX)
        , metalnessTexture(UINT32_MAX)
        , roughnessTexture(UINT32_MAX)
        , metallicRoughnessTexture(UINT32_MAX)
        , emissiveTexture(UINT32_MAX)
        , ambientOcclusionTexture(UINT32_MAX)
        , padding0(0)
        , padding1(0)
        , padding2(0)
    {
    }

    SYN_INLINE uint32_t PackTextureAndSampler(uint32_t textureIdx, uint32_t samplerIdx) {
        if (textureIdx == UINT32_MAX) 
            return UINT32_MAX;

        uint32_t tex = textureIdx & 0x00FFFFFF;
        uint32_t samp = INVALID_SAMPLER_INDEX;

        if (samplerIdx != UINT32_MAX) {
            samp = samplerIdx & 0xFF;
        }

        return (samp << 24) | tex;
    }

    GpuMaterial::GpuMaterial(const Material& material)
        : color(material.color)
        , emissiveColor(material.emissiveColor)
        , emissiveIntensity(material.emissiveIntensity)
        , uvScale(material.uvScale)
        , metalness(material.metalness)
        , roughness(material.roughness)
        , aoStrength(material.aoStrength)
        , albedoTexture(PackTextureAndSampler(material.albedoTexture, material.albedoSampler))
        , normalTexture(PackTextureAndSampler(material.normalTexture, material.normalSampler))
        , metalnessTexture(PackTextureAndSampler(material.metalnessTexture, material.metalnessSampler))
        , roughnessTexture(PackTextureAndSampler(material.roughnessTexture, material.roughnessSampler))
        , metallicRoughnessTexture(PackTextureAndSampler(material.metallicRoughnessTexture, material.metallicRoughnessSampler))
        , emissiveTexture(PackTextureAndSampler(material.emissiveTexture, material.emissiveSampler))
        , ambientOcclusionTexture(PackTextureAndSampler(material.ambientOcclusionTexture, material.ambientOcclusionSampler))
        , padding0(0)
        , padding1(0)
        , padding2(0)
    {
        uint32_t flags = 0;

        if (material.doubleSided)   flags |= (1 << 0);
        if (material.isTransparent) flags |= (1 << 1);

        this->packedFlags = flags;
    }
}