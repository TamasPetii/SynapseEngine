#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Mesh/Data/Common/MeshInstanceDescriptor.h"

namespace Syn
{
    template <>
    struct Schema<MeshInstanceDescriptor> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("name", d.name);
            ar.Property("meshIndex", d.meshIndex);
            ar.Property("nodeIndex", d.nodeIndex);
            ar.Property("parentNodeIndex", d.parentNodeIndex);
            ar.Property("vertexCount", d.vertexCount);
            ar.Property("indexCount", d.indexCount);
        }
    };
}