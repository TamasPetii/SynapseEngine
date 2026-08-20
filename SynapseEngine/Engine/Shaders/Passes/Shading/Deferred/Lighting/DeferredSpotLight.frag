// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Common/SpotLight.glsl"
#include "../../../../Includes/Utils/PbrMath.glsl"
#include "../../../../Includes/Utils/DepthMath.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"
#include "../../../../Includes/Utils/ShadowMath.glsl"

layout(location = 0) in flat uint inLightDenseIndex;
layout(location = 1) in flat uint inEntityLightIndex;
layout(location = 2) in flat uint inCameraIndex;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D normalRoughnessTexture;
layout(set = 2, binding = 2) uniform sampler2D depthTexture;
layout(set = 2, binding = 3) uniform sampler2D ssaoTexture;
layout(set = 2, binding = 4) uniform sampler2DShadow spotLightShadowAtlas;
layout(set = 2, binding = 5) uniform sampler2D spotLightShadowColorAtlas;

#include "../../../../Includes/PushConstants/DeferredSpotLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredSpotLightPC pc;
};

void main()
{
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    vec2 uv = gl_FragCoord.xy / vec2(ctx.screenWidth, ctx.screenHeight);
    float depth = texture(depthTexture, uv).r;

    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, inCameraIndex);
    vec3 position = ReconstructWorldPosition(uv, depth, camera.viewProjVulkanInv);

    SpotLightComponent light = GET_SPOT_LIGHT(ctx.spotLightDataBufferAddr, inLightDenseIndex);

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

    float defaultIor = 1.5;
    float defaultSpecularFactor = 1.0;
    vec3 defaultSpecularColor = vec3(1.0);
    float defaultClearcoatFactor = 0.0;
    float defaultClearcoatRoughness = 0.0;
    vec3 defaultClearcoatNormal = normal;

    // 3. Final Attenuation and Physically Based Rendering (PBR)
    vec3 viewDir = normalize(camera.eye.xyz - position); 
    vec3 lightDir = normalize(light.position.xyz - position);

    vec3 shadowFactor = CalculateSpotLightShadow(
        ctx.spotLightShadowDataBufferAddr,
        ctx.spotLightShadowSparseMapBufferAddr,
        inEntityLightIndex,
        position,
        normal,
        lightDir,
        spotLightShadowAtlas,
        spotLightShadowColorAtlas
    );

    vec3 radiance = SimulateSpotLight(
        ctx.spotLightDataBufferAddr, 
        inLightDenseIndex, 
        position,
        albedo, 
        normal, 
        viewDir, 
        roughness, 
        metallic,
        defaultIor, 
        defaultSpecularFactor, 
        defaultSpecularColor,
        defaultClearcoatFactor, 
        defaultClearcoatRoughness, 
        defaultClearcoatNormal
    );

    radiance *= shadowFactor;

    if (ctx.enableSsao == 1 && ctx.enableSsaoLight == 1) {
        float ssao = texture(ssaoTexture, uv).r;
        radiance *= ssao;
    }

    outColor = vec4(radiance, 1.0);
}