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

            ar.Property("isMeshletPipeline", b.isMeshletPipeline);

            if (b.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_TRADITIONAL) {
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