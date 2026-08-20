#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/Common/Texture.glsl" 
#include "../../../Includes/Common/Environment.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

#include "../../../Includes/PushConstants/SkyboxPC.glsl"

layout(push_constant) uniform PushConstants {
    SkyboxPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint cameraDenseIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraDenseIndex);

    EnvironmentData env = GET_ENVIRONMENT(ctx.environmentBufferAddr, pc.environmentIndex);

    vec2 ndc = inUV * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, 1.0, 1.0);
    vec4 viewPos = camera.projVulkanInv * clipPos;
    vec3 viewRay = (camera.viewInv * vec4(viewPos.xyz, 0.0)).xyz;
    vec3 viewDir = normalize(viewRay);

    vec3 rotatedDir = (env.skyRotationMatrix * vec4(viewDir, 0.0)).xyz;
    rotatedDir = normalize(rotatedDir);

    uint samplerIndex = GET_ENV_CUBE_SAMPLER(env);
    vec3 skyColor = SampleEnvironmentBaseCube(pc.environmentIndex, samplerIndex, rotatedDir, 0.0).rgb;
    
    skyColor *= env.skyTint;
    skyColor *= env.intensity;
    skyColor *= exp2(env.skyExposureEV);

    outColor = vec4(skyColor, 1.0);
}