#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Visibility.glsl"
#include "../../../Includes/Common/Outline.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform usampler2D entityIndexTexture;

#include "../../../Includes/PushConstants/SelectionOutlinePC.glsl"

layout(push_constant) uniform PushConstants {
    SelectionOutlinePC pc;
};

uint GetSelectionState(uint entityId, uint64_t sparseMapAddr, uint64_t maskAddr) {
    if (entityId == INVALID_INDEX) return 0;
    
    uint cleanEntityId = entityId & 0x7FFFFFFF;
    
    uint denseIdx = GET_SPARSE_INDEX(sparseMapAddr, cleanEntityId);
    if (denseIdx == INVALID_INDEX) return 0;
    
    return GET_SELECTION_MASK(maskAddr, denseIdx);
}

void main() {
    if (pc.enableSelectedOutline == 0) discard;

    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    uint64_t sparseMapAddr = ctx.hierarchySparseMapBufferAddr;
    uint64_t maskAddr = ctx.selectionOutlineBufferAddr;

    ivec2 texCoords = ivec2(gl_FragCoord.xy);
    uint centerData = texelFetch(entityIndexTexture, texCoords, 0).x;
    uint centerEntityId = UNPACK_VISIBILITY_ENTITY(centerData);
    
    uint centerState = GetSelectionState(centerEntityId, sparseMapAddr, maskAddr);
    
    if (centerState > 0) {
        discard;
    }
    
    int thickness = int(max(1.0, pc.outlineThickness));
    uint edgeState = 0;
    
    /*
    for (int y = -thickness; y <= thickness; ++y) {
        for (int x = -thickness; x <= thickness; ++x) {
            if (x == 0 && y == 0) continue;
            
            ivec2 neighborCoords = texCoords + ivec2(x, y);
            uvec2 neighborData = texelFetch(entityIndexTexture, neighborCoords, 0).xy;
            uint neighborEntityId = UNPACK_VISIBILITY_ENTITY(neighborData.x);
            
            uint nState = GetSelectionState(neighborEntityId, sparseMapAddr, maskAddr);
            if (nState > 0) {
                edgeState = nState;
                break;
            }
        }

        if (edgeState > 0) break;
    }
    */
    
    if (edgeState == 1) {
        outColor = pc.outlinePrimaryColor;
    } else if (edgeState == 2) {
        outColor = pc.outlineSecondaryColor;
    } else {
        discard;
    }
}