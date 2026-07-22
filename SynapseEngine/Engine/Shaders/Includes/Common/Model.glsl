#ifndef SYN_INCLUDES_COMMON_MODEL_GLSL
#define SYN_INCLUDES_COMMON_MODEL_GLSL

#extension GL_EXT_scalar_block_layout : require
#include "../Core.glsl"

struct ModelComponent { 
    uint entityIndex; 
    uint modelIndex; 
    uint flags; 
    uint materialOffset; 
    uint pipelineOffset;
};

layout(buffer_reference, scalar) readonly restrict buffer ModelComponentBuffer { ModelComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer InstanceBuffer       { uint data[]; };

#define GET_MODEL_COMP(addr, idx)       ModelComponentBuffer(addr).data[idx]
#define GET_INSTANCE(addr, idx)         InstanceBuffer(addr).data[idx]

#endif