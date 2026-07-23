#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Material/Material.h"

namespace Syn
{
    template <>
    struct Schema<Material> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& mat = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("color", mat.color);
            ar.Property("emissiveColor", mat.emissiveColor);
            ar.Property("emissiveIntensity", mat.emissiveIntensity);
            ar.Property("uvScale", mat.uvScale);
            ar.Property("metalness", mat.metalness);
            ar.Property("roughness", mat.roughness);
            ar.Property("aoStrength", mat.aoStrength);
            ar.Property("doubleSided", mat.doubleSided);
            ar.Property("isTransparent", mat.isTransparent);
            ar.Property("isAlphaTested", mat.isAlphaTested);

            ar.Property("clearcoatFactor", mat.clearcoatFactor);
            ar.Property("clearcoatRoughness", mat.clearcoatRoughness);
            ar.Property("ior", mat.ior);
            ar.Property("specularFactor", mat.specularFactor);
            ar.Property("specularColor", mat.specularColor);

            ar.Property("albedoTexture", mat.albedoTexture);
            ar.Property("normalTexture", mat.normalTexture);
            ar.Property("metalnessTexture", mat.metalnessTexture);
            ar.Property("roughnessTexture", mat.roughnessTexture);
            ar.Property("metallicRoughnessTexture", mat.metallicRoughnessTexture);
            ar.Property("emissiveTexture", mat.emissiveTexture);
            ar.Property("ambientOcclusionTexture", mat.ambientOcclusionTexture);
            ar.Property("opacityTexture", mat.opacityTexture);

            ar.Property("clearcoatTexture", mat.clearcoatTexture);
            ar.Property("clearcoatRoughnessTexture", mat.clearcoatRoughnessTexture);
            ar.Property("clearcoatNormalTexture", mat.clearcoatNormalTexture);
            ar.Property("specularTexture", mat.specularTexture);
            ar.Property("specularColorTexture", mat.specularColorTexture);

            ar.Property("albedoSampler", mat.albedoSampler);
            ar.Property("normalSampler", mat.normalSampler);
            ar.Property("metalnessSampler", mat.metalnessSampler);
            ar.Property("roughnessSampler", mat.roughnessSampler);
            ar.Property("metallicRoughnessSampler", mat.metallicRoughnessSampler);
            ar.Property("emissiveSampler", mat.emissiveSampler);
            ar.Property("ambientOcclusionSampler", mat.ambientOcclusionSampler);
            ar.Property("opacitySampler", mat.opacitySampler);

            ar.Property("clearcoatSampler", mat.clearcoatSampler);
            ar.Property("clearcoatRoughnessSampler", mat.clearcoatRoughnessSampler);
            ar.Property("clearcoatNormalSampler", mat.clearcoatNormalSampler);
            ar.Property("specularSampler", mat.specularSampler);
            ar.Property("specularColorSampler", mat.specularColorSampler);
        }
    };
}