#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "GpuVertexDataSchema.h"
#include "GpuIndexedDrawDataSchema.h"
#include "GpuMeshletDrawDataSchema.h"
#include "MaterialInfoSchema.h"
#include "MeshDrawBlueprintSchema.h"

#include "Engine/Mesh/Data/Cpu/CpuModelData.h"

namespace Syn
{
    template <>
    struct Schema<CpuModelData> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& m = const_cast<std::remove_const_t<U>&>(val);


            ar.Property("globalVertexCount", m.globalVertexCount);
            ar.Property("globalIndexCount", m.globalIndexCount);
            ar.Property("globalMeshCount", m.globalMeshCount);
            ar.Property("globalAverageLodIndexCount", m.globalAverageLodIndexCount);

            ar.Property("globalCollider", m.globalCollider);
            ar.Property("meshColliders", m.meshColliders);
            ar.Property("meshDescriptors", m.meshDescriptors);
            ar.Property("meshletDrawDescriptors", m.meshletDrawDescriptors);
            ar.Property("baseDrawCommands", m.baseDrawCommands);
            ar.Property("meshMaterialIndices", m.meshMaterialIndices);

            if (ar.IsBinary()) {
                BlitVector<glm::vec3> verts{ m.vertices };
                ar.Property("vertices", verts);

                BlitVector<uint32_t> inds{ m.indices };
                ar.Property("indices", inds);

                BlitVector<glm::vec3> physVerts{ m.physicsVertices };
                ar.Property("physicsVertices", physVerts);
            }
            else {
                ar.Property("vertices", m.vertices);
                ar.Property("indices", m.indices);
                ar.Property("physicsVertices", m.physicsVertices);
            }

            ar.Property("meshletVertexIndices", m.meshletVertexIndices);
            ar.Property("meshletTriangleIndices", m.meshletTriangleIndices);
            ar.Property("meshletDescriptors", m.meshletDescriptors);
            ar.Property("physicsIndicesPerLod", m.physicsIndicesPerLod);
        }
    };
}