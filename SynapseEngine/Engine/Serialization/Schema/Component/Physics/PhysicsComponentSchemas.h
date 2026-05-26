#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"

#include "Engine/Component/Physics/BoxColliderComponent.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"
#include "Engine/Component/Physics/CapsuleColliderComponent.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"

#include <type_traits>

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::BoxColliderComponent, "BoxColliderComponent");

    template <>
    struct SYN_API Schema<BoxColliderComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("halfExtents", comp.halfExtents);
            ar.Property("localOffset", comp.localOffset);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::SphereColliderComponent, "SphereColliderComponent");

    template <>
    struct SYN_API Schema<SphereColliderComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("radius", comp.radius);
            ar.Property("localOffset", comp.localOffset);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::CapsuleColliderComponent, "CapsuleColliderComponent");

    template <>
    struct SYN_API Schema<CapsuleColliderComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("radius", comp.radius);
            ar.Property("halfHeight", comp.halfHeight);
            ar.Property("localOffset", comp.localOffset);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::RigidBodyComponent, "RigidBodyComponent");

    template <>
    struct SYN_API Schema<RigidBodyComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("motionType", reinterpret_cast<std::underlying_type_t<PhysicsMotionType>&>(comp.motionType));
            ar.Property("mass", comp.mass);
            ar.Property("friction", comp.friction);
            ar.Property("restitution", comp.restitution);
            ar.Property("layer", comp.layer);
        }
    };
}