#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "GpuVertexDataSchema.h"
#include "GpuIndexedDrawDataSchema.h"
#include "GpuMeshletDrawDataSchema.h"
#include "MaterialInfoSchema.h"

#include "Engine/Mesh/Data/Gpu/GpuNodeTransform.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"

namespace Syn
{
    template <> struct Schema<GpuNodeTransform> {
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

    template <> struct Schema<GpuBatchedModel> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& m = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("vertexData", m.vertexData);
            ar.Property("indexedData", m.indexedData);
            ar.Property("meshletData", m.meshletData);

            if (ar.IsBinary()) {
                BlitVector<GpuNodeTransform> nodeTransforms{ m.nodeTransforms };
                ar.Property("nodeTransforms", nodeTransforms);
            }
            else {
                ar.Property("nodeTransforms", m.nodeTransforms);
            }

            ar.Property("materials", m.materials);
            ar.Property("globalCollider", m.globalCollider);
            ar.Property("globalVertexCount", m.globalVertexCount);
            ar.Property("globalIndexCount", m.globalIndexCount);
            ar.Property("globalMeshCount", m.globalMeshCount);
        }
    };
}