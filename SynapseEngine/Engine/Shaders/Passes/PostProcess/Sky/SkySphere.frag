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
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/Common/Texture.glsl" 
#include "../../../Includes/Common/Environment.glsl"
#include "../../../Includes/Utils/EnvironmentMath.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

#include "../../../Includes/PushConstants/SkySpherePC.glsl"

layout(push_constant) uniform PushConstants {
    SkySpherePC pc;
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

    vec2 finalUV = pc.mappingType == 0 ? SampleEquirectangular(rotatedDir) : SampleOctahedral(rotatedDir);
    finalUV.y = 1.0 - finalUV.y; 

    uint samplerIndex = GET_ENV_SPHERE_SAMPLER(env);
    vec3 skyColor = SampleTexture2DLod(env.skyTextureIndex, samplerIndex, finalUV, 0.0).rgb;
    
    skyColor *= env.skyTint;
    skyColor *= env.intensity;
    skyColor *= exp2(env.skyExposureEV);

    outColor = vec4(skyColor, 1.0);
}