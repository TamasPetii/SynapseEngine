#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::DirectionLightComponent, "DirectionLightComponent");

    template <>
    struct Schema<DirectionLightComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("color", val.color);
            ar.Property("strength", val.strength);
            ar.Property("useShadow", val.useShadow);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::PointLightComponent, "PointLightComponent");

    template <>
    struct Schema<PointLightComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("color", val.color);
            ar.Property("strength", val.strength);
            ar.Property("useShadow", val.useShadow);
            ar.Property("radius", val.radius);
            ar.Property("weakenDistance", val.weakenDistance);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::SpotLightComponent, "SpotLightComponent");

    template <>
    struct Schema<SpotLightComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("color", val.color);
            ar.Property("strength", val.strength);
            ar.Property("useShadow", val.useShadow);
            ar.Property("range", val.range);
            ar.Property("weakenDistance", val.weakenDistance);
            ar.Property("innerAngle", val.innerAngle);
            ar.Property("outerAngle", val.outerAngle);
        }
    };
}