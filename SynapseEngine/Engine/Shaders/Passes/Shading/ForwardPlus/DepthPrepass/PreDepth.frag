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

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/Visibility.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Material.glsl"
#include "../../../../Includes/Common/Texture.glsl"
#include "../../../../Includes/Utils/MaterialMath.glsl"

#include "../../../../Includes/PushConstants/TraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalMeshletPassPC pc;
};

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uvec3 inId;

layout(location = 0) out uvec2 outId;

#ifndef ENABLE_ALPHA_TEST
layout(early_fragment_tests) in;
#endif

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint packedEntity = inId.x;
    uint materialId   = inId.y;
    uint partial      = inId.z;

    uint pipelineFlag = UNPACK_VISIBILITY_PIPELINE(packedEntity);
    uint finalPayload = pipelineFlag == VIS_PIPELINE_MESH_SHADER 
                                    ? FINALIZE_VIS_MS(partial, gl_PrimitiveID)
                                    : FINALIZE_VIS_TRADITIONAL(partial, gl_PrimitiveID);
    
    // 1. Fetch Material
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 albedoAlpha = EvaluateAlbedoAlpha(ctx.textureMetadataBufferAddr, mat, finalUV);

    #ifdef ENABLE_ALPHA_TEST
    if (IS_ALPHA_TESTED(mat) && albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }
    #endif

    outId = uvec2(packedEntity, finalPayload);
}