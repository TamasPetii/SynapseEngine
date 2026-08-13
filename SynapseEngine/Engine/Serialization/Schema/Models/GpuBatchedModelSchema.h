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
#include "GpuVertexDataSchema.h"
#include "GpuIndexedDrawDataSchema.h"
#include "GpuMeshletDrawDataSchema.h"
#include "MaterialInfoSchema.h"
#include "MeshInstanceDescriptorSchema.h"

#include "Engine/Mesh/Data/Gpu/GpuNodeTransform.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"

namespace Syn
{
    template <> 
    struct Schema<GpuNodeTransform> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& t = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("transform", t.transform); 
            ar.Property("transformIT", t.transformIT);
        }
    };

    template <> 
    struct Schema<GpuBatchedModel> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& m = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("vertexData", m.vertexData);
            ar.Property("indexedData", m.indexedData);
            ar.Property("meshletData", m.meshletData);
            ar.Property("materials", m.materials);

            if (ar.IsBinary()) {
                BlitVector<GpuNodeTransform> nodeTransforms{ m.nodeTransforms };
                ar.Property("nodeTransforms", nodeTransforms);
            }
            else {
                ar.Property("nodeTransforms", m.nodeTransforms);
            }

            ar.Property("globalCollider", m.globalCollider);
            ar.Property("globalVertexCount", m.globalVertexCount);
            ar.Property("globalIndexCount", m.globalIndexCount);
            ar.Property("globalLod0IndexCount", m.globalLod0IndexCount);
            ar.Property("globalAverageLodIndexCount", m.globalAverageLodIndexCount);
            ar.Property("globalMeshCount", m.globalMeshCount);
            ar.Property("meshNodeDescriptors", m.meshNodeDescriptors);
        }
    };
}