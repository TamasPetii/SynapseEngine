#ifndef SYN_INCLUDES_UTILS_MATERIAL_MATH_GLSL
#define SYN_INCLUDES_UTILS_MATERIAL_MATH_GLSL

#include "../Includes/Common/Texture.glsl"
#include "../Includes/Common/Material.glsl"

vec4 EvaluateAlbedoAlpha(const Material mat, vec2 uv) {
    vec4 finalColor = mat.color;
    if (HAS_ALBEDO_TEX(mat)) {
        finalColor *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, uv);
    }
    return finalColor;
}

vec3 EvaluateNormal(Material mat, vec2 uv, vec3 vertexNormal, vec4 vertexTangent) {
    vec3 normal = normalize(vertexNormal);
    if (!HAS_NORMAL_TEX(mat)) {
        return normal;
    }

    vec3 tangent = normalize(vertexTangent.xyz);
    tangent = normalize(tangent - normal * dot(normal, tangent));
    vec3 bitangent = cross(normal, tangent) * vertexTangent.w;
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec3 normalMapSample = SampleTexture2D(mat.normalTexture, SAMPLER_LINEAR_ANISO, uv).rgb;
    vec3 tangentSpaceNormal = normalMapSample * 2.0 - 1.0;
    
    return normalize(TBN * tangentSpaceNormal);
}

vec2 EvaluateMetallicRoughness(Material mat, vec2 uv, float metalness, float roughness) {
    metalness = mat.metalness;
    roughness = mat.roughness;

    if (HAS_METALNESS_TEX(mat)) {
        metalness *= SampleTexture2D(mat.metalnessTexture, SAMPLER_LINEAR_ANISO, uv).r;
    }

    if (HAS_ROUGHNESS_TEX(mat)) {
        roughness *= SampleTexture2D(mat.roughnessTexture, SAMPLER_LINEAR_ANISO, uv).r;
    }

    if (HAS_METALLIC_ROUGHNESS_TEX(mat)) {
        vec4 mrSample = SampleTexture2D(mat.metallicRoughnessTexture, SAMPLER_LINEAR_ANISO, uv);
        roughness *= mrSample.g;
        metalness *= mrSample.b;
    }

    return vec2(metalness, roughness);
}

vec3 EvaluateEmissive(Material mat, vec2 uv) {
    vec3 emissive = mat.emissiveColor * mat.emissiveIntensity;
    if (HAS_EMISSIVE_TEX(mat)) {
        emissive *= SampleTexture2D(mat.emissiveTexture, SAMPLER_LINEAR_ANISO, uv).rgb;
    }
    return emissive;
}

float EvaluateAO(Material mat, vec2 uv) {
    float ao = mat.aoStrength;
    if (HAS_AO_TEX(mat)) {
        ao *= SampleTexture2D(mat.ambientOcclusionTexture, SAMPLER_LINEAR_ANISO, uv).r;
    }
    return ao;
}

#endif