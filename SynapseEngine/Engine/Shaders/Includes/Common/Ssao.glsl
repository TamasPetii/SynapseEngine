#ifndef SYN_INCLUDES_COMMON_SSAO_GLSL
#define SYN_INCLUDES_COMMON_SSAO_GLSL

#include "../Core.glsl"

struct SsaoKernel {
    vec4 samples[64];
};

layout(buffer_reference, std430) readonly restrict buffer SsaoKernelBuffer { SsaoKernel data; };

#define GET_SSAO_KERNEL_DATA(addr, idx)  SsaoKernelBuffer(addr).data.samples[idx]

#endif