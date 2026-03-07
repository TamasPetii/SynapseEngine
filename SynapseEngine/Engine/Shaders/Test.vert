#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

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

struct GpuNodeTransform {
    mat4 globalTransform;
    mat4 globalTransformIT;
};

layout(buffer_reference, std430) readonly buffer PositionBuffer {
    GpuVertexPosition data[];
};

layout(buffer_reference, std430) readonly buffer AttributeBuffer {
    GpuVertexAttributes data[];
};

layout(buffer_reference, std430) readonly buffer IndexBuffer {
    uint data[];
};

layout(buffer_reference, std430) readonly buffer NodeBuffer {
    GpuNodeTransform data[];
};

layout(push_constant) uniform PushConstants {
    uint64_t positionsAddr;
    uint64_t attributesAddr;
    uint64_t indicesAddr;
    uint64_t nodesAddr;
    mat4 viewProj;
} pc;

layout(location = 0) out vec3 outNormal;

void main() {
    PositionBuffer positions = PositionBuffer(pc.positionsAddr);
    AttributeBuffer attributes = AttributeBuffer(pc.attributesAddr);
    IndexBuffer indices = IndexBuffer(pc.indicesAddr);
    NodeBuffer nodes = NodeBuffer(pc.nodesAddr);

    uint realVertexIndex = indices.data[gl_VertexIndex];
    
    GpuVertexPosition v = positions.data[realVertexIndex];
    GpuVertexAttributes attr = attributes.data[realVertexIndex];

    uint nodeIndex = v.packedIndex & 0xFFFFu;
    uint meshIndex = (v.packedIndex >> 16) & 0xFFFFu; 

    mat4 modelMat = nodes.data[nodeIndex].globalTransform;
    mat4 modelMatIT = nodes.data[nodeIndex].globalTransformIT;

    gl_Position = pc.viewProj * modelMat * vec4(v.position, 1.0);
   
    outNormal = normalize(modelMatIT * vec4(attr.normal, 0)).xyz;
}