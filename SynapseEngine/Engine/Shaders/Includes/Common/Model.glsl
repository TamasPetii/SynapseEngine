#ifndef SYN_INCLUDES_COMMON_MODEL_GLSL
#define SYN_INCLUDES_COMMON_MODEL_GLSL

#include "../Core.glsl"

struct ModelComponent { 
    uint entityIndex; 
    uint modelIndex; 
    uint flags; 
    uint materialOffset;  
};

layout(buffer_reference, std430) readonly buffer ModelComponentBuffer { ModelComponent data[]; };
layout(buffer_reference, std430) readonly buffer InstanceBuffer       { uint data[]; };

#define GET_MODEL_COMP(addr, idx)       ModelComponentBuffer(addr).data[idx]
#define GET_INSTANCE(addr, idx)         InstanceBuffer(addr).data[idx]

#endif