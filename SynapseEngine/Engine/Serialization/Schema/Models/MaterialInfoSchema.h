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
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Mesh/Data/Common/MaterialInfo.h"

namespace Syn
{
    template <>
    struct Schema<TexturePayload> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& t = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("path", t.path);
            ar.Property("formatHint", t.formatHint);

            if (ar.IsBinary()) {
                BlitVector<uint8_t> embeddedData{ t.embeddedData };
                ar.Property("embeddedData", embeddedData);
            }
            else {
                ar.Property("embeddedData", t.embeddedData);
            }

            ar.Property("isUncompressed", t.isUncompressed);
            ar.Property("width", t.width);
            ar.Property("height", t.height);
        }
    };

    template <>
    struct Schema<MaterialInfo> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& m = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("name", m.name);
            ar.Property("albedo", m.albedo);
            ar.Property("normal", m.normal);
            ar.Property("metalness", m.metalness);
            ar.Property("roughness", m.roughness);
            ar.Property("metallicRoughness", m.metallicRoughness);
            ar.Property("emissive", m.emissive);
            ar.Property("ambientOcclusion", m.ambientOcclusion);
            ar.Property("opacity", m.opacity);

            ar.Property("color", m.color);
            ar.Property("emissiveFactor", m.emissiveFactor);
            ar.Property("emissiveIntensity", m.emissiveIntensity);
            ar.Property("uvScale", m.uvScale);
            ar.Property("metallicFactor", m.metallicFactor);
            ar.Property("roughnessFactor", m.roughnessFactor);
            ar.Property("aoStrength", m.aoStrength);
            ar.Property("doubleSided", m.doubleSided);
            ar.Property("isTransparent", m.isTransparent);
            ar.Property("isAlphaTested", m.isAlphaTested);
        }
    };
}