#ifndef SYN_INCLUDES_COMMON_TRANSFORM_GLSL
#define SYN_INCLUDES_COMMON_TRANSFORM_GLSL

#include "../Core.glsl"

struct TransformComponent {
    mat4 transform;
    mat4 transformIT;
};

struct GpuNodeTransform {
    mat4 globalTransform;
    mat4 globalTransformIT;
};

layout(buffer_reference, std430) readonly restrict buffer TransformPool { 
    TransformComponent data[]; 
};

layout(buffer_reference, std430) readonly restrict buffer NodeBuffer { 
    GpuNodeTransform data[];
};

#define GET_TRANSFORM_POOL(addr)        TransformPool(addr)
#define GET_TRANSFORM(addr, idx)        TransformPool(addr).data[idx]
#define GET_NODE_TRANSFORM(addr, idx)   NodeBuffer(addr).data[idx]

#endif