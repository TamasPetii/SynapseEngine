#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "GpuVertexDataSchema.h"
#include "GpuIndexedDrawDataSchema.h"
#include "GpuMeshletDrawDataSchema.h"
#include "MaterialInfoSchema.h"
#include "MeshDrawBlueprintSchema.h"
#include "MeshInstanceDescriptorSchema.h"

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

            if (ar.IsBinary()) 
            {
                BlitVector<glm::vec3> verts{ m.vertices };
                ar.Property("vertices", verts);

                BlitVector<uint32_t> inds{ m.indices };
                ar.Property("indices", inds);

                BlitVector<glm::vec3> physVerts{ m.physicsVertices };
                ar.Property("physicsVertices", physVerts);

                BlitVector<GpuMeshCollider> mCols{ m.meshColliders };
                ar.Property("meshColliders", mCols);

                BlitVector<GpuMeshDescriptor> mDescs{ m.meshDescriptors };
                ar.Property("meshDescriptors", mDescs);

                BlitVector<GpuMeshLodDescriptor> lDescs{ m.lodDescriptors };
                ar.Property("lodDescriptors", lDescs);

                BlitVector<MeshDrawBlueprint> bCmds{ m.baseDrawCommands };
                ar.Property("baseDrawCommands", bCmds);

                BlitVector<uint32_t> matInds{ m.meshMaterialIndices };
                ar.Property("meshMaterialIndices", matInds);

                BlitVector<uint32_t> mvInds{ m.meshletVertexIndices };
                ar.Property("meshletVertexIndices", mvInds);

                BlitVector<uint8_t> mtInds{ m.meshletTriangleIndices };
                ar.Property("meshletTriangleIndices", mtInds);

                BlitVector<GpuMeshletDescriptor> mlDescs{ m.meshletDescriptors };
                ar.Property("meshletDescriptors", mlDescs);

                BlitVector<GpuMeshletDrawDescriptor> mlDrawDescs{ m.meshletDrawDescriptors };
                ar.Property("meshletDrawDescriptors", mlDrawDescs);
            
                ar.Property("meshNodeDescriptors", m.meshNodeDescriptors);

                auto serializeNestedBlit = [&ar](const char* arrayName, auto& nestedVec) {
                    uint32_t outerSize = static_cast<uint32_t>(nestedVec.size());
                    ar.EnterArray(arrayName, outerSize);

                    if constexpr (std::is_base_of_v<IInputArchive, Archive>) {
                        nestedVec.resize(outerSize);
                    }

                    for (uint32_t i = 0; i < outerSize; ++i) {
                        BlitVector<uint32_t> innerBlit{ nestedVec[i] };
                        ar.Property("item", innerBlit);
                    }

                    ar.LeaveArray();
                    };

                serializeNestedBlit("batchedIndicesPerLod", m.batchedIndicesPerLod);
                serializeNestedBlit("physicsIndicesPerLod", m.physicsIndicesPerLod);
            }
            else {
                ar.Property("vertices", m.vertices);
                ar.Property("indices", m.indices);
                ar.Property("physicsVertices", m.physicsVertices);

                ar.Property("meshColliders", m.meshColliders);
                ar.Property("meshDescriptors", m.meshDescriptors);
                ar.Property("meshletDrawDescriptors", m.meshletDrawDescriptors);
                ar.Property("lodDescriptors", m.lodDescriptors);
                ar.Property("baseDrawCommands", m.baseDrawCommands);
                ar.Property("meshMaterialIndices", m.meshMaterialIndices);

                ar.Property("meshletVertexIndices", m.meshletVertexIndices);
                ar.Property("meshletTriangleIndices", m.meshletTriangleIndices);
                ar.Property("meshletDescriptors", m.meshletDescriptors);

                ar.Property("batchedIndicesPerLod", m.batchedIndicesPerLod);
                ar.Property("physicsIndicesPerLod", m.physicsIndicesPerLod);
                ar.Property("meshNodeDescriptors", m.meshNodeDescriptors);
            }
        }
    };
}