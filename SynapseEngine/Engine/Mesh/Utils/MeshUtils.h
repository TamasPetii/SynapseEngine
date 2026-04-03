#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Mesh/Data/Raw/RawModel.h"
#include <filesystem>
#include <glm/glm.hpp>

#include "Engine/Mesh/Data/Cooked/CookedMesh.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"
#include "Engine/Mesh/Data/Gpu/GpuMeshletDrawData.h"

namespace Syn 
{
    class MeshUtils {
    public:
        static void LogRawModel(const std::filesystem::path& path, const RawModel& model);
        static GpuMeshCollider TransformCollider(const GpuMeshCollider& local, const glm::mat4& transform);
        static GpuMeshletCollider TransformCollider(const GpuMeshletCollider& local, const glm::mat4& transform, const glm::mat4& transformIT);
    };
}

