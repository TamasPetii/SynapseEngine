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
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("color", comp.color);
            ar.Property("strength", comp.strength);
            ar.Property("useShadow", comp.useShadow);
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
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("color", comp.color);
            ar.Property("strength", comp.strength);
            ar.Property("useShadow", comp.useShadow);
            ar.Property("radius", comp.radius);
            ar.Property("weakenDistance", comp.weakenDistance);
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
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("color", comp.color);
            ar.Property("strength", comp.strength);
            ar.Property("useShadow", comp.useShadow);
            ar.Property("range", comp.range);
            ar.Property("weakenDistance", comp.weakenDistance);
            ar.Property("innerAngle", comp.innerAngle);
            ar.Property("outerAngle", comp.outerAngle);
        }
    };
}