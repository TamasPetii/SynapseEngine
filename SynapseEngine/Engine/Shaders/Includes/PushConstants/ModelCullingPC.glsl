#ifndef SYN_INCLUDES_PC_MODEL_CULLING_PASS_GLSL
#define SYN_INCLUDES_PC_MODEL_CULLING_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct ModelCullingPC {
    uint64_t animationAddressBuffer;
    uint64_t animationBufferAddr;
    uint64_t animationSparseMapBufferAddr;
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t transformBufferAddr;
    uint64_t transformSparseMapBufferAddr;
    uint64_t modelCompBufferAddr;
    uint64_t modelSparseMapBufferAddr;
    uint64_t meshAllocBufferAddr;
    uint64_t modelAllocBufferAddr;
    uint64_t modelAddressBufferAddr;  
    uint64_t visibleModelListAddr;
    uint64_t visibleModelCountAddr; 
    uint64_t globalIndirectCommandBuffers;
    uint64_t globalInstanceBufferAddr;   
    uint64_t materialLookupBufferAddr; 
    uint64_t materialBufferAddr;
    uint totalModelsToTest;
    uint activeCameraEntity;
    uint traditionalCommandCount;
    uint enableOcclusionCulling;
    float screenWidth;
    float screenHeight;
    vec2 padding;
};

#endif