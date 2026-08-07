#ifndef SYN_INCLUDES_UTILS_MATERIAL_MATH_GLSL
#define SYN_INCLUDES_UTILS_MATERIAL_MATH_GLSL

#include "../Common/Texture.glsl"
#include "../Common/Material.glsl"

uint ResolveSampler(uint64_t textureMetadataBufferAddr, uint packedTexData, uint texID) {
    uint sampID = UNPACK_SAMPLER_ID(packedTexData);
    if (sampID == INVALID_SAMPLER_INDEX) {
        uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, texID);
        return UnpackTextureMetadataSampler(meta);
    }
    return sampID;
}

vec4 EvaluateAlbedoAlpha(uint64_t textureMetadataBufferAddr, const Material mat, vec2 uv) {
    vec4 finalColor = mat.color;
    
    if (HAS_ALBEDO_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.albedoTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.albedoTexture, texID);
        finalColor *= SampleTexture2D(texID, sampID, uv);
    }

    if (HAS_VIDEO_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.videoTexture);
        uint sampID = UNPACK_SAMPLER_ID(mat.videoTexture);
        if (sampID == INVALID_SAMPLER_INDEX) {
            sampID = SAMPLER_LINEAR_CLAMP_EDGE;
        }
        finalColor *= SampleVideoTexture2D(texID, sampID, uv);
    }

    if (HAS_OPACITY_TEX(mat)) { 
        uint texID = UNPACK_TEXTURE_ID(mat.opacityTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.opacityTexture, texID);
        
        float opacityMask = SampleTexture2D(texID, sampID, uv).r;
        finalColor.a = opacityMask;
    }

    return finalColor;
}

vec3 EvaluateNormal(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv, vec3 vertexNormal, vec4 vertexTangent, bool isFrontFacing) {
    vec3 normal = normalize(vertexNormal);
    
    if (!isFrontFacing) {
        normal = -normal;
    }

    if (!HAS_NORMAL_TEX(mat)) {
        return normal;
    }

    uint texID = UNPACK_TEXTURE_ID(mat.normalTexture);
    uint meta = GET_TEXTURE_METADATA(textureMetadataBufferAddr, texID);
    
    uint defaultSamplerID;
    bool invertNormal;
    UnpackTextureMetadata(meta, defaultSamplerID, invertNormal);

    uint sampID = UNPACK_SAMPLER_ID(mat.normalTexture);
    if (sampID == INVALID_SAMPLER_INDEX) {
        sampID = defaultSamplerID;
    }

    vec3 tangent = normalize(vertexTangent.xyz);
    tangent = normalize(tangent - normal * dot(normal, tangent));

    vec3 bitangent = cross(normal, tangent) * vertexTangent.w;
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec3 tangentSpaceNormal;
    tangentSpaceNormal.xy = SampleTexture2D(texID, sampID, uv).xy * 2.0 - 1.0;
    tangentSpaceNormal.z = sqrt(max(1.0 - dot(tangentSpaceNormal.xy, tangentSpaceNormal.xy), 0.0));
    tangentSpaceNormal.y *= invertNormal ? -1.0 : 1.0;

    return normalize(TBN * tangentSpaceNormal);
}

vec2 EvaluateMetallicRoughness(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv) {
    float metalness = mat.metalness;
    float roughness = mat.roughness;
    
    if (HAS_METALNESS_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.metalnessTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.metalnessTexture, texID);
        metalness *= SampleTexture2D(texID, sampID, uv).r;
    }

    if (HAS_ROUGHNESS_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.roughnessTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.roughnessTexture, texID);
        roughness *= SampleTexture2D(texID, sampID, uv).r;
    }

    if (HAS_METALLIC_ROUGHNESS_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.metallicRoughnessTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.metallicRoughnessTexture, texID);
        vec4 mrSample = SampleTexture2D(texID, sampID, uv);
        roughness *= mrSample.g;
        metalness *= mrSample.b;
    }

    return vec2(metalness, roughness);
}

vec3 EvaluateEmissive(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv) {
    vec3 emissive = mat.emissiveColor * mat.emissiveIntensity;
    
    if (HAS_EMISSIVE_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.emissiveTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.emissiveTexture, texID);
        emissive *= SampleTexture2D(texID, sampID, uv).rgb;
    }
    return emissive;
}

float EvaluateAO(uint64_t textureMetadataBufferAddr, Material mat, vec2 uv) {
    float ao = mat.aoStrength;
    
    if (HAS_AO_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.ambientOcclusionTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.ambientOcclusionTexture, texID);
        ao *= SampleTexture2D(texID, sampID, uv).r;
    }
    return ao;
}

void EvaluateClearcoat(
    uint64_t textureMetadataBufferAddr, 
    Material mat, 
    vec2 uv, 
    vec3 vertexNormal, 
    vec4 vertexTangent, 
    bool isFrontFacing,
    out float outFactor, 
    out float outRoughness, 
    out vec3 outNormal
) {
    outFactor = mat.clearcoatFactor;
    outRoughness = mat.clearcoatRoughness;
    
    if (HAS_CLEARCOAT_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.clearcoatTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.clearcoatTexture, texID);
        outFactor *= SampleTexture2D(texID, sampID, uv).r;
    }

    if (HAS_CLEARCOAT_ROUGHNESS_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.clearcoatRoughnessTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.clearcoatRoughnessTexture, texID);
        outRoughness *= SampleTexture2D(texID, sampID, uv).g;
    }

    vec3 geomNormal = normalize(vertexNormal);
    if (!isFrontFacing) geomNormal = -geomNormal;

    if (HAS_CLEARCOAT_NORMAL_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.clearcoatNormalTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.clearcoatNormalTexture, texID);
        
        vec3 tangent = normalize(vertexTangent.xyz);
        tangent = normalize(tangent - geomNormal * dot(geomNormal, tangent));
        vec3 bitangent = cross(geomNormal, tangent) * vertexTangent.w;
        mat3 TBN = mat3(tangent, bitangent, geomNormal);

        vec3 tangentSpaceNormal;
        tangentSpaceNormal.xy = SampleTexture2D(texID, sampID, uv).xy * 2.0 - 1.0;
        tangentSpaceNormal.z = sqrt(max(1.0 - dot(tangentSpaceNormal.xy, tangentSpaceNormal.xy), 0.0));
        
        outNormal = normalize(TBN * tangentSpaceNormal);
    } else {
        outNormal = geomNormal;
    }
}

void EvaluateSpecular(
    uint64_t textureMetadataBufferAddr, 
    Material mat, 
    vec2 uv, 
    out float outSpecularFactor, 
    out vec3 outSpecularColor
) {
    outSpecularFactor = mat.specularFactor;
    outSpecularColor = mat.specularColor;

    if (HAS_SPECULAR_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.specularTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.specularTexture, texID);
        outSpecularFactor *= SampleTexture2D(texID, sampID, uv).a; 
    }

    if (HAS_SPECULAR_COLOR_TEX(mat)) {
        uint texID = UNPACK_TEXTURE_ID(mat.specularColorTexture);
        uint sampID = ResolveSampler(textureMetadataBufferAddr, mat.specularColorTexture, texID);
        outSpecularColor *= SampleTexture2D(texID, sampID, uv).rgb;
    }
}

#endif