#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Material.glsl"
#include "../../Includes/Common/Texture.glsl"
#include "../../Includes/Utils/WboitMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; // (EntityID, MaterialID, MeshIndex, LodIndex) 

layout(location = 0) out vec4 outAccum;
layout(location = 1) out float outReveal;

#include "../../Includes/PushConstants/TraditionalPassPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalPassPC pc;
};

vec3 simulateAmbientLight(vec3 normal, vec3 albedo, float roughness, float metallic, float ao) {
    // TODO: Később IBL
    vec3 ambient = albedo * 0.05;
    return ambient * ao;
}

vec3 simulateDirectionalLight(vec3 normal, vec3 albedo, float roughness, float metallic) { 
    return vec3(0.0);
}

vec3 simulatePointLights(vec3 normal, vec3 albedo, float roughness, float metallic) { 
    return vec3(0.0);
}

vec3 simulateSpotLights(vec3 normal, vec3 albedo, float roughness, float metallic) {
    return vec3(0.0);
}

void main() 
{ 
    uint materialId = inId.y;
    
    // 1. Fetch Material
    Material mat = GET_MATERIAL(pc.materialBuffer, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Albedo & Alpha
    vec4 baseColor = mat.color;
    if (HAS_ALBEDO_TEX(mat)) {
        baseColor *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, finalUV);
    }

    // 3. Normals & TBN
    vec3 normal = normalize(inNormal);
    vec3 finalNormal = normal;
    if (HAS_NORMAL_TEX(mat)) {
        vec3 tangent = normalize(inTangent.xyz);
        tangent = normalize(tangent - finalNormal * dot(finalNormal, tangent));
        vec3 bitangent = cross(finalNormal, tangent) * inTangent.w;
        mat3 TBN = mat3(tangent, bitangent, finalNormal);

        vec3 normalMapSample = SampleTexture2D(mat.normalTexture, SAMPLER_LINEAR_ANISO, finalUV).rgb;
        vec3 tangentSpaceNormal = normalMapSample * 2.0 - 1.0;
        finalNormal = normalize(TBN * tangentSpaceNormal);
    }

    // 4. Metalness & Roughness
    float finalMetalness = mat.metalness;
    float finalRoughness = mat.roughness;
    
    if (HAS_METALNESS_TEX(mat)) {
        finalMetalness *= SampleTexture2D(mat.metalnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    }
    if (HAS_ROUGHNESS_TEX(mat)) {
        finalRoughness *= SampleTexture2D(mat.roughnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    }
    if (HAS_METALLIC_ROUGHNESS_TEX(mat)) {
        vec4 mrSample = SampleTexture2D(mat.metallicRoughnessTexture, SAMPLER_LINEAR_ANISO, finalUV);
        finalRoughness *= mrSample.g;
        finalMetalness *= mrSample.b;
    }

    finalRoughness = clamp(finalRoughness, 0.04, 1.0);

    // 5. Emissive
    vec3 finalEmissive = mat.emissiveColor * mat.emissiveIntensity;
    if (HAS_EMISSIVE_TEX(mat)) {
        finalEmissive *= SampleTexture2D(mat.emissiveTexture, SAMPLER_LINEAR_ANISO, finalUV).rgb;
    }

    // 6. Ambient Occlusion
    float finalAo = mat.aoStrength;
    if (HAS_AO_TEX(mat)) {
        finalAo *= SampleTexture2D(mat.ambientOcclusionTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    }

    // 7. Calculate Lighting
    vec3 finalColor = vec3(0.0);
    finalColor += simulateAmbientLight(finalNormal, baseColor.rgb, finalRoughness, finalMetalness, finalAo);
    finalColor += simulateDirectionalLight(finalNormal, baseColor.rgb, finalRoughness, finalMetalness);
    finalColor += simulatePointLights(finalNormal, baseColor.rgb, finalRoughness, finalMetalness);
    finalColor += simulateSpotLights(finalNormal, baseColor.rgb, finalRoughness, finalMetalness);
    finalColor += finalEmissive;

    // 8. Write WBOIT Accumulation
    vec3 premultipliedColor = finalColor * baseColor.a;
    float weight = calculateWboitWeight(gl_FragCoord.z, baseColor.a);

    outAccum = vec4(premultipliedColor, baseColor.a) * weight;
    outReveal = baseColor.a;
}