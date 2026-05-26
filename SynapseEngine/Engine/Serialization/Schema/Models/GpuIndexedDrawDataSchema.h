#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"

namespace Syn
{
    template <> struct Schema<GpuMeshCollider> {
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
        }
    };

    template <> struct Schema<GpuMeshDescriptor> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("vertexOffset", d.vertexOffset); 
            ar.Property("vertexCount", d.vertexCount);
            ar.Property("indexOffset", d.indexOffset); 
            ar.Property("indexCount", d.indexCount);
            ar.Property("materialIndex", d.materialIndex);
        }
    };

    template <> struct Schema<GpuMeshLodDescriptor> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("meshOffset", d.meshOffset); 
            ar.Property("meshCount", d.meshCount);
            ar.Property("indexOffset", d.indexOffset); 
            ar.Property("indexCount", d.indexCount);
            ar.Property("distanceThreshold", d.distanceThreshold);
        }
    };

    template <> struct Schema<GpuIndexedDrawData> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);

            if (ar.IsBinary()) {
                BlitVector<uint32_t> indices{ d.indices };
                ar.Property("indices", indices);

                BlitVector<GpuMeshDescriptor> meshDescriptors{ d.meshDescriptors };
                ar.Property("meshDescriptors", meshDescriptors);

                BlitVector<GpuMeshLodDescriptor> lodDescriptors{ d.lodDescriptors };
                ar.Property("lodDescriptors", lodDescriptors);

                BlitVector<GpuMeshCollider> meshColliders{ d.meshColliders };
                ar.Property("meshColliders", meshColliders);
            }
            else {
                ar.Property("indices", d.indices);
                ar.Property("meshDescriptors", d.meshDescriptors);
                ar.Property("lodDescriptors", d.lodDescriptors);
                ar.Property("meshColliders", d.meshColliders);
            }
        }
    };
}