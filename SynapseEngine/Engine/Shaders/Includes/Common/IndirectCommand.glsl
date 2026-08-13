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

#ifndef SYN_INCLUDES_COMMON_INDIRECT_COMMAND_GLSL
#define SYN_INCLUDES_COMMON_INDIRECT_COMMAND_GLSL

#include "../Core.glsl"

struct VkDispatchIndirectCommand { 
    uint groupCountX; 
    uint groupCountY; 
    uint groupCountZ; 
};

struct VkDrawIndirectCommand { 
    uint vertexCount; 
    uint instanceCount; 
    uint firstVertex; 
    uint firstInstance; 
};

struct VkDrawMeshTasksIndirectCommandEXT { 
    uint groupCountX; 
    uint groupCountY; 
    uint groupCountZ; 
};

layout(buffer_reference, std430) restrict buffer VkDispatchIndirectBuffer         { VkDispatchIndirectCommand data; };
layout(buffer_reference, std430) restrict buffer VkDrawIndirectBuffer             { VkDrawIndirectCommand data[]; };
layout(buffer_reference, std430) restrict buffer VkDrawMeshTasksIndirectBuffer     { VkDrawMeshTasksIndirectCommandEXT data[]; };

#define GET_VK_DISPATCH_CMD(addr)           VkDispatchIndirectBuffer(addr).data
#define GET_VK_DRAW_CMD(addr, idx)          VkDrawIndirectBuffer(addr).data[idx]
#define GET_VK_MESH_TASKS_CMD(addr, idx)    VkDrawMeshTasksIndirectBuffer(addr).data[idx]

#endif