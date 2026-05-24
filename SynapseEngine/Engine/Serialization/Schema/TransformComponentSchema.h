#pragma once
#include "Schema.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn
{
    template <>
    struct Schema<TransformComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            ar.Property("translation", val.translation);
            ar.Property("rotation", val.rotation);
            ar.Property("scale", val.scale);
        }
    };
}