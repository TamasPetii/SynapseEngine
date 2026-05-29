#ifndef SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_MESH_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_MESH_PC_GLSL

#include "../SharedGpuTypes.glsl"

#define WIREFRAME_MESH_SHAPE_TYPE_CUBE   0
#define WIREFRAME_MESH_SHAPE_TYPE_SPHERE 1

struct WireframeMeshPC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t indexBufferAddr;
    uint64_t vertexPositionBufferAddr;
    uint shapeType;
};

#endif