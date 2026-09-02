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

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uint inCameraIndex;

layout(location = 0) out vec4 outColor;

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Utils/DepthMath.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D emissiveAoTexture;
layout(set = 2, binding = 2) uniform sampler2D ssaoTexture;
layout(set = 2, binding = 3) uniform sampler2D normalRoughnessTexture;
layout(set = 2, binding = 4) uniform sampler2D depthTexture;

#include "../../../../Includes/PushConstants/DeferredEmissiveAoPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredEmissiveAoPC pc;
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
    vec3 viewDir = normalize(camera.eye.xyz - position);  

    vec4 colorMetallic = texture(colorMetallicTexture, inUV);
    vec4 normalRoughness = texture(normalRoughnessTexture, inUV);
    vec4 emissiveAo = texture(emissiveAoTexture, inUV);
    
    vec3 albedo = colorMetallic.rgb;
    float metalness = colorMetallic.a;
    vec3 normal = normalize(normalRoughness.xyz);
    float roughness = clamp(normalRoughness.a, 0.04, 1.0);
    
    vec3 emissive = emissiveAo.rgb;
    float ao = emissiveAo.a;

    float ssao = 1.0;
    if (ctx.enableSsao == 1) {
        ssao = texture(ssaoTexture, inUV).r;  
    }
    float finalAo = ao * ssao;

    float ior = 1.5;
    float specularFactor = 1.0;
    vec3 specularColor = vec3(1.0);

    vec3 ambientResult = SimulateEnvironmentLight(
        ctx.environmentBufferAddr, ctx.activeEnvironmentIndex, ctx.brdfLutTextureIndex,
        albedo, normal, viewDir, roughness, metalness,
        ior, specularFactor, specularColor, finalAo, ctx.ambientStrength
    );

    vec3 emissiveResult = SimulateBloom(emissive, 1.0, ctx.emissiveStrength);

    outColor = vec4(ambientResult + emissiveResult, 1.0);
}