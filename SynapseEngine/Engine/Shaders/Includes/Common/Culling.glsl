#ifndef SYN_INCLUDES_COMMON_CULLING_GLSL
#define SYN_INCLUDES_COMMON_CULLING_GLSL

#include "../Core.glsl"
#include "Mesh.glsl"
#include "IndirectCommand.glsl"

#define PIPELINE_TRADITIONAL 0u
#define PIPELINE_MESHLET     1u

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
    uint isMeshletPipeline; 
    uint padding[2]; 
    uint indirectIndices[4];
    uint instanceOffsets[4];
    uint activeTypes[4];
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