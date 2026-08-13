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
#include "GpuVertexData.h"
#include "GpuIndexedDrawData.h"
#include "GpuMeshletDrawData.h"
#include "GpuNodeTransform.h"
#include "../Common/MaterialInfo.h"
#include "../Common/MeshInstanceDescriptor.h"

namespace Syn
{
    struct SYN_API GpuBatchedModel
    {
        GpuVertexData vertexData;
        GpuIndexedDrawData indexedData;
        GpuMeshletDrawData meshletData;
        std::vector<uint32_t> meshMaterialIndices;
        std::vector<MaterialInfo> materials;
        std::vector<GpuNodeTransform> nodeTransforms;
        GpuMeshCollider globalCollider;
        uint32_t globalVertexCount = 0;
        uint32_t globalIndexCount = 0;
        uint32_t globalLod0IndexCount = 0;
        uint32_t globalAverageLodIndexCount = 0;
        uint32_t globalMeshCount = 0;

        std::vector<MeshInstanceDescriptor> meshNodeDescriptors;
    };
}

