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
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("castShadow", comp.castShadow);
            ar.Property("receiveShadow", comp.receiveShadow);
            ar.Property("hasDirectxNormals", comp.hasDirectxNormals);
            ar.Property("modelIndex", comp.modelIndex);
            ar.Property("materialOffset", comp.materialOffset);
        }
    };
}