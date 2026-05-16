#ifndef SYN_INCLUDES_COMMON_FORWARD_PLUS_DISPATCH_GLSL
#define SYN_INCLUDES_COMMON_FORWARD_PLUS_DISPATCH_GLSL

#include "../Core.glsl"

#define MAX_FAST_PATH_LIGHTS 512

struct ForwardPlusDispatchArgs {
    uvec4 pointFastPath;
    uvec4 pointSlowCount;
    uvec4 pointSlowWrite;
    
    uvec4 spotFastPath;
    uvec4 spotSlowCount;
    uvec4 spotSlowWrite;
    
    uvec4 prefixSum;
};

layout(buffer_reference, std430) restrict buffer ForwardPlusDispatchArgsBuffer { 
    ForwardPlusDispatchArgs data; 
};

#define GET_FORWARD_PLUS_DISPATCH_ARGS(addr) ForwardPlusDispatchArgsBuffer(addr).data

#endif