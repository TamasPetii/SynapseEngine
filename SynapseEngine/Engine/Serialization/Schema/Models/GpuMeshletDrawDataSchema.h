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
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Mesh/Data/Gpu/GpuMeshletDrawData.h"

namespace Syn
{
    template <> 
    struct Schema<GpuMeshletDescriptor> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("vertexIndicesOffset", d.vertexIndicesOffset); 
            ar.Property("vertexCount", d.vertexCount);
            ar.Property("triangleIndicesOffset", d.triangleIndicesOffset); 
            ar.Property("triangleCount", d.triangleCount);
        }
    };

    template <> 
    struct Schema<GpuMeshletCollider> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& c = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("center", c.center); 
            ar.Property("radius", c.radius);
            ar.Property("aabbMin", c.aabbMin); 
            ar.Property("aabbMax", c.aabbMax);
            ar.Property("apex", c.apex); 
            ar.Property("cutoff", c.cutoff);
            ar.Property("axis", c.axis);
        }
    };

    template <> 
    struct Schema<GpuMeshletDrawDescriptor> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("meshletOffset", d.meshletOffset); 
            ar.Property("meshletCount", d.meshletCount);
            ar.Property("materialIndex", d.materialIndex);
        }
    };

    template <> 
    struct Schema<GpuMeshletDrawData> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);
            if (ar.IsBinary()) 
            {
                BlitVector<uint32_t> vertexIndices{ d.vertexIndices };
                ar.Property("vertexIndices", vertexIndices);

                BlitVector<uint8_t> triangleIndices{ d.triangleIndices };
                ar.Property("triangleIndices", triangleIndices);

                BlitVector<GpuMeshletDescriptor> meshletDescriptors{ d.meshletDescriptors };
                ar.Property("meshletDescriptors", meshletDescriptors);

                BlitVector<GpuMeshletDrawDescriptor> drawDescriptors{ d.drawDescriptors };
                ar.Property("drawDescriptors", drawDescriptors);

                BlitVector<GpuMeshletCollider> meshletColliders{ d.meshletColliders };
                ar.Property("meshletColliders", meshletColliders);
            }
            else {
                ar.Property("vertexIndices", d.vertexIndices);
                ar.Property("triangleIndices", d.triangleIndices);
                ar.Property("meshletDescriptors", d.meshletDescriptors);
                ar.Property("drawDescriptors", d.drawDescriptors);
                ar.Property("meshletColliders", d.meshletColliders);
            }
        }
    };
}