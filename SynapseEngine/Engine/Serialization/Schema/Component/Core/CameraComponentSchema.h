#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Component/Core/CameraComponent.h"

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::CameraComponent, "CameraComponent");

    template <>
    struct Schema<CameraComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("nearPlane", comp.nearPlane);
            ar.Property("farPlane", comp.farPlane);
            ar.Property("fov", comp.fov);
            ar.Property("width", comp.width);
            ar.Property("height", comp.height);
            ar.Property("speed", comp.speed);
            ar.Property("sensitivity", comp.sensitivity);
            ar.Property("distance", comp.distance);
        }
    };
}