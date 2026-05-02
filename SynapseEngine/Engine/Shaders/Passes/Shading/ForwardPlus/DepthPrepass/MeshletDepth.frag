#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/Material.glsl"
#include "../../../Includes/Common/Texture.glsl"
#include "../../../Includes/PushConstants/MeshletPassPC.glsl"

layout(early_fragment_tests) in;

layout(push_constant) uniform PushConstants {
   MeshletPassPC pc;
};

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uvec2 inId;

layout(location = 0) out uint outEntityIndex;

void main() {
    uint entityId = inId.x;
    uint materialId = inId.y;

    // 1. Fetch Material
    Material mat = GET_MATERIAL(pc.materialBuffer, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 finalColor = mat.color;
    if (HAS_ALBEDO_TEX(mat)) {
        finalColor *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, finalUV);
    }

    if (finalColor.a < 0.05) {
        discard;
    }

    outEntityIndex = entityId;
}