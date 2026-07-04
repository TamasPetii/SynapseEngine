#ifndef SYN_INCLUDES_UTILS_MATERIAL_MATH_GLSL
#define SYN_INCLUDES_UTILS_MATERIAL_MATH_GLSL

#include "../Common/Texture.glsl"
#include "../Common/Material.glsl"

vec4 EvaluateAlbedoAlpha(uint64_t textureMetadataBufferAddr, const Material mat, vec2 uv) {
    vec4 finalColor = mat.color;
    if (HAS_ALBEDO_TEX(mat)) {
        uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, mat.albedoTexture);
        uint samplerID = UnpackTextureMetadataSampler(meta);

        finalColor *= SampleTexture2D(mat.albedoTexture, samplerID, uv);
    }
    return finalColor;
}

vec3 EvaluateNormal(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv, vec3 vertexNormal, vec4 vertexTangent) {
    vec3 normal = normalize(vertexNormal);

    if (!HAS_NORMAL_TEX(mat)) {
        return normal;
    }

    uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, mat.normalTexture);

    uint samplerID;
    bool invertNormal;
    UnpackTextureMetadata(meta, samplerID, invertNormal);

    vec3 tangent = normalize(vertexTangent.xyz);
    tangent = normalize(tangent - normal * dot(normal, tangent));

    vec3 bitangent = cross(normal, tangent) * vertexTangent.w;
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec3 tangentSpaceNormal;
    tangentSpaceNormal.xy = SampleTexture2D(mat.normalTexture, SAMPLER_NEAREST_ANISO, uv).xy * 2.0 - 1.0;
    tangentSpaceNormal.z = sqrt(max(1.0 - dot(tangentSpaceNormal.xy, tangentSpaceNormal.xy), 0.0));
    tangentSpaceNormal.y *= invertNormal ? -1.0 : 1.0;

    return normalize(TBN * tangentSpaceNormal);
}

vec2 EvaluateMetallicRoughness(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv) {
    float metalness = mat.metalness;
    float roughness = mat.roughness;


    if (HAS_METALNESS_TEX(mat)) {
        uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, mat.metalnessTexture);
        uint samplerID = UnpackTextureMetadataSampler(meta);
        metalness *= SampleTexture2D(mat.metalnessTexture, samplerID, uv).r;
    }

    if (HAS_ROUGHNESS_TEX(mat)) {
        uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, mat.roughnessTexture);
        uint samplerID = UnpackTextureMetadataSampler(meta);
        roughness *= SampleTexture2D(mat.roughnessTexture, samplerID, uv).r;
    }

    if (HAS_METALLIC_ROUGHNESS_TEX(mat)) {
        uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, mat.metallicRoughnessTexture);
        uint samplerID = UnpackTextureMetadataSampler(meta);
        vec4 mrSample = SampleTexture2D(mat.metallicRoughnessTexture, samplerID, uv);
        roughness *= mrSample.g;
        metalness *= mrSample.b;
    }

    return vec2(metalness, roughness);
}

vec3 EvaluateEmissive(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv) {
    vec3 emissive = mat.emissiveColor * mat.emissiveIntensity;
    if (HAS_EMISSIVE_TEX(mat)) {
        uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, mat.emissiveTexture);
        uint samplerID = UnpackTextureMetadataSampler(meta);
        emissive *= SampleTexture2D(mat.emissiveTexture, samplerID, uv).rgb;
    }
    return emissive;
}

float EvaluateAO(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv) {
    float ao = mat.aoStrength;
    if (HAS_AO_TEX(mat)) {
        uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, mat.ambientOcclusionTexture);
        uint samplerID = UnpackTextureMetadataSampler(meta);
        ao *= SampleTexture2D(mat.ambientOcclusionTexture, samplerID, uv).r;
    }
    return ao;
}

#endif