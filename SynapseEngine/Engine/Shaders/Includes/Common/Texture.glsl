#ifndef SYN_INCLUDES_COMMON_BINDLESS_GLSL
#define SYN_INCLUDES_COMMON_BINDLESS_GLSL

#include "../Core.glsl"

layout(set = 0, binding = 0) uniform sampler globalSamplers[];
layout(set = 0, binding = 1) uniform texture2D bindlessTextures[];

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