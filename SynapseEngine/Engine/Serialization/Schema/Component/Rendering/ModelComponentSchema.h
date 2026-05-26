#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Component/Rendering/ModelComponent.h" 

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::ModelComponent, "ModelComponent");

    template <>
    struct Schema<ModelComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);

            ar.Property("castShadow", val.castShadow);
            ar.Property("receiveShadow", val.receiveShadow);
            ar.Property("hasDirectxNormals", val.hasDirectxNormals);
            ar.Property("modelIndex", val.modelIndex);
            ar.Property("materialOffset", val.materialOffset);
        }
    };
}