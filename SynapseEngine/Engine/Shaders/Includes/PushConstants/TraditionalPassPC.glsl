#ifndef SYN_INCLUDES_PC_TRADITIONAL_PASS_GLSL
#define SYN_INCLUDES_PC_TRADITIONAL_PASS_GLSL

#include "../Core.glsl"

layout(push_constant) uniform PushConstants {
    uint64_t modelAddressBuffer; 
    uint64_t animationAddressBuffer;
    uint64_t animationBufferAddr;
    uint64_t animationSparseMapBufferAddr;
    uint64_t globalDrawCountBuffers; 
    uint64_t globalInstanceBuffers;
    uint64_t globalIndirectCommandBuffers; 
    uint64_t globalIndirectCommandDescriptorBuffers;   
    uint64_t globalModelAllocationBuffers;
    uint64_t globalMeshAllocationBuffers; 
    uint64_t cameraBufferAddr; 
    uint64_t cameraSparseMapBufferAddr; 
    uint64_t transformBufferAddr; 
    uint64_t transformSparseMapBufferAddr; 
    uint64_t modelBufferAddr; 
    uint64_t modelSparseMapBufferAddr;
    uint64_t materialLookupBuffer; 
    uint64_t materialBuffer;  
    uint activeCameraEntity;
    uint baseDescriptorOffset;
    uint materialRenderType;
} pc;

#endif