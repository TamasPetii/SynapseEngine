#ifndef COMMON_UTILITY_TEXTURE_GLSL
#define COMMON_UTILITY_TEXTURE_GLSL

#extension GL_EXT_nonuniform_qualifier : require

vec4 sampleTexture2D(texture2D tex, sampler sampler, uint samplerID, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(tex, u_samplers[samplerID])), uv);
}

#endif