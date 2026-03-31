#ifndef SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_SETUP_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_SETUP_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct WireframeSetupPC {
    uint64_t globalIndirectCmdsAddr;
    uint64_t aabbCmdsAddr;
    uint64_t sphereCmdsAddr;
    uint totalTraditionalCommands; 
    uint totalCommands;
};

#endif