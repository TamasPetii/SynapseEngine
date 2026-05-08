#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Material.glsl"
#include "../../../../Includes/Common/Texture.glsl"
#include "../../../../Includes/Utils/MaterialMath.glsl"

layout(early_fragment_tests) in;

#include "../../../../Includes/PushConstants/TraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalMeshletPassPC pc;
};

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uvec2 inId;

layout(location = 0) out uint outEntityIndex;

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint entityId = inId.x;
    uint materialId = inId.y;

    // 1. Fetch Material
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 albedoAlpha = EvaluateAlbedoAlpha(mat, finalUV);
    if (albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    outEntityIndex = entityId;
}