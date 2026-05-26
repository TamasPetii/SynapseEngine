#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::DirectionLightShadowComponent, "DirectionLightShadowComponent");

    template <>
    struct Schema<DirectionLightShadowComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            ar.Property("shadowFarPlane", val.shadowFarPlane);
            ar.Property("cascadeSplits", val.cascadeSplits);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::PointLightShadowComponent, "PointLightShadowComponent");

    template <>
    struct Schema<PointLightShadowComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            ar.Property("nearPlane", val.nearPlane);
            ar.Property("farPlane", val.farPlane);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::SpotLightShadowComponent, "SpotLightShadowComponent");

    template <>
    struct Schema<SpotLightShadowComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            ar.Property("nearPlane", val.nearPlane);
            ar.Property("farPlane", val.farPlane);
        }
    };
}