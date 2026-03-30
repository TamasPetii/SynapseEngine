#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) out flat uint outLightDenseIndex;
layout(location = 1) out flat uint outShadowDenseIndex;
layout(location = 2) out flat uint outCameraIndex;

struct GpuVertexPosition {
    vec3 position;
    uint packedIndex;
};

struct CameraComponent {
    mat4 view;
    mat4 viewInv;
    mat4 proj;
    mat4 projInv;
    mat4 projVulkan;
    mat4 projVulkanInv;
    mat4 viewProj;
    mat4 viewProjInv;
    mat4 viewProjVulkan;
    mat4 viewProjVulkanInv;
    vec4 eye;
    vec4 params;
    vec4 frustum[6];
};

struct PointLightComponent {
    vec3 position;
    float radius;
    vec3 color;
    float strength;
    float weakenDistance;
    uint flags;
    uint padding;
    uint padding2;
};

layout(buffer_reference, std430) readonly buffer PositionBuffer       { GpuVertexPosition data[]; };
layout(buffer_reference, std430) readonly buffer IndexBuffer          { uint data[]; };
layout(buffer_reference, std430) readonly buffer PointLightDataBuffer { PointLightComponent data[]; };
layout(buffer_reference, std430) readonly buffer SparseMapBuffer      { uint data[]; };
layout(buffer_reference, std430) readonly buffer VisibleLightBuffer   { uint data[]; };
layout(buffer_reference, std430) readonly buffer CameraPool           { CameraComponent data[]; };

layout(push_constant) uniform PushConstants {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t pointLightDataAddr;
    uint64_t pointLightSparseMapAddr;
    uint64_t visibleLightAddr;
    uint64_t vertexPositionsAddr;
    uint64_t indicesAddr;
    uint64_t pointLightShadowSparseMapAddr;
    uint64_t pointLightShadowDataAddr;
    
    uint activeCameraEntity;
    float screenWidth;
    float screenHeight;
} pc;

void main() 
{
    uint entityId = VisibleLightBuffer(pc.visibleLightAddr).data[gl_InstanceIndex];   
    uint lightDenseIndex = SparseMapBuffer(pc.pointLightSparseMapAddr).data[entityId];   
    uint shadowDenseIndex = 0xFFFFFFFFu; 

    if (pc.pointLightShadowSparseMapAddr != 0) {
        shadowDenseIndex = SparseMapBuffer(pc.pointLightShadowSparseMapAddr).data[entityId];
    }

    PointLightComponent light = PointLightDataBuffer(pc.pointLightDataAddr).data[lightDenseIndex];

    uint vertexIndex = IndexBuffer(pc.indicesAddr).data[gl_VertexIndex];
    vec3 localPos = PositionBuffer(pc.vertexPositionsAddr).data[vertexIndex].position;

    mat4 model = mat4(1.0);
    model[0][0] = light.radius;
    model[1][1] = light.radius;
    model[2][2] = light.radius;
    model[3] = vec4(light.position, 1.0);
    
    SparseMapBuffer cameraMap = SparseMapBuffer(pc.cameraSparseMapBufferAddr);

    uint cameraIndex = cameraMap.data[pc.activeCameraEntity];
    CameraComponent camera = CameraPool(pc.cameraBufferAddr).data[cameraIndex];

    gl_Position = camera.viewProjVulkan * model * vec4(localPos, 1.0);
    
    outLightDenseIndex = lightDenseIndex;
    outShadowDenseIndex = shadowDenseIndex;
    outCameraIndex = cameraIndex;
}