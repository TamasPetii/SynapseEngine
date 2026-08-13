// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"

#include "Engine/Component/Physics/BoxColliderComponent.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"
#include "Engine/Component/Physics/CapsuleColliderComponent.h"
#include "Engine/Component/Physics/ConvexColliderComponent.h"
#include "Engine/Component/Physics/MeshColliderComponent.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"

#include "Engine/Physics/PhysicsTypes.h"

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

    SYN_REGISTER_COMPONENT(Syn::ConvexColliderComponent, "ConvexColliderComponent");

    template <>
    struct SYN_API Schema<ConvexColliderComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("targetLodLevel", comp.targetLodLevel);
            ar.Property("localOffset", comp.localOffset);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::MeshColliderComponent, "MeshColliderComponent");

    template <>
    struct SYN_API Schema<MeshColliderComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("targetLodLevel", comp.targetLodLevel);
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