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

            ar.Property("albedoTexture", mat.albedoTexture);
            ar.Property("normalTexture", mat.normalTexture);
            ar.Property("metalnessTexture", mat.metalnessTexture);
            ar.Property("roughnessTexture", mat.roughnessTexture);
            ar.Property("metallicRoughnessTexture", mat.metallicRoughnessTexture);
            ar.Property("emissiveTexture", mat.emissiveTexture);
            ar.Property("ambientOcclusionTexture", mat.ambientOcclusionTexture);
        }
    };
}