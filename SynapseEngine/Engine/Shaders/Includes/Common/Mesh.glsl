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

layout(buffer_reference, std430) readonly buffer PositionBuffer        { GpuVertexPosition data[]; };
layout(buffer_reference, std430) readonly buffer AttributeBuffer       { GpuVertexAttributes data[]; };
layout(buffer_reference, std430) readonly buffer IndexBuffer           { uint data[]; };
layout(buffer_reference, std430) readonly buffer ModelAddressBuffer    { GpuModelAddresses data[]; };
layout(buffer_reference, std430) readonly buffer DescriptorBuffer      { MeshDrawDescriptor data[]; };
layout(buffer_reference, std430) readonly buffer MeshletDrawDescBuffer  { GpuMeshletDrawDescriptor data[]; };

layout(buffer_reference, std430) readonly buffer MeshColliderBuffer     { GpuMeshCollider data[]; };
layout(buffer_reference, std430) readonly buffer MeshletColliderBuffer  { GpuMeshletCollider data[]; };

#define GET_VERTEX_POS(addr, idx)          PositionBuffer(addr).data[idx]
#define GET_VERTEX_ATTR(addr, idx)         AttributeBuffer(addr).data[idx]
#define GET_INDEX(addr, idx)               IndexBuffer(addr).data[idx]
#define GET_MODEL_ADDRESSES(addr, idx)     ModelAddressBuffer(addr).data[idx]
#define GET_DRAW_DESCRIPTOR(addr, idx)     DescriptorBuffer(addr).data[idx]
#define GET_MESHLET_DRAW_DESC(addr, idx)   MeshletDrawDescBuffer(addr).data[idx]
#define GET_MESH_COLLIDER(addr, idx)       MeshColliderBuffer(addr).data[idx]
#define GET_MESHLET_COLLIDER(addr, idx)    MeshletColliderBuffer(addr).data[idx]

#endif