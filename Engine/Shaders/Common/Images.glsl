#ifndef COMMON_IMAGES_GLSL
#define COMMON_IMAGES_GLSL

#extension GL_EXT_nonuniform_qualifier : require

#define NEAREST_SAMPLER_ID 0
#define LINEAR_SAMPLER_ID 1
#define NEAREST_ANISOTROPY_SAMPLER_ID 2
#define LINEAR_ANISOTROPY_SAMPLER_ID 3

layout(set = 0, binding = 0) uniform sampler u_samplers[];
layout(set = 0, binding = 1) uniform texture2D u_textures[];

vec4 sampleTexture2D(uint textureID, uint samplerID, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(u_textures[textureID], u_samplers[samplerID])), uv);
}

#endif