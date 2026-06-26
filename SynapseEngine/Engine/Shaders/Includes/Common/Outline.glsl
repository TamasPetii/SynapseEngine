#ifndef SYN_INCLUDES_COMMON_OUTLINE_GLSL
#define SYN_INCLUDES_COMMON_OUTLINE_GLSL

#include "../Core.glsl"

layout(buffer_reference, std430) readonly restrict buffer SelectionMaskBuffer {
    uint data[];
};

#define GET_SELECTION_MASK(addr, idx) SelectionMaskBuffer(addr).data[idx]

#endif