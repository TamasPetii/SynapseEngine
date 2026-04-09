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