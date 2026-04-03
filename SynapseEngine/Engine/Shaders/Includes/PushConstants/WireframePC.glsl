#ifndef SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct WireframePC {
    uint64_t animationAddressBuffer;
    uint64_t animationBufferAddr;
    uint64_t animationSparseMapBufferAddr;

    uint64_t modelAddressBuffer;
    uint64_t globalInstanceBuffers;
    uint64_t globalIndirectCommandDescriptorBuffers;

    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t transformBufferAddr;
    uint64_t transformSparseMapBufferAddr;

    uint64_t indexBufferAddr;
    uint64_t vertexBufferAddr;

    uint64_t debugInstanceBufferAddr;
    uint64_t modelBufferAddr;
    uint64_t modelSparseMapBufferAddr;

    uint activeCameraEntity;
    uint isSphere;
    uint drawIdOffset;
    uint padding;
    
    vec4 debugColor;
};

#endif