#ifndef SYN_INCLUDES_COMMON_BINDLESS_TEXTURES_GLSL
#define SYN_INCLUDES_COMMON_BINDLESS_TEXTURES_GLSL

#include "../Core.glsl"

layout(set = 0, binding = 0) uniform sampler globalSamplers[];
layout(set = 0, binding = 1) uniform texture2D bindlessTextures[];
layout(set = 1, binding = 0) uniform texture2D bindlessVideoTextures[];

layout(buffer_reference, std430, buffer_reference_align = 4) readonly restrict buffer TextureMetadataBuffer {
    uint data[];
};

#define GET_TEXTURE_METADATA(addr, texID) TextureMetadataBuffer(addr).data[texID]

uint UnpackTextureMetadataSampler(uint meta) {
    return meta & 0x7FFFFFFF;
}

void UnpackTextureMetadata(uint meta, out uint samplerID, out bool invertTangent) {
    samplerID = meta & 0x7FFFFFFF;
    invertTangent = bool(meta >> 31);
}

#define SAMPLER_LINEAR_REPEAT        0
#define SAMPLER_LINEAR_CLAMP_EDGE    1
#define SAMPLER_NEAREST_REPEAT       2
#define SAMPLER_NEAREST_CLAMP_EDGE   3
#define SAMPLER_LINEAR_ANISO         4
#define SAMPLER_NEAREST_ANISO        5
#define SAMPLER_MAX_REDUCTION        6
#define SAMPLER_BLOOM                7
#define SAMPLER_SHADOW               8

vec4 SampleTexture2D(uint textureID, uint samplerID, vec2 uv) { 
    return texture(
        sampler2D(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), 
        uv
    );
}

vec4 SampleVideoTexture2D(uint textureID, uint samplerID, vec2 uv) { 
    return texture(
        sampler2D(bindlessVideoTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), 
        uv
    );
}

vec4 SampleTexture2DLod(uint textureID, uint samplerID, vec2 uv, float lod) { 
    return textureLod(
        sampler2D(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), 
        uv, 
        lod
    );
}

vec4 SampleTexture2DGrad(uint textureID, uint samplerID, vec2 uv, vec2 dx, vec2 dy) { 
    return textureGrad(
        sampler2D(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), 
        uv, 
        dx, 
        dy
    );
}

vec4 FetchTexture2D(uint textureID, uint samplerID, ivec2 coord, int lod) {
    return texelFetch(
        sampler2D(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), 
        coord, 
        lod
    );
}

float SampleShadowMap(uint textureID, uint samplerID, vec3 shadowCoord) {
    return texture(
        sampler2DShadow(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), 
        shadowCoord
    );
}

#endif