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