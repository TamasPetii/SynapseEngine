#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::MaterialOverrideComponent, "MaterialOverrideComponent");

    template <>
    struct Schema<MaterialOverrideComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);

            ar.Property("materials", val.materials);
            ar.Property("sharedMaterialEntity", val.sharedMaterialEntity);
        }
    };
}