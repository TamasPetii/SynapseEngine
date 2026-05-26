#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Mesh/Data/Gpu/GpuVertexData.h"

namespace Syn
{
    template <> struct Schema<GpuVertexPosition> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& v = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("position", v.position); 
            ar.Property("packedIndex", v.packedIndex);
        }
    };

    template <> struct Schema<GpuVertexAttributes> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& a = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("normal", a.normal); 
            ar.Property("uv_x", a.uv_x);
            ar.Property("tangent", a.tangent); 
            ar.Property("uv_y", a.uv_y);
        }
    };

    template <> struct Schema<GpuVertexData> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) {

            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);

            if (ar.IsBinary()) {
                BlitVector<GpuVertexPosition> vertexPositions{ d.vertexPositions };
                ar.Property("vertexPositions", vertexPositions);
                BlitVector<GpuVertexAttributes> vertexAttributes{ d.vertexAttributes };
                ar.Property("vertexAttributes", vertexAttributes);
            }
            else {
                ar.Property("vertexPositions", d.vertexPositions);
                ar.Property("vertexAttributes", d.vertexAttributes);
            }
        }
    };
}