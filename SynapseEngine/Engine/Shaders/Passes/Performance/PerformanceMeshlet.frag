#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Material.glsl"
#include "../../Includes/Common/Texture.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in flat uint inMaterialId;

layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/MeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   MeshletPassPC pc;
};

void main() {
    // 1. Fetch Material
    Material mat = GET_MATERIAL(pc.materialBuffer, inMaterialId);
    vec2 finalUV = inUV * mat.uvScale;
    vec4 finalColor = mat.color;
    
    // 2. Sample Albedo Texture
    if (HAS_ALBEDO_TEX(mat)) {
        finalColor *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, finalUV);
    }

    // 3. Alpha Testing (Discard fully transparent pixels in opaque pass)
    if (finalColor.a < 0.05) {
        discard;
    }

    // 4. Output final color directly to Main target
    outColor = finalColor;
}