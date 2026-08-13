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

