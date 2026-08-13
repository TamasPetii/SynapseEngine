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
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Visibility.glsl"
#include "../../../Includes/Common/Outline.glsl"
#include "../../../Includes/Common/Camera.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform usampler2D entityIndexTexture;
layout(set = 2, binding = 1) uniform sampler2D depthTexture;

#include "../../../Includes/PushConstants/SelectionOutlinePC.glsl"

layout(push_constant) uniform PushConstants {
    SelectionOutlinePC pc;
};

uint GetSelectionState(uint rawEntityData, uint64_t sparseMapAddr, uint64_t maskAddr) {
    if (rawEntityData == 0xFFFFFFFF) return 0;
    
    uint entityId = UNPACK_VISIBILITY_ENTITY(rawEntityData);
    
    uint denseIdx = GET_SPARSE_INDEX(sparseMapAddr, entityId);
    if (denseIdx == INVALID_INDEX) return 0;
    
    return GET_SELECTION_MASK(maskAddr, denseIdx);
}

void main() {
    if (pc.enableSelectedOutline == 0) discard;

    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    uint64_t sparseMapAddr = ctx.hierarchySparseMapBufferAddr;
    uint64_t maskAddr = ctx.selectionOutlineBufferAddr;

    ivec2 texCoords = ivec2(gl_FragCoord.xy);
    uint rawEntityId = texelFetch(entityIndexTexture, texCoords, 0).x;
    float centerDepth = texelFetch(depthTexture, texCoords, 0).r;

    uint centerState = GetSelectionState(rawEntityId, sparseMapAddr, maskAddr);
    
    if (centerState > 0) {
        discard;
    }
    
    int thickness = int(max(1.0, pc.outlineThickness));
    uint edgeState = 0;
    ivec2 texSize = ivec2(ctx.screenWidth, ctx.screenHeight);

    float worldSpaceBias = 0.00005; 

    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, ctx.mainCameraEntity);
    float zNear = GET_CAMERA_NEAR(camera);
    float zFar  = GET_CAMERA_FAR(camera);

    float clipRange = max(zFar - zNear, 1.0);
    float normalizedDepthBias = worldSpaceBias / clipRange;

    for (int y = -thickness; y <= thickness; ++y) {
        for (int x = -thickness; x <= thickness; ++x) {
            if (x == 0 && y == 0) continue;
            
            ivec2 neighborCoords = texCoords + ivec2(x, y);
            neighborCoords = clamp(neighborCoords, ivec2(0), texSize - ivec2(1));

            uint rawNeighborEntityId = texelFetch(entityIndexTexture, neighborCoords, 0).x;

            float neighborDepth = texelFetch(depthTexture, neighborCoords, 0).r;
            
            if (neighborDepth <= centerDepth + normalizedDepthBias) {
                if (rawNeighborEntityId != rawEntityId) {
                    uint nState = GetSelectionState(rawNeighborEntityId, sparseMapAddr, maskAddr);

                    if (nState > 0) {
                        edgeState = nState;
                        break;
                    }
                }
            }
        }
        if (edgeState > 0) break;
    }
    
    if (edgeState == 1) {
        outColor = pc.outlinePrimaryColor;
    } else if (edgeState == 2) {
        outColor = pc.outlineSecondaryColor;
    } else {
        discard;
    }
}