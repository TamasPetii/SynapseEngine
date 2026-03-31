#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/SpotLight.glsl"
#include "../../Includes/Utils/PbrMath.glsl"
#include "../../Includes/Utils/DepthMath.glsl"

layout(location = 0) in flat uint inLightDenseIndex;
layout(location = 1) in flat uint inShadowDenseIndex;
layout(location = 2) in flat uint inCameraIndex;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D normalRoughnessTexture;
layout(set = 2, binding = 2) uniform sampler2D depthTexture;

#include "../../Includes/PushConstants/DeferredSpotLightPC.glsl"

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(pc.screenWidth, pc.screenHeight);
    float depth = texture(depthTexture, uv).r;

    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, inCameraIndex);
    vec3 position = ReconstructWorldPosition(uv, depth, camera.viewProjVulkanInv);

    SpotLightComponent light = GET_SPOT_LIGHT(pc.spotLightDataAddr, inLightDenseIndex);

    // 1. Distance-based Culling
    float distToLight = distance(position, light.position);
    if (distToLight > light.range) {
        discard;
    }

    // 2. Angle-based Culling (Spot Light Cone)
    vec3 lightDirToFrag = normalize(position - light.position);
    vec3 spotDirection = normalize(light.direction);
    float theta = dot(lightDirToFrag, spotDirection);
    
    if (theta < light.outerCosAngle) {
        discard;
    }

    // 3. Sample G-Buffer Textures
    vec4 colorMetallic  = texture(colorMetallicTexture, uv);
    vec4 normalRoughness = texture(normalRoughnessTexture, uv);
    
    vec3 albedo    = colorMetallic.rgb;
    float metallic = colorMetallic.a;
    vec3 normal    = normalize(normalRoughness.xyz);
    float roughness = clamp(normalRoughness.a, 0.04, 1.0);

    // 4. Calculate Spot Light Edge Softness
    float epsilon = light.innerCosAngle - light.outerCosAngle;
    float spotIntensity = clamp((theta - light.outerCosAngle) / epsilon, 0.0, 1.0);

    // 5. Final Attenuation and Physically Based Rendering (PBR)
    vec3 viewDir = normalize(camera.eye.xyz - position);
    vec3 lightDir = normalize(light.position - position);
    
    float attenuation = clamp(1.0 - (distToLight * distToLight) / (light.range * light.range), 0.0, 1.0);
    attenuation *= attenuation;
    attenuation *= spotIntensity;

    vec3 radiance = ShadePhysicallyBased(
        albedo, 
        normal, 
        viewDir, 
        lightDir, 
        roughness, 
        metallic, 
        light.color, 
        attenuation, 
        light.strength
    );

    outColor = vec4(radiance, 1.0);
}