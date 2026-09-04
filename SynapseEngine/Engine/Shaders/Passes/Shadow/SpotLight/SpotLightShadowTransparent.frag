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
#extension GL_EXT_mesh_shader : require
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uint inMaterialId; 

layout(location = 0) out vec4 outShadowColorAndDepth;

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Material.glsl"
#include "../../../Includes/Common/Texture.glsl"
#include "../../../Includes/Utils/MaterialMath.glsl"
#include "../../../Includes/PushConstants/SpotLightShadowTraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   SpotLightShadowTraditionalMeshletPassPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, inMaterialId);
    vec2 finalUV = inUV * mat.uvScale;
    
    vec4 albedoAlpha = EvaluateAlbedoAlpha(ctx.textureMetadataBufferAddr, mat, finalUV);
    
    if (IS_ALPHA_TESTED(mat) && albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    vec3 shadowFilterColor = mix(vec3(1.0), albedoAlpha.rgb, albedoAlpha.a);
    outShadowColorAndDepth = vec4(shadowFilterColor, gl_FragCoord.z);
}