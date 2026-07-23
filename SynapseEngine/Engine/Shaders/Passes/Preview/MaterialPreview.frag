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

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/MaterialPreviewPC.glsl"

layout(push_constant) uniform PushConstants {
    MaterialPreviewPC pc;
};

void main() {
    vec2 checkerUV = inUV * 10.0;
    float checker = mod(floor(checkerUV.x) + floor(checkerUV.y), 2.0);
    vec3 bgColor = mix(vec3(0.15), vec3(0.25), checker);

    vec2 p = inUV * 2.0 - 1.0;
    float r2 = dot(p, p);

    if (r2 > 0.85) {
        outColor = vec4(bgColor, 1.0);
        return;
    }

    float z = sqrt(1.0 - r2);
    vec3 localNormal = vec3(p.x, -p.y, z);

    vec2 sphereUV = vec2(
        atan(localNormal.x, localNormal.z) / (2.0 * PI) + 0.5,
        asin(localNormal.y) / PI + 0.5
    );

    vec3 t = cross(vec3(0.0, 1.0, 0.0), localNormal);
    vec3 tangent = (dot(t, t) < 1e-6) ? vec3(1.0, 0.0, 0.0) : normalize(t);
    vec4 inTangent = vec4(tangent, 1.0);

    float radius = 1.0;
    vec3 inWorldPos = localNormal * radius;
    vec3 viewDir = vec3(0.0, 0.0, 1.0);

    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, pc.materialId);

    vec4 albedoAlpha = EvaluateAlbedoAlpha(ctx.textureMetadataBufferAddr, mat, sphereUV);

    if (IS_ALPHA_TESTED(mat) && albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    bool frontFacing = IS_DOUBLE_SIDED(mat) ? gl_FrontFacing : true;
    vec3 finalNormal = EvaluateNormal(ctx.textureMetadataBufferAddr, mat, sphereUV, localNormal, inTangent, frontFacing);
    vec2 metalRough = EvaluateMetallicRoughness(ctx.textureMetadataBufferAddr, mat, sphereUV);
    vec3 emissive = EvaluateEmissive(ctx.textureMetadataBufferAddr, mat, sphereUV);
    float ao = EvaluateAO(ctx.textureMetadataBufferAddr, mat, sphereUV);

    float finalMetalness = clamp(metalRough.x, 0.0, 1.0);
    float finalRoughness = clamp(metalRough.y, 0.04, 1.0);

    float clearcoatFactor, clearcoatRoughness;
    vec3 clearcoatNormal;
    EvaluateClearcoat(ctx.textureMetadataBufferAddr, mat, sphereUV, localNormal, inTangent, frontFacing, clearcoatFactor, clearcoatRoughness, clearcoatNormal);

    float specularFactor;
    vec3 specularColor;
    EvaluateSpecular(ctx.textureMetadataBufferAddr, mat, sphereUV, specularFactor, specularColor);
    
    float ior = mat.ior;

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

    // Ambient & Bloom
    totalRadiance += SimulateAmbientLight(albedoAlpha.rgb, ao, ctx.ambientStrength);
    totalRadiance += SimulateBloom(emissive, 1.0, ctx.emissiveStrength);

    vec3 finalColor = mix(bgColor, totalRadiance, IS_TRANSPARENT(mat) ? albedoAlpha.a : 1.0);
    outColor = vec4(finalColor, 1.0);
}