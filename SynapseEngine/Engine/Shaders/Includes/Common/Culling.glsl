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

#ifndef SYN_INCLUDES_COMMON_CULLING_GLSL
#define SYN_INCLUDES_COMMON_CULLING_GLSL

#include "../Core.glsl"
#include "Mesh.glsl"
#include "IndirectCommand.glsl"

#define PIPELINE_TRADITIONAL 0u
#define PIPELINE_MESHLET     1u
#define PIPELINE_COUNT 2u

#define MATERIAL_RENDER_COUNT 8u

struct VisibleModelData { 
    uint entityId; 
    uint modelIndex;
};

struct ModelAllocationInfo { 
    uint maxInstances; 
    uint meshAllocationOffset; 
    uint meshAllocationCount; 
    uint padding; 
};

struct MeshAllocationInfo { 
    uint descriptorIndex; 
    uint padding[3]; 
    uint indirectIndices[PIPELINE_COUNT][MATERIAL_RENDER_COUNT];
    uint instanceOffsets[PIPELINE_COUNT][MATERIAL_RENDER_COUNT];
    uint activeTypes[PIPELINE_COUNT][MATERIAL_RENDER_COUNT];
};

layout(buffer_reference, std430) readonly restrict buffer ModelAllocBuffer   { ModelAllocationInfo data[]; };
layout(buffer_reference, std430) readonly restrict buffer MeshAllocBuffer    { MeshAllocationInfo data[]; };
layout(buffer_reference, std430) restrict buffer VisibleModelList            { VisibleModelData data[]; };

#define GET_MODEL_ALLOC(addr, idx)      ModelAllocBuffer(addr).data[idx]
#define GET_MESH_ALLOC(addr, idx)       MeshAllocBuffer(addr).data[idx]
#define GET_VISIBLE_MODEL(addr, idx)    VisibleModelList(addr).data[idx]

#define GET_DISPATCH_CMD(addr)          GET_VK_DISPATCH_CMD(addr)
#define GET_TRADITIONAL_CMD(addr, idx)  GET_VK_DRAW_CMD(addr, idx)
#define GET_MESHLET_CMD(addr, idx)      GET_VK_MESH_TASKS_CMD(addr, idx)

#endif