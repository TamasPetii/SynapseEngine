#version 460
#extension GL_EXT_mesh_shader : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) in vec3 inNormal;
layout(location = 1) flat in uint inLodIndex;
layout(location = 2) in vec3 inMeshletColor;

layout(location = 0) out vec4 outFragColor;

layout(push_constant) uniform PushConstants {
    uint64_t modelAddressBuffer;

    uint64_t globalDrawCountBuffers;
    uint64_t globalInstanceBuffers;
    uint64_t globalIndirectCommandBuffers;
    uint64_t globalIndirectCommandDescriptorBuffers;  
    uint64_t globalModelAllocationBuffers;
    uint64_t globalMeshAllocationBuffers;

    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t transformBufferAddr;
    uint64_t transformSparseMapBufferAddr;
    uint64_t modelBufferAddr;
    uint64_t modelSparseMapBufferAddr;

    uint activeCameraEntity;
    uint meshletOffsetStart;
    uint visualizeMeshlet;
    uint padding1;
} pc;

void main() {
    vec3 color;

    if (inLodIndex == 0) {
        color = vec3(0, 0.9, 0);
    } else if (inLodIndex == 1) {
        color = vec3(0, 0.6, 0);
    } else if (inLodIndex == 2) {
        color = vec3(0, 0.35, 0);
    } else {
        color = vec3(0, 0.15, 0);
    }

    color *= pc.visualizeMeshlet == 1 ? inMeshletColor.x : 1.0;

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float ndotl = max(dot(normalize(inNormal), lightDir), 0.0);
    vec3 finalColor = color;

    outFragColor = vec4(finalColor, 1.0);
}