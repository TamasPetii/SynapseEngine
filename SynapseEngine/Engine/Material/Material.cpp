#include "Material.h"
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

    GpuMaterial::GpuMaterial(const Material& material)
        : color(material.color)
        , emissiveColor(material.emissiveColor)
        , emissiveIntensity(material.emissiveIntensity)
        , uvScale(material.uvScale)
        , metalness(material.metalness)
        , roughness(material.roughness)
        , aoStrength(material.aoStrength)
        , albedoTexture(material.albedoTexture == UINT32_MAX ? UINT32_MAX : material.albedoTexture)
        , normalTexture(material.normalTexture == UINT32_MAX ? UINT32_MAX : material.normalTexture)
        , metalnessTexture(material.metalnessTexture == UINT32_MAX ? UINT32_MAX : material.metalnessTexture)
        , roughnessTexture(material.roughnessTexture == UINT32_MAX ? UINT32_MAX : material.roughnessTexture)
        , metallicRoughnessTexture(material.metallicRoughnessTexture == UINT32_MAX ? UINT32_MAX : material.metallicRoughnessTexture)
        , emissiveTexture(material.emissiveTexture == UINT32_MAX ? UINT32_MAX : material.emissiveTexture)
        , ambientOcclusionTexture(material.ambientOcclusionTexture == UINT32_MAX ? UINT32_MAX : material.ambientOcclusionTexture)
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