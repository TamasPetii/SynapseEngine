#ifndef COMMON_UTILITY_TEXTURE_LOADED_GLSL
#define COMMON_UTILITY_TEXTURE_LOADED_GLSL

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 1) uniform texture2D u_textures[];

vec4 sampleLoadedTexture2D(uint textureID, uint samplerID, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(u_textures[textureID], u_samplers[samplerID])), uv);
}

#endif