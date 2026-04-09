#ifndef SYN_INCLUDES_COMMON_MESH_GLSL
#define SYN_INCLUDES_COMMON_MESH_GLSL

#include "../Core.glsl"

struct GpuVertexPosition {
    vec3 position;
    uint packedIndex;
};

struct GpuVertexAttributes {
    vec3 normal;
    float uv_x;
    vec3 tangent;
    float uv_y;
};

struct GpuMeshCollider { 
    vec3 center;
    float radius; 
    vec3 aabbMin; 
    float padding0; 
    vec3 aabbMax; 
    float padding1;
};

struct GpuMeshletCollider {
    vec3 center;
    float radius;
    vec3 aabbMin;
    float padding0;
    vec3 aabbMax;
    float padding1;
    vec3 apex;
    float cutoff;
    vec3 axis;
    float padding2;
};

struct GpuModelAddresses {
    uint64_t vertexPositions;
    uint64_t vertexAttributes;
    uint64_t indices;
    uint64_t meshDescriptors;
    uint64_t meshColliders;
    uint64_t lodDescriptors;
    uint64_t meshletVertexIndices;
    uint64_t meshletTriangleIndices;
    uint64_t meshletDescriptors;
    uint64_t meshletDrawDescriptors;
    uint64_t meshletColliders;
    uint64_t nodeTransforms;
    GpuMeshCollider globalCollider;
    uint vertexCount;
    uint indexCount;
    uint averageLodIndexCount;
    uint meshCount;
};

struct MeshDrawDescriptor {
    uint modelIndex;
    uint meshIndex;
    uint lodIndex;
    uint instanceOffset;
    uint maxInstances;
    uint indirectIndex;
    uint isMeshletPipeline;
    uint padding;
};

struct GpuMeshletDrawDescriptor {
    uint meshletOffset;
    uint meshletCount;
    uint materialIndex;
    uint padding;
};

struct GpuMeshletDescriptor {
    uint vertexIndicesOffset;
    uint vertexCount;
    uint triangleIndicesOffset;
    uint triangleCount;
};

struct DebugMeshletInstance {
    uint entityId;
    uint globalMeshletIdx;
};

layout(buffer_reference, std430) readonly restrict buffer PositionBuffer         { GpuVertexPosition data[]; };
layout(buffer_reference, std430) readonly restrict buffer AttributeBuffer        { GpuVertexAttributes data[]; };
layout(buffer_reference, std430) readonly restrict buffer IndexBuffer            { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer ModelAddressBuffer     { GpuModelAddresses data[]; };
layout(buffer_reference, std430) readonly restrict buffer DescriptorBuffer       { MeshDrawDescriptor data[]; };
layout(buffer_reference, std430) readonly restrict buffer MeshletDrawDescBuffer  { GpuMeshletDrawDescriptor data[]; };
layout(buffer_reference, std430) readonly restrict buffer MeshletDescBuffer      { GpuMeshletDescriptor data[]; };
layout(buffer_reference, std430) readonly restrict buffer MeshColliderBuffer     { GpuMeshCollider data[]; };
layout(buffer_reference, std430) readonly restrict buffer MeshletColliderBuffer  { GpuMeshletCollider data[]; };
layout(buffer_reference, std430) readonly restrict buffer VertexIndicesBuffer    { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer TriangleIndicesBuffer  { uint8_t data[]; };
layout(buffer_reference, std430) readonly restrict buffer DebugMeshletInstanceBuffer    { DebugMeshletInstance data[]; };

#define GET_VERTEX_POS(addr, idx)             PositionBuffer(addr).data[idx]
#define GET_VERTEX_ATTR(addr, idx)            AttributeBuffer(addr).data[idx]
#define GET_INDEX(addr, idx)                  IndexBuffer(addr).data[idx]
#define GET_MODEL_ADDRESSES(addr, idx)        ModelAddressBuffer(addr).data[idx]
#define GET_DRAW_DESCRIPTOR(addr, idx)        DescriptorBuffer(addr).data[idx]
#define GET_MESHLET_DRAW_DESC(addr, idx)      MeshletDrawDescBuffer(addr).data[idx]
#define GET_MESHLET_DESC(addr, idx)           MeshletDescBuffer(addr).data[idx]
#define GET_MESH_COLLIDER(addr, idx)          MeshColliderBuffer(addr).data[idx]
#define GET_MESHLET_COLLIDER(addr, idx)       MeshletColliderBuffer(addr).data[idx]
#define GET_MESHLET_VERTEX_INDEX(addr, idx)   VertexIndicesBuffer(addr).data[idx]
#define GET_MESHLET_TRIANGLE_INDEX(addr, idx) TriangleIndicesBuffer(addr).data[idx]
#define GET_DEBUG_MESHLET_INSTANCE_BUFFER(addr, idx)  DebugMeshletInstanceBuffer(addr).data[idx]

#endif