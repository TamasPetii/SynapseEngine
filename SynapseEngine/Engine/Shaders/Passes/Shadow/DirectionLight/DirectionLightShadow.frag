#version 460
#extension GL_EXT_mesh_shader : require
#extension GL_GOOGLE_include_directive : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Material.glsl"
#include "../../../Includes/Common/Texture.glsl"
#include "../../../Includes/Utils/MaterialMath.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uint inMaterialId; 

#include "../../../Includes/PushConstants/DirectionLightShadowTraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   DirectionLightShadowTraditionalMeshletPassPC pc;
};

#ifndef ENABLE_ALPHA_TEST
layout(early_fragment_tests) in;
#endif

void main() {
    #ifdef ENABLE_ALPHA_TEST

    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    
    // 1. Fetch Material
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, inMaterialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 albedoAlpha = EvaluateAlbedoAlpha(ctx.textureMetadataBufferAddr, mat, finalUV);
    if (IS_ALPHA_TESTED(mat) && albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    #endif
}