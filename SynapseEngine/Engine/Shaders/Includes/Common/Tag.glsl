#ifndef SYN_INCLUDES_COMMON_TAG_GLSL
#define SYN_INCLUDES_COMMON_TAG_GLSL

#include "../Core.glsl"

layout(buffer_reference, std430) readonly restrict buffer TagDataBuffer { 
    uint data[]; 
};

#define GET_TAG_DATA(addr, idx) TagDataBuffer(addr).data[idx]

#endif