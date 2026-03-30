#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) in flat uint inLightDenseIndex;
layout(location = 1) in flat uint inShadowDenseIndex;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D normalRoughnessTexture;
layout(set = 2, binding = 2) uniform sampler2D depthTexture;

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

struct PointLightShadowComponent {
    vec4 planes;
    mat4 viewProjs[6];
    vec4 atlasRects[6];
};

layout(buffer_reference, std430) readonly buffer PointLightDataBuffer { PointLightComponent data[]; };
layout(buffer_reference, std430) readonly buffer PointLightShadowDataBuffer { PointLightShadowComponent data[]; };
layout(buffer_reference, std430) readonly buffer SparseMapBuffer      { uint data[]; };
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
    vec2 uv = gl_FragCoord.xy / vec2(pc.screenWidth, pc.screenHeight);
    float depth = texture(depthTexture, uv).r;

    SparseMapBuffer cameraMap = SparseMapBuffer(pc.cameraSparseMapBufferAddr);
    CameraComponent camera = CameraPool(pc.cameraBufferAddr).data[cameraMap.data[pc.activeCameraEntity]];

    vec4 ndc = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 worldPos = camera.viewProjVulkanInv * ndc;
    vec3 position = worldPos.xyz / worldPos.w;

    PointLightComponent light = PointLightDataBuffer(pc.pointLightDataAddr).data[inLightDenseIndex];

    if (distance(position, light.position) > light.radius) {
        discard;
    }

    vec4 colorMetallic  = texture(colorMetallicTexture, uv);
    vec4 normalRoughness = texture(normalRoughnessTexture, uv);
    
    vec3 albedo    = colorMetallic.rgb;
    float metallic = colorMetallic.a;
    
    vec3 normal    = normalize(normalRoughness.xyz); 
    float roughness = normalRoughness.a;

    outColor = vec4(albedo * normal * light.color * light.strength, 1.0);
}