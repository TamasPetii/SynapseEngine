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

#include "Engine/Mesh/MeshDrawBlueprint.h"

namespace Syn
{
    template <>
    struct Schema<MeshDrawBlueprint> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& b = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("pipelineRenderType", b.pipelineRenderType);

            if (b.pipelineRenderType == PipelineRenderType::Traditional) {
                ar.Property("vertexCount", b.traditionalCmd.vertexCount);
                ar.Property("instanceCount", b.traditionalCmd.instanceCount);
                ar.Property("firstVertex", b.traditionalCmd.firstVertex);
                ar.Property("firstInstance", b.traditionalCmd.firstInstance);
            }
            else {
                ar.Property("groupCountX", b.meshletCmd.groupCountX);
                ar.Property("groupCountY", b.meshletCmd.groupCountY);
                ar.Property("groupCountZ", b.meshletCmd.groupCountZ);
            }
        }
    };
}