#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/FrameGlobalContext.glsl"
#include "../../Includes/Common/Material.glsl"
#include "../../Includes/Common/Texture.glsl"
#include "../../Includes/Utils/MaterialMath.glsl"
#include "../../Includes/Utils/PbrMath.glsl"
#include "../../Includes/Utils/LightMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uint inMaterialId;

layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/AnimationPreviewPC.glsl"

layout(push_constant) uniform PushConstants {
    AnimationPreviewPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, inMaterialId);

    vec4 albedoAlpha = EvaluateAlbedoAlpha(ctx.textureMetadataBufferAddr, mat, inUV);
    if (IS_ALPHA_TESTED(mat) && albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    bool frontFacing = IS_DOUBLE_SIDED(mat) ? gl_FrontFacing : true;
    vec3 finalNormal = EvaluateNormal(ctx.textureMetadataBufferAddr, mat, inUV, inNormal, inTangent, frontFacing);
    vec2 metalRough = EvaluateMetallicRoughness(ctx.textureMetadataBufferAddr, mat, inUV);
    vec3 emissive = EvaluateEmissive(ctx.textureMetadataBufferAddr, mat, inUV);
    float ao = EvaluateAO(ctx.textureMetadataBufferAddr, mat, inUV);

    float finalMetalness = clamp(metalRough.x, 0.0, 1.0);
    float finalRoughness = clamp(metalRough.y, 0.04, 1.0);

    float clearcoatFactor, clearcoatRoughness;
    vec3 clearcoatNormal;
    EvaluateClearcoat(ctx.textureMetadataBufferAddr, mat, inUV, inNormal, inTangent, frontFacing, clearcoatFactor, clearcoatRoughness, clearcoatNormal);

    float specularFactor;
    vec3 specularColor;
    EvaluateSpecular(ctx.textureMetadataBufferAddr, mat, inUV, specularFactor, specularColor);
    
    float ior = mat.ior;

    vec3 viewDir = vec3(0.0, 0.0, 1.0); 
    vec3 totalRadiance = vec3(0.0);

    // Key Light
    vec3 keyLightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 keyLightColor = vec3(1.0, 1.0, 1.0);
    float keyLightStrength = 2.5;
    totalRadiance += ShadePhysicallyBased(
        albedoAlpha.rgb, finalNormal, viewDir, keyLightDir, 
        finalRoughness, finalMetalness, ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal,
        keyLightColor, 1.0, keyLightStrength
    );

    // Fill Light
    vec3 fillLightDir = normalize(vec3(-1.0, 0.2, -0.5));
    vec3 fillLightColor = vec3(0.5, 0.6, 0.8);
    float fillLightStrength = 1.0;
    totalRadiance += ShadePhysicallyBased(
        albedoAlpha.rgb, finalNormal, viewDir, fillLightDir, 
        finalRoughness, finalMetalness, ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal,
        fillLightColor, 1.0, fillLightStrength
    );

    totalRadiance += SimulateAmbientLight(albedoAlpha.rgb, ao, ctx.ambientStrength);
    totalRadiance += SimulateBloom(emissive, 1.0, ctx.emissiveStrength);

    vec3 bgColor = vec3(0.15);
    vec3 finalColor = mix(bgColor, totalRadiance, IS_TRANSPARENT(mat) ? albedoAlpha.a : 1.0);

    outColor = vec4(finalColor, 1.0);
}