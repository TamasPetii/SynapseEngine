#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEBUG_VISIBILITY_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEBUG_VISIBILITY_PC_GLSL

#include "../SharedGpuTypes.glsl"

#define DEBUG_MODE_ENTITY_ID        0
#define DEBUG_MODE_PIPELINE_TYPE    1
#define DEBUG_MODE_LOD_LEVEL        2
#define DEBUG_MODE_MESH_INDEX       3
#define DEBUG_MODE_MESHLET_INDEX    4
#define DEBUG_MODE_TRIANGLE_INDEX   5
#define DEBUG_MODE_TRIANGLE_INDEX   5
#define DEBUG_MODE_ALL_COMBINED     6
#define DEBUG_MODE_MATERIAL_TYPE    7
#define DEBUG_MODE_CLUSTER_SLICES   8
#define DEBUG_MODE_CLUSTER_LIGHTS   9

struct DebugVisibilityPC {
    uint64_t frameGlobalContextBufferAddr;
    uint debugMode;
};

#endif