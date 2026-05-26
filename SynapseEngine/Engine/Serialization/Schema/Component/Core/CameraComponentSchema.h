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

            ar.Property("nearPlane", val.nearPlane);
            ar.Property("farPlane", val.farPlane);
            ar.Property("fov", val.fov);
            ar.Property("width", val.width);
            ar.Property("height", val.height);
            ar.Property("speed", val.speed);
            ar.Property("sensitivity", val.sensitivity);
            ar.Property("distance", val.distance);
        }
    };
}