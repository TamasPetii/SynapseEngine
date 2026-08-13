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
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("shadowFarPlane", comp.shadowFarPlane);
            ar.Property("cascadeSplits", comp.cascadeSplits);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::PointLightShadowComponent, "PointLightShadowComponent");

    template <>
    struct Schema<PointLightShadowComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);
            
            ar.Property("nearPlane", comp.nearPlane);
            ar.Property("farPlane", comp.farPlane);
        }
    };

    SYN_REGISTER_COMPONENT(Syn::SpotLightShadowComponent, "SpotLightShadowComponent");

    template <>
    struct Schema<SpotLightShadowComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("nearPlane", comp.nearPlane);
            ar.Property("farPlane", comp.farPlane);
        }
    };
}