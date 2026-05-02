#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/DirectionLight.glsl"
#include "../../Includes/Utils/PbrMath.glsl"
#include "../../Includes/Utils/DepthMath.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uint inLightDenseIndex;
layout(location = 2) in flat uint inShadowDenseIndex;
layout(location = 3) in flat uint inCameraIndex;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D normalRoughnessTexture;
layout(set = 2, binding = 2) uniform sampler2D depthTexture;

#include "../../Includes/PushConstants/DeferredDirectionLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredDirectionLightPC pc;
};

void main()
{
    float depth = texture(depthTexture, inUV).r;
    
    if (depth == 1.0) {
        discard;
    }

    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, inCameraIndex);
    vec3 position = ReconstructWorldPosition(inUV, depth, camera.viewProjVulkanInv);

    vec4 colorMetallic  = texture(colorMetallicTexture, inUV);
    vec4 normalRoughness = texture(normalRoughnessTexture, inUV);
    
    vec3 albedo    = colorMetallic.rgb;
    float metallic = colorMetallic.a;
    vec3 normal    = normalize(normalRoughness.xyz);
    float roughness = clamp(normalRoughness.a, 0.04, 1.0);

    vec3 viewDir = normalize(camera.eye.xyz - position);
    
    DirectionLightComponent light = GET_DIRECTION_LIGHT(pc.directionLightDataAddr, inLightDenseIndex);

    vec3 lightDir = normalize(-light.direction);
        
    vec3 radiance = ShadePhysicallyBased(
        albedo, 
        normal, 
        viewDir, 
        lightDir, 
        roughness, 
        metallic, 
        light.color, 
        1.0,
        light.strength
    );

    outColor = vec4(radiance, 1.0);
}