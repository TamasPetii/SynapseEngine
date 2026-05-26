#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::TransformComponent, "TransformComponent");

    template <>
    struct Schema<TransformComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("translation", comp.translation);
            ar.Property("rotation", comp.rotation);
            ar.Property("scale", comp.scale);
        }
    };
}