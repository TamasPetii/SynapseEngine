#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CULLING_COMMAND_RESET_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CULLING_COMMAND_RESET_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct CullingCommandResetPC {
    uint64_t globalIndirectCmdsAddr;
    uint64_t modelComputeCountAddr;
    uint totalTraditionalCommands;
    uint totalCommands;
};

#endif