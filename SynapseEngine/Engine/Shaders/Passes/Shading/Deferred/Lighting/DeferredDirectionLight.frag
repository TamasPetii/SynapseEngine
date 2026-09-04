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
#include "../../../../Includes/Common/DirectionLight.glsl"
#include "../../../../Includes/Utils/PbrMath.glsl"
#include "../../../../Includes/Utils/DepthMath.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"
#include "../../../../Includes/Utils/ShadowMath.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uint inLightDenseIndex;
layout(location = 2) in flat uint inEntityLightIndex;
layout(location = 3) in flat uint inCameraIndex;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D normalRoughnessTexture;
layout(set = 2, binding = 2) uniform sampler2D depthTexture;
layout(set = 2, binding = 3) uniform sampler2D ssaoTexture;
layout(set = 2, binding = 4) uniform sampler2DShadow dirLightShadowAtlas;
layout(set = 2, binding = 5) uniform sampler2D dirLightShadowColorAtlas;
layout(set = 2, binding = 6) uniform sampler2DShadow dirLightStaticShadowAtlas;
layout(set = 2, binding = 7) uniform sampler2D dirLightStaticShadowColorAtlas;


#include "../../../../Includes/PushConstants/DeferredDirectionLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredDirectionLightPC pc;
};

void main()
{
    float depth = texture(depthTexture, inUV).r;
    
    if (depth == 1.0) {
        discard;
    }

    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, inCameraIndex);
    vec3 position = ReconstructWorldPosition(inUV, depth, camera.viewProjVulkanInv);

    vec4 colorMetallic  = texture(colorMetallicTexture, inUV);
    vec4 normalRoughness = texture(normalRoughnessTexture, inUV);
    
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

    vec3 dirLightDirection = GET_DIRECTION_LIGHT(ctx.directionLightDataBufferAddr, inLightDenseIndex).direction.xyz;
    vec3 lightDir = normalize(-dirLightDirection);

    vec4 viewPos = camera.view * vec4(position, 1.0);
    float viewDepth = abs(viewPos.z);
    vec3 viewDir = normalize(camera.eye.xyz - position);  

    uint debugCascadeIndex = 0;

    vec3 shadowFactor = CalculateDirectionalLightShadow(
        ctx.directionLightShadowDataBufferAddr,
        ctx.directionLightShadowSparseMapBufferAddr,
        inEntityLightIndex,
        position,
        normal,
        lightDir,
        viewDepth,
        dirLightShadowAtlas,
        dirLightShadowColorAtlas,
        dirLightStaticShadowAtlas,
        dirLightStaticShadowColorAtlas,
        debugCascadeIndex
    );
    
    vec3 radiance = SimulateDirectionalLight(
        ctx.directionLightDataBufferAddr, 
        inLightDenseIndex, 
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
        float ssao = texture(ssaoTexture, inUV).r;
        radiance *= ssao;
    }

    outColor = vec4(radiance, 1.0);
}