#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Common/PointLight.glsl"
#include "../../../../Includes/Utils/PbrMath.glsl"
#include "../../../../Includes/Utils/DepthMath.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"

layout(location = 0) in flat uint inLightDenseIndex;
layout(location = 1) in flat uint inShadowDenseIndex;
layout(location = 2) in flat uint inCameraIndex;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D normalRoughnessTexture;
layout(set = 2, binding = 2) uniform sampler2D depthTexture;

#include "../../../../Includes/PushConstants/DeferredPointLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredPointLightPC pc;
};

void main()
{
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    // 1. Reconstruct World Position from Depth
    vec2 uv = gl_FragCoord.xy / vec2(ctx.screenWidth, ctx.screenHeight);
    float depth = texture(depthTexture, uv).r;

    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, inCameraIndex);
    vec3 position = ReconstructWorldPosition(uv, depth, camera.viewProjVulkanInv);

    // 2. Fetch Point Light Component
    PointLightComponent light = GET_POINT_LIGHT(ctx.pointLightDataBufferAddr, inLightDenseIndex);

    // 3. Spherical Culling
    float distToLight = distance(position, light.position);
    if (distToLight > light.radius) {
        discard;
    }

    // 4. Sample G-Buffer
    vec4 colorMetallic  = texture(colorMetallicTexture, uv);
    vec4 normalRoughness = texture(normalRoughnessTexture, uv);
    
    vec3 albedo    = colorMetallic.rgb;
    float metallic = colorMetallic.a;
    vec3 normal    = normalize(normalRoughness.xyz);
    float roughness = clamp(normalRoughness.a, 0.04, 1.0);

    // 5. Physically Based Rendering (PBR) Light Calculation
    vec3 viewDir = normalize(camera.eye.xyz - position);
    vec3 radiance = SimulatePointLight(ctx.pointLightDataBufferAddr, inLightDenseIndex, albedo, normal, viewDir, roughness, metallic);

    outColor = vec4(radiance, 1.0);
}