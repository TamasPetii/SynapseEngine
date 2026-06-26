#ifndef SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_MESHLET_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_MESHLET_PC_GLSL

#include "../SharedGpuTypes.glsl"

#define WIREFRAME_MESHLET_SHAPE_TYPE_CUBE   0
#define WIREFRAME_MESHLET_SHAPE_TYPE_SPHERE 1
#define WIREFRAME_MESHLET_SHAPE_TYPE_CONE   2

struct WireframeMeshletPC {
    uint64_t frameGlobalContextBufferAddr;
    uint baseDescriptorOffset;
    uint materialRenderType;
    uint disableConeCulling;
    uint shapeType;
    uint64_t indexBufferAddr;
    uint64_t vertexPositionBufferAddr;
    uint vertexCount;
    uint indexCount;
};

#endif