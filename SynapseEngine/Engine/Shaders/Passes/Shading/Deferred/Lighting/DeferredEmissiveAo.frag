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

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D emissiveAoTexture;
layout(set = 2, binding = 2) uniform sampler2D ssaoTexture;

#include "../../../../Includes/PushConstants/DeferredEmissiveAoPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredEmissiveAoPC pc;
};

void main()
{
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    vec3 albedo = texture(colorMetallicTexture, inUV).xyz;
    vec4 emissiveAo = texture(emissiveAoTexture, inUV);
    
    vec3 emissive = emissiveAo.rgb;
    float ao = emissiveAo.a;

    float ssao = 1.0;
    if (ctx.enableSsao == 1)
        ssao = texture(ssaoTexture, inUV).r;  

    float finalAo = ao * ssao;

    vec3 ambientResult = SimulateAmbientLight(albedo, finalAo, ctx.ambientStrength);
    vec3 emissiveResult = SimulateBloom(emissive, 1.0, ctx.emissiveStrength);

    outColor = vec4(ambientResult + emissiveResult, 1.0);
}