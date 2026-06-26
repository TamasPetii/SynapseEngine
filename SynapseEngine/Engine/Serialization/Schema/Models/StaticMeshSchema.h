#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "GpuVertexDataSchema.h"
#include "GpuIndexedDrawDataSchema.h"
#include "GpuMeshletDrawDataSchema.h"
#include "MaterialInfoSchema.h"
#include "CpuModelDataSchema.h"
#include "MeshDrawBlueprintSchema.h"
#include "GpuBatchedModelSchema.h"
#include "Engine/Mesh/Data/StaticMesh.h"

namespace Syn
{
    template <>
    struct Schema<StaticMesh> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& m = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("cpuData", m.cpuData);
            ar.Property("gpuData", *m.transientGpuData);
        }
    };
}